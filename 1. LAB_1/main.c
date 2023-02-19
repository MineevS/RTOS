/*!
* 
* @file		main.c 
* @data		18-FEB-23 
* @author	Minee S. A. 
* @email	mineeff20@yandex.ru 
* @version	v1.0 
* @license	FS(FreeSoft) 
* @brief	- 
* @detail	- 
*
*/
 
#include <stdio.h>			// printf();
#include <stdlib.h>			// calloc;
#include <pthread.h>			// pthread_*;
#include <unistd.h>			// getopt; stat;
#include <string.h>			// strlen;strcpy;
#include <stdint.h>			// int64_t;
#include <getopt.h>			// struct option; getopt_long;
#include <fcntl.h>			// open;
#include <sys/types.h>			// struct stat;
#include <sys/stat.h>			// fstat;
#include <sys/sysinfo.h>		// get_nprocs_conf; get_nprocs;
#include <malloc.h>			// mallop;
#include <sys/mman.h>			// meminfo;
#include <math.h>			// pow;

#define UNUSED(x) (void)(x) 		//
#define MAX_COUNT_PARAMS_ARGV 6		//
#define SIZE_BLOCK 1000000		// max: 10^6

#define ERROR_INPUT_FILE -1
#define ERROR_OUTPUT_FILE -2
#define ERROR_OPEN_FILE -3
#define ERROR_FSTAT_FILE -4
#define ERROR_CREATE_THREAD -5
#define ERROR_INIT_BARRIER -5
#define ERROR_WAIT_BARRIER -7
#define ERROR_READ_FILE -8
#define ERROR_WRITE_FILE -9

typedef struct{
	short count;
	_Bool status_struct;

	_Bool status_path_input;
	char* path_input;

	_Bool status_path_output;
	char* path_output;

	int64_t x0;
	int64_t a;
	int64_t c;
	int64_t m;
} PARAMETERS;

typedef struct{
	int index;
	int64_t x0;
	int64_t a;
	int64_t c;
	int64_t m;
	size_t size_data;
	char* data;
} PARAMETERS_GPSP;

void* generator_PSP(void*  param_gpsp){
	typedef PARAMETERS_GPSP PGPSP;

	((PGPSP*)param_gpsp)->data = calloc(((PGPSP*)param_gpsp)->size_data, sizeof(char*));

	//printf("x0: %ld\n", param_gpsp->x0);

	int k = 0;
	if(!(((PGPSP*)param_gpsp)->index)){
			((PGPSP*)param_gpsp)->data[0] = ((PGPSP*)param_gpsp)->x0;
			++k;
	}
	
	for(size_t i = k; i < ((PGPSP*)param_gpsp)->size_data; ++i){
		((PGPSP*)param_gpsp)->data[i] = ( (((PGPSP*)param_gpsp)->a) * 
						(i == 0 ? ((PGPSP*)param_gpsp)->x0 : 
								    	((PGPSP*)param_gpsp)->data[i - 1] ) 
					+ ((PGPSP*)param_gpsp)->c ) % ((PGPSP*)param_gpsp)->m;

		//printf("param_data[%ld] = %ld\n", i, param_gpsp->data[i]);
	}

	return NULL;
}

typedef struct{
	int index_worker;
	int max_index_worker;
	pthread_barrier_t* barrier;
	size_t size_data;
	char* data_notepad;
	char* data_opentext;
	_Bool status_data_output;
	char* data_output;
} CONTEXT;

void* Work(void* context){

	//printf("index_worker: %d\n", ((CONTEXT*) context)->index_worker);

	((CONTEXT*) context)->data_output = calloc(((CONTEXT*) context)->size_data, sizeof(char));

	((CONTEXT*) context)->status_data_output = 1;

	for(size_t i = 0; i < ((CONTEXT*) context)->size_data; ++i){
		((CONTEXT*) context)->data_output[i] 
			= (char) (  ((CONTEXT*) context)->data_notepad[i]) ^ 
			         (  ((CONTEXT*) context)->data_opentext[i]);
	}

	pthread_barrier_wait(((CONTEXT*) context)->barrier);

	if(((CONTEXT*) context)->index_worker == ((CONTEXT*) context)->max_index_worker){
		return (void*) PTHREAD_BARRIER_SERIAL_THREAD;
	}else
		return NULL;
}

typedef struct{
	int index;
	PARAMETERS* g_params;
	size_t size_block;
	int fd_input;
	int fd_output;
} WWB;

void* work_with_block(WWB* wwb){

	/*
 	printf("index: %d\n", wwb->index);
	printf("size_block: %ld\n", wwb->size_block);
	printf("fd_i: %d\n", wwb->fd_input);
	printf("fd_o: %d\n", wwb->fd_output);
	*/

	char data_input[wwb->size_block];
	memset(data_input, 0, wwb->size_block);

	lseek(wwb->fd_input, SIZE_BLOCK*wwb->index, SEEK_SET);
	
	if( read(wwb->fd_input, data_input, wwb->size_block) !=  (ssize_t) wwb->size_block ){
		perror("read not success!");
		exit(ERROR_READ_FILE);
	}

	static int64_t x0;
	if(!wwb->index){
		x0 = wwb->g_params->x0;
	}

	PARAMETERS_GPSP param_gpsp;
	param_gpsp.x0 = x0;
	param_gpsp.index = wwb->index;
	param_gpsp.a = wwb->g_params->a;
	param_gpsp.c = wwb->g_params->c;
	param_gpsp.m = wwb->g_params->m;
	param_gpsp.size_data = wwb->size_block;

	pthread_t thread_GPSP;
	pthread_create(&thread_GPSP, NULL, &generator_PSP, (void*) &param_gpsp);
	pthread_join(thread_GPSP, NULL);
	x0 = param_gpsp.data[param_gpsp.size_data - 1];
	//
	int count_pthreads = sysconf(_SC_NPROCESSORS_ONLN);
	
	printf("count_thread: %d\n", count_pthreads);
	
	/*	
	long NPC = get_nprocs_conf();
	printf("count cores: %ld\n", NPC);

	long NP = get_nprocs();
	printf("count cores: %ld\n", NP);
	*/

	int size_data_works_c;
	int size_data_works_o;

	if( wwb->size_block % count_pthreads != 0 ){
		size_data_works_c = wwb->size_block / (count_pthreads - 1);
		size_data_works_o = wwb->size_block % (count_pthreads - 1);
	}else{
		size_data_works_c = wwb->size_block / count_pthreads;
		size_data_works_o = wwb->size_block % count_pthreads;
	}
	
	pthread_barrier_t barrier;

	int status;
	if ( (status = pthread_barrier_init(&barrier, NULL, count_pthreads + 1)) != 0) {
        	printf("main error: can't init barrier, status = %d\n", status);
        	exit(ERROR_INIT_BARRIER);
    }

	pthread_t thread_works[count_pthreads];
	CONTEXT contexts[count_pthreads];
	for(int i = 0; i < count_pthreads; ++i){
		contexts[i].index_worker = i;
		contexts[i].max_index_worker = count_pthreads - 1;
		contexts[i].barrier = &barrier;
		contexts[i].status_data_output = 0;

		if(size_data_works_o != 0){
			contexts[i].size_data = (i < count_pthreads - 1) ? 
				 size_data_works_c : size_data_works_o ;
		}else
			contexts[i].size_data = size_data_works_c;

		contexts[i].data_notepad = calloc(contexts[i].size_data, sizeof(char));
		contexts[i].data_notepad = memcpy(contexts[i].data_notepad, 
				param_gpsp.data + i*size_data_works_c, contexts[i].size_data);
		contexts[i].data_opentext = calloc(contexts[i].size_data, sizeof(char));
		contexts[i].data_opentext = memcpy(contexts[i].data_opentext, 
				data_input + i*size_data_works_c, contexts[i].size_data);
 
		if( (status = pthread_create(&thread_works[i], NULL, &Work, (void*) &contexts[i])) != 0){
			printf("error_create_thread: %d\n i: %d\n ", status, i);
			exit(ERROR_CREATE_THREAD);
		}
	} 

	/*for(int i = 0; i < count_pthreads; ++i)
		pthread_join(thread_works[i], NULL);
	*/

	status = pthread_barrier_wait(&barrier);
	if( (status !=  0) && (status != PTHREAD_BARRIER_SERIAL_THREAD ) ){
		printf("Error in barrier wait: %d\n", status);
		exit(ERROR_WAIT_BARRIER);
	}
	else{
		pthread_barrier_destroy(&barrier);
	}

	for(int i = 0; i < count_pthreads; ++i){
		if(contexts[i].status_data_output){
			if(( write(wwb->fd_output, 
				contexts[i].data_output, 
				contexts[i].size_data)) != (ssize_t) contexts[i].size_data )
			{
				printf("Error write file, i = %d\n", i);
				exit(ERROR_WRITE_FILE);
			}
			 
			free(contexts[i].data_output);
		}

		free(contexts[i].data_notepad);
		free(contexts[i].data_opentext);
	}

	//printf("END!\n");
	
	free(param_gpsp.data);

	return NULL;
}
 
/*! 
* 
* @brief		Точка входа в программу. 
* @param[in]	argv	@brief Количество принимаемых параметров из командной строки. 
* @param[in]	argc	@brief массив принимаемых параметров из командной строки. 
* @return	{@brief	описание_возвращаемого_значения} 
* 
*/ 
/// @snippet this main 
//! [main] 
int main(int argv, char** argc)
{
	//printf("Start programm! \n");
	
	PARAMETERS parameters = {0, 0, 0, NULL, 0, NULL, 0, 0, 0, 0};

	char optval = 0;
	int optind = -1;

	static struct option long_options[] = {
		{"x0", required_argument, 0, 'x'},
		{NULL, 0, 0, 0}
	};

	while((optval = getopt_long(argv, argc, "i:o:x:a:c:m:", 
					long_options, &optind)) != -1){
		switch(optval){
			case 'i':
				parameters.path_input = (char*) calloc((strlen(optarg) + 1), sizeof(char));
				parameters.path_input = strcpy(parameters.path_input, optarg);
				parameters.status_path_input = !parameters.status_path_input;
				++parameters.count;
				break;
			case 'o':
				parameters.path_output = (char*) calloc((strlen(optarg) + 1), sizeof(char));
				parameters.path_output = strcpy(parameters.path_output, optarg);
				parameters.status_path_output = !parameters.status_path_output;
				++parameters.count;
				break;
			case 'x':
				parameters.x0 = atoi(optarg);
				++parameters.count;
				break;
			case 'a':
				parameters.a = atoi(optarg);
				++parameters.count;
				break;
			case 'c':
				parameters.c = atoi(optarg);
				++parameters.count;
				break;
			case 'm':
				parameters.m = atoi(optarg);
				++parameters.count;
				break;
			default:
				perror("Error in getopt!");
		}
	}

	if( parameters.count == MAX_COUNT_PARAMS_ARGV )
		parameters.status_struct = !parameters.status_struct;
	
	if( parameters.status_struct ){

		/*
		printf("i: %s\n", parameters.path_input);
		printf("o: %s\n", parameters.path_output);
		
		printf("x0: %ld\n", parameters.x0);
		printf("a: %ld\n", parameters.a);
		printf("c: %ld\n", parameters.c);
		printf("m: %ld\n", parameters.m);
		*/

		// JOB FILE:
		int fd_file_output;
		if( (fd_file_output = open(parameters.path_output, O_WRONLY| O_CREAT | O_TRUNC, 
			S_IRUSR | S_IWUSR )) == -1){
			perror("Error: Not open OutputFile!");
			return ERROR_OPEN_FILE;
		}
		
		int fd_file_input;
		if( (fd_file_input = open(parameters.path_input, O_RDONLY)) == -1 ){
			perror("Input file open Error!");
			return ERROR_OPEN_FILE;
		}

		struct stat sb;
		if(fstat(fd_file_input, &sb) == -1){
			perror("Error in stat file_input!");
			return ERROR_FSTAT_FILE;
		}

		//printf("size file_input: %ld\n", sb.st_size);
		
		//int c_blck = sb.st_size / SIZE_BLOCK;
		
		int p_blck = sb.st_size % SIZE_BLOCK;

		//printf("c_blck: %d\n", c_blck);
		//printf("p_blck: %d\n", p_blck);

		int count_blocks = ceil( sb.st_size / (double) SIZE_BLOCK );
		
		//printf("count_blocks: %d\n", count_blocks);

		for(int i = 0; i < count_blocks; ++i){
			WWB wwb;
			wwb.index = i;
			wwb.g_params = &parameters;  

			wwb.size_block = (( i < count_blocks - 1 ) 
				|| ( ( i == count_blocks - 1 ) && ( p_blck == 0 ))) ? 
					SIZE_BLOCK : p_blck ;

			wwb.fd_input = fd_file_input;
			wwb.fd_output = fd_file_output;
			work_with_block(&wwb);
		}
			
		if( close(fd_file_input) == -1 ){
			perror("file_input not close!");
			return ERROR_INPUT_FILE;
		}
		
		if( close(fd_file_output) == -1 ){
			perror("file_output not close!");
			return ERROR_OUTPUT_FILE;
		}

	}else{
		perror("Not all parameters passed to argv!");
	}

	if( parameters.status_path_input )
		free(parameters.path_input);

	if( parameters.status_path_output )
		free(parameters.path_output);

	return 0;
}
//! [main]
