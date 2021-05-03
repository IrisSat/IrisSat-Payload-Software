/*
 * application.c
 *
 *  Created on: Mar. 17, 2021
 *      Author: Joseph Howarth
 */

#include "application.h"
#include "telemetry.h"
#include "adc.h"
#include "cmsis_os.h"
#include "queue.h"
#include <csp/csp.h>
#include "csp/interfaces/csp_if_can.h"
#include "flash.h"
#include <math.h>
#include "yaffsfs.h"
#include "filesystemUtilites.h"


void imageTransfer(void * pvParams);
void handleCommand(telemetryPacket_t* command,csp_conn_t * connection);
void rx_image(uint8_t * chunk,uint16_t size,uint16_t num);
void takeImage(uint8_t camNum,Calendar_t * time);
void powerCamera(uint8_t camNum, uint8_t onOrOff);
void resetCamera(uint8_t camNum);

QueueHandle_t imageSendQueue;


void commandHandler(void * pvparams){

		//Setup the CAN interface. Values don't actually matter here, change in the CAN init functions instead (can.c).
		struct csp_can_config can_conf;
		can_conf.bitrate=250000;
		can_conf.clock_speed=250000;
		can_conf.ifc = "CAN";

		/* Init buffer system with 5 packets of maximum 256 bytes each */
		int resp = csp_buffer_init(5, 256);//The 256 number is from the MTU of the CAN interface.

		/* Init CSP with address 0 */
		resp = csp_init(PAYLOAD_CSP_ADDRESS);

		/* Init the CAN interface with hardware filtering */
		resp = csp_can_init(CSP_CAN_MASKED, &can_conf);

		/* Setup default route to CAN interface */
		resp = csp_rtable_set(CSP_DEFAULT_ROUTE,0, &csp_if_can,CSP_NODE_MAC);

//		size_t freSpace = xPortGetFreeHeapSize();//Check how much space on the heap, to make sure were not running out of memory.

		/* Start router task with 100 word stack, OS task priority 1 */
		resp = csp_route_start_task(400, 1);

		csp_conn_t * conn = NULL;
		csp_packet_t * packet= NULL;
		csp_socket_t * socket = csp_socket(0);
		csp_bind(socket, CSP_ANY);
		csp_listen(socket,4);

		//Start the file system.
		startupFilesystem();


		//Make a queue for the image transfer commands.
		imageSendQueue = xQueueCreate(4,sizeof(uint8_t));

		//Create the task for sending images.
		//This is a task since sending images can take long, and we don't want payload unresponsive while sending image.
		//Transfer is started by passing the desired image through the imageSendQueue.
		xTaskCreate(imageTransfer, "Img Xfer", 512, (void *)imageSendQueue, 2, NULL);

	while(1){

		//Wait for a connection...
		conn = csp_accept(socket, 1000);
		if(conn){
			packet = csp_read(conn,0);

            //Handle the message based on the port it was sent to.
            int dest_port = csp_conn_dport(conn);

            switch(dest_port){

            case CSP_CMD_PORT:{

							telemetryPacket_t command;

							unpackTelemetry(packet->data, &command);

							handleCommand(&command,conn);

                    break;
            }

            case CSP_TELEM_PORT:

            		//Payload doesn't read in any telemetry, so ignore it if we somehow get sent something.
                    break;

                default:
                	//Handle the CSP services, like ping, list process, free mem, etc.
                    csp_service_handler(conn,packet);
                    break;
            }
            //Should buffer free be here? Example doesn't call this after csp_service handler.
            csp_buffer_free(packet);
			csp_close(conn);
		}


	}

}

void handleCommand(telemetryPacket_t* command,csp_conn_t * connection){

	switch(command->telem_id){

		case PAYLOAD_POWER_GOOD_CMD:{

			double powerGood = 0;

			ADC_ChannelConfTypeDef sConfig = {0};


		  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
		  */
		  sConfig.Channel = ADC_CHANNEL_15;
		  sConfig.Rank = ADC_REGULAR_RANK_1;
		  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
		  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
		  {
			return 0;
		  }

			//Now read the value.
			if (HAL_ADC_Start(&hadc1) != HAL_OK)
			{
				/* Start Conversation Error */
				// Error_Handler();
			}
			if (HAL_ADC_PollForConversion(&hadc1, 500) != HAL_OK)
			{
				/* End Of Conversion flag not set on time */
				// Error_Handler();

				return 0;
			}
			else
			{
				/* ADC conversion completed */

				uint32_t ADCValue = HAL_ADC_GetValue(&hadc1);
				//Turn the integer value into a float (12-bit ADC,3V reference)
				powerGood = ((double)ADCValue/4096*3 );

			}

			HAL_ADC_Stop(&hadc1);

//			if(result){
				//Send value to CDH.
				telemetryPacket_t telemetry;
				telemetry.telem_id= PAYLOAD_BOARD_TEMP_ID;
				telemetry.timestamp = command->timestamp;//Is this correct? Are we keeping time on payload? Or should we ping CDH to get the time?
				telemetry.length =  1* sizeof(double); //one double value.
				telemetry.data = (uint8_t*) &powerGood;

				sendTelemetry(&telemetry);
//			}
//			else{
//				//Failed to get thermistor reading. Send an error instead.
//				telemetryPacket_t telemetry;
//				telemetry.telem_id= PAYLOAD_ERROR_ID;
//				telemetry.timestamp = command->timestamp;//Is this correct? Are we keeping time on payload? Or should we ping CDH to get the time?
//				telemetry.length =  0; //no data. Could add error code later.
//
//				sendTelemetry(&telemetry);
//
//			}
//
//			break;

			break;
		}

		case PAYLOAD_BOARD_TEMP_CMD:{
			double boardTemp = 0;
			int result = getTemp(BOARD_THERMISTOR, &boardTemp);

			if(result){
				//Send value to CDH.
				telemetryPacket_t telemetry;
				telemetry.telem_id= PAYLOAD_BOARD_TEMP_ID;
				telemetry.timestamp = command->timestamp;//Is this correct? Are we keeping time on payload? Or should we ping CDH to get the time?
				telemetry.length =  1* sizeof(double); //one double value.
				telemetry.data = (uint8_t*) &boardTemp;

				sendTelemetry(&telemetry);
			}
			else{
				//Failed to get thermistor reading. Send an error instead.
				telemetryPacket_t telemetry;
				telemetry.telem_id= PAYLOAD_ERROR_ID;
				telemetry.timestamp = command->timestamp;//Is this correct? Are we keeping time on payload? Or should we ping CDH to get the time?
				telemetry.length =  0; //no data. Could add error code later.

				sendTelemetry(&telemetry);

			}

			break;
		}

		case PAYLOAD_SAMPLE_TEMP_CMD:{

			double sampleTemp = 0;
			int result = getTemp(SAMPLE_THERMISTOR, &sampleTemp);

			if(result){
				//Send value to CDH.
				telemetryPacket_t telemetry;
				telemetry.telem_id= PAYLOAD_SAMPLE_TEMP_ID;
				telemetry.timestamp = command->timestamp;//Is this correct? Are we keeping time on payload? Or should we ping CDH to get the time?
				telemetry.length =  1* sizeof(double); //one double value.
				telemetry.data = (uint8_t*) &sampleTemp;

				sendTelemetry(&telemetry);
			}
			else{
				//Failed to get thermistor reading. Send an error instead.
				telemetryPacket_t telemetry;
				telemetry.telem_id= PAYLOAD_ERROR_ID;
				telemetry.timestamp = command->timestamp;//Is this correct? Are we keeping time on payload? Or should we ping CDH to get the time?
				telemetry.length =  0; //no data. Could add error code later.

				sendTelemetry(&telemetry);

			}



			break;
		}

		case PAYLOAD_FULL_IMAGE_CMD:{


			uint8_t fileNum = *command->data;
			BaseType_t res = xQueueSend(imageSendQueue,&fileNum,100);

			break;
		}

		case PAYLOAD_TAKE_IMAGE_CAM1_CMD:{

			takeImage(1,&command->timestamp);

			break;
		}
		case PAYLOAD_TAKE_IMAGE_CAM2_CMD:{

			takeImage(2,&command->timestamp);

			break;
		}

		case PAYLOAD_CAM1_OFF_CMD:{

			powerCamera(1, 0);

			break;
		}
		case PAYLOAD_CAM2_OFF_CMD:{

			powerCamera(2, 0);

			break;
		}
		case PAYLOAD_CAM1_ON_CMD:{

			powerCamera(1, 1);

			break;
		}
		case PAYLOAD_CAM2_ON_CMD:{

			powerCamera(2, 1);

			break;
		}
		case PAYLOAD_CAM1_RESET_CMD:{

			resetCamera(1);

			break;
		}
		case PAYLOAD_CAM2_RESET_CMD:{

			resetCamera(2);

			break;
		}
		case PAYLOAD_FILE_LIST_CMD:{

//			yaffs_DIR * dir = yaffs_opendir("flash/");
//			struct yaffs_dirent* ent = yaffs_readdir(dir);
//			uint8_t nameLen = strlen(ent->d_name);
//			char names [225];
//			uint8_t namesIdx =0;
//
//
//			telemetryPacket_t telemetry={0};

			break;
		}
//		case PAYLOAD_FULL_IMAGE_RX_CMD:{
//
//
//			uint16_t imgChunkNum = *((uint16_t*)&command->data[0]);
//			uint16_t chunkLen = command->length-sizeof(uint16_t);
//
//			rx_image(&command->data[2],chunkLen,imgChunkNum);
//
//
//			//Send ACK.
//			telemetryPacket_t telemetry={0};
//			telemetry.telem_id= PAYLOAD_ACK;
//			telemetry.timestamp = command->timestamp;
//			*((uint16_t*)&telemetry.data[0]) =imgChunkNum;
//			telemetry.length = 2;
//			sendTelemetry_direct(&telemetry,connection);
//			break;
//		}
		default:

			break;


	}

}


void imageTransfer(void * pvParams){
	//make this a task.

	int file;
	uint32_t size = 0;
	uint32_t numChunks =0;
	uint8_t chunk[CHUNKSIZE+sizeof(uint32_t)];
	char name[15];
	uint8_t fileNum;

	QueueHandle_t requestQ = (QueueHandle_t) pvParams;
	Calendar_t time = {0};

	while(1){

		//Wait until we get a request to transfer image.
		xQueueReceive(requestQ, &fileNum, portMAX_DELAY);
		UBaseType_t full =  uxQueueMessagesWaiting( imageSendQueue);
		//Convert the image num to a file name.Limit is 2 digit filenum!
		snprintf(name,15,"flash/%02d.jpg",fileNum);

		file = yaffs_open(name,O_RDONLY,0);

		size = 0;
		numChunks =0;


		if(file<0){
			//send negative response.

			uint8_t error = 1; //File not found or can't be accessed. Add more detailed error handling later...
			telemetryPacket_t telemetry;
			telemetry.telem_id= PAYLOAD_ERROR_ID;
			telemetry.timestamp = time;
			telemetry.length =  sizeof(uint8_t);
			telemetry.data = &error;

			sendTelemetryAddr(&telemetry,GROUND_CSP_ADDRESS);
			continue; //Skip the file send since we've got invalid file.
		}
		else{

			size = yaffs_lseek(file,0,SEEK_END);
			yaffs_lseek(file,0,SEEK_SET);
			numChunks = (size%CHUNKSIZE==0) ? size/CHUNKSIZE: size/CHUNKSIZE+1;
			//Send info packet.
			uint32_t info[2] = {size,numChunks};
			telemetryPacket_t telemetry;
			telemetry.telem_id= PAYLOAD_ACK;
			telemetry.timestamp = time;
			telemetry.length =  sizeof(uint32_t)*2;
			telemetry.data = (uint8_t*)info;

			sendTelemetryAddr(&telemetry,GROUND_CSP_ADDRESS);
		}
		vTaskDelay(pdMS_TO_TICKS(500));
		for(int i=0; i< numChunks;i++){
			int readSize = yaffs_read(file,&chunk[sizeof(uint32_t)],CHUNKSIZE);
			*((uint32_t*)&chunk[0]) = i*CHUNKSIZE; //Send the position (in bytes) where this packet belongs.
			telemetryPacket_t telemetry;
			telemetry.telem_id= PAYLOAD_FULL_IMAGE_ID;
			telemetry.timestamp = time;
			telemetry.length =  sizeof(uint32_t)+readSize;
			telemetry.data = chunk;

			sendTelemetryAddr(&telemetry,GROUND_CSP_ADDRESS);
			vTaskDelay(10);
		}

		yaffs_close(file);

	}
}

uint8_t imgBuff[2048];
uint16_t imgBuffIdx =0;

void rx_image(uint8_t * chunk,uint16_t size,uint16_t num){

	static int imgNum = 0;
	static int state = 0;
	static uint16_t numChunks =0;
	static uint32_t flashAddr = 0;

	//For the first chunk save the chunk num since it is the length.
	if(state ==0){
		state = 1;
		numChunks = num;
	}

	if(state == 1){

		memcpy(&imgBuff[imgBuffIdx],chunk,size);
		imgBuffIdx += size;

		if(imgBuffIdx == 2048){

			writeFlash(imgBuff, flashAddr, 1);
			flashAddr ++;
			imgBuffIdx = 0;

		}
		else if(num == 0){

			writeFlash(imgBuff, flashAddr, 1);
			imgNum ++;
			flashAddr = imgNum * ceil((1200*1600*2)/2048);
			state = 0;
			imgBuffIdx = 0;
		}
	}


}


void takeImage(uint8_t camNum,Calendar_t * time){

	uint8_t imageNum;
	//Find which file we want to save to....
	int file = allocateImageFile(&imageNum);

	//Start image capture.
	int imageSize =0;

	//Setup the correct camera
	if(camNum == 1){

	}
	else if(camNum == 2){

	}

	//Capture the image.
	//stuff...

	//Once the image is taken, we should send a message to CDH to let it know it can request the image transfer.
	//We let CDH know some metadata as well: image number and the size.CDH should already know which camera, and the timestamp.
	telemetryPacket_t t;

	t.telem_id = PAYLOAD_IMAGE_INFO;
	t.length = sizeof(uint8_t) + sizeof(uint32_t);

	uint8_t imageinfo[sizeof(uint8_t) + sizeof(uint32_t)];
	imageinfo[0] = imageNum;
	*((uint32_t*)&imageinfo[1]) = imageSize;

	t.data = imageinfo;
	t.timestamp = *time;

	sendTelemetry(&t);

	//Now lets also keep a log on the payload filesystem, as a backup.

	int fd = yaffs_open(IMAGE_LOG_FILE_PATH,O_CREAT|O_RDWR,S_IREAD| S_IWRITE);

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

	}else{

		char logMsg[100];
		snprintf(logMsg,100,"%d.%s: CAM%d, %d bytes, %d:%d:%d %d-%d-%d\n",imageNum,IMAGE_FORMAT_TYPE,camNum,imageSize,time->hour,time->minute,time->second,time->year,time->month,time->day);

		yaffs_write(fd, logMsg, strlen(logMsg));
		yaffs_close(fd);
	}



}

void powerCamera(uint8_t camNum, uint8_t onOrOff){}
void resetCamera(uint8_t camNum){}
