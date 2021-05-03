/*
 * filesystemUtilites.h
 *
 *  Created on: May 3, 2021
 *      Author: Joseph Howarth
 */

#ifndef INC_FILESYSTEMUTILITES_H_
#define INC_FILESYSTEMUTILITES_H_

#include <stdint.h>

//Some important files:
#define FILESYSTEM_ROOT		"flash"
#define BOOTCOUNT_FILE_PATH	"flash/bootcount.txt"
#define RESET_FILE_PATH		"flash/lastReset.txt"
#define IMAGE_LOG_FILE_PATH	"flash/imageLog.txt"


#define IMAGE_FORMAT_TYPE	"jpg"

//Space to allocate for an image in bytes. 1Mb currently, jpeg should be smaller, RAW will be bigger? Change as needed.
#define IMAGE_SIZE			1000000


extern int yaffsErrorCode;

//Returns the latest error code from the filesystem.
int yaffsfs_GetError();

//This does the following steps, in order to start and check the file system:
//		Mount the filesystem at "flash/"
//		Read and update the boot count.
//
//This should be called after  yaffsfs_OSInitialisation(),  fs_nand_install_drv(fileSystemDevice), and flash initialization, and after CSP setup.
//Any errors will be sent to cdh as telemetry.
void startupFilesystem();

//This will close the file system properly so we don't lose data. Unexpected power loss shouldn't corrupt file system, but calling this function should prevent data loss.
void shutdownFilesystem();

//This function opens a file to save an image to.
//Since images are stored in numbered files, this will open the file with the next number.
//If there is not enough space left in memory, the file number will wrap around, and overwrite the oldest image.
//This function opens a file, so the user code must close the file!!!
//
//Parameters: imageNum pointer. The image number will be copied to this memory location.
//Returns: The file handle, or -1 if the file can't be opened.
int allocateImageFile(uint8_t* imageNum);




#endif /* INC_FILESYSTEMUTILITES_H_ */
