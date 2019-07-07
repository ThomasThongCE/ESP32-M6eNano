#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "tm_reader.h"

#define MAXPOWER 2700

void hwTaskInit();
void destroyTagdata(TMR_TagReadData *tagData);

QueueHandle_t getTagQueue, writeTagQueue, getSizeTagQueue;
EventGroupHandle_t eventGroup ;

enum eventBit 
{
    GET_TAG = 1,
    WRITE_TAG, 
    STOP_GET_TAG, 
    POWER,
};
