#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <string.h>
#include <bbs.h>
#include <devctl.h>	//_DEVCTL_DATA(msg->i)
#include <stdlib.h>// rand();
#include <cmath>	// fmod; pow; // -lm in common;
#include <iostream>
#include <bitset>

static resmgr_connect_funcs_t    connect_funcs;
static resmgr_io_funcs_t         io_funcs;
static iofunc_attr_t             attr;

//int handle_devctl(resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb);
int global_integer = 0;
bbs::BBSParams g_bbs_param = {1, 2, 3};
/*bbs_param.seed = 1;
bbs_param.p = 2;
bbs_param.q = 3;*/

bool parity_bit (std::uint32_t x)
{
	x ^= x >> 1;
	x ^= x >> 2;
	x = (x & 0x11111111U) * 0x11111111U;
	return (x >> 28) & 1;
}

bool flag = 1;
std::uint32_t x_n;
std::uint32_t BBS(){
	std::uint32_t res = 0;

	for(std::uint32_t i = 0; i < 32; ++i){
		if(flag == 1){
			x_n = std::fmod( std::pow( g_bbs_param.seed , 2) , g_bbs_param.p*g_bbs_param.q);
			flag = 0;
		}else{
			x_n = std::fmod(  std::pow( x_n, 2)  , g_bbs_param.p*g_bbs_param.q);
		}
		res = res | (parity_bit(x_n) << i);
		std::cout << "x: " << x_n << " - binary: " <<  std::bitset<10>(x_n) << " - parity_bit: " << ((res & (1u << i)) != 0) << std::endl;
	}

	return res;
}

int io_devctl(resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb)
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
		  g_bbs_param.seed = rx_data->bbs_param.seed;
		  g_bbs_param.p = rx_data->bbs_param.p;
		  g_bbs_param.q = rx_data->bbs_param.q;
		  flag = 1; // ������������� ���� ��� ����, ����� BBS() ������ ������ �� ���������� x_0;
		  nbytes = 0;
		  break;
	  case MY_DEVCTL_GETVAL_BBS:
		  rx_data->bbs_param.seed = g_bbs_param.seed;
		  rx_data->bbs_param.p = g_bbs_param.p;
		  rx_data->bbs_param.q = g_bbs_param.q;
		  nbytes = sizeof(rx_data->bbs_param);
		  break;
	  case MY_DEVCTL_GETVAL_UINT32t:
		  rx_data->val = BBS();
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

int main(int argc, char **argv)
{
	std::cout << "Start server! " << std::endl;
    /* declare variables we'll be using */
    resmgr_attr_t        resmgr_attr;
    dispatch_t           *dpp;
    dispatch_context_t   *ctp;
    int                  id;

    /* initialize dispatch interface */
    if((dpp = dispatch_create()) == NULL) {
        fprintf(stderr,
                "%s: Unable to allocate dispatch handle.\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    /* initialize resource manager attributes */
    memset(&resmgr_attr, 0, sizeof resmgr_attr);
    resmgr_attr.nparts_max = 1;
    resmgr_attr.msg_max_size = 2048;

    /* initialize functions for handling messages */
    iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs,
                     _RESMGR_IO_NFUNCS, &io_funcs);

    /* initialize attribute structure used by the device */
    iofunc_attr_init(&attr, S_IFNAM | 0666, 0, 0);

    /* attach our device name */
    id = resmgr_attach(
            dpp,            /* dispatch handle        */
            &resmgr_attr,   /* resource manager attrs */
            "/dev/cryptobbs",  /* device name            */
            _FTYPE_ANY,     /* open type              */
            0,              /* flags                  */
            &connect_funcs, /* connect routines       */
            &io_funcs,      /* I/O routines           */
            &attr);         /* handle                 */
    if(id == -1) {
        fprintf(stderr, "%s: Unable to attach name.\n", argv[0]);
        return EXIT_FAILURE;
    }
    /**/
    io_funcs.devctl = io_devctl; // ��� ��������� _IO_DEVCTL ��������� �������� devctl();

    /* allocate a context structure */
    ctp = dispatch_context_alloc(dpp);

    /* start the resource manager message loop */
    while(1) {
        if((ctp = dispatch_block(ctp)) == NULL) {
            fprintf(stderr, "block error\n");
            return EXIT_FAILURE;
        }
        dispatch_handler(ctp);
    }

    std::cout << "End server! " << std::endl;

    return EXIT_SUCCESS; // never go here
}

