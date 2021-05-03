/*
 * filesystemUtilities.c
 *
 *  Created on: May 3, 2021
 *      Author: Joseph Howarth
 */

#include "filesystemUtilites.h"
#include "yaffsfs.h"
#include "telemetry.h"

static int imageFileNumber =0;

int yaffsfs_GetError(){

	return yaffsErrorCode ;
}

void shutdownFilesystem(){

	//We will try and unmount the file system 3 times, waiting 400,then 800, then 1200 etc. ms before the next retry.
	int retries = 3;
	int backoffDelay = 400;
	int complete = 0;

	for(int i=0; i<retries;i++){

		int res = yaffs_unmount(FILESYSTEM_ROOT);

		if(res<0){
			//There must be some files open. Wait some time, then try again.
			vTaskDelay(backoffDelay * (i+1));
		}
		else{
			//we successfully unmount file system.
			complete =1 ;
			break;
		}
	}

	if(complete != 1){
		//We still haven't unmounted filesystem. Force it this time.  Could result in data loss of any open files.
		yaffs_unmount2(FILESYSTEM_ROOT, 1);
	}
}

void startupFilesystem(){


	//Mount the file system.
	int result = yaffs_mount(FILESYSTEM_ROOT);

	if(result != 1){

		telemetryPacket_t t;
		Calendar_t now = {0}; //Set to zero, since payload does not keep track of time. CDH will timestamp on receipt.

		t.telem_id = PAYLOAD_ERROR_ID;
		t.timestamp = now;
		char errorMsg[30];
		snprintf(errorMsg,30,"Can't mount FS: %d\n",yaffsfs_GetError());

		t.length = strlen(errorMsg);
		t.data = (uint8_t*)errorMsg;

		sendTelemetry(&t);

	}

	//Check the file exists.
	result = yaffs_access(BOOTCOUNT_FILE_PATH, 0);
	if(result < 0){

		telemetryPacket_t t;
		Calendar_t now = {0}; //Set to zero, since payload does not keep track of time. CDH will timestamp on receipt.

		t.telem_id = PAYLOAD_ERROR_ID;
		t.timestamp = now;
		char errorMsg[50];
		snprintf(errorMsg,50,"error access 0 on %s: %d\n",BOOTCOUNT_FILE_PATH,yaffsfs_GetError());

		t.length = strlen(errorMsg);
		t.data = (uint8_t*)errorMsg;

		sendTelemetry(&t);
	}
//		result = yaffs_access("flash/testFile.txt", R_OK);
//		result = yaffs_access("flash/testFile.txt", W_OK);

		//Make sure the file has read/write permissions.
		result = yaffs_chmod(BOOTCOUNT_FILE_PATH, S_IREAD| S_IWRITE);
//		result = yaffs_access("flash/testFile.txt", R_OK);
//		result = yaffs_access("flash/testFile.txt", W_OK);

	//This file will keep track of how many times the system has started up/reset.
	int fd = yaffs_open(BOOTCOUNT_FILE_PATH,O_CREAT|O_RDWR,S_IREAD| S_IWRITE);

	if(fd>0){

		int runCount;
		char runCountStr[4];

		//Read in the current boot count, and increment and write back.
		result = yaffs_read(fd,runCountStr,3);
		runCount = atoi(runCountStr);
		snprintf(runCountStr,4,"%3d",runCount+1);
		yaffs_lseek(fd, 0, SEEK_SET);
		yaffs_write(fd, runCountStr,strlen(runCountStr));
		yaffs_close(fd);

		telemetryPacket_t t;
		Calendar_t now = {0}; //Set to zero, since payload does not keep track of time. CDH will timestamp on receipt.

		t.telem_id = PAYLOAD_META_ID;
		t.timestamp = now;
		char errorMsg[50];
		snprintf(errorMsg,50,"Payload has started up for the %dth time.\n",runCount);

		t.length = strlen(errorMsg);
		t.data = (uint8_t*)errorMsg;

		sendTelemetry(&t);

	}
	else{


		telemetryPacket_t t;
		Calendar_t now = {0}; //Set to zero, since payload does not keep track of time. CDH will timestamp on receipt.

		t.telem_id = PAYLOAD_ERROR_ID;
		t.timestamp = now;
		char errorMsg[50];
		snprintf(errorMsg,50,"can't open %s: %d\n",BOOTCOUNT_FILE_PATH,yaffsfs_GetError());

		t.length = strlen(errorMsg);
		t.data = (uint8_t*)errorMsg;

		sendTelemetry(&t);
	}

}



int allocateImageFile(uint8_t * img_num){


	//Check how much free space we have left.
	long remainingSpace = yaffs_freespace(FILESYSTEM_ROOT);

	if(remainingSpace<IMAGE_SIZE || imageFileNumber>99){

		imageFileNumber = 0; //Reset the file number, since we don't have space for new image. Overwirte the oldest image.
	}

	char filename[20];
	snprintf(filename,15,"%s/%d.%s",FILESYSTEM_ROOT,imageFileNumber,IMAGE_FORMAT_TYPE);

	int fd = yaffs_open(filename,O_CREAT|O_RDWR,S_IREAD| S_IWRITE);

	if(fd<0){
		telemetryPacket_t t;
		Calendar_t now = {0}; //Set to zero, since payload does not keep track of time. CDH will timestamp on receipt.

		t.telem_id = PAYLOAD_ERROR_ID;
		t.timestamp = now;
		char errorMsg[50];
		snprintf(errorMsg,50,"can't open %s: %d\n",BOOTCOUNT_FILE_PATH,yaffsfs_GetError());

		t.length = strlen(errorMsg);
		t.data = (uint8_t*)errorMsg;

		sendTelemetry(&t);
		return -1;
	}
	else{

		telemetryPacket_t t;
		Calendar_t now = {0}; //Set to zero, since payload does not keep track of time. CDH will timestamp on receipt.

		t.telem_id = PAYLOAD_META_ID;
		t.timestamp = now;
		char errorMsg[60];
		snprintf(errorMsg,60,"Payload has allocated image file %s.\n",filename);

		t.length = strlen(errorMsg);
		t.data = (uint8_t*)errorMsg;

		sendTelemetry(&t);

		imageFileNumber ++;

		return fd;
	}

}
