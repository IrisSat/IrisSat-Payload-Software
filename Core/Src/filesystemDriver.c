
#include "filesystemDriver.h"
#include "flash.h"
#include "fmc.h"
#include "FreeRTOS.h"

#include "yaffsfs.h"


int fs_writeChunk(struct yaffs_dev *dev, int nand_chunk, const u8 *data, int data_len,const u8 *oob, int oob_len){

	uint8_t dataBuff[2048]={0};
	uint8_t* dataPointer = dataBuff;
	if(data_len != 2048){
		memcpy(dataBuff,data,data_len);
	}
	else{
		dataPointer = data;
	}
	//Add some sort of logging or more descriptive return code if possible.
	if(nand_chunk > (hnand1.Config.BlockNbr) * (hnand1.Config.BlockSize)){
		//Page address out of bounds.
		return YAFFS_FAIL;
	}
	else if(data_len > hnand1.Config.PageSize){
		//Too much data for one page
		return YAFFS_FAIL;
	}
	else if(oob_len>59){
		//Too much spare data. We have 64-4(ecc) bytes of space...
		return YAFFS_FAIL;

	}

	//Create the address struct needed for HAL functions.
	NAND_AddressTypeDef addr;
	addr.Page = nand_chunk % hnand1.Config.BlockSize;
	addr.Block = nand_chunk/hnand1.Config.BlockSize;
	addr.Plane = 0;

	uint32_t eccVal=0;

	//Write one page at a time? This might minimize effects of disabling interrupts.
	__disable_irq();
	HAL_NAND_ECC_Enable(&hnand1);
	HAL_NAND_Write_Page_8b(&hnand1, &addr,dataPointer, 1);
	HAL_NAND_GetECC(&hnand1, &eccVal, 50);
	HAL_NAND_ECC_Disable(&hnand1);
	__enable_irq();

	uint8_t spareBuff[64];
	uint32_t* eccLoc = (uint32_t*)(&spareBuff[1]);

	*eccLoc = eccVal & 0xFFFFFFF;//ECC is 28 bits for 2048 page size.

	memcpy(&spareBuff[5],oob,oob_len);
	__disable_irq();
	HAL_NAND_Write_SpareArea_8b(&hnand1, &addr, spareBuff, 1);
	__enable_irq();


	return YAFFS_OK;
}

int fs_readChunk (struct yaffs_dev *dev, int nand_chunk, u8 *data, int data_len,u8 *oob, int oob_len,enum yaffs_ecc_result *ecc_result){

	uint8_t dataBuff[2048]={0};
	uint8_t* dataPointer = dataBuff;
	if(data_len == 2048){
		dataPointer = data;
	}
	//Add some sort of logging or more descriptive return code if possible.
	if(nand_chunk > (hnand1.Config.BlockNbr) * (hnand1.Config.BlockSize)){
		//Page address out of bounds.
		return YAFFS_FAIL;
	}
	else if(data_len > hnand1.Config.PageSize){
		//Too much data for one page
		return YAFFS_FAIL;
	}
	else if(oob_len>59){
		//Too much spare data. We have 64-4(ecc) bytes of space...
		return YAFFS_FAIL;

	}

	//Create the address struct needed for HAL functions.
	NAND_AddressTypeDef addr;
	addr.Page = nand_chunk % hnand1.Config.BlockSize;
	addr.Block = nand_chunk/hnand1.Config.BlockSize;
	addr.Plane = 0;

	uint32_t eccVal=0;


	__disable_irq();
	HAL_NAND_ECC_Enable(&hnand1);
	HAL_NAND_Read_Page_8b(&hnand1, &addr,dataPointer, 1);
	HAL_NAND_GetECC(&hnand1, &eccVal, 50);
	HAL_NAND_ECC_Disable(&hnand1);
	__enable_irq();

	uint8_t spareBuff[64];

	__disable_irq();
	HAL_NAND_Read_SpareArea_8b(&hnand1, &addr, spareBuff, 1);
	__enable_irq();

	memcpy(oob,&spareBuff[5],oob_len);

	uint32_t eccRead = *((uint32_t*)&spareBuff[1]);
	eccRead = eccRead & 0xFFFFFFF;
	int empty = 1;
	for(int i=0; i<2048;i++){
		if(data[i] != 0xFF){
			empty =0;
		}
	}

	if((eccVal&0xFFFFFFF)	 == eccRead || (empty && (eccRead==0xFFFFFFF))){
		*ecc_result = YAFFS_ECC_RESULT_NO_ERROR;
	}
	else{
		*ecc_result = YAFFS_ECC_RESULT_UNFIXED;
	}
	//Add error correction here...

	return YAFFS_OK;
}

int fs_erase(struct yaffs_dev *dev, int block_no){

	//Erase the block.
	eraseFlashPages(block_no, 1);
//	uint8_t dump[2048];
	//Update ECC values for these pages
//	for(int i=0; i< hnand1.Config.BlockSize;i++){
//		uint32_t eccVal=0;
//		NAND_AddressTypeDef addr;
//		addr.Page=i;
//		addr.Block = block_no;
//		addr.Plane = 0;
//		__disable_irq();
//		HAL_NAND_ECC_Enable(&hnand1);
//		HAL_NAND_Read_Page_8b(&hnand1, &addr,dump, 1);
//		HAL_NAND_GetECC(&hnand1, &eccVal, 50);
//		HAL_NAND_ECC_Disable(&hnand1);
//		__enable_irq();
//
//
//		uint8_t spareBuff[64];
//
//		__disable_irq();
//		HAL_NAND_Read_SpareArea_8b(&hnand1, &addr, spareBuff, 1);
//		__enable_irq();
//
//		*((uint32_t*)&spareBuff[1]) = eccVal;
//		__disable_irq();
//		HAL_NAND_Write_SpareArea_8b(&hnand1, &addr, spareBuff, 1);
//		__enable_irq();
//		HAL_NAND_Address_Inc(&hnand1, &addr);
//	}

	return YAFFS_OK;
}

int fs_markBad(struct yaffs_dev *dev, int block_no){

	uint8_t spare[64];
	memset(spare,5,64);//Just set everything to 5 since we have a bad block. The first byte not being FF(or 0) indicates bad block.

	writeSpare(spare,block_no*hnand1.Config.BlockSize);

	return YAFFS_OK;
}

int fs_checkBad(struct yaffs_dev *dev, int block_no){

	uint8_t spare[64] ={0};

	readSpare(spare, block_no*hnand1.Config.BlockSize);
	if(spare[0] == 0xFF || spare[0] == 0x00){

		return YAFFS_OK;
	}
	else{
		return YAFFS_FAIL;
	}


}

int fs_driverInit(struct yaffs_dev *dev){

	int results = initFlash();
	if(results == 1){
		return YAFFS_OK;
	}
	else return YAFFS_FAIL;
}

int fs_driverDeInit(struct yaffs_dev *dev){

	return YAFFS_OK;
}

int fs_nand_install_drv(struct yaffs_dev *dev){

	static int started =0;

	if(started>0){
		return YAFFS_FAIL;
	}
	else{
		dev = pvPortMalloc(sizeof(struct yaffs_dev));

		//Assign our driver functions.
		dev->drv.drv_write_chunk_fn= fs_writeChunk;
		dev->drv.drv_read_chunk_fn = fs_readChunk;
		dev->drv.drv_erase_fn = fs_erase;
		dev->drv.drv_mark_bad_fn = fs_markBad;
		dev->drv.drv_check_bad_fn=fs_checkBad;
		dev->drv.drv_initialise_fn=fs_driverInit;
		dev->drv.drv_deinitialise_fn=fs_driverDeInit;

		dev->param.name = "flash";
		dev->param.total_bytes_per_chunk = 2048;
		dev->param.chunks_per_block = 64;
		dev->param.spare_bytes_per_chunk = 59;
		dev->param.start_block =0;
		dev->param.end_block = 2047;
		dev->param.n_caches=5;
		dev->param.is_yaffs2 = 1;
		dev->param.n_reserved_blocks=5;

		dev->driver_context = (void*)1;
		yaffs_add_device(dev);
		started  = 1;

		int res = initFlash();
		if(res<0){
			return YAFFS_FAIL;
		}

		return YAFFS_OK;
	}
}
