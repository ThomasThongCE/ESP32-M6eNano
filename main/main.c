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
	TMR_TagOp tagop;
	uint8_t epcData[] = {
	  0x01, 0x23, 0x45, 0x67, 0x89, 0xAB,
	  0xCD, 0xEF, 0x01, 0x23, 0x45, 0x67,
	};
	uint8_t readLength = 0x00;
	uint8_t antennaList[1] = {1};

	int tagCount;

	preader = &reader;
	//reader = (TMR_Reader *) malloc(sizeof(TMR_Reader));
	ret = TMR_create(preader, "tmr:///");
	checkerr(preader, ret, "Creating reader");

	do{
		ret = TMR_connect(preader);
		checkerr(preader, ret, "Connecting reader");
	}while (TMR_SUCCESS != ret);
	// set region support 868mhz
	region = TMR_REGION_EU3;
	//region = TMR_REGION_IN;
	ret = TMR_paramSet(preader, TMR_PARAM_REGION_ID, &region);
	checkerr(preader, ret, "Setting region");

	// Create simple plan using antenna 1
	ret = TMR_RP_init_simple(&plan, 1, &antennaList, TMR_TAG_PROTOCOL_GEN2, 1000);
	checkerr(preader, ret, "Create plan");

	// Add tagop
	ret = TMR_TagOp_init_GEN2_ReadData(&tagop, \
			(TMR_GEN2_BANK_USER \
			| TMR_GEN2_BANK_TID_ENABLED \
			| TMR_GEN2_BANK_USER_ENABLED)\
			, 0, readLength);
	checkerr(preader, ret, "Create tagop");

	// set tagop into plan
	ret = TMR_RP_set_tagop(&plan, &tagop);
	checkerr(preader, ret, "setting tagop");

	/* Commit read plan */
	ret = TMR_paramSet(preader, TMR_PARAM_READ_PLAN, &plan);
	checkerr(preader, ret, "commit read plan");

	while(1)
	{
		ret = TMR_read(&reader, 1000, &tagCount);
		checkerr(preader, ret, "Reading reader");

		while (TMR_SUCCESS == TMR_hasMoreTags(&reader))
		{
			TMR_TagReadData trd;
			uint8_t dataBuf[258];
			uint8_t dataBuf1[258];
			uint8_t dataBuf2[258];
			char epcStr[128];
			TMR_TagData epc;
			TMR_ReadPlan mplan;

			ret = TMR_TRD_init_data(&trd, sizeof(dataBuf)/sizeof(uint8_t), dataBuf);
			checkerr(preader, ret, "creating tag read data");

			trd.userMemData.list = dataBuf1;
			trd.tidMemData.list = dataBuf2;

			trd.userMemData.max = 258;
			trd.userMemData.len = 0;
			trd.tidMemData.max = 258;
			trd.tidMemData.len = 0;

			ret = TMR_getNextTag(&reader, &trd);
			checkerr(preader, ret, "Next tag");

			TMR_bytesToHex(trd.tag.epc, trd.tag.epcByteCount, epcStr);
			printf("ecpstr: %s, freq: %d, Rssi: %d\n", epcStr, trd.frequency, trd.rssi );
			if (0 < trd.data.len)
			{
			  char dataStr[255];
			  TMR_bytesToHex(trd.data.list, trd.data.len, dataStr);
			  printf("  data(%d): %s\n", trd.data.len, dataStr);
			}

			if (0 < trd.tidMemData.len)
			{
			  char tidStr[258];
			  printf("tiddata lenght: %d\n", trd.tidMemData.len);
			  TMR_bytesToHex(trd.tidMemData.list, trd.tidMemData.len, tidStr);
			  printf("  tidData(%d): %s\n", trd.tidMemData.len, tidStr);
			}

			epc.epcByteCount = sizeof(epcData) / sizeof(epcData[0]);
			memcpy(epc.epc, epcData, epc.epcByteCount * sizeof(uint8_t));
			ret = TMR_TagOp_init_GEN2_WriteTag(&tagop, &epc);
			checkerr(preader, ret, "initializing GEN2_WriteTag");
			ret = TMR_executeTagOp(preader, &tagop, NULL, NULL);
			checkerr(preader, ret, "executing the write tag operation");

			// Create simple plan using antenna 1
			ret = TMR_RP_init_simple(&mplan, 1, &antennaList, TMR_TAG_PROTOCOL_GEN2, 1000);
			checkerr(preader, ret, "Create plan");

			// set tagop into plan
			ret = TMR_RP_set_tagop(&mplan, &tagop);
			checkerr(preader, ret, "setting tagop");

			/* Commit read plan */
			ret = TMR_paramSet(preader, TMR_PARAM_READ_PLAN, &mplan);
			checkerr(preader, ret, "commit read plan");
		}
		//printf("testing\n");
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}
