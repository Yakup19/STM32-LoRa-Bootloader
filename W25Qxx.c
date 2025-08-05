/*
 * W25Qxx.c
 *
 *  Created on: Aug 7, 2023
 *      Author: yakup
 */
#include "main.h"
#include "W25Qxx.h"


W25Q_HandleTypeDef w25qxx;


uint8_t W25qxx_Spi(uint8_t Data){
	uint8_t ret;
	HAL_SPI_TransmitReceive(&W25Q_SPI, &Data, &ret, 1, 100);
	return ret;
}

void W25qxx_WaitForWriteEnd(void)
{
	uint32_t count=0;
	HAL_Delay(1);
	Cs_Low;
	W25qxx_Spi(W25Q_READ_SR1);
	do
	{
		w25qxx.StatusRegister1 = W25qxx_Spi(Dummy_Byte);
		HAL_Delay(1);
		count++;
	} while ((w25qxx.StatusRegister1 & 0x01) == 0x01);
	Cs_High;
}

void W25Q_Reset(void){
	uint8_t Tdata[2];
	Tdata[0]=0x66;
	Tdata[1]=0x99;
	Cs_Low;
	W25qxx_Spi(Tdata[0]);
	W25qxx_Spi(Tdata[1]);
	Cs_High;
	HAL_Delay(100);
}

uint32_t W25Q_ReadID(void){
	uint8_t Tdata= 0x9F;
	uint8_t Rdata[3];
	Cs_Low;
	W25qxx_Spi(Tdata);
	for(int i= 0;i<4; i++)
	Rdata[i] = W25qxx_Spi(Dummy_Byte);
	Cs_High;

	return ((Rdata[0]<<16)|(Rdata[1]<<8)|(Rdata[2]));
}

uint32_t W25Q_Read_Manufacturer_ID(void){
	uint8_t Tdata[]= {W25Q_FULLID, 0x00};
	uint8_t Rdata[2];
	Cs_Low;
	W25qxx_Spi(Tdata[0]);
	for(int i=0;i<3;i++)
		W25qxx_Spi(Dummy_Byte);
	W25qxx_Spi(Tdata[1]);
	for(int i=0;i<2;i++)
		Rdata[i] = W25qxx_Spi(Dummy_Byte);
	Cs_High;


	return ((Rdata[0]<<16)|(Rdata[1]<<8)|(Rdata[2]));
}
uint64_t W25Q_Read_UID(void){
	uint8_t Rdata[8];
	uint64_t UID=0;
	Cs_Low;
	W25qxx_Spi(W25Q_READ_UID);
	for(int i=0;i<5;i++)
		W25qxx_Spi(Dummy_Byte);
	for(int i=0;i<9;i++)
		Rdata[i] = W25qxx_Spi(Dummy_Byte);
	Cs_High;
	UID	|=(((uint64_t)Rdata[0]<<24) | ((uint64_t)Rdata[1]<<16) | ((uint64_t)Rdata[2]<<8) | ((uint64_t)Rdata[3]))<<32;
	UID |=((Rdata[4]<<24) | (Rdata[5]<<16) | (Rdata[6]<<8)  | (Rdata[7]));
	return	UID;
}
void W25Q_Write_Enable(void)
{
	Cs_Low;
	W25qxx_Spi(W25Q_WRITE_ENABLE);
	Cs_High;
}

void W25Q_Write_Disable(void)
{
	Cs_Low;
	W25qxx_Spi(W25Q_WRITE_DISABLE);
	Cs_High;
}
void W25Q_Page_Program(uint32_t Adress, uint64_t *Data_Buffer)//Program fonksiyonun sorun var !!!!!!!
{
	/*su kisim henüz denenmedi*/
	W25qxx_WaitForWriteEnd();
	W25Q_Write_Enable();

	uint8_t Datas[256]={'\0'};
	uint8_t adres[3]={'\0'};
	/*Gelen 24 bitlik Adres bilgisi 8 bit olarak parçalanıyor*/
	adres[0]  = (Adress >> 16) & 0xFF;
	adres[1]  = (Adress >> 8) & 0xFF;
	adres[2]  = (Adress ) & 0xFF;
	/*Veriler Kullanılan flaş entegresine 256Bytelık parçalar halinde yazılacak */
	for(int a=0; a<32;a++){/*32 Tane uint64_t veri, 1 bytelik verilere paraçlanıyor*/
	for (int i = 0; i < 8; i++) {
		Datas[(a*8)+i] = (*Data_Buffer >> (56-(i * 8))) & 0xFF;
		}
	Data_Buffer += 1;
	}
	Cs_Low;
	W25qxx_Spi(W25Q_PAGE_PROGRAM);
	for(int i=0;i<3;i++)
		W25qxx_Spi(adres[i]);
	HAL_SPI_Transmit(&W25Q_SPI, Datas, 256, HAL_MAX_DELAY);
	Cs_High;
	HAL_Delay(1);/*Pg. 66 Page Program Time Typ 0,7ms Max 3ms*/
	W25qxx_WaitForWriteEnd();
	W25Q_Write_Disable();

}

void W25Q_Buf_Program(uint32_t Adress, uint8_t *Buffer, uint8_t len)
{
	W25qxx_WaitForWriteEnd();
	W25Q_Write_Enable();

	uint8_t Datas[256] = { '\0' };
	uint8_t adres[3] = { '\0' };
	/*Gelen 24 bitlik Adres bilgisi 8 bit olarak parçalanıyor*/
	adres[0] = (Adress >> 16) & 0xFF;
	adres[1] = (Adress >> 8) & 0xFF;
	adres[2] = (Adress) & 0xFF;

	for(int i=0; i<256;i++)
	{
		Datas[i]= *Buffer;
		Buffer+=1;
	}

	Cs_Low;
	W25qxx_Spi(W25Q_PAGE_PROGRAM);
	for (int i = 0; i < 3; i++)
		W25qxx_Spi(adres[i]);
	HAL_SPI_Transmit(&W25Q_SPI, Datas, 256, HAL_MAX_DELAY);
	Cs_High;
	/*Pg. 66 Page Program Time Typ 0,7ms Max 3ms*/
	W25qxx_WaitForWriteEnd();
	W25Q_Write_Disable();
}



void W25Q_Write_DoubleWord(uint32_t Adress, uint64_t *Data)
{
	W25qxx_WaitForWriteEnd();
	W25Q_Write_Enable();
	uint8_t Datas[128];
	uint8_t adres[3]={'\0'};
	/*Gelen 24 bitlik Adres bilgisi 8 bit olarak parçalanıyor*/
	adres[0]  = (Adress >> 16) & 0xFF;
	adres[1]  = (Adress >> 8) & 0xFF;
	adres[2]  = (Adress ) & 0xFF;
	/*Veriler Kullanılan flaş entegresine 8 Bytelık parçalar halinde yazılacak */
	for (int i = 0; i < 8; i++) {
		Datas[i] = (*Data >> (56-(i * 8))) & 0xFF;
		}
	Data += 1;
	for (int i = 8; i < 16; i++) {
			Datas[i] = (*Data >> (56-(i * 8))) & 0xFF;
		}
	Cs_Low;
	W25qxx_Spi(W25Q_PAGE_PROGRAM);
	for(int i=0;i<3;i++)
		W25qxx_Spi(adres[i]);
	for(int i=0;i<128;i++)
		W25qxx_Spi(Datas[i]);
	Cs_High;
	W25qxx_WaitForWriteEnd();
	W25Q_Write_Disable();
}

uint8_t W25Q_Read_Word(uint32_t Adress, uint32_t *Buffer)
{
	HAL_StatusTypeDef status;
	uint8_t Adresses[3]={'\0'};
	Adresses[0] = (Adress >> 16) & 0xFF;
	Adresses[1] = (Adress >> 8) & 0xFF;
	Adresses[2] = Adress & 0xFF;
	uint8_t Datas[4]={'\0'};
	Cs_Low;
	W25qxx_Spi(W25Q_READ_DATA);
	for(int i=0;i<3;i++)
		W25qxx_Spi(Adresses[i]);
	status = HAL_SPI_Receive(&W25Q_SPI, Datas, 4, HAL_MAX_DELAY);
	Cs_High;
	return status;
}
uint8_t W25Q_Read_DoubleWord(uint32_t Adress, uint64_t *Buffer)
{
	int8_t Adresses[3]={'\0'};
	HAL_StatusTypeDef status = HAL_OK;
	Adresses[0] = (Adress >> 16) & 0xFF;
	Adresses[1] = (Adress >> 8) & 0xFF;
	Adresses[2] = Adress & 0xFF;
	uint8_t Datas[8]={'\0'};
	Cs_Low;
	W25qxx_Spi(W25Q_READ_DATA);
	for (int i = 0; i < 3; i++)
		W25qxx_Spi(Adresses[i]);
	//for(int i=0;i<=256;i++)
	status = HAL_SPI_Receive(&W25Q_SPI, Datas, 8, HAL_MAX_DELAY);
	Cs_High;
	*Buffer	|=(((uint64_t)Datas[0]<<24) | ((uint64_t)Datas[1]<<16) | ((uint64_t)Datas[2]<<8) | ((uint64_t)Datas[3]))<<32;
	*Buffer	|=((Datas[4]<<24) | (Datas[5]<<16) | (Datas[6]<<8)  | (Datas[7]));

	return status;
}
uint8_t W25Q_Read_Page(uint32_t Adress, uint8_t *Buffer)
{
	W25qxx_WaitForWriteEnd();
	int8_t Adresses[3]={'\0'};
	HAL_StatusTypeDef status = HAL_OK;
	Adresses[0] = (Adress >> 16) & 0xFF;
	Adresses[1] = (Adress >> 8) & 0xFF;
	Adresses[2] = Adress & 0xFF;
	Cs_Low;
	W25qxx_Spi(W25Q_READ_DATA);
	for (int i = 0; i < 3; i++)
		W25qxx_Spi(Adresses[i]);
	//for(int i=0;i<=256;i++)
	status = HAL_SPI_Receive(&W25Q_SPI, Buffer, 256, HAL_MAX_DELAY);
	Cs_High;

	return status;
}
uint8_t W25Q_Read_Fast(uint32_t Adress, uint8_t *Buffer)
{
	W25qxx_WaitForWriteEnd();

	int8_t Adresses[3] = { '\0' };
	HAL_StatusTypeDef status = HAL_OK;
	Adresses[0] = (Adress >> 16) & 0xFF;
	Adresses[1] = (Adress >> 8) & 0xFF;
	Adresses[2] = Adress & 0xFF;
	Cs_Low;
	W25qxx_Spi(W25Q_FAST_READ);
	for (int i = 0; i < 3; i++)
		W25qxx_Spi(Adresses[i]);
	W25qxx_Spi(Dummy_Byte);
	status = HAL_SPI_Receive(&W25Q_SPI, Buffer, 256, HAL_MAX_DELAY);
	Cs_High;

	return status;
}


void W25Q_Chip_Erase(void)
{
	W25Q_Write_Enable();
	uint8_t Tdata=W25Q_CHIP_ERASE;
	Cs_Low;
	W25qxx_Spi(Tdata);
	Cs_High;
	W25qxx_WaitForWriteEnd();


}
