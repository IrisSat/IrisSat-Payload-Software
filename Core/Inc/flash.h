/*
 * flash.h
 *
 *  Created on: Apr 5, 2021
 *      Author: Joseph Howarth
 */

#ifndef INC_FLASH_H_
#define INC_FLASH_H_

#include <stdint.h>

/*initFlash(): This initializes the flash memory.
 *
 * 	The function disables the write protect, and checks the device ID.
 * 	Should be called after MX_FMC_Init();
 *
 * 	returns 1 on success, -1 if ID doesn't match or cant be read.
 */
int initFlash();

/* writeFlash: This writes pages of data to the flash memory.
 *
 * 	data should be a pointer to the data to write. The data buffer must be a multiple of 2048!
 * 	pageAddr is the starting address to write data to.
 * 	len_pages is the number of pages of data to write.
 *
 *  returns 1 on success, -1 if the pageAddr is invalid, and -2 if there is not enough room at the specified address for len_pages of data.
 */
int8_t writeFlash(uint8_t * data, uint32_t pageAddr, uint32_t len_pages);

int8_t writeSpare(uint8_t * data,uint32_t pageAddr);

int8_t readSpare(uint8_t * data,uint32_t pageAddr);

/* readFlash: This reads pages of data from the flash memory.
 *
 * 	data should be a pointer to where the data will be read to. The data buffer must be a multiple of 2048!
 * 	pageAddr is the starting address to read from.
 * 	len_pages is the number of pages of data to read.
 *
 *  returns 1 on success, -1 if the pageAddr is invalid, and -2 if there is not enough pages to read len_pages of data.
 */
int8_t readFlash(uint8_t* data, uint32_t pageAddr, uint32_t len_pages);


/*eraseFlashDevice(): This erases the whole flash memory.
 *
 * 	The function will erase all blocks of the memory.
 */
void eraseFlashDevice();

/* eraseFlashPages: This erases pages of data from the flash memory.
 *
 * 	startPage is the starting address to erase.
 * 	numBlocks is the number of blocks to erase. With W29N02GV that is 64 pages per block
 *
 *  returns 1 on success, -1 if the startBlock is invalid, and -2 if startBlock+numBlocks is invalid.
 */
int8_t eraseFlashPages(uint32_t startBlock, uint32_t numBlocks);



#endif /* INC_FLASH_H_ */
