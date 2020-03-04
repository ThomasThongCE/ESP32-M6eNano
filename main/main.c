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

//static const esp_spp_mode_t esp_spp_mode = ESP_SPP_MODE_CB;
static const esp_spp_mode_t esp_spp_mode = ESP_SPP_MODE_CB;

static struct timeval time_new, time_old;
static long data_num = 0;

static const esp_spp_sec_t sec_mask = ESP_SPP_SEC_AUTHENTICATE;
static const esp_spp_role_t role_slave = ESP_SPP_ROLE_SLAVE;

//Control RFID
volatile uint32_t temp = 0;
volatile uint8_t control, flag = true;

void getTag();
void sendBluetoothString(char *data);
void sendBluetoothData(uint32_t len, unsigned char*data);
void sendBluetooth(uint32_t len, unsigned char* data);
void endPackage();

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
        temp = 0;
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

void InitDevice()
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
    InitDevice();

    xTaskCreate(&getTag,"getTag",4096,NULL,10,NULL);
    // xEventGroupSetBits(eventGroup, GET_TAG);
//    xEventGroupSetBits(eventGroup, GET_TAG);
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

            ESP_LOGD(DATA_TAG, "data len: %d\r\n", trd->data.len);
            ESP_LOGD(DATA_TAG, "userMemData len: %d\r\n", trd->userMemData.len);
            ESP_LOGD(DATA_TAG, "epcMemData len: %d\r\n", trd->epcMemData.len);
            ESP_LOGD(DATA_TAG, "reservedMemData len: %d\r\n", trd->reservedMemData.len);
            ESP_LOGD(DATA_TAG, "tidMemData len: %d\r\n", trd->tidMemData.len);

            printf("number of time tag read: %d\r\n", trd->readCount);

            if (0 < trd->data.len)
            {
                ESP_LOGD(DATA_TAG, "inside data\r\n");
                TMR_bytesToHex(trd->data.list, trd->data.len, dataStr);
                ESP_LOGD(DATA_TAG, "  data(%d): %s\n", trd->data.len, dataStr);

                sendBluetoothString("data: ");
                sendBluetoothData(trd->data.len*2, (unsigned char *)dataStr);
                sendBluetoothString("\r\n");
            }

            if (0 < trd->userMemData.len)
            {
                ESP_LOGD (DATA_TAG, "inside userMemData \r\n");
                TMR_bytesToHex(trd->userMemData.list, trd->userMemData.len, dataStr);
                ESP_LOGD(DATA_TAG, "  userMemData(%d): %s\n", trd->userMemData.len, dataStr);

                sendBluetoothString("userMemData: ");
                sendBluetoothData(trd->userMemData.len*2, (unsigned char *)dataStr);
                sendBluetoothString("\r\n");
            }

            if (0 < trd->epcMemData.len)
            {
                ESP_LOGD (DATA_TAG, "inside epcMemData \r\n");
                TMR_bytesToHex(trd->epcMemData.list, trd->epcMemData.len, dataStr);
                ESP_LOGD(DATA_TAG, "  epcMemData(%d): %s\n", trd->epcMemData.len, dataStr);

                sendBluetoothString("epcMemData: ");
                sendBluetoothData(trd->epcMemData.len*2, (unsigned char *)dataStr);
                sendBluetoothString("\r\n");
            }

            if (0 < trd->reservedMemData.len)
            {
                ESP_LOGD (DATA_TAG, "inside reservedMemData \r\n");
                TMR_bytesToHex(trd->reservedMemData.list, trd->reservedMemData.len, dataStr);
                ESP_LOGD(DATA_TAG, "  reservedMemData(%d): %s\n", trd->reservedMemData.len, dataStr);

                sendBluetoothString("reservedMemData: ");
                sendBluetoothData(trd->reservedMemData.len*2, (unsigned char *)dataStr);
                sendBluetoothString("\r\n");
            }

            if (0 < trd->tidMemData.len)
            {
                ESP_LOGD (DATA_TAG, "inside tidmemdata \r\n");
                TMR_bytesToHex(trd->tidMemData.list, trd->tidMemData.len, dataStr);
                ESP_LOGD(DATA_TAG, "  tidMemData(%d): %s\n", trd->tidMemData.len, dataStr);

                sendBluetoothString("tidMemData: ");
                sendBluetoothData(trd->tidMemData.len*2, (unsigned char *)dataStr);
                sendBluetoothString("\r\n");
            }

			destroyTagdata(trd);

            ESP_LOGD (DATA_TAG, "\r\n");

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
        esp_spp_write(temp, len, data);

    while (!flag);
}