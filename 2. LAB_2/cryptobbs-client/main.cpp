#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <devctl.h>	// devctl;
#include <sys/dcmd_chr.h>
#include <vector>
#include <cstdio>	// stdout
#include <signal.h>	// signal;
#include <setjmp.h>

jmp_buf return_to_top_level;
//volatile sig_atomic_t sig = 1;

#include <bbs.h>

#define UNUSED(x) ((x)=(x))
#define SIZE_MAX_VACTOR 1024

void handler(int sim){

    //sig = 0;
    longjmp (return_to_top_level, 1);
}

int main( int argc, char **argv )
{
	std::cout << "Start Client! " << std::endl;

	int fd, ret;

	if ((fd = open("/dev/cryptobbs", O_RDONLY)) == -1) {
		std::cerr << "E: unable to open server connection: " << strerror(errno) << std::endl;
		return EXIT_FAILURE;
	}

	/* Выясняем, какое значение установлено при инициализации */
    bbs::BBSParams bbs_param;
    bbs_param.seed = -1;
    bbs_param.p = -1;
    bbs_param.q = -1;
	ret = devctl(fd, MY_DEVCTL_GETVAL_BBS, &bbs_param, sizeof(bbs_param), NULL);
	std::cout << "GET вернул " << ret << " w/ значение сервера:\n " <<
			"bbs_param.seed: " << bbs_param.seed <<  " \n" <<
			"bbs_param.p: " << bbs_param.p <<  " \n" <<
			"bbs_param.q: " << bbs_param.q <<  " \n";
	//printf("GET вернул %d w/ значение сервера %d \n", ret, val);

	/* Устанавливаем какое-нибудь другое значение */
    bbs_param.seed = 866;
    bbs_param.p = 3;
    bbs_param.q = 263;
	ret = devctl(fd, MY_DEVCTL_SETVAL_BBS, &bbs_param, sizeof(bbs_param), NULL);
	std::cout << "SET вернул " << ret << " w/ значение сервера:\n ";

	std::vector<std::uint32_t> vector(SIZE_MAX_VACTOR, 0);
	int SIZE_VECTOR = 0;
	signal(SIGINT, handler);
	while(1){
		if(setjmp(return_to_top_level) == 1)
			break;

		if( SIZE_VECTOR == SIZE_MAX_VACTOR )
			SIZE_VECTOR = 0;

		std::uint32_t val = 0;
		ret = devctl(fd, MY_DEVCTL_GETVAL_UINT32t, &val, sizeof(val), NULL);
		//std::cout << "SET вернул " << ret << "  - значение сервера: "<< ret << "\n ";

		vector[SIZE_VECTOR] = val;

		SIZE_VECTOR++;
	}

	for(int i = 0; i < SIZE_MAX_VACTOR; ++i){
		std::cout << "vector[" << i << "] - значение: "<< vector[i] << "\n ";
	}

	close(fd);

	std::cout << "End Client! " << std::endl;

	return EXIT_SUCCESS;
}



