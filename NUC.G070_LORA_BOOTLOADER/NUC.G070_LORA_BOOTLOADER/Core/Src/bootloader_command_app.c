/*
 * bootloader_command_app.c
 *
 *  Created on: 10 Oca 2021
 *      Author: mfati
 */

#include "bootloader_command_app.h"
#include "main.h"
#include "ssd1306.h"
#include "fonts.h"
#include "string.h"
#include "W25Qxx.h"
extern uint8_t supported_commands[];
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart2;
extern I2C_HandleTypeDef hi2c1;
extern DMA_HandleTypeDef hdma_usart3_rx;
void printMessage(char *format, ...);

extern CRC_HandleTypeDef hcrc;
const uint32_t go_to_address = 0;
uint8_t EXT_Flash_Buf[256]={'\0'};
uint32_t EXT_Flash_Write_Start_Addr=0x00000000 ;


void bootloader_get_ver_cmd(uint8_t *bl_rx_data) {
	uint8_t bl_Version[4] = {0};
	uint32_t host_crc=0;

	printMessage("Bootloaer_Get_Ver_Cmd");
	printMessage(" BL_VER :%#x,%#x  ", bl_rx_data[1], bl_rx_data[2]);

	uint32_t command_packet_length = bl_rx_data[0] + 1;

	host_crc |=(bl_rx_data[5]&0xFFFFFFFF)<<24;
	host_crc |=(bl_rx_data[4]&0xFFFFFFFF)<<16;
	host_crc |=(bl_rx_data[3]&0xFFFFFFFF)<<8;
	host_crc |=(bl_rx_data[2]&0xFFFFFFFF);
	// crc control
	if (!bootloader_verify_crc(&bl_rx_data[0], command_packet_length - 4, host_crc)) {
		printMessage("Checksum success");
		bootloader_send_ack(1);
		bl_Version[3] = bootloader_get_version();
		printMessage(" BL_VER : %d %#x  ", bl_Version[3], bl_Version[3]);
		bl_Version[0]=TARGET_LORA_HIGH;
		bl_Version[1]=TARGET_LORA_LOW;
		bl_Version[2]=TARGET_LORA_CHANNEL;
		bootloader_uart_write_data(bl_Version, 4);
	} else {
		printMessage("Checksum fail ");
		bootloader_send_nack();
	}
}

void bootloader_get_help_cmd(uint8_t *bl_rx_data) {
	printMessage("bootloader_get_help_cmd");
	uint32_t host_crc = 0;
	uint32_t command_packet_len = bl_rx_data[0] + 1;


	host_crc |=(bl_rx_data[5]&0xFFFFFFFF)<<24;
	host_crc |=(bl_rx_data[4]&0xFFFFFFFF)<<16;
	host_crc |=(bl_rx_data[3]&0xFFFFFFFF)<<8;
	host_crc |=(bl_rx_data[2]&0xFFFFFFFF);

	if (!bootloader_verify_crc(&bl_rx_data[0], command_packet_len - 4, host_crc)) {
		printMessage("Checksum success");
		bootloader_send_ack(strlen(supported_commands));

		bootloader_uart_write_data(supported_commands, strlen(supported_commands));
		for (int i = 0; i < strlen(supported_commands); i++) {
			printMessage("%#x ", supported_commands[i]);
		}
	} else {
		printMessage("Checksum fail");
		bootloader_send_nack();
	}
}

void bootloader_get_cid_cmd(uint8_t *bl_rx_data) {
	uint8_t cID[5] = {0};
	uint32_t host_crc = 0;
	uint16_t val=0;
	printMessage("bootloader_get_cid_cmd ");

	uint32_t command_packet_len = bl_rx_data[0] + 1;

	//uint32_t host_crc = *((uint32_t*) ((uint32_t*)bl_rx_data + command_packet_len - 4));
	host_crc |=(bl_rx_data[5]&0xFFFFFFFF)<<24;
	host_crc |=(bl_rx_data[4]&0xFFFFFFFF)<<16;
	host_crc |=(bl_rx_data[3]&0xFFFFFFFF)<<8;
	host_crc |=(bl_rx_data[2]&0xFFFFFFFF);

	if (!bootloader_verify_crc(&bl_rx_data[0], command_packet_len - 4,
			host_crc)) {
		printMessage("Checksum succes ");
		bootloader_send_ack(2);
		val = get_mcu_chip_id();
		cID[3] = val & 0xFF; //High 8bit
		cID[4] = (val >> 8) & 0xFF; //High 8bit
		cID[0]=TARGET_LORA_HIGH;
		cID[1]=TARGET_LORA_LOW;
		cID[2]=TARGET_LORA_CHANNEL;
		printMessage("Chip Id: %#x ", cID[3], cID[3]);
		bootloader_uart_write_data(cID, 5);
	} else {
		printMessage("Checksum fail ");
		bootloader_send_nack();
	}
}

void bootloader_go_to_addr_cmd(uint8_t *bl_rx_data) {
	uint32_t go_to_address = 0;
	uint8_t addr_valid[4]={0x18,0x66,0x13,ADDR_VALID};
	uint8_t addr_invalid[4]={0x18,0x66,0x13,ADDR_INVALID};
	uint32_t host_crc = 0;

	printMessage("bootlodaer_go_to_addr_cmd ");

	uint32_t command_packet_len = bl_rx_data[0] + 1;

	//uint32_t host_crc = *((uint32_t*) ((uint32_t*)bl_rx_data + command_packet_len - 4));
	host_crc |=(bl_rx_data[command_packet_len-1]&0xFFFFFFFF)<<24;
	host_crc |=(bl_rx_data[command_packet_len-2]&0xFFFFFFFF)<<16;
	host_crc |=(bl_rx_data[command_packet_len-3]&0xFFFFFFFF)<<8;
	host_crc |=(bl_rx_data[command_packet_len-4]&0xFFFFFFFF);

	if (!bootloader_verify_crc(&bl_rx_data[0], command_packet_len - 4,
			host_crc)) {
		printMessage("Checksum succes ");
		bootloader_send_ack(1);

		//go_to_address = ((uint32_t*)((uint32_t*)&bl_rx_data[2]));

		go_to_address |= ((bl_rx_data[2]) & 0xFFFFFFFF);
		go_to_address |= ((bl_rx_data[3]) & 0xFFFFFFFF) << 8;
		go_to_address |= ((bl_rx_data[4]) & 0xFFFFFFFF) << 16;
		go_to_address |= ((bl_rx_data[5]) & 0xFFFFFFFF) << 24;
		printMessage("GO Addr: %#x ", go_to_address);

		if (bootloader_verify_address(go_to_address) == ADDR_VALID) {

			bootloader_uart_write_data(addr_valid, 4);
			printMessage("Going to Address ");
			SCB->VTOR = go_to_address;
			//__set_MSP(mspValue);	// Bu fonksiyon F407 De calisiyordu ama
			//L053 de çalışmıyor
			SysTick->CTRL = 0;
			SysTick->LOAD = 0;
			SysTick->VAL = 0;
			HAL_I2C_DeInit(&hi2c1);
			HAL_UART_MspDeInit(&huart2);
			HAL_UART_MspDeInit(&huart3);
			HAL_GPIO_DeInit(GPIOC, GPIO_PIN_13);
			HAL_DMA_DeInit(&hdma_usart3_rx);
			HAL_CRC_DeInit(&hcrc);
			HAL_RCC_DeInit();
			HAL_DeInit();
			uint32_t jump_address = *((volatile uint32_t*) (go_to_address + 4));
			void (*jump_to_app)(void) = (void *)jump_address;
			jump_to_app();
		} else {
			printMessage("Go Address Invalid ");
			bootloader_uart_write_data(addr_invalid, 4);
		}
	} else {
		printMessage("Checksum fail ");
		bootloader_send_nack();
	}
}

void bootloader_flash_erase_cmd(uint8_t *bl_rx_data) {
	uint8_t eraseStatus [4]= {0};
	uint32_t host_crc = 0;

	printMessage("bootloader_flash_erase_cmd ");

	uint32_t command_packet_len = bl_rx_data[0] + 1;

	//uint32_t host_crc = *((uint32_t*) ((uint32_t*)bl_rx_data + command_packet_len - 4));
	host_crc |=(bl_rx_data[command_packet_len-1]&0xFFFFFFFF)<<24;
	host_crc |=(bl_rx_data[command_packet_len-2]&0xFFFFFFFF)<<16;
	host_crc |=(bl_rx_data[command_packet_len-3]&0xFFFFFFFF)<<8;
	host_crc |=(bl_rx_data[command_packet_len-4]&0xFFFFFFFF);

	if (!bootloader_verify_crc(&bl_rx_data[0], command_packet_len - 4, host_crc)) {
		printMessage("Checksum success ");
		bootloader_send_ack(1);
		printMessage("Initial Sector: %d Number Of Sectors: %d ", bl_rx_data[2],
				bl_rx_data[3]);
		/*Daha denemedim page 16dan 63 e kadar silmeli*/
		eraseStatus[3] =execute_flash_erase(bl_rx_data[2], bl_rx_data[3]);


		printMessage(" Flash Erase Status : %d ", eraseStatus[3]);
		eraseStatus [0]=TARGET_LORA_HIGH;
		eraseStatus [1]=TARGET_LORA_LOW;
		eraseStatus [2]=TARGET_LORA_CHANNEL;

		bootloader_uart_write_data(eraseStatus, 4);
	}
	else {
		printMessage(" Checksum fail ");
		bootloader_send_nack();
	}
}

void bootloader_mem_write_cmd(uint8_t *bl_rx_data) {
	uint8_t addrValid = ADDR_VALID;
	uint8_t writeStatus[4] = {0x00};
	uint8_t checkSum = 0;
	uint8_t length = 0;
	uint32_t memAddress=0;
	length = bl_rx_data[0];
	uint32_t host_crc = 0;

	uint8_t payloadLength = bl_rx_data[6];


	memAddress |= ((bl_rx_data[2]) & 0xFFFFFFFF);
	memAddress |= ((bl_rx_data[3]) & 0xFFFFFFFF) << 8;
	memAddress |= ((bl_rx_data[4]) & 0xFFFFFFFF) << 16;
	memAddress |= ((bl_rx_data[5]) & 0xFFFFFFFF) << 24;

	checkSum = bl_rx_data[length];

	printMessage(" bootloader_mem_write_cmd ");

	uint32_t command_packet_len = bl_rx_data[0] + 1;

	//uint32_t host_crc = *((uint32_t*) ((uint32_t*)bl_rx_data + command_packet_len - 4));
	host_crc |=(bl_rx_data[command_packet_len-1]&0xFFFFFFFF)<<24;
	host_crc |=(bl_rx_data[command_packet_len-2]&0xFFFFFFFF)<<16;
	host_crc |=(bl_rx_data[command_packet_len-3]&0xFFFFFFFF)<<8;
	host_crc |=(bl_rx_data[command_packet_len-4]&0xFFFFFFFF);

	if (!bootloader_verify_crc(&bl_rx_data[0], command_packet_len - 4, host_crc)) {
		printMessage(" Checksum success ");
		bootloader_send_ack(1);

		printMessage(" Memory Write Address: %#x ", memAddress);

		if (bootloader_verify_address(memAddress) == ADDR_VALID) {
			printMessage(" Valid Memory Write Address ");
			writeStatus[3] = execute_memory_write(&bl_rx_data[7], memAddress, payloadLength);
			writeStatus[0] = TARGET_LORA_HIGH;
			writeStatus[1] = TARGET_LORA_LOW;
			writeStatus[2] = TARGET_LORA_CHANNEL;
			bootloader_uart_write_data(writeStatus, 4);
			SSD1306_GotoXY(0, 32);




		} else {
			printMessage(" Invalid Memory Write Address ");
			writeStatus[3] = ADDR_INVALID;
			writeStatus[0]= TARGET_LORA_HIGH;
			writeStatus[1]= TARGET_LORA_LOW;
			writeStatus[2]= TARGET_LORA_CHANNEL;
			bootloader_uart_write_data(writeStatus, 4);

		}
	} else {
		printMessage(" Checksum fail ");
		//bootloader_send_nack();
	}
}

void bootloader_enable_read_write_protect_cmd(uint8_t *bl_rx_data) {
	uint8_t status = 0;
	uint32_t host_crc = 0;

	printMessage(" bootloader_enable_read_write_protect_cmd ");

	uint32_t command_packet_len = bl_rx_data[0] + 1;

	//uint32_t host_crc = *((uint32_t*) ((uint32_t*)bl_rx_data + command_packet_len - 4));
	host_crc |=(bl_rx_data[5]&0xFFFFFFFF)<<24;
	host_crc |=(bl_rx_data[4]&0xFFFFFFFF)<<16;
	host_crc |=(bl_rx_data[3]&0xFFFFFFFF)<<8;
	host_crc |=(bl_rx_data[2]&0xFFFFFFFF);

	if (!bootloader_verify_crc(&bl_rx_data[0], command_packet_len - 4, host_crc)) {
		printMessage(" Checksum success ");
		bootloader_send_ack(1);

		status = configure_flash_sector_r_w_protection(bl_rx_data[2],
				bl_rx_data[3], 0);

		printMessage(" Status: %d", status);

		bootloader_uart_write_data(&status, 1);
	} else {
		printMessage(" Checksum fail ");
		bootloader_send_nack();
	}
}

void bootloader_go_to_bootloader_cmd(uint8_t *bl_rx_data)
{
	uint32_t host_crc = 0;
	uint8_t BL_Lora[4]={0x18,0x66,0x13, BL_BOOTLOADER_ACTIVE};

	uint32_t command_packet_len = bl_rx_data[0] + 1;

	//uint32_t host_crc = *((uint32_t*) ((uint32_t*)bl_rx_data + command_packet_len - 4));
	host_crc |=(bl_rx_data[command_packet_len-1]&0xFFFFFFFF)<<24;
	host_crc |=(bl_rx_data[command_packet_len-2]&0xFFFFFFFF)<<16;
	host_crc |=(bl_rx_data[command_packet_len-3]&0xFFFFFFFF)<<8;
	host_crc |=(bl_rx_data[command_packet_len-4]&0xFFFFFFFF);

	if (!bootloader_verify_crc(&bl_rx_data[0], command_packet_len - 4, host_crc)) {
		printMessage("Checksum succes ");
		bootloader_send_ack(1);
		printMessage(" Bootloader already running  ");
		bootloader_uart_write_data(BL_Lora, 4);


		}
	else {
			printMessage("Checksum fail ");
			bootloader_send_nack();
		}
}

void bootloader_ext_mem_to_mem_write_cmd(uint8_t *bl_rx_data)
{
	uint8_t bl_Version[4] = { '\0' };
	uint32_t host_crc = 0;

	printMessage("bootloader_ext_to_mem_cmd");

	uint32_t command_packet_length = bl_rx_data[0] + 1;

	host_crc |= (bl_rx_data[5] & 0xFFFFFFFF) << 24;
	host_crc |= (bl_rx_data[4] & 0xFFFFFFFF) << 16;
	host_crc |= (bl_rx_data[3] & 0xFFFFFFFF) << 8;
	host_crc |= (bl_rx_data[2] & 0xFFFFFFFF);
	// crc control
	if (!bootloader_verify_crc(&bl_rx_data[0], command_packet_length - 4,
			host_crc)) {
		printMessage("Checksum success");
		bootloader_send_ack(1);
		bl_Version[3] = 0x20;	// BL_EXT_MEM_TO_MEM_WRITE 	working
		printMessage("MEM_WRITE_WORKING ");
		bl_Version[0] = TARGET_LORA_HIGH;
		bl_Version[1] = TARGET_LORA_LOW;
		bl_Version[2] = TARGET_LORA_CHANNEL;
		bootloader_uart_write_data(bl_Version, 4);
		EXT_Flash_Write_Start_Addr = 0x000000;
		while (EXT_Flash_Buf[0] != 0xFF) {
			memset(EXT_Flash_Buf, '\0', 256);
			//HAL_Delay(3);
			W25Q_Read_Fast(EXT_Flash_Write_Start_Addr, EXT_Flash_Buf);
			bootloader_mem_write_cmd(EXT_Flash_Buf);
			// Yazılacak page öteleniyor
			EXT_Flash_Write_Start_Addr = 256 + EXT_Flash_Write_Start_Addr;
		}
		printMessage("MEM_WRITE_DONE ");
		W25Q_Chip_Erase();
	} else {
		printMessage("Checksum fail ");
		bootloader_send_nack();
	}



}

void bootloader_ext_mem_write_cmd(uint8_t *bl_rx_data)
{
	uint8_t addrValid = ADDR_VALID;
	uint8_t writeStatus[4] = { 0x00 };
	uint8_t checkSum = 0;
	uint8_t length = 0;
	uint32_t memAddress = 0;
	length = bl_rx_data[0];
	uint32_t host_crc = 0;

	uint8_t payloadLength = bl_rx_data[6];

	memAddress |= ((bl_rx_data[2]) & 0xFFFFFFFF);
	memAddress |= ((bl_rx_data[3]) & 0xFFFFFFFF) << 8;
	memAddress |= ((bl_rx_data[4]) & 0xFFFFFFFF) << 16;
	memAddress |= ((bl_rx_data[5]) & 0xFFFFFFFF) << 24;

	checkSum = bl_rx_data[length];

	printMessage(" bootloader_mem_write_cmd ");

	uint32_t command_packet_len = bl_rx_data[0] + 1;

	host_crc |= (bl_rx_data[command_packet_len - 1] & 0xFFFFFFFF) << 24;
	host_crc |= (bl_rx_data[command_packet_len - 2] & 0xFFFFFFFF) << 16;
	host_crc |= (bl_rx_data[command_packet_len - 3] & 0xFFFFFFFF) << 8;
	host_crc |= (bl_rx_data[command_packet_len - 4] & 0xFFFFFFFF);

	if (!bootloader_verify_crc(&bl_rx_data[0], command_packet_len - 4,host_crc)) {
		printMessage(" Checksum success ");
		bootloader_send_ack(1);

		printMessage(" Memory Write Address: %#x ", memAddress);
		memset(EXT_Flash_Buf, '\0', 256);
		if (bootloader_verify_address(memAddress) == ADDR_VALID) {
			printMessage(" Valid Memory Write Address ");
			W25Q_Write_Enable();
			writeStatus[3] = execute_ext_mem_write(bl_rx_data, payloadLength);
			W25Q_Write_Disable();
			W25Q_Read_Fast(0x00000000, EXT_Flash_Buf);
			writeStatus[0] = TARGET_LORA_HIGH;
			writeStatus[1] = TARGET_LORA_LOW;
			writeStatus[2] = TARGET_LORA_CHANNEL;
			bootloader_uart_write_data(writeStatus, 4);
			SSD1306_GotoXY(0, 32);

		} else {
			printMessage(" Invalid Memory Write Address ");
			writeStatus[3] = ADDR_INVALID;
			writeStatus[0] = TARGET_LORA_HIGH;
			writeStatus[1] = TARGET_LORA_LOW;
			writeStatus[2] = TARGET_LORA_CHANNEL;
			bootloader_uart_write_data(writeStatus, 4);

		}
	} else {
		printMessage(" Checksum fail ");
		//bootloader_send_nack();
	}
}
//void bootloader_read_sector_protection_status_cmd(uint8_t *bl_rx_data) {
//	uint16_t status = 0;
//
//	printMessage(" bootloader_read_sector_protection_status_cmd ");
//
//	uint32_t command_packet_len = bl_rx_data[0] + 1;
//
//	uint32_t host_crc = *((uint32_t*) ((uint32_t*)bl_rx_data + command_packet_len - 4));
//
//	if (!bootloader_verify_crc(&bl_rx_data[0], command_packet_len - 4,
//			host_crc)) {
//		printMessage(" Checksum success ");
//		bootloader_send_ack(1);
//
//		status = read_OB_r_w_protection_status();
//
//		printMessage(" nWRP status: %#", status);
//		bootloader_uart_write_data((uint8_t*) &status, 2);
//	} else {
//		printMessage(" Checksum fail ");
//		bootloader_send_nack();
//	}
//
//}

void bootloader_disable_read_write_protect_cmd(uint8_t *bl_rx_data) {
	uint8_t status = 0;
	uint32_t host_crc = 0;

	printMessage(" bootloader_disable_read_write_protect_cmd ");

	uint32_t command_packet_len = bl_rx_data[0] + 1;

	//uint32_t host_crc = *((uint32_t*) ((uint32_t*)bl_rx_data + command_packet_len - 4));
	host_crc |=(bl_rx_data[5]&0xFFFFFFFF)<<24;
	host_crc |=(bl_rx_data[4]&0xFFFFFFFF)<<16;
	host_crc |=(bl_rx_data[3]&0xFFFFFFFF)<<8;
	host_crc |=(bl_rx_data[2]&0xFFFFFFFF);

	if (!bootloader_verify_crc(&bl_rx_data[0], command_packet_len - 4,
			host_crc)) {
		printMessage(" Checksum success ");
		bootloader_send_ack(1);

		status = configure_flash_sector_r_w_protection(0, 0, 1);

		printMessage(" Status: %d", status);

		bootloader_uart_write_data(&status, 1);
	} else {
		printMessage(" Checksum fail ");
		bootloader_send_nack();
	}
}

void bootloader_uart_write_data(uint8_t *Buffer, uint32_t len) {

	HAL_UART_Transmit(&huart3, Buffer, len, HAL_MAX_DELAY);
}

uint8_t bootloader_verify_crc(uint8_t* Buffer, uint32_t len, uint32_t crcHost) {
	uint32_t crcValue = 0xFF;
		uint32_t data = 0;

		for (uint32_t i = 0; i < len; i++) {
			data = Buffer[i];
			crcValue = HAL_CRC_Accumulate(&hcrc, &data, 1);
		}
	__HAL_CRC_DR_RESET(&hcrc);

	if (crcValue == crcHost) {
		return CRC_SUCCESS;
	}

	return CRC_FAIL;
}

void bootloader_get_rdp_cmd(uint8_t *bl_rx_data) {
	uint8_t rdpLevel [4]= {0};
	uint32_t host_crc = 0;
	printMessage(" bootloader_get_rdp_cmd ");

	uint32_t command_packet_len = bl_rx_data[0] + 1;

	//uint32_t host_crc = *((uint32_t*) ((uint32_t*)bl_rx_data + command_packet_len - 4));

	host_crc |=(bl_rx_data[5]&0xFFFFFFFF)<<24;
	host_crc |=(bl_rx_data[4]&0xFFFFFFFF)<<16;
	host_crc |=(bl_rx_data[3]&0xFFFFFFFF)<<8;
	host_crc |=(bl_rx_data[2]&0xFFFFFFFF);


	if (!bootloader_verify_crc(&bl_rx_data[0], command_packet_len - 4,
			host_crc)) {
		printMessage(" Checksum succes ");
		bootloader_send_ack(1);

		rdpLevel[3] = get_flash_rdp_level();
		rdpLevel[0]= TARGET_LORA_HIGH;
		rdpLevel[1]= TARGET_LORA_LOW;
		rdpLevel[2]= TARGET_LORA_CHANNEL;
		printMessage("RDP Level: %d", rdpLevel);
		bootloader_uart_write_data(rdpLevel, 4);
	} else {
		printMessage(" Checksum fail ");
		bootloader_send_nack();
	}

}

void bootloader_send_ack(uint8_t followLength) {
	uint8_t ackBuffer[5];
	ackBuffer[0] = TARGET_LORA_HIGH;
	ackBuffer[1] = TARGET_LORA_LOW;
	ackBuffer[2] = TARGET_LORA_CHANNEL;
	ackBuffer[3] = BL_ACK_VALUE;
	ackBuffer[4] = followLength;

	HAL_UART_Transmit(&huart3, ackBuffer, 5, HAL_MAX_DELAY);
}

void bootloader_send_nack() {
	uint8_t nackValue[4] ;
	nackValue[0]= TARGET_LORA_HIGH;
	nackValue[1]= TARGET_LORA_LOW;
	nackValue[2]= TARGET_LORA_CHANNEL;
	nackValue[3]= BL_NACK_VALUE;
	HAL_UART_Transmit(&huart3, nackValue, 4, HAL_MAX_DELAY);
}

uint8_t bootloader_get_version(void) {
	return BL_VER;
}

uint16_t get_mcu_chip_id(void) {
	uint16_t cID;
	cID = (uint16_t) (DBG->IDCODE) & 0x0FFF;
	return cID;
}

uint8_t get_flash_rdp_level(void) {
	uint8_t rdp_status = 0;

#if	1

	volatile uint32_t *OB_Addr = (uint32_t*) RDP_REG;
	rdp_status = (uint8_t) ((*OB_Addr )& 0x00000000FF);


#else

	FLASH_OBProgramInitTypeDef OB_InitStruct;
	HAL_FLASHEx_OBGetConfig(&OB_InitStruct);
	rdp_level = (uint8_t) OB_InitStruct.RDPLevel;

#endif

	return rdp_status;
}

uint8_t bootloader_verify_address(uint32_t goAddress) {
	if (goAddress >= FLASH_BASE && goAddress <= G0_FLASH_END)
		return ADDR_VALID;
	else
	return ADDR_INVALID;
}
/*
 * sectorNumber Silinecek sektör numarası
 * numberOfSector sectorNumber'dan sonraki silinecek sektör sayısı
 *
 * */
uint8_t execute_flash_erase(uint8_t sectorNumber, uint8_t numberOfSector) {
	FLASH_EraseInitTypeDef FlashEraseInitStruct = { 0 };
	uint32_t SectorError = 0;
	HAL_StatusTypeDef status = { 0 };

	if (sectorNumber > 63)
		return INVALID_SECTOR;

	if ((sectorNumber <= 63) || (sectorNumber == 0xFF)) {
		if (sectorNumber == 0xFF) {
			FlashEraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
			FlashEraseInitStruct.Page = ((FLASH_END_ADDRESS-FLASH_APP_BASE_ADDRESS)/FLASH_BANK_SIZE);
			FlashEraseInitStruct.NbPages = FLASH_PAGE_NB;

		} else {


			FlashEraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
			FlashEraseInitStruct.Page = sectorNumber;

			FlashEraseInitStruct.NbPages = numberOfSector;
		}
		FlashEraseInitStruct.Banks = FLASH_BANK_1;

		HAL_FLASH_Unlock();
		status = (uint8_t) HAL_FLASHEx_Erase(&FlashEraseInitStruct, &SectorError);
		HAL_FLASH_Lock();

		return status;
	}

	return INVALID_SECTOR;
}

uint8_t execute_memory_write(uint8_t *Buffer, uint32_t memAddress, uint32_t len) {
	uint8_t status = HAL_ERROR;
	uint64_t data;

	__HAL_FLASH_CLEAR_FLAG(FLASH_ECCR_ECCD);
	__HAL_FLASH_CLEAR_FLAG(FLASH_ECCR_ECCC);
	__HAL_FLASH_CLEAR_FLAG(FLASH_ECCR_ECCCIE );
	__HAL_FLASH_CLEAR_FLAG(FLASH_ECCR_SYSF_ECC);
	__HAL_FLASH_CLEAR_FLAG(FLASH_ECCR_ADDR_ECC );
	HAL_FLASH_Lock();

	for (uint32_t i = 0; i < len; i= i+8) {
			//while ((FLASH->SR & FLASH_SR_BSY1)) {}
			HAL_FLASH_Unlock();

			data=0;
			data |= ((uint64_t)(Buffer[i+7] &0xFFFFFFFF))<<56;
			data |= ((uint64_t)(Buffer[i+6] &0xFFFFFFFF))<<48;
			data |= ((uint64_t)(Buffer[i+5] &0xFFFFFFFF))<<40;
			data |= ((uint64_t)(Buffer[i+4] &0xFFFFFFFF))<<32;
			data |= ((uint64_t)(Buffer[i+3] &0xFFFFFFFF))<<24;
			data |= ((uint64_t)(Buffer[i+2] &0xFFFFFFFF))<<16;
			data |= ((uint64_t)(Buffer[i+1] &0xFFFFFFFF))<<8;
			data |= ((uint64_t)(Buffer[i]   &0xFFFFFFFF));


			status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, memAddress+i, data);
			HAL_Delay(3);
			HAL_FLASH_Lock();
	}

	return status;
}
uint8_t execute_ext_mem_write(uint8_t *Buffer, uint32_t len)
{
	//uint8_t status = HAL_ERROR;

	W25Q_Buf_Program(EXT_Flash_Write_Start_Addr, Buffer, len);
	EXT_Flash_Write_Start_Addr = 256 + EXT_Flash_Write_Start_Addr;
	return HAL_OK;
}

uint8_t configure_flash_sector_r_w_protection(uint8_t sector_details,
		uint8_t protection_mode, uint8_t enableOrDisable) {
	volatile uint32_t *pOPTCR = (uint32_t*) 0x40023C14;

	// enableOrDisable == 0 -> en_w_r_protect | enableOrDisable == 1 -> dis_r_w_protect

	if (enableOrDisable) {
		HAL_FLASH_OB_Unlock();

		while (__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET)
			;

		*pOPTCR |= (0xFF << 16);

		*pOPTCR |= (1 << 1);

		while (__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET)
			;

		HAL_FLASH_OB_Lock();

		return 0;
	}

	if (protection_mode == 1)	// write protection
			{
		HAL_FLASH_OB_Unlock();

		while (__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET)
			;

		*pOPTCR &= ~(sector_details << 16);

		*pOPTCR |= (1 << 1);

		while (__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET)
			;

		HAL_FLASH_OB_Lock();
	} else if (protection_mode == 2) // read / write protection
			{
		HAL_FLASH_OB_Unlock();

		while (__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET)
			;

		*pOPTCR &= ~(0xFF << 16);				// write protecton all sector
		*pOPTCR |= (sector_details << 16);

		*pOPTCR |= (0xFF << 8);					// read protection all sector

		*pOPTCR |= (1 << 1);

		while (__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET)
			;

		HAL_FLASH_OB_Lock();
	}

	return 0;
}

