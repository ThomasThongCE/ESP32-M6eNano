#include "m6e.h"
#include <string.h>
#include "esp_heap_caps.h"

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
    char str[64];

	int tagCount;

	rp = (TMR_Reader *) malloc(sizeof(TMR_Reader));
	ret = TMR_create(rp, "tmr:///");
	checkerr(rp, ret, "Creating reader");

	do{
		ret = TMR_connect(rp);
		checkerr(rp, ret, "Connecting reader");
	}while (TMR_SUCCESS != ret);

    // set maximum power
    readPower = 0;
    writePower = 0;
	ret = TMR_paramGet(rp, TMR_PARAM_RADIO_READPOWER, &readPower);
	checkerr(rp, ret, "Getting current read power");
	ret = TMR_paramGet(rp, TMR_PARAM_RADIO_POWERMAX, &writePower);
	checkerr(rp, ret, "Getting current write power");
	printf("readpower: %d, TMR_PARAM_RADIO_POWERMAX: %u\n", readPower, writePower);

    readPower = 2000;
    writePower = 0;

    ret = TMR_paramSet(rp, TMR_PARAM_RADIO_READPOWER, &readPower);
	checkerr(rp, ret, "Setting current read power");
	ret = TMR_paramSet(rp, TMR_PARAM_RADIO_WRITEPOWER, &writePower);
	checkerr(rp, ret, "Setting current write power");
	printf("readpower: %d, writepower: %d\n", readPower, writePower);

    ret = TMR_paramGet(rp, TMR_PARAM_RADIO_READPOWER, &readPower);
	checkerr(rp, ret, "Getting current read power");
	ret = TMR_paramGet(rp, TMR_PARAM_RADIO_WRITEPOWER, &writePower);
	checkerr(rp, ret, "Getting current write power");
	printf("readpower: %d, writepower: %d\n", readPower, writePower);

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

    // commit read plan
    ret = TMR_paramSet(rp, TMR_PARAM_READ_PLAN, &plan);
	checkerr(rp, ret, "commit read plan");
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
                printf ("++++++++++++++++++++++++++++++\r\n");
                printf("+++tag count : %d ++++\n", tagCount);
                printf ("++++++++++++++++++++++++++++++\r\n");
            } while (tagCount == 0);

            while (TMR_SUCCESS == TMR_hasMoreTags(rp))
            {
                TMR_TagReadData *trd;
                uint8_t *dataBuf, *dataBuf1, *dataBuf2, *dataBuf3, *dataBuf4, buflen = 255;

                trd = (TMR_TagReadData *) malloc(sizeof(TMR_TagReadData));
                dataBuf = (uint8_t *) malloc(sizeof(uint8_t)*buflen);
                dataBuf1 = (uint8_t *) malloc(sizeof(uint8_t)*buflen);
                dataBuf2 = (uint8_t *) malloc(sizeof(uint8_t)*buflen);
                dataBuf3 = (uint8_t *) malloc(sizeof(uint8_t)*buflen);
                dataBuf4 = (uint8_t *) malloc(sizeof(uint8_t)*buflen);
                // printf("trd: %d, dataBuf: %d, dataBuf4: %d \r\n", (int)trd, (int)dataBuf, (int)dataBuf4);
                // printf("free heap: %d, largest heap: %d \r\n", esp_get_free_heap_size(), heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));

                ret = TMR_TRD_init_data(trd, buflen, dataBuf);
                checkerr(rp, ret, "creating tag read data");

                trd->userMemData.list = dataBuf1;
                trd->userMemData.max = buflen;
                trd->userMemData.len = 0;

                trd->epcMemData.list = dataBuf2;
                trd->epcMemData.max = buflen;
                trd->epcMemData.len = 0;

                trd->reservedMemData.list = dataBuf3;
                trd->reservedMemData.max = buflen;
                trd->reservedMemData.len = 0;

                trd->tidMemData.list = dataBuf4;
                trd->tidMemData.max = buflen;
                trd->tidMemData.len = 0;

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
                //     char dataStr[128];
                //     printf ("len: %d, value: %d\r\n", trd->data.len, (int)trd->data.list);
                //     TMR_bytesToHex(trd->data.list, trd->data.len, dataStr);
                //     printf("  data(%d): %s\n", trd->data.len, dataStr);
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
    xTaskCreate(&hwGetTag,"hwGetTag",10240,NULL,5,NULL);
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
        if (tagData->tidMemData.list != NULL)
            free(tagData->userMemData.list);

        if (tagData->tidMemData.list != NULL)
            free(tagData->epcMemData.list);

        if (tagData->tidMemData.list != NULL)
            free(tagData->reservedMemData.list);

        if (tagData->tidMemData.list != NULL)
            free(tagData->tidMemData.list);

        if (tagData->data.list != NULL)
            free(tagData->data.list);

        free (tagData);
    }
}
