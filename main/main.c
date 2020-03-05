#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "nvs.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tm_reader.h"
#include "m6e.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_bt_device.h"
#include "esp_spp_api.h"

#include "time.h"
#include "sys/time.h"

// #define BLUETOOTH_DEBUG
// #define DEBUG

#define SPP_TAG "SPP_ACCEPTOR_DEMO"
#define DATA_TAG "data"
#define SPP_SERVER_NAME "SPP_SERVER"
#define EXCAMPLE_DEVICE_NAME "ANDROID UHF"
#define SPP_SHOW_DATA 0
#define SPP_SHOW_SPEED 1
#define SPP_SHOW_MODE SPP_SHOW_DATA    /*Choose show mode: show data or speed*/

#define DEBOUNCE_TIME 5
#define BTN_GPIO 19
#define ESP_INTR_FLAG_DEFAULT 0

//static const esp_spp_mode_t esp_spp_mode = ESP_SPP_MODE_CB;
static const esp_spp_mode_t esp_spp_mode = ESP_SPP_MODE_CB;

static struct timeval time_new, time_old;
static long data_num = 0;

static const esp_spp_sec_t sec_mask = ESP_SPP_SEC_AUTHENTICATE;
static const esp_spp_role_t role_slave = ESP_SPP_ROLE_SLAVE;

//Control RFID
volatile uint32_t temp = NULL, debounceTimeout = 0;
volatile uint8_t control, flag = true, lastValue = 1;
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

void getTag();
void sendBluetoothString(char *data);
void sendBluetoothData(uint32_t len, unsigned char*data);
void sendBluetooth(uint32_t len, unsigned char* data);
void endPackage();

void IRAM_ATTR handleButtonInterrupt() {
    portENTER_CRITICAL_ISR(&mux);
    uint8_t curretnValue = gpio_get_level(BTN_GPIO);
    uint32_t currentTime = xTaskGetTickCount();
    // ets_printf("time cur: %d, last: %d, true/false: %d\r\n", currentTime, debounceTimeout, (currentTime - debounceTimeout) >= DEBOUNCE_TIME);
    if ((currentTime - debounceTimeout) >= DEBOUNCE_TIME)
    {
        // if (lastValue != curretnValue)
        // {
        //     lastValue = curretnValue;
        BaseType_t xHigherPriorityTaskWoken, xResult;

        xHigherPriorityTaskWoken = pdFALSE;

        if (lastValue == 0 )
        {
            xResult = xEventGroupSetBitsFromISR(eventGroup, STOP_GET_TAG, xHigherPriorityTaskWoken);
            lastValue = 1;
        }
            
        else 
        {
            xResult = xEventGroupSetBitsFromISR(eventGroup, GET_TAG, xHigherPriorityTaskWoken);
            lastValue = 0;
        }

        if( xResult != pdFAIL )
        {
            portYIELD_FROM_ISR();
        }
        // ets_printf("---interrupt %d\r\n", lastValue);
        // }
    }
    debounceTimeout = currentTime;

    portEXIT_CRITICAL_ISR(&mux);    
}

static void esp_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
    switch (event) {
    case ESP_SPP_INIT_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_INIT_EVT");
        esp_bt_dev_set_device_name(EXCAMPLE_DEVICE_NAME);
        esp_bt_gap_set_scan_mode(ESP_BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE);
        esp_spp_start_srv(sec_mask,role_slave, 0, SPP_SERVER_NAME);
        break;
    case ESP_SPP_DISCOVERY_COMP_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_DISCOVERY_COMP_EVT");
        break;
    case ESP_SPP_OPEN_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_OPEN_EVT");

        break;
    case ESP_SPP_CLOSE_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_CLOSE_EVT");
        temp = NULL;
        break;
    case ESP_SPP_START_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_START_EVT");
        break;
    case ESP_SPP_CL_INIT_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_CL_INIT_EVT");
        break;
    case ESP_SPP_DATA_IND_EVT:
#if (SPP_SHOW_MODE == SPP_SHOW_DATA)
        ESP_LOGI(SPP_TAG, "ESP_SPP_DATA_IND_EVT len=%d handle=%d",
                 param->data_ind.len, param->data_ind.handle);
        esp_log_buffer_hex("",param->data_ind.data,param->data_ind.len);
        control = *param->data_ind.data;
        switch (control)
        {
            case 'a':
                xEventGroupSetBits(eventGroup, GET_TAG);
                break;
            case 'b':
                xEventGroupSetBits(eventGroup, STOP_GET_TAG);
                break;
            default:
                break;
        }
//        printf("%d", control);
//        ESP_LOGI(SPP_TAG, "ESP_RECEIVE_ANDROID DATA=%d",
//                         control);
        //esp_spp_write(param->cong.handle, SPP_DATA_LEN, spp_data);
#else
        gettimeofday(&time_new, NULL);
        data_num += param->data_ind.len;
        if (time_new.tv_sec - time_old.tv_sec >= 3) {
            print_speed();
        }
#endif
        break;
    case ESP_SPP_CONG_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_CONG_EVT");
        break;
    case ESP_SPP_WRITE_EVT:
        // ESP_LOGI(SPP_TAG, "ESP_SPP_WRITE_EVT");
        flag = true;
        break;
    case ESP_SPP_SRV_OPEN_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_SRV_OPEN_EVT");
        temp = param->cong.handle;
        gettimeofday(&time_old, NULL);
        break;
    default:
        break;
    }
}

void esp_bt_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
    switch (event) {
    case ESP_BT_GAP_AUTH_CMPL_EVT:{
        if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS) {
            ESP_LOGI(SPP_TAG, "authentication success: %s", param->auth_cmpl.device_name);
            esp_log_buffer_hex(SPP_TAG, param->auth_cmpl.bda, ESP_BD_ADDR_LEN);
        } else {
            ESP_LOGE(SPP_TAG, "authentication failed, status:%d", param->auth_cmpl.stat);
        }
        break;
    }
    case ESP_BT_GAP_PIN_REQ_EVT:{
        ESP_LOGI(SPP_TAG, "ESP_BT_GAP_PIN_REQ_EVT min_16_digit:%d", param->pin_req.min_16_digit);
        if (param->pin_req.min_16_digit) {
            ESP_LOGI(SPP_TAG, "Input pin code: 0000 0000 0000 0000");
            esp_bt_pin_code_t pin_code = {0};
            esp_bt_gap_pin_reply(param->pin_req.bda, true, 16, pin_code);
        } else {
            ESP_LOGI(SPP_TAG, "Input pin code: 1234");
            esp_bt_pin_code_t pin_code;
            pin_code[0] = '1';
            pin_code[1] = '2';
            pin_code[2] = '3';
            pin_code[3] = '4';
            esp_bt_gap_pin_reply(param->pin_req.bda, true, 4, pin_code);
        }
        break;
    }

#if (CONFIG_BT_SSP_ENABLED == true)
    case ESP_BT_GAP_CFM_REQ_EVT:
        ESP_LOGI(SPP_TAG, "ESP_BT_GAP_CFM_REQ_EVT Please compare the numeric value: %d", param->cfm_req.num_val);
        esp_bt_gap_ssp_confirm_reply(param->cfm_req.bda, true);
        break;
    case ESP_BT_GAP_KEY_NOTIF_EVT:
        ESP_LOGI(SPP_TAG, "ESP_BT_GAP_KEY_NOTIF_EVT passkey:%d", param->key_notif.passkey);
        break;
    case ESP_BT_GAP_KEY_REQ_EVT:
        ESP_LOGI(SPP_TAG, "ESP_BT_GAP_KEY_REQ_EVT Please enter passkey!");
        break;
#endif

    default: {
        ESP_LOGI(SPP_TAG, "event: %d", event);
        break;
    }
    }
    return;
}

void InitBluetooth()
{
	//Bluetooth//
	esp_err_t ret = nvs_flash_init();
	    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
	        ESP_ERROR_CHECK(nvs_flash_erase());
	        ret = nvs_flash_init();
	    }
	    ESP_ERROR_CHECK( ret );

	    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

	    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
	    if ((ret = esp_bt_controller_init(&bt_cfg)) != ESP_OK) {
	        ESP_LOGE(SPP_TAG, "%s initialize controller failed: %s\n", __func__, esp_err_to_name(ret));
	        return;
	    }

	    if ((ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT)) != ESP_OK) {
	        ESP_LOGE(SPP_TAG, "%s enable controller failed: %s\n", __func__, esp_err_to_name(ret));
	        return;
	    }

	    if ((ret = esp_bluedroid_init()) != ESP_OK) {
	        ESP_LOGE(SPP_TAG, "%s initialize bluedroid failed: %s\n", __func__, esp_err_to_name(ret));
	        return;
	    }

	    if ((ret = esp_bluedroid_enable()) != ESP_OK) {
	        ESP_LOGE(SPP_TAG, "%s enable bluedroid failed: %s\n", __func__, esp_err_to_name(ret));
	        return;
	    }

	    if ((ret = esp_bt_gap_register_callback(esp_bt_gap_cb)) != ESP_OK) {
	        ESP_LOGE(SPP_TAG, "%s gap register failed: %s\n", __func__, esp_err_to_name(ret));
	        return;
	    }

	    if ((ret = esp_spp_register_callback(esp_spp_cb)) != ESP_OK) {
	        ESP_LOGE(SPP_TAG, "%s spp register failed: %s\n", __func__, esp_err_to_name(ret));
	        return;
	    }

	    if ((ret = esp_spp_init(esp_spp_mode)) != ESP_OK) {
	        ESP_LOGE(SPP_TAG, "%s spp init failed: %s\n", __func__, esp_err_to_name(ret));
	        return;
	    }
}
void app_main()
{
	hwTaskInit();
    InitBluetooth();

    gpio_pad_select_gpio(BTN_GPIO);
    gpio_set_direction(BTN_GPIO, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BTN_GPIO, GPIO_PULLUP_ONLY);

    gpio_set_intr_type(BTN_GPIO, GPIO_INTR_ANYEDGE);
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(BTN_GPIO, handleButtonInterrupt, NULL);

    xTaskCreate(&getTag,"getTag",4096,NULL,10,NULL);
    // xEventGroupSetBits(eventGroup, GET_TAG);
}

void getTag()
{
	char epcStr[128], dataStr[128], newLine[2];
	TMR_TagReadData *trd;
    newLine[0] = '\n';
    newLine[1] = '\r';

	while(1)
	{
		while(1)
		{
			xQueueReceive(getTagQueue, &trd, portMAX_DELAY);

			TMR_bytesToHex(trd->tag.epc, trd->tag.epcByteCount, epcStr);
            sendBluetoothString("epc byte: ");
            sendBluetoothData(trd->tag.epcByteCount, (unsigned char *)epcStr);
            sendBluetoothString("\r\n");
            printf("ecpstr: %s, freq: %d, Rssi: %d\n", epcStr, trd->frequency, trd->rssi );

            printf("number of time tag read: %d\r\n", trd->readCount);

            if (0 < trd->data.len)
            {
                TMR_bytesToHex(trd->data.list, trd->data.len, dataStr);

                sendBluetoothString("data: ");
                sendBluetoothData(trd->data.len*2, (unsigned char *)dataStr);
                sendBluetoothString("\r\n");
            }

            if (0 < trd->userMemData.len)
            {
                TMR_bytesToHex(trd->userMemData.list, trd->userMemData.len, dataStr);

                sendBluetoothString("userMemData: ");
                sendBluetoothData(trd->userMemData.len*2, (unsigned char *)dataStr);
                sendBluetoothString("\r\n");
            }

            if (0 < trd->epcMemData.len)
            {
                TMR_bytesToHex(trd->epcMemData.list, trd->epcMemData.len, dataStr);

                sendBluetoothString("epcMemData: ");
                sendBluetoothData(trd->epcMemData.len*2, (unsigned char *)dataStr);
                sendBluetoothString("\r\n");
            }

            if (0 < trd->reservedMemData.len)
            {
                TMR_bytesToHex(trd->reservedMemData.list, trd->reservedMemData.len, dataStr);

                sendBluetoothString("reservedMemData: ");
                sendBluetoothData(trd->reservedMemData.len*2, (unsigned char *)dataStr);
                sendBluetoothString("\r\n");
            }

            if (0 < trd->tidMemData.len)
            {
                TMR_bytesToHex(trd->tidMemData.list, trd->tidMemData.len, dataStr);

                sendBluetoothString("tidMemData: ");
                sendBluetoothData(trd->tidMemData.len*2, (unsigned char *)dataStr);
                sendBluetoothString("\r\n");
            }

			destroyTagdata(trd);

            printf("\r\n\r\n");

            sendBluetoothString("\r\n\r\n");
            endPackage();
		}
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

void sendBluetoothString(char *data)
{
    #ifdef BLUETOOTH_DEBUG
    sendBluetooth(strlen(data),(unsigned char *) data);
    #endif
}

void sendBluetoothData(uint32_t len, unsigned char *data)
{
    uint8_t i = 0;
    for (; len > 10 ; len -= 10, ++i)
    {
        sendBluetooth(10, data+(10*i));
    }

    sendBluetooth (len, data+(10*i));
    sendBluetooth (1, (unsigned char*) "\0");

    vTaskDelay(10 / portTICK_PERIOD_MS);
}

void endPackage()
{
    sendBluetooth (1, (unsigned char*) "\n");
}

void sendBluetooth(uint32_t len, unsigned char* data)
{
    flag = false;
    if (NULL != temp)
    {
        esp_spp_write(temp, len, data);
        while (!flag);
    }
}