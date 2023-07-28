#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "app_uart.h"
#include "app_error.h"
#include "nrf_delay.h"
#include "nrf.h"
#include "bsp.h"
#if defined (UART_PRESENT)
#include "nrf_uart.h"
#endif
#if defined (UARTE_PRESENT)
#include "nrf_uarte.h"
#endif

#define MAX_TEST_DATA_BYTES (15U) /**< max number of test bytes to be used for tx and rx. */
#define UART_TX_BUF_SIZE 256 /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE 256 /**< UART RX buffer size. */

void uart_error_handle(app_uart_evt_t * p_event)
{
if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
{
APP_ERROR_HANDLER(p_event->data.error_communication);
}
else if (p_event->evt_type == APP_UART_FIFO_ERROR)
{
APP_ERROR_HANDLER(p_event->data.error_code);
}
}



#define UART_HWFC APP_UART_FLOW_CONTROL_DISABLED


int main(void)
{
uint32_t err_code;

const app_uart_comm_params_t comm_params =
{
RX_PIN_NUMBER,
TX_PIN_NUMBER,
RTS_PIN_NUMBER,
CTS_PIN_NUMBER,
UART_HWFC,
false,
#if defined (UART_PRESENT)
NRF_UART_BAUDRATE_9600
#else
NRF_UARTE_BAUDRATE_9600
#endif
};

APP_UART_FIFO_INIT(&comm_params,
UART_RX_BUF_SIZE,
UART_TX_BUF_SIZE,
uart_error_handle,
APP_IRQ_PRIORITY_LOWEST,
err_code);

APP_ERROR_CHECK(err_code);


printf("\r\nUART example started.\r\n");

    
  char rx_data;
    char nmea_buffer[100];
    int nmea_index = 0;
    bool is_gpgga = false;

    while (true) {
        if (app_uart_get((uint8_t *)&rx_data) == NRF_SUCCESS) {
            if (rx_data == '$') {
                nmea_buffer[0] = rx_data;
                nmea_index = 1;
                is_gpgga = false;
            } else {
                if (nmea_index > 0) {
                    nmea_buffer[nmea_index] = rx_data;
                    nmea_index++;

                    if (nmea_index == 6 && memcmp(nmea_buffer, "$GPGGA", 6) == 0) {
                        is_gpgga = true;
                    }

                    if (is_gpgga) {
                        if (rx_data == '\n') {
                            nmea_buffer[nmea_index] = '\0';

                            // $GPGGA satýrýnýn içindeki veriyi yazdýr
                            char *data = strtok(nmea_buffer, ",");
                            int count = 0;

                            while (data != NULL && count <= 6) {
                                count++;
                                if (count >= 3 && count <= 6) {
                                    printf("%s", data);
                                    if (count < 6) {
                                        printf(",");
                                    }
                                }
                                data = strtok(NULL, ",");
                            }

                            printf("\r\n");
                            nmea_index = 0;
                            is_gpgga = false;
                        }
                    }
                }
            }
        }
    }
}


