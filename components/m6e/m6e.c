#include "m6e.h"
#include <string.h>
#include "esp_heap_caps.h"
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"

#define numberof(x) (sizeof((x))/sizeof((x)[0]))

static SemaphoreHandle_t mutex;
static TMR_Reader *rp;
static TMR_ReadPlan plan;
uint8_t antennaList[1] = {1};
TMR_Region region;
TMR_TagOp tagop;

static void checkerr(TMR_Reader* rp, TMR_Status ret, const char *msg);
void printU32List(TMR_uint32List *list);

void hwInit()
{
    TMR_Status ret;
    int readPower, writePower, temp;
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

    readPower = 2700;
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
 
    // set tag encoding
    TMR_GEN2_TagEncoding tag = TMR_GEN2_MILLER_M_4;
 
    ret = TMR_paramSet(rp, TMR_PARAM_GEN2_TAGENCODING, &tag);
    checkerr(rp, ret, "Setting tag encoding");
 
    ret = TMR_paramGet(rp, TMR_PARAM_GEN2_TAGENCODING, &tag);
    checkerr(rp, ret, "Getting tag encoding");
    printf ("tag encoding: %d \r\n", tag);
 
    // set Q value
    TMR_GEN2_Q value ;
    value.type = TMR_SR_GEN2_Q_DYNAMIC;
    ret = TMR_paramSet(rp, TMR_PARAM_GEN2_Q, &value);
    checkerr(rp, ret, "Setting q");
 
    ret = TMR_paramGet(rp, TMR_PARAM_GEN2_Q, &value);
    checkerr(rp, ret, "Getting q");
    if (value.type == TMR_SR_GEN2_Q_DYNAMIC)
    {
      printf("DynamicQ\n");
    }
    else if (value.type == TMR_SR_GEN2_Q_STATIC)
    {
      printf("StaticQ(%d)\n", value.u.staticQ.initialQ);
    }
 
    // set session
    TMR_GEN2_Session session = TMR_GEN2_SESSION_S0;
 
    ret = TMR_paramSet(rp, TMR_PARAM_GEN2_SESSION, &session);
    checkerr(rp, ret, "Setting session");
 
    ret = TMR_paramGet(rp, TMR_PARAM_GEN2_SESSION, &session);
    checkerr(rp, ret, "Getting session");
    printf ("session: %d \r\n", session);
 
 
    // set target
    TMR_GEN2_Target target = TMR_GEN2_TARGET_A ;
 
    ret = TMR_paramSet(rp, TMR_PARAM_GEN2_TARGET, &target);
    checkerr(rp, ret, "Setting target");
 
    ret = TMR_paramGet(rp, TMR_PARAM_GEN2_TARGET, &target);
    checkerr(rp, ret, "Getting taget");
    printf ("target: %d \r\n", target);

	// set region support 868mhz
	region = TMR_REGION_EU3;
	ret = TMR_paramSet(rp, TMR_PARAM_REGION_ID, &region);
	checkerr(rp, ret, "Setting region");

    // change frequency hop table
    uint32_t myList[1];
    TMR_uint32List hopfreq;
    hopfreq.max = 4;
    hopfreq.list = myList;
    hopfreq.len = 1;
    myList[0] = 867000;
 
    ret = TMR_paramSet(rp, TMR_PARAM_REGION_HOPTABLE, &hopfreq);
    checkerr(rp, ret, "Setting hoptable");
 
    hopfreq.max = numberof(myList);
    hopfreq.list = myList;
    
    ret = TMR_paramGet(rp, TMR_PARAM_REGION_HOPTABLE, &hopfreq);
    printU32List(&hopfreq);
    putchar('\n');

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
    TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE;
    TIMERG0.wdt_feed=1;
    TIMERG0.wdt_wprotect=0;
 
    while(1)
    {
        xEventGroupWaitBits(eventGroup, GET_TAG, pdTRUE, pdFALSE, portMAX_DELAY);
 
        xSemaphoreTake(mutex, portMAX_DELAY);
        while(1)
        {
            uxBits = xEventGroupWaitBits(eventGroup, STOP_GET_TAG, pdTRUE, pdFALSE, 0);
            printf("bit %d\r\n", uxBits);
            if ((uxBits & STOP_GET_TAG) == STOP_GET_TAG)
            {
                printf("stop\r\n");
 
                break;
            }
                
            ret = TMR_read(rp, 100, &tagCount);
            checkerr(rp, ret, "Reading reader");
            printf ("++++++++++++++++++++++++++++++\r\n");
            printf("+++tag count : %d ++++\n", tagCount);
            printf ("++++++++++++++++++++++++++++++\r\n");
 
 
            while (TMR_SUCCESS == TMR_hasMoreTags(rp))
            {
                TMR_TagReadData *trd;
                uint8_t *dataBuf, *dataBuf1, *dataBuf2, *dataBuf3, *dataBuf4, buflen = 255;
 
                trd = (TMR_TagReadData *) malloc(sizeof(TMR_TagReadData));
                dataBuf = (uint8_t *) malloc(sizeof(uint8_t)*buflen);
                // dataBuf1 = (uint8_t *) malloc(sizeof(uint8_t)*buflen);
                // dataBuf2 = (uint8_t *) malloc(sizeof(uint8_t)*buflen);
                // dataBuf3 = (uint8_t *) malloc(sizeof(uint8_t)*buflen);
                // dataBuf4 = (uint8_t *) malloc(sizeof(uint8_t)*buflen);
                // printf("trd: %d, dataBuf: %d, dataBuf4: %d \r\n", (int)trd, (int)dataBuf, (int)dataBuf4);
                // printf("free heap: %d, largest heap: %d \r\n", esp_get_free_heap_size(), heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
 
                ret = TMR_TRD_init_data(trd, buflen, dataBuf);
                // checkerr(rp, ret, "creating tag read data");
 
                trd->userMemData.list = NULL;
                trd->userMemData.max = buflen;
                trd->userMemData.len = 0;
 
                trd->epcMemData.list = NULL;
                trd->epcMemData.max = buflen;
                trd->epcMemData.len = 0;
 
                trd->reservedMemData.list = NULL;
                trd->reservedMemData.max = buflen;
                trd->reservedMemData.len = 0;
 
                trd->tidMemData.list = NULL;
                trd->tidMemData.max = buflen;
                trd->tidMemData.len = 0;
 
                ret = TMR_getNextTag(rp, trd);
                // checkerr(rp, ret, "Next tag");
                
                if (xQueueSend(getTagQueue,(void *)&trd,(TickType_t )0) == pdTRUE)
                {
                    // printf("value sent on queue \n");
                } else printf("tag send queue error\n");
                // readcount += trd->readCount;
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
            // vTaskDelay(10 / portTICK_PERIOD_MS);
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
    xTaskCreate(&hwGetTag,"hwGetTag",10240,NULL,10,NULL);
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

// helper function
void printU32List(TMR_uint32List *list)
{
    int i;

    putchar('[');
    for (i = 0; i < list->len && i < list->max; i++)
    {
        printf("%d%s", list->list[i],
            ((i + 1) == list->len) ? "" : ",");
    }
    if (list->len > list->max)
    {
        printf("...");
    }
    putchar(']');
}