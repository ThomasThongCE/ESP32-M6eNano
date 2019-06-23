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

#define ECHO_TEST_TXD  (GPIO_NUM_1)
#define ECHO_TEST_RXD  (GPIO_NUM_3)
#define ECHO_TEST_RTS  (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS  (UART_PIN_NO_CHANGE)
#define FILTER
#define WRITE_EPC


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
	ret = TMR_paramSet(preader, TMR_PARAM_REGION_ID, &region);
	checkerr(preader, ret, "Setting region");

	// Create simple plan using antenna 1
	ret = TMR_RP_init_simple(&plan, 1, &antennaList, TMR_TAG_PROTOCOL_GEN2, 1000);
	checkerr(preader, ret, "Create plan");

	// add tag filter
	TMR_TagData epc;
	uint8_t epcData[] = {
		0x01, 0x23, 0x45, 0x67, 0x89, 0xAB,
		0xCD, 0xEF, 0x01, 0x23, 0x45, 0x67,
	};
	epc.protocol = TMR_TAG_PROTOCOL_GEN2;
	epc.epcByteCount = sizeof(epcData) / sizeof(epcData[0]);
	memcpy(epc.epc, epcData, epc.epcByteCount * sizeof(uint8_t));

#ifdef FILTER
	static TMR_TagFilter filt;

	ret = TMR_TF_init_tag(&filt, &epc);
	checkerr(preader, ret, "creating tag filter");
	ret = TMR_RP_set_filter(&plan, &filt);
	checkerr(preader, ret, "setting tag filter");
#endif

	// add tagop
	ret = TMR_TagOp_init_GEN2_ReadData(&tagop, \
			( TMR_GEN2_BANK_EPC \
			| TMR_GEN2_BANK_TID_ENABLED ) \
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
		printf("tag count : %d\n", tagCount);
		while (TMR_SUCCESS == TMR_hasMoreTags(&reader))
		{
			TMR_TagReadData trd;
			uint8_t dataBuf[258];
			uint8_t dataBuf1[258];
			uint8_t dataBuf2[258];
			uint8_t dataBuf3[258];
    		uint8_t dataBuf4[258];
			char epcStr[128];
			char dataStr[258];
			// TMR_ReadPlan mplan;

			ret = TMR_TRD_init_data(&trd, sizeof(dataBuf)/sizeof(uint8_t), dataBuf);
			checkerr(preader, ret, "creating tag read data");

			trd.userMemData.list = dataBuf1;
			trd.epcMemData.list = dataBuf2;
			trd.reservedMemData.list = dataBuf3;
			trd.tidMemData.list = dataBuf4;

			trd.userMemData.max = 258;
			trd.userMemData.len = 0;
			trd.epcMemData.max = 258;
			trd.epcMemData.len = 0;
			trd.reservedMemData.max = 258;
			trd.reservedMemData.len = 0;
			trd.tidMemData.max = 258;
			trd.tidMemData.len = 0;

			ret = TMR_getNextTag(&reader, &trd);
			checkerr(preader, ret, "Next tag");

			TMR_bytesToHex(trd.tag.epc, trd.tag.epcByteCount, epcStr);
			printf("\necpstr: %s, freq: %d, Rssi: %d\n", epcStr, trd.frequency, trd.rssi );

			if (0 < trd.data.len)
			{
			  TMR_bytesToHex(trd.data.list, trd.data.len, dataStr);
			  printf("  data(%d): %s\n", trd.data.len, dataStr);
			}

			if (0 < trd.userMemData.len)
			{
			  printf("\nuserMemData lenght: %d\n", trd.userMemData.len);
			  TMR_bytesToHex(trd.userMemData.list, trd.userMemData.len, dataStr);
			  printf("  userMemData(%d): %s\n", trd.userMemData.len, dataStr);
			}

			if (0 < trd.epcMemData.len)
			{
			  printf("\nepcMemData lenght: %d\n", trd.epcMemData.len);
			  TMR_bytesToHex(trd.epcMemData.list, trd.epcMemData.len, dataStr);
			  printf("  epcMemData(%d): %s\n", trd.epcMemData.len, dataStr);
			}

			if (0 < trd.reservedMemData.len)
			{
			  printf("\nreservedMemData lenght: %d\n", trd.reservedMemData.len);
			  TMR_bytesToHex(trd.reservedMemData.list, trd.reservedMemData.len, dataStr);
			  printf("  reservedMemData(%d): %s\n", trd.reservedMemData.len, dataStr);
			}

			if (0 < trd.tidMemData.len)
			{
			  printf("\ntidMemData lenght: %d\n", trd.tidMemData.len);
			  TMR_bytesToHex(trd.tidMemData.list, trd.tidMemData.len, dataStr);
			  printf("  tidMemData(%d): %s\n", trd.tidMemData.len, dataStr);
			}

			// // Create simple plan using antenna 1
			// ret = TMR_RP_init_simple(&mplan, 1, &antennaList, TMR_TAG_PROTOCOL_GEN2, 1000);
			// checkerr(preader, ret, "Create plan");

			// // set tagop into plan
			// ret = TMR_RP_set_tagop(&mplan, &tagop);
			// checkerr(preader, ret, "setting tagop");

			// /* Commit read plan */
			// ret = TMR_paramSet(preader, TMR_PARAM_READ_PLAN, &mplan);
			// checkerr(preader, ret, "commit read plan");
		}
		#ifdef WRITE_EPC
		// Write epc data into tag 
		TMR_TagOp tagop2;
		
		ret = TMR_TagOp_init_GEN2_WriteTag(&tagop2, &epc);
		checkerr(preader, ret, "initializing GEN2_WriteTag");
		ret = TMR_executeTagOp(preader, &tagop2, NULL, NULL);
		checkerr(preader, ret, "executing the write tag operation");

		ret = TMR_paramSet(preader, TMR_PARAM_REGION_ID, &region);
		checkerr(preader, ret, "Setting region");

		ret = TMR_paramSet(preader, TMR_PARAM_READ_PLAN, &plan);
		checkerr(preader, ret, "commit read plan");	
		#endif
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}

	TMR_destroy(preader);
}
