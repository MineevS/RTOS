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
 int tx; // Заполняется клиентом при отсылке (send)
 int rx; // Заполняется клиентом при отклике (reply)
} data_t;


/*
 * Команды управления устройством настраиваются с помощью следующих макросов:
	__DIOF(класс, cmd, данные)
		Получать информацию с устройства.
	__DION(класс, cmd)
		Команда без связанных данных.
	__DIOT(класс, cmd, данные)
		Передавать информацию на устройство.
	__DIOTF(класс, cmd, данные)
		Передайте некоторую информацию на устройство и получите от него некоторую информацию.
 */
#define MY_CMD_CODE 1
#define MY_DEVCTL_GETVAL_BBS __DIOF(_DCMD_MISC, MY_CMD_CODE + 0, bbs::BBSParams)	// ЗАПРОС с кодом: 1;
#define MY_DEVCTL_SETVAL_BBS __DIOT(_DCMD_MISC, MY_CMD_CODE + 1, bbs::BBSParams) 	// ЗАПРОС с кодом: 2;
#define MY_DEVCTL_SETGET_BBS __DIOTF(_DCMD_MISC, MY_CMD_CODE + 2, bbs::BBSParams) 	// ЗАПРОС с кодом: 3;

#define MY_DEVCTL_GETVAL_UINT32t __DIOF(_DCMD_MISC, MY_CMD_CODE + 3, std::uint32_t)	// ЗАПРОС с кодом: 4;
#define MY_DEVCTL_SETVAL_UINT32t __DIOT(_DCMD_MISC, MY_CMD_CODE + 4,  std::uint32_t) // ЗАПРОС с кодом: 5;
#define MY_DEVCTL_SETGET_UINT32t __DIOTF(_DCMD_MISC, MY_CMD_CODE + 2,  std::uint32_t) // ЗАПРОС с кодом: 6;

#define MY_DEVCTL_FLAG __DION(_DCMD_MISC, MY_CMD_CODE + 4) 							// ЗАПРОС с кодом: 7;

#endif /* BBS_H_ */
