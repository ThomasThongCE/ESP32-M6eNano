/**
 *  @file serial_transport_dummy.c
 *  @brief Mercury API - Sample serial transport that does nothing
 *  @author Nathan Williams
 *  @date 2/24/2010
 */


/*
 * Copyright (c) 2010 ThingMagic, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "tm_reader.h"

#define BUF_SIZE (1024)
//#define ECHO_TEST_TXD  (GPIO_NUM_1)
//#define ECHO_TEST_RXD  (GPIO_NUM_3)
#define ECHO_TEST_TXD  (GPIO_NUM_10)
#define ECHO_TEST_RXD  (GPIO_NUM_9)
#define ECHO_TEST_RTS  (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS  (UART_PIN_NO_CHANGE)
/* Stub implementation of serial transport layer routines. */

static TMR_Status
s_open(TMR_SR_SerialTransport *this)
{

  /* This routine should open the serial connection */

	uart_config_t uart_config = {
		.baud_rate = 115200,
		.data_bits = UART_DATA_8_BITS,
		.parity    = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE
	};
	uart_param_config(UART_NUM_1, &uart_config);
	uart_set_pin(UART_NUM_1, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS);
	uart_driver_install(UART_NUM_1, BUF_SIZE * 2, 0, 0, NULL, 0);

  return TMR_SUCCESS;
}


static TMR_Status
s_sendBytes(TMR_SR_SerialTransport *this, uint32_t length, 
                uint8_t* message, const uint32_t timeoutMs)
{

  /* This routine should send length bytes, pointed to by message on
   * the serial connection. If the transmission does not complete in
   * timeoutMs milliseconds, it should return TMR_ERROR_TIMEOUT.
   */
	uart_write_bytes(UART_NUM_1, (const char *) message, length);

	return TMR_SUCCESS;
}


static TMR_Status
s_receiveBytes(TMR_SR_SerialTransport *this, uint32_t length, 
                   uint32_t* messageLength, uint8_t* message, const uint32_t timeoutMs)
{

  /* This routine should receive exactly length bytes on the serial
   * connection and store them into the memory pointed to by
   * message. If the required number of bytes are note received in
   * timeoutMs milliseconds, it should return TMR_ERROR_TIMEOUT.
   */
	int len = uart_read_bytes(UART_NUM_1, message, length, timeoutMs / portTICK_RATE_MS);
	*messageLength = len;
	if (len < length)
		return TMR_ERROR_TIMEOUT;
	return TMR_SUCCESS;
}


static TMR_Status
s_setBaudRate(TMR_SR_SerialTransport *this, uint32_t rate)
{
	esp_err_t ret = NULL;

	uart_config_t uart_config = {
			.baud_rate = rate,
			.data_bits = UART_DATA_8_BITS,
			.parity    = UART_PARITY_DISABLE,
			.stop_bits = UART_STOP_BITS_1,
			.flow_ctrl = UART_HW_FLOWCTRL_DISABLE
		};

	ret = uart_param_config(UART_NUM_1, &uart_config);
	if (ret == ESP_FAIL)
		return TMR_ERROR_INVALID;
	return TMR_SUCCESS;
  /* This routine should change the baud rate of the serial connection
   * to the specified rate, or return TMR_ERROR_INVALID if the rate is
   * not supported.
   */


}


static TMR_Status
s_shutdown(TMR_SR_SerialTransport *this)
{

  /* This routine should close the serial connection and release any
   * acquired resources.
   */
	uart_driver_delete(UART_NUM_1);

  return TMR_SUCCESS;
}

static TMR_Status
s_flush(TMR_SR_SerialTransport *this)
{
	uart_flush(UART_NUM_1);
	return TMR_SUCCESS;
  /* This routine should empty any input or output buffers in the
   * communication channel. If there are no such buffers, it may do
   * nothing.
   */
}



/* This function is not part of the API as such. This is for
 * application code to call to fill in the transport object before
 * initializing the reader object itself, as in the following code:
 * 
 * TMR_Reader reader;
 *
 * TMR_SR_SerialTransportDummyInit(&reader.u.serialReader.transport, myArgs);
 * TMR_SR_SerialReader_init(&reader);
 *
 * The initialization should not actually open a communication channel
 * or acquire other communication resources at this time.
 */
TMR_Status
TMR_SR_SerialTransportDummyInit(TMR_SR_SerialTransport *transport,
							 void *other)
{

  /* Each of the callback functions will be passed the transport
   * pointer, and they can use the "cookie" member of the transport
   * structure to store the information specific to the transport,
   * such as a file handle or the memory address of the FIFO.
   */


  return TMR_SUCCESS;
}

/**
 * Initialize a TMR_SR_SerialTransport structure with a given serial device.
 *
 * @param transport The TMR_SR_SerialTransport structure to initialize.
 * @param context A TMR_SR_SerialPortNativeContext structure for the callbacks to use.
 * @param device The path or name of the serial device (@c /dev/ttyS0, @c COM1)
 */
TMR_Status
TMR_SR_SerialTransportNativeInit(TMR_SR_SerialTransport *transport,
                                 TMR_SR_SerialPortNativeContext *context,
                                 const char *device)
{
  transport->cookie = context;

  transport->open = s_open;
  transport->sendBytes = s_sendBytes;
  transport->receiveBytes = s_receiveBytes;
  transport->setBaudRate = s_setBaudRate;
  transport->shutdown = s_shutdown;
  transport->flush = s_flush;

#if TMR_MAX_SERIAL_DEVICE_NAME_LENGTH > 0
  if (strlen(device) + 1 > TMR_MAX_SERIAL_DEVICE_NAME_LENGTH)
  {
    return TMR_ERROR_INVALID;
  }
  strcpy(context->devicename, device);
  return TMR_SUCCESS;
#else
  //return s_open(transport);
  return TMR_SUCCESS;
#endif
}

