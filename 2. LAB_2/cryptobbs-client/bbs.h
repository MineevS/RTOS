/*
 * bbs.h
 *
 *  Created on: 18.02.2023
 *      Author: MSA_I
 */

#ifndef BBS_H_
#define BBS_H_

#include <cstdint>

namespace bbs {

	struct BBSParams
	{
		std::uint32_t seed;
		std::uint32_t p;
		std::uint32_t q;
	};

}

typedef union _my_devctl_msg {
 int tx; // ����������� �������� ��� ������� (send)
 int rx; // ����������� �������� ��� ������� (reply)
} data_t;


/*
 * ������� ���������� ����������� ������������� � ������� ��������� ��������:
	__DIOF(�����, cmd, ������)
		�������� ���������� � ����������.
	__DION(�����, cmd)
		������� ��� ��������� ������.
	__DIOT(�����, cmd, ������)
		���������� ���������� �� ����������.
	__DIOTF(�����, cmd, ������)
		��������� ��������� ���������� �� ���������� � �������� �� ���� ��������� ����������.
 */
#define MY_CMD_CODE 1
#define MY_DEVCTL_GETVAL_BBS __DIOF(_DCMD_MISC, MY_CMD_CODE + 0, bbs::BBSParams)	// ������ � �����: 1;
#define MY_DEVCTL_SETVAL_BBS __DIOT(_DCMD_MISC, MY_CMD_CODE + 1, bbs::BBSParams) 	// ������ � �����: 2;
#define MY_DEVCTL_SETGET_BBS __DIOTF(_DCMD_MISC, MY_CMD_CODE + 2, bbs::BBSParams) 	// ������ � �����: 3;

#define MY_DEVCTL_GETVAL_UINT32t __DIOF(_DCMD_MISC, MY_CMD_CODE + 3, std::uint32_t)	// ������ � �����: 4;
#define MY_DEVCTL_SETVAL_UINT32t __DIOT(_DCMD_MISC, MY_CMD_CODE + 4,  std::uint32_t) // ������ � �����: 5;
#define MY_DEVCTL_SETGET_UINT32t __DIOTF(_DCMD_MISC, MY_CMD_CODE + 2,  std::uint32_t) // ������ � �����: 6;

#define MY_DEVCTL_FLAG __DION(_DCMD_MISC, MY_CMD_CODE + 4) 							// ������ � �����: 7;

#endif /* BBS_H_ */
