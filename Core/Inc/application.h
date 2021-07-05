/*
 * application.h
 *
 *  Created on: Mar. 17, 2021
 *      Author: Joseph Howarth
 */

#ifndef INC_APPLICATION_H_
#define INC_APPLICATION_H_

#define CHUNKSIZE	80 //100 works with color bars and all pictures at 640x480, 133 is max with camera not capped but still observed errors,
						//when fifo was tuned using register wizard and tested again 133 was too much and got chunk errors again,
						//returned to 100 which seems to safely avoid midtransfer failures.

void commandHandler(void * pvparams);


#endif /* INC_APPLICATION_H_ */
