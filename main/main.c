/* UART Echo Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tm_reader.h"
/**
 * This is an example which echos any data it receives on UART1 back to the sender,
 * with hardware flow control turned off. It does not use UART driver event queue.
 *
 * - Port: UART1
 * - Receive (Rx) buffer: on
 * - Transmit (Tx) buffer: off
 * - Flow control: off
 * - Event queue: off
 * - Pin assignment: see defines below
 */

#define ECHO_TEST_TXD  (GPIO_NUM_1)
#define ECHO_TEST_RXD  (GPIO_NUM_3)
#define ECHO_TEST_RTS  (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS  (UART_PIN_NO_CHANGE)

uint8_t myEPC[12]; //Most EPCs are 12 bytes
int myEPClength;
int responseType = 0;

void checkerr(TMR_Reader* rp, TMR_Status ret, const char *msg)
{
	if (TMR_SUCCESS != ret)
	{
		printf( "Error %s: %s\n", msg, TMR_strerr(rp, ret));
	}
	else
		printf("%s success\n", msg);
}

void app_main()
{
	TMR_Status ret;
	TMR_Reader reader, *preader;
	TMR_Region region;
	TMR_ReadPlan plan;
	int tagCount;

	preader = &reader;
	//reader = (TMR_Reader *) malloc(sizeof(TMR_Reader));
	ret = TMR_create(preader, "tmr:///");
	checkerr(preader, ret, "Creating reader");

	ret = TMR_connect(preader);
	checkerr(preader, ret, "Connecting reader");

	// set region support 868mhz
	region = TMR_REGION_EU3;
	//region = TMR_REGION_IN;
	ret = TMR_paramSet(preader, TMR_PARAM_REGION_ID, &region);
	checkerr(preader, ret, "Setting region");

	// Create simple plan using antenna 1
	ret = TMR_RP_init_simple(&plan, 1, 1, TMR_TAG_PROTOCOL_GEN2, 1000);
	checkerr(preader, ret, "Create plan");

	ret = TMR_read(&reader, 1000, &tagCount);
	checkerr(preader, ret, "Reading reader");

	while (TMR_SUCCESS == TMR_hasMoreTags(&reader))
	{
		TMR_TagReadData trd;
		char epcStr[128];

		ret = TMR_getNextTag(&reader, &trd);
		checkerr(preader, ret, "Next tag");

//		TMR_bytesToHex(trd.tag.epc, trd.tag.epcByteCount, epcStr);
//		printf("%s\n", epcStr);
//		if (0 < trd.data.len)
//		{
//		  char dataStr[255];
//		  TMR_bytesToHex(trd.data.list, trd.data.len, dataStr);
//		  printf("  data(%d): %s\n", trd.data.len, dataStr);
//		}
	}

	while(1)
	{
		printf("testing\n");
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}
