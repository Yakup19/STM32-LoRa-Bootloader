/*
 * W25Qxx.h
 *
 *  Created on: Aug 7, 2023
 *      Author: yakup
 */

#ifndef INC_W25QXX_H_
#define INC_W25QXX_H_

#include "main.h"



extern SPI_HandleTypeDef 	hspi1;
#define W25Q_SPI			hspi1

#define Cs_Low				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET)
#define Cs_High				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET)

#define numBlock 64	//toplam blok sayisi
#define Dummy_Byte 0xA5


/**********COMMAND CODES********/
#define W25Q_WRITE_ENABLE 0x06U			///< sets WEL bit, must be set before any write/program/erase
#define W25Q_WRITE_DISABLE 0x04U		///< resets WEL bit (state after power-up)
#define W25Q_ENABLE_VOLATILE_SR 0x50U	///< check 7.1 in datasheet
#define W25Q_READ_SR1 0x05U				///< read status-register 1
#define W25Q_READ_SR2 0x35U				///< read status-register 2
#define W25Q_READ_SR3 0x15U				///< read ststus-register 3
#define W25Q_WRITE_SR1 0x01U			///< write status-register 1 (8.2.5)
#define W25Q_WRITE_SR2 0x31U			///< write status-register 2 (8.2.5)
#define W25Q_WRITE_SR3 0x11U			///< write status-register 3 (8.2.5)
#define W25Q_READ_EXT_ADDR_REG 0xC8U	///< read extended addr reg (only in 3-byte mode)
#define W25Q_WRITE_EXT_ADDR_REG 0xC8U	///< write extended addr reg (only in 3-byte mode)
#define W25Q_ENABLE_4B_MODE 0xB7U			///< enable 4-byte mode (128+ MB address)
#define W25Q_DISABLE_4B_MODE 0xE9U			///< disable 4-byte mode (<=128MB)
#define W25Q_READ_DATA 0x03U				///< read data by standard SPI
#define W25Q_READ_DATA_4B 0x13U				///< read data by standard SPI in 4-byte mode
#define W25Q_FAST_READ 0x0BU				///< highest FR speed (8.2.12)
#define W25Q_FAST_READ_4B 0x0CU				///< fast read in 4-byte mode
#define W25Q_FAST_READ_DUAL_OUT 0x3BU		///< fast read in dual-SPI OUTPUT (8.2.14)
#define W25Q_FAST_READ_DUAL_OUT_4B 0x3CU	///< fast read in dual-SPI OUTPUT in 4-byte mode
#define W25Q_FAST_READ_QUAD_OUT 0x6BU		///< fast read in quad-SPI OUTPUT (8.2.16)
#define W25Q_FAST_READ_QUAD_OUT_4B 0x6CU	///< fast read in quad-SPI OUTPUT in 4-byte mode
#define W25Q_FAST_READ_DUAL_IO 0xBBU		///< fast read in dual-SPI I/O (address transmits by both lines)
#define W25Q_FAST_READ_DUAL_IO_4B 0xBCU		///< fast read in dual-SPI I/O in 4-byte mode
#define W25Q_FAST_READ_QUAD_IO 0xEBU		///< fast read in quad-SPI I/O (address transmits by quad lines)
#define W25Q_FAST_READ_QUAD_IO_4B 0xECU		///< fast read in quad-SPI I/O in 4-byte mode
#define W25Q_SET_BURST_WRAP 0x77U			///< use with quad-I/O (8.2.22)
#define W25Q_PAGE_PROGRAM 0x02U				///< program page (256bytes) by single SPI line
#define W25Q_PAGE_PROGRAM_4B 0x12U			///< program page by single SPI in 4-byte mode
#define W25Q_PAGE_PROGRAM_QUAD_INP 0x32U	///< program page (256bytes) by quad SPI lines
#define W25Q_PAGE_PROGRAM_QUAD_INP_4B 0x34U ///< program page by quad SPI in 4-byte mode
#define W25Q_SECTOR_ERASE 0x20U				///< sets all 4Kbyte sector with 0xFF (erases it)
#define W25Q_SECTOR_ERASE_4B 0x21U			///< sets all 4Kbyte sector with 0xFF in 4-byte mode
#define W25Q_32KB_BLOCK_ERASE 0x52U			///< sets all 32Kbyte block with 0xFF
#define W25Q_64KB_BLOCK_ERASE 0xD8U			///< sets all 64Kbyte block with 0xFF
#define W25Q_64KB_BLOCK_ERASE_4B 0xDCU		///< sets all 64Kbyte sector with 0xFF in 4-byte mode
#define W25Q_CHIP_ERASE 0xC7U				///< fill all the chip with 0xFF
//#define W25Q_CHIP_ERASE 0x60U				///< another way to erase chip
#define W25Q_ERASEPROG_SUSPEND 0x75U		///< suspend erase/program operation (can be applied only when SUS=0, BYSY=1)
#define W25Q_ERASEPROG_RESUME 0x7AU			///< resume erase/program operation (if SUS=1, BUSY=0)
#define W25Q_POWERDOWN 0xB9U				///< powers down the chip (power-up by reading ID)
#define W25Q_POWERUP 0xABU					///< release power-down
#define W25Q_DEVID 0xABU					///< read Device ID (same as powerup)
#define W25Q_FULLID 0x90U					///< read Manufacturer ID & Device ID
#define W25Q_FULLID_DUAL_IO 0x92U			///< read Manufacturer ID & Device ID by dual I/O
#define W25Q_FULLID_QUAD_IO 0x94U			///< read Manufacturer ID & Device ID by quad I/O
#define W25Q_READ_UID 0x4BU					///< read unique chip 64-bit ID
#define W25Q_READ_JEDEC_ID 0x9FU			///< read JEDEC-standard ID
#define W25Q_READ_SFDP 0x5AU				///< read SFDP register parameters
#define W25Q_ERASE_SECURITY_REG 0x44U		///< erase security registers
#define W25Q_PROG_SECURITY_REG 0x42U		///< program security registers
#define W25Q_READ_SECURITY_REG 0x48U		///< read security registers
#define W25Q_IND_BLOCK_LOCK 0x36U			///< make block/sector read-only
#define W25Q_IND_BLOCK_UNLOCK 0x39U			///< disable block/sector protection
#define W25Q_READ_BLOCK_LOCK 0x3DU			///< check block/sector protection
#define W25Q_GLOBAL_LOCK 0x7EU				///< global read-only protection enable
#define W25Q_GLOBAL_UNLOCK 0x98U			///< global read-only protection disable
#define W25Q_ENABLE_RST 0x66U				///< enable software-reset ability
#define W25Q_RESET 0x99U					///< make software reset

typedef struct {
	uint64_t ID;
	uint8_t UniqID[8];
	uint16_t PageSize;
	uint32_t PageCount;
	uint32_t SectorSize;
	uint32_t SectorCount;
	uint32_t BlockSize;
	uint32_t BlockCount;
	uint32_t CapacityInKiloByte;
	uint8_t StatusRegister1;
	uint8_t StatusRegister2;
	uint8_t StatusRegister3;
	uint8_t Lock;

} W25Q_HandleTypeDef;



uint8_t W25qxx_Spi(uint8_t Data);
void W25qxx_WaitForWriteEnd(void);
void W25Q_Reset(void);	/*Calşmiyor olabilir !!!!!*/
void W25Q_Chip_Erase(void);
uint32_t W25Q_ReadID(void);
uint32_t W25Q_Read_Manufacturer_ID(void);
uint64_t W25Q_Read_UID(void);
void W25Q_Write_Enable(void);
void W25Q_Write_Disable(void);
void W25Q_Page_Program(uint32_t Adress,uint64_t *Data);
void W25Q_Buf_Program(uint32_t Adress,uint8_t *Buffer, uint8_t len);
uint8_t W25Q_Read_Word(uint32_t Adress, uint32_t *Buffer);
uint8_t W25Q_Read_DoubleWord(uint32_t Adress, uint64_t *Buffer);
uint8_t W25Q_Read_QWORD(uint64_t Adress, uint64_t *Buffer);
uint8_t W25Q_Read_Page(uint32_t Adress, uint8_t *Buffer);
uint8_t W25Q_Read_Fast(uint32_t Adress, uint8_t *Buffer);





#endif /*INC_W25QXX_H_*/
