/*
 * filesystemDriver.h
 *
 *  Created on: Apr. 19, 2021
 *      Author: Joseph Howarth
 */

#ifndef INC_FILESYSTEMDRIVER_H_
#define INC_FILESYSTEMDRIVER_H_

#include "yaffs_yaffs2.h"


int fs_writeChunk(struct yaffs_dev *dev, int nand_chunk,const u8 *data, int data_len,const u8 *oob, int oob_len);

int fs_readChunk (struct yaffs_dev *dev, int nand_chunk, u8 *data, int data_len,u8 *oob, int oob_len,enum yaffs_ecc_result *ecc_result);

int fs_erase(struct yaffs_dev *dev, int block_no);

int fs_markBad(struct yaffs_dev *dev, int block_no);

int fs_checkBad(struct yaffs_dev *dev, int block_no);

int fs_driverInit(struct yaffs_dev *dev);

int fs_driverDeInit(struct yaffs_dev *dev);

int fs_nand_install_drv(struct yaffs_dev *dev);

#endif /* INC_FILESYSTEMDRIVER_H_ */
