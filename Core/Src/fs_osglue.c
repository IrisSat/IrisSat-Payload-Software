/*
 * fs_osglue.c
 *
 *  Created on: Apr 19, 2021
 *      Author: Joseph Howarth
 */


#include "yaffs_osglue.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "yaffs_trace.h"

SemaphoreHandle_t fs_lock;

unsigned yaffs_trace_mask = YAFFS_TRACE_SCAN | YAFFS_TRACE_GC |YAFFS_TRACE_ERASE |YAFFS_TRACE_ERROR |YAFFS_TRACE_TRACING |YAFFS_TRACE_ALLOCATE |YAFFS_TRACE_BAD_BLOCKS |YAFFS_TRACE_VERIFY |0;


void yaffsfs_Lock(void){

	xSemaphoreTake(fs_lock,portMAX_DELAY);
}
void yaffsfs_Unlock(void){

	xSemaphoreGive(fs_lock);
}

u32 yaffsfs_CurrentTime(void){

	return xTaskGetTickCount();
}

void yaffsfs_SetError(int err){

	int a = err;
}

void *yaffsfs_malloc(size_t size){

	return pvPortMalloc(size);
}
void yaffsfs_free(void *ptr){

	vPortFree(ptr);
}

void yaffsfs_get_malloc_values(unsigned *current, unsigned *high_water){


}


int yaffsfs_CheckMemRegion(const void *addr, size_t size, int write_request){

	//Should actually do somethign here, but I dont understand exactly how this works...
	//Maybe checkout old filesystem branch from cdh software, since I tried integrating yaffs once before...
	return 0;
}

void yaffsfs_OSInitialisation(void){

	fs_lock = xSemaphoreCreateMutex();
}

void yaffs_bug_fn(const char *file_name, int line_no){


}
