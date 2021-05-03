/*
 * flash.c
 *
 *  Created on: Apr 5, 2021
 *      Author: Joseph Howarth
 */

#include "flash.h"
#include "gpio.h"
#include "fmc.h"
#include <string.h>



//Returns 1 if pageAddr is valid and there is len pages before the end of memory.
//Returns -1 if pageAddr is invalid. Returns -2 if pageAddr +len is invalid (not enough space).
int8_t validateAddress(NAND_HandleTypeDef *hnand,uint32_t pageAddr,uint32_t len){

	if(pageAddr >= ((hnand->Config.BlockNbr) * (hnand->Config.BlockSize))){
		return -1;
	}
	else if((pageAddr+len) >= (hnand->Config.BlockNbr * hnand->Config.BlockSize)){
		return -2;
	}
	else{
		return 1;
	}

}

int initFlash(){

	//Write Protect Pin should be high.
	HAL_GPIO_WritePin(FLASH_WP_GPIO_Port, FLASH_WP_Pin, 1);

	//Reset the memory.
	HAL_NAND_Reset(&hnand1);

	NAND_IDTypeDef nandInfo;

	//For reference, the W29N02GV id:
	//	nandInfo.Device_Id = 0xDA;
	//	nandInfo.Maker_Id = 0xEF;
	//	nandInfo.Third_Id=90;
	//	nandInfo.Fourth_Id =95;


	HAL_StatusTypeDef res =  HAL_NAND_Read_ID(&hnand1,&nandInfo);
	if(nandInfo.Device_Id != 0xDA || nandInfo.Maker_Id != 0xEF || nandInfo.Third_Id != 0x90 || nandInfo.Fourth_Id != 0x95){
		return -1;	//ID doesn't match.
	}

	return 1;
}

//Write len_pages pages of data from the data pointer to flash, starting at page pageAddr.
int8_t writeFlash(uint8_t * data, uint32_t pageAddr, uint32_t len_pages){


	//Validate write request
	int8_t result = validateAddress(&hnand1, pageAddr, len_pages);
	if(result != 1){
		return result;
	}

	//Create the address struct needed for HAL functions.
	NAND_AddressTypeDef addr;
	addr.Page = pageAddr % hnand1.Config.BlockSize;
	addr.Block = pageAddr/hnand1.Config.BlockSize;
	addr.Plane = 0;
	uint32_t eccVal=0;
	for(int i=0; i<len_pages; i++){
		//Write one page at a time? This might minimize effects of disabling interrupts.
		__disable_irq();
		HAL_NAND_ECC_Enable(&hnand1);
		HAL_NAND_Write_Page_8b(&hnand1, &addr,&(data[i*2048]), 1);
		HAL_NAND_GetECC(&hnand1, &eccVal, 50);
		HAL_NAND_ECC_Disable(&hnand1);
		__enable_irq();


		HAL_NAND_Address_Inc(&hnand1, &addr);

	}

	return 1;
}

int8_t writeSpare(uint8_t * data,uint32_t pageAddr){

	NAND_AddressTypeDef addr;
	addr.Page = pageAddr % hnand1.Config.BlockSize;
	addr.Block = pageAddr/hnand1.Config.BlockSize;
	addr.Plane = 0;
	__disable_irq();
	HAL_NAND_Write_SpareArea_8b(&hnand1, &addr, data, 1);
	__enable_irq();
	return 1;
}

int8_t readSpare(uint8_t * data,uint32_t pageAddr){
//	uint8_t spareAreaBuf[64]={0};
	NAND_AddressTypeDef addr;
	addr.Page = pageAddr % hnand1.Config.BlockSize;
	addr.Block = pageAddr/hnand1.Config.BlockSize;
	addr.Plane = 0;
	__disable_irq();
	HAL_NAND_Read_SpareArea_8b(&hnand1, &addr, data, 1);
	__enable_irq();
//	memcpy(data,spareAreaBuf,64);
//	memset(spareAreaBuf,0,64);
	return 1;
}

int8_t readFlash(uint8_t* data, uint32_t pageAddr, uint32_t len_pages){

	//Validate read request
	int8_t result = validateAddress(&hnand1, pageAddr, len_pages);
	if(result != 1){
		return result;
	}

	NAND_AddressTypeDef addr;
	addr.Page = pageAddr % hnand1.Config.BlockSize;
	addr.Block = pageAddr/hnand1.Config.BlockSize;
	addr.Plane = 0;

	for(int i=0; i<len_pages; i++){
		//Read one page at a time? This might minimize effects of disabling interrupts.
		__disable_irq();
		HAL_NAND_Read_Page_8b(&hnand1,  &addr,&data[i*2048], 1);
		__enable_irq();
		HAL_NAND_Address_Inc(&hnand1, &addr);
	}

	return 1;

}

void eraseFlashDevice(){

	NAND_AddressTypeDef addr;
	addr.Page=0;
	addr.Plane=0;
	addr.Block = 0;
	for(int i=0; i< hnand1.Config.BlockNbr;i++){
		__disable_irq();
		HAL_NAND_Erase_Block(&hnand1, &addr);
		__enable_irq();
		addr.Block = addr.Block+1;
		HAL_Delay(20);
	}

}

int8_t eraseFlashPages(uint32_t startBlock, uint32_t numBlocks){

	NAND_AddressTypeDef addr;
	addr.Block = startBlock;
	addr.Plane =0;
	addr.Page = 0;

	if(startBlock >= hnand1.Config.BlockNbr){
		return -1;
	}
	else if((startBlock+numBlocks)>=hnand1.Config.BlockNbr){
		return -2;
	}

	for(int i=0; i< numBlocks;i++){
		__disable_irq();
		HAL_NAND_Erase_Block(&hnand1, &addr);
		__enable_irq();
		addr.Block = addr.Block+1;

	}
	return 1;
}

