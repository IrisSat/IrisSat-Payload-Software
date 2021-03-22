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

void handleCommand(telemetryPacket_t* command);

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
		resp = csp_route_start_task(100, 1);

	csp_conn_t * conn = NULL;
	csp_packet_t * packet= NULL;
	csp_socket_t * socket = csp_socket(0);
	csp_bind(socket, CSP_ANY);
	csp_listen(socket,4);

	while(1){

		conn = csp_accept(socket, 1000);
		if(conn){
			packet = csp_read(conn,0);

			//This is a command
			if(packet->id.src==CDH_CSP_ADDRESS && packet->id.dport ==CSP_CMD_PORT){

					telemetryPacket_t command;
					unpackTelemetry(packet->data, &command);

					handleCommand(&command);
			}

			csp_buffer_free(packet);
			csp_close(conn);
		}


	}

}

void handleCommand(telemetryPacket_t* command){

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

		default:

			break;


	}

}
