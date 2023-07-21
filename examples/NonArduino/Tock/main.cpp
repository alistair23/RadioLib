/*
  RadioLib Non-Arduino Tock Library test application

  Licensed under the MIT License

  Copyright (c) 2023 Alistair Francis <alistair@alistair23.me>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

// include the library
#include <RadioLib.h>

// include the hardware abstraction layer
#include "libtockHal.h"

// Include some libtock-c helpers
#include <libtock/temperature.h>
#include <libtock/humidity.h>

#define BUFFER_LEN 64

// Define if we should send or receieve data
// #define SEND

// the entry point for the program
int main(void) {
  char buffer[BUFFER_LEN];

  printf("[SX1261] Initialising Radio ... \n");

  // create a new instance of the HAL class
  TockHal* hal = new TockHal();

  // now we can create the radio module
  // pinout corresponds to the SparkFun LoRa Thing Plus - expLoRaBLE
  // NSS pin:   0
  // DIO1 pin:  2
  // NRST pin:  4
  // BUSY pin:  1
  Module* tock_module = new Module(hal, RADIO_NSS, RADIO_DIO_1, RADIO_RESET, RADIO_BUSY);
  SX1262* radio = new SX1262(tock_module);

  // Setup the radio
  int state = radio->begin(915.0, 125.0, 7, 5, RADIOLIB_SX126X_SYNC_WORD_PRIVATE, 0, 8, 1.7, true);

  if (state != RADIOLIB_ERR_NONE) {
    printf("failed, code %d\r\n", state);
    return 1;
  }
  printf("success!\r\n");

#ifdef SEND
  int temp = 0;
  unsigned humi = 0;

  // loop forever
  for(;;) {
    // Ensure there are no pending callbacks
    yield_no_wait();

    // Read some sensor data from the board
    temperature_read_sync(&temp);
    humidity_read_sync(&humi);

    snprintf(buffer, BUFFER_LEN, "Temp: %d, Hum: %u", temp, humi);

    // send a packet
    printf("[SX1261] Transmitting '%s' \r\n", buffer);

    state = radio->transmit(buffer);

    if(state == RADIOLIB_ERR_NONE) {
      // the packet was successfully transmitted
      printf("success!\r\n");

      // wait for a second before transmitting again
      hal->delay(1000);
    } else {
      printf("failed, code %d\r\n", state);
    }
  }
#else /* SEND */
  printf("[SX1261] Receiving...\r\n");

  // loop forever
  for(;;) {
    // Ensure there are no pending callbacks
    yield_no_wait();

    state = radio->receive((uint8_t*)buffer, BUFFER_LEN);

    if(state == RADIOLIB_ERR_NONE) {
      // the packet was successfully transmitted
      printf("success!: %s\r\n", buffer);

      // wait for a second before transmitting again
      hal->delay(1000);
    } else {
      // printf("failed, code %d\r\n", state);
    }
  }
#endif

  return 0;
}
