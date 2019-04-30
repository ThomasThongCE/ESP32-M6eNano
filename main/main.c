/* UART Echo Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
//#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
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

TMR_Status
TMR_SR_SerialTransportDummyInit(TMR_SR_SerialTransport *transport,
								void *other);

#define BUF_SIZE (1024)
uint8_t *data;
uint8_t myEPC[12]; //Most EPCs are 12 bytes
int myEPClength;
int responseType = 0;
static void echo_task()
{
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_0, &uart_config);
    uart_set_pin(UART_NUM_0, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS);
    uart_driver_install(UART_NUM_0, BUF_SIZE * 2, 0, 0, NULL, 0);

     //Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);

//    while (1) {
//        // Read data from the UART
//    	char *a = "asdasdasd\n";
//    	uart_write_bytes(UART_NUM_0, (const char *) a, 10);
//        //int len = uart_read_bytes(UART_NUM_0, data, BUF_SIZE, 20 / portTICK_RATE_MS);
//        // Write data back to the UART
//        //uart_write_bytes(UART_NUM_0, (const char *) data, len);
//    }
}

void app_main()
{
	char *msg1 = "\nerror in TMR_read\n";
	char *msg2 = "\nerror in TMR_getNextTag\n";
	char *msg3 = "haha\n";

	echo_task();

	char *a = "asdasdasd\n";
	    	uart_write_bytes(UART_NUM_0, (const char *) a, 10);

	int tagCount;
	TMR_Reader reader ;
	//reader = (TMR_Reader *) malloc(sizeof(TMR_Reader));
	TMR_SR_SerialTransportDummyInit(&reader.u.serialReader.transport, NULL);
	uart_write_bytes(UART_NUM_0, (const char *) a, 10);
	TMR_SR_SerialReader_init(&reader);
	uart_write_bytes(UART_NUM_0, (const char *) a, 10);

	TMR_Status err = TMR_read(&reader, 1000, &tagCount);
	uart_write_bytes(UART_NUM_0, (const char *) a, 10);
	if (TMR_SUCCESS != err) {
		uart_write_bytes(UART_NUM_0, (const char *)msg1, 19);
		//uart_write_bytes(UART_NUM_0, err, 1);
	};
	while (TMR_SUCCESS == TMR_hasMoreTags(&reader))
	{
		TMR_TagReadData trd;
		err = TMR_getNextTag(&reader, &trd);
		if (TMR_SUCCESS != err) {
			uart_write_bytes(UART_NUM_0, (const char *)msg2, 25);
			//uart_write_bytes(UART_NUM_0, err, 1);
		}
		else
			uart_write_bytes(UART_NUM_0,(const char *)msg3 , 5);
	}
}
