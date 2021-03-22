/*
 * telemetry.c
 *
 *  Created on: Mar. 22, 2021
 *      Author: Joseph Howarth
 */

#include "telemetry.h"



void unpackTelemetry(uint8_t * data, telemetryPacket_t* output){

	memcpy(output,data,sizeof(telemetryPacket_t));

}
