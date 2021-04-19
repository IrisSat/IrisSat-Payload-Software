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
#include <csp/csp.h>
#include "csp/interfaces/csp_if_can.h"
#include "flash.h"
#include <math.h>

void handleCommand(telemetryPacket_t* command,csp_conn_t * connection);
void rx_image(uint8_t * chunk,uint16_t size,uint16_t num);

void commandHandler(void * pvparams){

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

		size_t freSpace = xPortGetFreeHeapSize();
		/* Start router task with 100 word stack, OS task priority 1 */
		resp = csp_route_start_task(400, 1);

	csp_conn_t * conn = NULL;
	csp_packet_t * packet= NULL;
	csp_socket_t * socket = csp_socket(0);
	csp_bind(socket, CSP_ANY);
	csp_listen(socket,4);

	while(1){
//		freSpace = xPortGetFreeHeapSize();
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
                    //xQueueSendToBack(queues->command_queue,packet->data,100);
                    break;
            }

            case CSP_TELEM_PORT:
                    //xQueueSendToBack(queues->data_queue,packet->data,100);
                    break;

                default:
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

		case PAYLOAD_POWER_GOOD_ID:{

			break;
		}

		case PAYLOAD_BOARD_TEMP_ID:{
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
				telemetry.length =  0; //no data. Culd add error code later.

				sendTelemetry(&telemetry);

			}

			break;
		}

		case PAYLOAD_SAMPLE_TEMP_ID:{

			break;
		}

		case PAYLOAD_FULL_IMAGE_ID:{

			break;
		}

		case PAYLOAD_R_REFLECTANCE_ID:{

			break;
		}

		case PAYLOAD_G_REFLECTANCE_ID:{

			break;
		}

		case PAYLOAD_B_REFLECTANCE_ID:{

			break;
		}

		case PAYLOAD_SAMPLE_LOC_ID:{

			break;
		}

		case PAYLOAD_CAMERA_TIME_ID:{

			break;
		}
		case PAYLOAD_FULL_IMAGE_RX:{


			uint16_t imgChunkNum = *((uint16_t*)&command->data[0]);
			uint16_t chunkLen = command->length-sizeof(uint16_t);

			rx_image(&command->data[2],chunkLen,imgChunkNum);


			//Send ACK.
			telemetryPacket_t telemetry={0};
			telemetry.telem_id= PAYLOAD_ACK;
			telemetry.timestamp = command->timestamp;
			*((uint16_t*)&telemetry.data[0]) =imgChunkNum;
			telemetry.length = 2;
			sendTelemetry_direct(&telemetry,connection);
			break;
		}
		default:

			break;


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
