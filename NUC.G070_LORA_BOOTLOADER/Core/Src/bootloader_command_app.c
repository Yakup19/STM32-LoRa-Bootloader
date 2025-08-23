/*
 * bootloader_command_app.c
 */
#include "bootloader_command_app.h"


extern uint8_t supported_commands[];
extern CRC_HandleTypeDef hcrc;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart2;
extern I2C_HandleTypeDef hi2c1;
extern DMA_HandleTypeDef hdma_usart3_rx;
void printMessage(char *format, ...);

/*LORA için paket başlangıcı*/
const uint8_t RESPONSE_HEADER[] = {
    TARGET_LORA_HIGH,
    TARGET_LORA_LOW,
    TARGET_LORA_CHANNEL
};

const uint32_t go_to_address = 0;
static uint8_t EXT_Flash_Buf[256]={'\0'};
static uint32_t EXT_Flash_Write_Start_Addr=0x00000000 ;


void send_ack_response(uint8_t payload_length, const uint8_t *payload, uint32_t payload_size) {
    uint8_t ack_buffer[5 + payload_size];
    memcpy(ack_buffer, RESPONSE_HEADER, sizeof(RESPONSE_HEADER));
    ack_buffer[3] = BL_ACK_VALUE;
    ack_buffer[4] = payload_length;
    if (payload_size > 0) {
        memcpy(&ack_buffer[5], payload, payload_size);
    }
    HAL_UART_Transmit(&huart3, ack_buffer, sizeof(RESPONSE_HEADER) + 2 + payload_size, HAL_MAX_DELAY);
}


uint32_t assemble_crc_from_packet(const uint8_t *packet, uint32_t packet_len) {
    // CRC located in last 4 bytes of packet, LSB first
    return ( (uint32_t)packet[packet_len - 4]        ) |
           (((uint32_t)packet[packet_len - 3]) << 8 ) |
           (((uint32_t)packet[packet_len - 2]) << 16) |
           (((uint32_t)packet[packet_len - 1]) << 24);
}

uint32_t assemble_crc_from_fixed_position(const uint8_t *packet) {
    // CRC located at packet[2]..packet[5], LSB first
    return ( (uint32_t)packet[2]        ) |
           (((uint32_t)packet[3]) << 8 ) |
           (((uint32_t)packet[4]) << 16) |
           (((uint32_t)packet[5]) << 24);
}

void bootloader_get_ver_cmd(uint8_t *bl_rx_data) {
	printMessage("Bootloaer_Get_Ver_Cmd");

	uint32_t command_packet_length = bl_rx_data[0] + 1;
	uint32_t host_crc = assemble_crc_from_fixed_position(bl_rx_data);

	if (!bootloader_verify_crc(&bl_rx_data[0], command_packet_length - 4, host_crc)) {
        uint8_t bl_Version = bootloader_get_version();

        send_ack_response(1, &bl_Version, sizeof(bl_Version));
		printMessage(" BL_VER : %d %#x  ", bl_Version, bl_Version);

	} else {
		printMessage("Checksum fail ");
		bootloader_send_nack();
	}
}

void bootloader_get_help_cmd(uint8_t *bl_rx_data) {
	printMessage("bootloader_get_help_cmd");

	uint32_t command_packet_len = bl_rx_data[0] + 1;
	uint32_t host_crc = assemble_crc_from_fixed_position(bl_rx_data);

	if (!bootloader_verify_crc(&bl_rx_data[0], command_packet_len - 4, host_crc)) {
		send_ack_response(SUPPORTED_COMMANDS_COUNT, supported_commands, SUPPORTED_COMMANDS_COUNT);

	} else {
		printMessage("Checksum fail");
		bootloader_send_nack();
	}
}

void bootloader_get_cid_cmd(uint8_t *bl_rx_data) {

	uint32_t command_packet_len = bl_rx_data[0] + 1;
	uint32_t host_crc = assemble_crc_from_fixed_position(bl_rx_data);

	if (!bootloader_verify_crc(&bl_rx_data[0], command_packet_len - 4, host_crc)) {
		uint16_t val = get_mcu_chip_id();
        uint8_t cID[2] = {val & 0xFF, /*High 8bit*/(val >> 8) & 0xFF};

		printMessage("Chip Id: %#x ", cID[0], cID[1]);
		send_ack_response(2, cID, sizeof(cID));
	} else {
		printMessage("Checksum fail ");
		bootloader_send_nack();
	}
}

void bootloader_go_to_addr_cmd(uint8_t *bl_rx_data) {
	printMessage("bootlodaer_go_to_addr_cmd ");

	uint32_t command_packet_len = bl_rx_data[0] + 1;
	uint32_t host_crc = assemble_crc_from_packet(bl_rx_data, command_packet_len);

	if (!bootloader_verify_crc(&bl_rx_data[0], command_packet_len - 4, host_crc)) {

		uint32_t go_to_address =
			  ((uint32_t)bl_rx_data[2])
			| ((uint32_t)bl_rx_data[3] << 8)
			| ((uint32_t)bl_rx_data[4] << 16)
			| ((uint32_t)bl_rx_data[5] << 24);

		if (bootloader_verify_address(go_to_address) == ADDR_VALID) {
			send_ack_response(1, (uint8_t[]){ADDR_VALID}, 1);
			printMessage("Going to Address ");

			/*JUMP öncesi herşey temizleniyor*/
			//__set_MSP(mspValue);	// Bu fonksiyon F407 De calisiyordu ama L053 de çalışmıyor
			SysTick->CTRL = 0;
			SysTick->LOAD = 0;
			SysTick->VAL = 0;

			HAL_DeInit();
			HAL_RCC_DeInit();
			HAL_I2C_DeInit(&hi2c1);
			HAL_UART_MspDeInit(&huart3);
			HAL_DMA_DeInit(&hdma_usart3_rx);
			HAL_CRC_DeInit(&hcrc);

			SCB->VTOR = go_to_address;
			uint32_t jump_address = *((volatile uint32_t*) (go_to_address + 4));
			void (*jump_to_app)(void) = (void *)jump_address;
			jump_to_app();
		} else {
			printMessage("Go Address Invalid ");
			send_ack_response(1, (uint8_t[]){ADDR_INVALID}, 1);
		}
	} else {
		printMessage("Checksum fail ");
		bootloader_send_nack();
	}
}

void bootloader_flash_erase_cmd(uint8_t *bl_rx_data) {
	printMessage("bootloader_flash_erase_cmd ");

	uint32_t command_packet_len = bl_rx_data[0] + 1;
	uint32_t host_crc = assemble_crc_from_packet(bl_rx_data, command_packet_len);

	if (!bootloader_verify_crc(&bl_rx_data[0], command_packet_len - 4, host_crc)) {
		printMessage("Initial Sector: %d Number Of Sectors: %d ", bl_rx_data[2], bl_rx_data[3]);

		send_ack_response(1, (uint8_t[]){execute_flash_erase(bl_rx_data[2], bl_rx_data[3])}, 1);
	}
	else {
		printMessage(" Checksum fail ");
		bootloader_send_nack();
	}
}

void bootloader_mem_write_cmd(uint8_t *bl_rx_data) {
	printMessage(" bootloader_mem_write_cmd ");
	uint8_t payloadLength = bl_rx_data[6];

	uint32_t memAddress =
		  ((uint32_t)bl_rx_data[2])
		| ((uint32_t)bl_rx_data[3] << 8)
		| ((uint32_t)bl_rx_data[4] << 16)
		| ((uint32_t)bl_rx_data[5] << 24);

	uint32_t command_packet_len = bl_rx_data[0] + 1;
	uint32_t host_crc = assemble_crc_from_packet(bl_rx_data, command_packet_len);

	if (!bootloader_verify_crc(&bl_rx_data[0], command_packet_len - 4, host_crc)) {
		printMessage(" Memory Write Address: %#x ", memAddress);
		if (bootloader_verify_address(memAddress) == ADDR_VALID) {
			printMessage(" Valid Memory Write Address ");
			send_ack_response(1, (uint8_t[]){execute_memory_write(&bl_rx_data[7], memAddress, payloadLength)}, 1);

		} else {
			printMessage(" Invalid Memory Write Address ");
			send_ack_response(1, (uint8_t[]){ADDR_INVALID}, 1);
		}
	} else {
		printMessage(" Checksum fail ");
		bootloader_send_nack();
	}
}

void bootloader_enable_read_write_protect_cmd(uint8_t *bl_rx_data) {
	uint8_t status = 0;
	uint32_t host_crc = 0;
	printMessage(" bootloader_enable_read_write_protect_cmd ");

	uint32_t command_packet_len = bl_rx_data[0] + 1;

	host_crc = assemble_crc_from_fixed_position(bl_rx_data);

	if (!bootloader_verify_crc(&bl_rx_data[0], command_packet_len - 4, host_crc)) {
		status = configure_flash_sector_r_w_protection(bl_rx_data[2], bl_rx_data[3], 0);
		printMessage(" Status: %d", status);

		send_ack_response(1, (uint8_t[]){status}, 1);

	} else {
		printMessage(" Checksum fail ");
		bootloader_send_nack();
	}
}

void bootloader_go_to_bootloader_cmd(uint8_t *bl_rx_data)
{
	uint32_t command_packet_len = bl_rx_data[0] + 1;
	uint32_t host_crc = assemble_crc_from_packet(bl_rx_data, command_packet_len);

	if (!bootloader_verify_crc(&bl_rx_data[0], command_packet_len - 4, host_crc)) {
		printMessage(" Bootloader already running  ");
		send_ack_response(1, (uint8_t[]){BL_BOOTLOADER_ACTIVE}, 1);
		}
	else {
			printMessage("Checksum fail ");
			bootloader_send_nack();
		}
}

void bootloader_ext_mem_to_mem_write_cmd(uint8_t *bl_rx_data)
{
	printMessage("bootloader_ext_to_mem_cmd");
	uint32_t command_packet_length = bl_rx_data[0] + 1;
	uint32_t host_crc = assemble_crc_from_fixed_position(bl_rx_data);

	if (!bootloader_verify_crc(&bl_rx_data[0], command_packet_length - 4, host_crc)) {
		printMessage("MEM_WRITE_WORKING ");
		send_ack_response(1, (uint8_t[]){0x20}, 1);/* BL_EXT_MEM_TO_MEM_WRITE working*/

		EXT_Flash_Write_Start_Addr = 0x000000;
		while (EXT_Flash_Buf[0] != 0xFF) {
			memset(EXT_Flash_Buf, '\0', 256);
			W25Q_Read_Fast(EXT_Flash_Write_Start_Addr, EXT_Flash_Buf);
			bootloader_mem_write_cmd(EXT_Flash_Buf);

			// Yazılacak page öteleniyor
			EXT_Flash_Write_Start_Addr += 256;
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
	printMessage(" bootloader_mem_write_cmd ");

	uint8_t payloadLength = bl_rx_data[6];

	uint32_t memAddress =
		  ((uint32_t)bl_rx_data[2])
		| ((uint32_t)bl_rx_data[3] << 8)
		| ((uint32_t)bl_rx_data[4] << 16)
		| ((uint32_t)bl_rx_data[5] << 24);

	uint32_t command_packet_len = bl_rx_data[0] + 1;

	uint32_t host_crc = assemble_crc_from_fixed_position(bl_rx_data);
	if (!bootloader_verify_crc(&bl_rx_data[0], command_packet_len - 4,host_crc)) {
		printMessage(" Memory Write Address: %#x ", memAddress);
		memset(EXT_Flash_Buf, '\0', 256);
		if (bootloader_verify_address(memAddress) == ADDR_VALID) {
			W25Q_Write_Enable();
			send_ack_response(1, (uint8_t[]){execute_ext_mem_write(bl_rx_data, payloadLength)}, 1);

			W25Q_Write_Disable();

			SSD1306_GotoXY(0, 32);

		} else {
			printMessage(" Invalid Memory Write Address ");
			send_ack_response(1, (uint8_t[]){ADDR_INVALID}, 1);
		}
	} else {
		printMessage(" Checksum fail ");
		bootloader_send_nack();
	}
}
void bootloader_read_sector_protection_status_cmd(uint8_t *bl_rx_data) {
	printMessage(" bootloader_read_sector_protection_status_cmd ");

	uint32_t command_packet_len = bl_rx_data[0] + 1;
	uint32_t host_crc = assemble_crc_from_fixed_position(bl_rx_data);

	if (!bootloader_verify_crc(&bl_rx_data[0], command_packet_len - 4, host_crc)) {
		uint16_t val = read_OB_r_w_protection_status();
        uint8_t status[2] = {val & 0xFF, /*High 8bit*/(val >> 8) & 0xFF};
		printMessage(" nWRP status: %#", val);
		send_ack_response(2, status, sizeof(status));
	} else {
		printMessage(" Checksum fail ");
		bootloader_send_nack();
	}

}

void bootloader_disable_read_write_protect_cmd(uint8_t *bl_rx_data) {
	printMessage(" bootloader_disable_read_write_protect_cmd ");

	uint32_t command_packet_len = bl_rx_data[0] + 1;
	uint32_t host_crc = assemble_crc_from_fixed_position(bl_rx_data);

	if (!bootloader_verify_crc(&bl_rx_data[0], command_packet_len - 4, host_crc)) {
		uint8_t status = configure_flash_sector_r_w_protection(0, 0, 1);
		printMessage(" Status: %d", status);
		send_ack_response(1, (uint8_t[]){status}, 1);

	} else {
		printMessage(" Checksum fail ");
		bootloader_send_nack();
	}
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
	printMessage(" bootloader_get_rdp_cmd ");
	uint32_t command_packet_len = bl_rx_data[0] + 1;

	uint32_t host_crc = assemble_crc_from_fixed_position(bl_rx_data);

	if (!bootloader_verify_crc(&bl_rx_data[0], command_packet_len - 4, host_crc)) {
		printMessage("RDP Level: %d", get_flash_rdp_level);
		send_ack_response(1, (uint8_t[]){get_flash_rdp_level()}, 1);

	} else {
		printMessage(" Checksum fail ");
		bootloader_send_nack();
	}

}


void bootloader_send_nack() {

	uint8_t nackValue[5] = {
		TARGET_LORA_HIGH,
		TARGET_LORA_LOW,
		TARGET_LORA_CHANNEL,
		BL_NACK_VALUE
	};
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
	HAL_StatusTypeDef status = HAL_ERROR;

	 if (sectorNumber > (64 - 1) && sectorNumber != 0xFF) {
		 printMessage("Invalid sector number: %d", sectorNumber);
	     return INVALID_SECTOR;
	}
	HAL_FLASH_Unlock();
	if (sectorNumber == 0xFF) {
		FlashEraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
		FlashEraseInitStruct.Page = ((FLASH_END_ADDRESS-FLASH_APP_BASE_ADDRESS) / FLASH_BANK_SIZE);
		FlashEraseInitStruct.NbPages = FLASH_PAGE_NB;
		FlashEraseInitStruct.Banks = FLASH_BANK_1;

		status = (uint8_t) HAL_FLASHEx_Erase(&FlashEraseInitStruct, &SectorError);

	} else {
		FlashEraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
		FlashEraseInitStruct.Page = sectorNumber;
		FlashEraseInitStruct.NbPages = numberOfSector;
		FlashEraseInitStruct.Banks = FLASH_BANK_1;

		status = (uint8_t) HAL_FLASHEx_Erase(&FlashEraseInitStruct, &SectorError);
	}
	HAL_FLASH_Lock();

	return (uint8_t)status;
}

uint8_t execute_memory_write(uint8_t *Buffer, uint32_t memAddress, uint32_t len) {
	uint8_t status = HAL_ERROR;
	uint64_t data ;

	__HAL_FLASH_CLEAR_FLAG(FLASH_ECCR_ECCD);
	__HAL_FLASH_CLEAR_FLAG(FLASH_ECCR_ECCC);
	__HAL_FLASH_CLEAR_FLAG(FLASH_ECCR_ECCCIE );
	__HAL_FLASH_CLEAR_FLAG(FLASH_ECCR_SYSF_ECC);
	__HAL_FLASH_CLEAR_FLAG(FLASH_ECCR_ADDR_ECC );

	while (HAL_FLASH_Lock() != HAL_OK){};
	for (uint32_t i = 0; i < len; i= i+8) {
			//while ((FLASH->SR & FLASH_SR_BSY1)) {}
		while (HAL_FLASH_Unlock() != HAL_OK){};
	        data =
	        	   ((uint64_t)Buffer[i])
	             | (((uint64_t)Buffer[i+1]) << 8)
	             | (((uint64_t)Buffer[i+2]) << 16)
	             | (((uint64_t)Buffer[i+3]) << 24)
	             | (((uint64_t)Buffer[i+4]) << 32)
	             | (((uint64_t)Buffer[i+5]) << 40)
	             | (((uint64_t)Buffer[i+6]) << 48)
	             | (((uint64_t)Buffer[i+7]) << 56);

	        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, memAddress+i, data);
	        HAL_Delay(3);
	}
	HAL_FLASH_Lock();

	return status;
}
uint8_t execute_ext_mem_write(uint8_t *Buffer, uint32_t len)
{
	W25Q_Buf_Program(EXT_Flash_Write_Start_Addr, Buffer, len);
	EXT_Flash_Write_Start_Addr += 256;
	return HAL_OK;
}

uint8_t configure_flash_sector_r_w_protection(uint8_t sector_details, uint8_t protection_mode, uint8_t enableOrDisable) {
	volatile uint32_t *pOPTCR = (uint32_t*) 0x40023C14;


    while (HAL_FLASH_OB_Unlock() != HAL_OK) {};

	if (enableOrDisable) {
		*pOPTCR |= (0xFF << 16);
		*pOPTCR |= (1 << 1);

		return 0;
	}
	if (protection_mode == 1){	// write protection
		*pOPTCR &= ~(sector_details << 16);
		*pOPTCR |= (1 << 1);
	}
	else if (protection_mode == 2){ // read / write protection
		*pOPTCR &= ~(0xFF << 16);				// write protecton all sector
		*pOPTCR |= (sector_details << 16);
		*pOPTCR |= (0xFF << 8);					// read protection all sector
		*pOPTCR |= (1 << 1);
	}
    while (__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET){};
	HAL_FLASH_OB_Lock();

	return 0;
}
