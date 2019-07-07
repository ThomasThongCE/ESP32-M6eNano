/* UART Echo Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tm_reader.h"
#include "m6e.h"

void getTag();

void app_main()
{
	hwTaskInit();

	xTaskCreate(&getTag,"getTag",2048,NULL,10,NULL);
}

void getTag()
{
	char epcStr[128], dataStr[128];
	TMR_TagReadData *trd;

	while(1)
	{
		xEventGroupSetBits(eventGroup, GET_TAG);

		while(1)
		{
			xQueueReceive(getTagQueue, &trd, portMAX_DELAY);
			printf("data received\n");

			TMR_bytesToHex(trd->tag.epc, trd->tag.epcByteCount, epcStr);
			printf("\necpstr: %s, freq: %d, Rssi: %d\n", epcStr, trd->frequency, trd->rssi );

			// if (0 < trd->data.len)
			// {
			// 	TMR_bytesToHex(trd->data.list, trd->data.len, dataStr);
			// 	printf("  data(%d): %s\n", trd->data.len, dataStr);
			// }

			destroyTagdata(trd);
		}
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}