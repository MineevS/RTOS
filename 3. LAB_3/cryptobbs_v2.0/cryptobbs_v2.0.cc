#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>

/*
 *  ���������� ��������� THREAD_POOL_PARAM_T ����� ��������� ��������������
 *  ����������� ��� ������������� ������� ��������� dispatch_*()
 *  (����������� ���������� �����  <sys/iofunc.h>, <sys/dispatch.h> );
 */

// 1:
//#define THREAD_POOL_PARAM_T dispatch_context_t

// 2:
#define THREAD_POOL_PARAM_T resmgr_context_t

#define UNUSED(x) ((x)=(x))

#include <sys/iofunc.h>
#include <sys/dispatch.h>

///////////////

#include <string.h>
#include <bbs.h>
#include <devctl.h>	//_DEVCTL_DATA(msg->i)
#include <stdlib.h>// rand();
#include <cmath>	// fmod; pow; // -lm in common;
#include <iostream>
#include <bitset>
#include <map>

//////////////

static resmgr_connect_funcs_t    connect_funcs;
static resmgr_io_funcs_t         io_funcs;
static iofunc_attr_t             attr;

bbs::BBSParams g_bbs_param = {1, 2, 3}; //default;
typedef struct _context_thread{
	bool fp;
	bbs::BBSParams bp;
	std::uint32_t x_n;
} thread_context_t;

std::map<int, thread_context_t> contexts;

int io_devctl(THREAD_POOL_PARAM_T *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb);
int io_open (THREAD_POOL_PARAM_T *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle, void *extra);
int io_close_dup(THREAD_POOL_PARAM_T *ctp, io_close_t *msg, RESMGR_OCB_T *ocb);
int io_close_ocb(THREAD_POOL_PARAM_T *ctp, void *reserved, RESMGR_OCB_T *ocb);

std::uint32_t BBS(int rcvid);
bool parity_bit (std::uint32_t x);

main(int argc, char **argv)
{
	std::cout << "Start Server! " << std::endl;

    thread_pool_attr_t   pool_attr;
    resmgr_attr_t        resmgr_attr;
    dispatch_t           *dpp;
    thread_pool_t        *tpp;
    THREAD_POOL_PARAM_T   *ctp;
    int                  id;

    UNUSED(ctp);

    /* ������������� ���������� ��������������� */
    if((dpp = dispatch_create()) == NULL) {
        fprintf(stderr,
                "%s: Unable to allocate dispatch handle.\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    /* ������������� ��������� �� - ��������� IOV */
    memset(&resmgr_attr, 0, sizeof resmgr_attr);
    resmgr_attr.nparts_max = 1;
    resmgr_attr.msg_max_size = 2048;

    /* ������������� ��������� �������-������������ ��������� */
    iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs,
                     _RESMGR_IO_NFUNCS, &io_funcs);

    /* ������������� ��������� ����������*/
    iofunc_attr_init(&attr, S_IFNAM | 0666, 0, 0);

    connect_funcs.open = io_open;

    /* ������������ � ����� ������������ � ������������ ��� ����� */
    id = resmgr_attach(
            dpp,            /* ����� ���������� ��������������� */
            &resmgr_attr,   /* �������� �� */
            "/dev/sample",  /* ����� ������������ */
            _FTYPE_ANY,     /* open type              */
            0,              /* �����                  */
            &connect_funcs, /* ������� ������������ ���������� */
            &io_funcs,      /* ������� �����-������   */
            &attr);         /* ����� ��������� ���������� */
    if(id == -1) {
        fprintf(stderr, "%s: Unable to attach name.\n", argv[0]);
        return EXIT_FAILURE;
    }

    //
    io_funcs.close_dup = io_close_dup;
    io_funcs.close_ocb = io_close_ocb;
    io_funcs.devctl = io_devctl; // ��� ��������� _IO_DEVCTL ��������� �������� devctl();


    /* ������������� ��������� ���� ������� */
    memset(&pool_attr, 0, sizeof pool_attr);
    pool_attr.handle = dpp;

    // 1:
	// ������� ������������ � ������� ��� ��������:
    /*pool_attr.context_alloc = dispatch_context_alloc;
    pool_attr.block_func = dispatch_block;
    pool_attr.unblock_func = dispatch_unblock;
    pool_attr.handler_func = dispatch_handler;
    pool_attr.context_free = dispatch_context_free;//*/

    // 2:
    // ������� ������������ � ������� ��� ��������� ������;
    pool_attr.context_alloc = resmgr_context_alloc;
    pool_attr.block_func = resmgr_block;
    pool_attr.unblock_func = resmgr_unblock;
    pool_attr.handler_func = resmgr_handler;
    pool_attr.context_free = resmgr_context_free; //*/

    pool_attr.lo_water = 2;
    pool_attr.hi_water = 4;
    pool_attr.increment = 1;
    pool_attr.maximum = 50;

    /* ������������� ���� ������� */
    if((tpp = thread_pool_create(&pool_attr,
                                 POOL_FLAG_EXIT_SELF)) == NULL) {
        fprintf(stderr, "%s: Unable to initialize thread pool.\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    /* ��������� ������, ����������� ������� */
    thread_pool_start(tpp);
    /* ����� �� �� ���������, ������� */

    std::cout << "End Server! " << std::endl;
}

bool parity_bit (std::uint32_t x)
{
	x ^= x >> 1;
	x ^= x >> 2;
	x = (x & 0x11111111U) * 0x11111111U;
	return (x >> 28) & 1;
}

std::uint32_t BBS(int rcvid){

	std::uint32_t res = 0;

	for(std::uint32_t i = 0; i < 32; ++i){
		if(contexts[rcvid].fp == 1){
			contexts[rcvid].x_n = std::fmod( std::pow( contexts[rcvid].bp.seed, 2), contexts[rcvid].bp.p * contexts[rcvid].bp.q );
			contexts[rcvid].fp = 0;
		}else{
			contexts[rcvid].x_n = std::fmod(  std::pow( contexts[rcvid].x_n, 2), contexts[rcvid].bp.p * contexts[rcvid].bp.q );
		}

		res = res | ( parity_bit(contexts[rcvid].x_n) << i );

		std::cout << "x: " << contexts[rcvid].x_n << " - binary: " <<  std::bitset<10>(contexts[rcvid].x_n) << " - parity_bit: " << ((res & (1u << i)) != 0) << std::endl;
	}

	return res;
}

int io_open(THREAD_POOL_PARAM_T *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle, void *extra)
{
    printf("Open the Resource Manger\n");

    printf("rcvid: %d\n", ctp->rcvid);

    thread_context_t tct = {1, {0, 0, 0}, 0};

    /*tct.fp = 1;
    tct.bp.seed = 0;
    tct.bp.p = 0;
    tct.bp.q = 0;*/

    contexts.insert(std::pair<int, thread_context_t>(ctp->rcvid, tct));

    return iofunc_open_default(ctp, msg, handle, extra);
}

int io_close_dup(THREAD_POOL_PARAM_T *ctp, io_close_t *msg, RESMGR_OCB_T *ocb)
{
    printf("Close the dup\n");

    printf("rcvid: %d\n", ctp->rcvid);

    contexts.erase(ctp->rcvid);

    return iofunc_close_dup_default(ctp, msg, ocb);
}

int io_close_ocb(THREAD_POOL_PARAM_T *ctp, void *reserved, RESMGR_OCB_T *ocb)
{
    printf("Close the ocb\n");

    printf("rcvid: %d\n", ctp->rcvid);

    return iofunc_close_ocb_default(ctp, reserved, ocb);
}

int io_devctl(THREAD_POOL_PARAM_T *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb)
{
	 int nbytes, status;
	 union U {
	 bbs::BBSParams bbs_param;
	 std::uint32_t val;
	 // ... ������ ���� devctl, ������� �� ������ ��������
	 };

	 U* rx_data;
	 /*
	 ��� ����� ��� ��������� � ������� DCMD_ALL_*
	 �� ������ ��������� ��� ����� ��� ����� ����, ��� �� �������������� devctl. ����� �� ��
	 ���������� ������� ��������������� ��������, �������� ������� ��������� ��������� �������.
	 */
	  if ((status = iofunc_devctl_default(ctp, msg, ocb)) != _RESMGR_DEFAULT) {
		  return(status);
	  }
	  status = nbytes = 0;

	 /*
	 ��������: ����� ��������������, ��� �� ������ ������������ ���� ����� ������ ��� devctl �
	 ���� ���������. � ���������������� ��, ��������, �������� ��������� MsgReadv(), ����� ��
	 ������ ��� ���������, ������� ���������, ����� ���������� ��� ������, ������ ���� �����
	 ������������, ��� ��� ������ ������������ � ���������.
	 �� ������������� � ����� main���������, ��� ��������� ����� ������ ��������� �� 2�, ��� ��� �� �� ����������� �� ���� �
	 ������ �������, ��� ����� ���� � ������ (int).
	 */
	  rx_data = reinterpret_cast<U*>(_DEVCTL_DATA(msg->i));

	 /*
	 ��� ������� �������� devctl.
	  SET: ��������� �������� (int) �� �������
	  GET: ��������� �������� (int) � �������
	  SETGET: ��������� ������ �������� � ������� ����������� ��������
	 */
	  switch (msg->i.dcmd) {
	  case MY_DEVCTL_SETVAL_BBS:
		  contexts[ctp->rcvid].bp.seed 	= rx_data->bbs_param.seed;
		  contexts[ctp->rcvid].bp.p 	= rx_data->bbs_param.p;
		  contexts[ctp->rcvid].bp.q 	= rx_data->bbs_param.q;
		  nbytes = 0;
		  break;
	  case MY_DEVCTL_GETVAL_BBS:
		  rx_data->bbs_param.seed 	= g_bbs_param.seed;
		  rx_data->bbs_param.p 		= g_bbs_param.p;
		  rx_data->bbs_param.q 		= g_bbs_param.q;
		  nbytes = sizeof(rx_data->bbs_param);
		  break;
	  case MY_DEVCTL_GETVAL_UINT32t:
		  printf("rcvid: %d\n", ctp->rcvid);
		  printf("seed: %d\n", 	contexts[ctp->rcvid].bp.seed);
		  printf("p: %d\n", 	contexts[ctp->rcvid].bp.p);
		  printf("q: %d\n\n", 	contexts[ctp->rcvid].bp.q);
		  rx_data->val = BBS(ctp->rcvid);
		  nbytes = sizeof(rx_data->val);
		  break;
	  case MY_DEVCTL_SETVAL_UINT32t:
		  // ...
		  nbytes = 0;
		  break;
	  case MY_DEVCTL_SETGET_BBS:
		  //...
		  nbytes = 0;
		  break;
	   default:
		  return(ENOSYS);
	   }

	   /* ������� ��������� return ... ��������, ��� �� ��������� ���� ������ �� ����� */
	   memset(&msg->o, 0, sizeof(msg->o));

	   /*
	   ���� �� ������ �������� � ���� return ��� devctl() ���-�� ������, �� ������ ������� ��� �
	   ������� ����� ����.
	   */
		msg->o.ret_val = status;

		/* ��������� ����� ������ � ������������ ��������� */
		msg->o.nbytes = nbytes;
		return(_RESMGR_PTR(ctp, &msg->o, sizeof(msg->o) + nbytes));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




