#include "m6e.h"

static SemaphoreHandle_t mutex;
static TMR_Reader *rp;
static TMR_ReadPlan plan;
uint8_t antennaList[1] = {1};
TMR_Region region;
TMR_TagOp tagop;

static void checkerr(TMR_Reader* rp, TMR_Status ret, const char *msg);

void hwInit()
{
    TMR_Status ret;
    int readPower, writePower;

	int tagCount;

	rp = (TMR_Reader *) malloc(sizeof(TMR_Reader));
	ret = TMR_create(rp, "tmr:///");
	checkerr(rp, ret, "Creating reader");

	do{
		ret = TMR_connect(rp);
		checkerr(rp, ret, "Connecting reader");
	}while (TMR_SUCCESS != ret);

    // set maximum power
    readPower = MAXPOWER;
    writePower = MAXPOWER;
	// ret = TMR_paramSet(rp, TMR_PARAM_RADIO_READPOWER, &readPower);
	// checkerr(rp, ret, "Setting current read power");
	// ret = TMR_paramSet(rp, TMR_PARAM_RADIO_WRITEPOWER, &writePower);
	// checkerr(rp, ret, "Setting current write power");
	// printf("readpower: %d, writepower: %d\n", readPower, writePower);

	// set region support 868mhz
	region = TMR_REGION_EU3;
	ret = TMR_paramSet(rp, TMR_PARAM_REGION_ID, &region);
	checkerr(rp, ret, "Setting region");

	// Create simple plan using antenna 1
	ret = TMR_RP_init_simple(&plan, 1, &antennaList, TMR_TAG_PROTOCOL_GEN2, 1000);
	checkerr(rp, ret, "Create plan");

	// add tagop
	ret = TMR_TagOp_init_GEN2_ReadData(&tagop, TMR_GEN2_BANK_TID , 0, 0);
	checkerr(rp, ret, "Create tagop");

	// set tagop into plan
	ret = TMR_RP_set_tagop(&plan, &tagop);
	checkerr(rp, ret, "setting tagop");

}

void hwGetTag()
{
    TMR_Status ret;
    int32_t tagCount;
    EventBits_t uxBits;

    while(1)
    {
        xEventGroupWaitBits(eventGroup, GET_TAG, pdTRUE, pdFALSE, portMAX_DELAY);

        /* Commit read plan */
        ret = TMR_paramSet(rp, TMR_PARAM_READ_PLAN, &plan);
        checkerr(rp, ret, "commit read plan");
        xSemaphoreTake(mutex, portMAX_DELAY);
        while(1)
        {
        	uxBits = xEventGroupWaitBits(eventGroup, STOP_GET_TAG, pdTRUE, pdFALSE, 0);
        	if ((uxBits & STOP_GET_TAG) == STOP_GET_TAG)
        		break;
            do {
                ret = TMR_read(rp, 1000, &tagCount);
                checkerr(rp, ret, "Reading reader");
                printf("tag count : %d\n", tagCount);
            } while (tagCount == 0);

            while (TMR_SUCCESS == TMR_hasMoreTags(rp))
            {
                TMR_TagReadData *trd;
                uint8_t *dataBuf, buflen = 256;

                trd = (TMR_TagReadData *) malloc(sizeof(TMR_TagReadData));
                dataBuf = (uint8_t *) malloc(sizeof(uint8_t)*buflen);

                ret = TMR_TRD_init_data(trd, buflen*sizeof(uint8_t), dataBuf);
                checkerr(rp, ret, "creating tag read data");

                // trd->userMemData.max = 258;
                // trd->userMemData.len = 0;
                // trd->epcMemData.max = 258;
                // trd->epcMemData.len = 0;
                // trd->reservedMemData.max = 258;
                // trd->reservedMemData.len = 0;
                // trd->tidMemData.max = 258;
                // trd->tidMemData.len = 0;

                ret = TMR_getNextTag(rp, trd);
                checkerr(rp, ret, "Next tag");
                
                if (xQueueSend(getTagQueue,(void *)&trd,(TickType_t )0) == pdTRUE)
                {
                    printf("value sent on queue \n");
                } else printf("tag send queue error\n");

                // TMR_bytesToHex(trd->tag.epc, trd->tag.epcByteCount, epcStr);
                // printf("\necpstr: %s, freq: %d, Rssi: %d\n", epcStr, trd->frequency, trd->rssi );

                // if (0 < trd->data.len)
                // {
                //   TMR_bytesToHex(trd->data.list, trd->data.len, dataStr);
                //   printf("  data(%d): %s\n", trd->data.len, dataStr);
                // }
            }
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }

        xSemaphoreGive(mutex);
    }
}

void hwTaskInit()
{
    getTagQueue = xQueueCreate(10,sizeof(TMR_TagReadData *));
    writeTagQueue = xQueueCreate(1, sizeof(TMR_TagData));
    getSizeTagQueue = xQueueCreate(1, sizeof(int32_t));
	mutex = xSemaphoreCreateMutex();
	eventGroup = xEventGroupCreate();

    hwInit();

    printf("Create task\n");
    xTaskCreate(&hwGetTag,"hwGetTag",4096,NULL,5,NULL);
}

void hwConfigPower()
{
    TMR_Status ret;

    while(1)
    {
        xEventGroupWaitBits(eventGroup, POWER, pdTRUE, pdFALSE, portMAX_DELAY);
    }
}

// helpper function 
static void checkerr(TMR_Reader* rp, TMR_Status ret, const char *msg)
{
	if (TMR_SUCCESS != ret)
	{
		printf( "Error %s: %s\n", msg, TMR_strerr(rp, ret));
	}
	else
		printf("%s success\n", msg);
}

void destroyTagdata(TMR_TagReadData *tagData)
{
    if (tagData != NULL)
    {
        // if ((tagData->userMemData.len > 0) && (tagData->tidMemData.list != NULL))
        //     free(tagData->userMemData.list);

        // if ((tagData->epcMemData.len > 0) && (tagData->tidMemData.list != NULL))
        //     free(tagData->epcMemData.list);

        // if ((tagData->reservedMemData.len > 0) && (tagData->tidMemData.list != NULL))
        //     free(tagData->reservedMemData.list);

        // if ((tagData->tidMemData.len > 0) && (tagData->tidMemData.list != NULL))
        //     free(tagData->tidMemData.list);

        // if ((tagData->data.len > 0) && (tagData->data.list != NULL))
        //     free(tagData->data.list);

        free (tagData);
    }
}
