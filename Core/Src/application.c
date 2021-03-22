/*
 * application.c
 *
 *  Created on: Mar. 17, 2021
 *      Author: Joseph Howarth
 */

#include "application.h"
#include "telemetry.h"
#include "cmsis_os.h"
#include <csp/csp.h>
#include "csp/interfaces/csp_if_can.h"

void handleCommand(telemetryPacket_t* command);

void commandHandler(void * pvparams){

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
