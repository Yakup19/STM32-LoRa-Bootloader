/*
 * bootloader_command_code.c
 *
 *  Created on: Aug 23, 2025
 *      Author: yakup
 */

#include "bootloader_command_code.h"

uint8_t supported_commands[SUPPORTED_COMMANDS_COUNT] =
{

	BL_GET_VER,
	BL_GET_HELP,
	BL_GET_CID,
	BL_GET_RDP_STATUS,
	BL_GO_TO_ADDR,
	BL_FLASH_ERASE,
	BL_MEM_WRITE,
	BL_EN_RW_PROTECT,
	BL_MEM_READ,
	BL_READ_SECTOR_P_STATUS,
	BL_OTP_READ,
	BL_DIS_R_W_PROTECT
};

