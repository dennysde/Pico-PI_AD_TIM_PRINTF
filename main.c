/*
 *Author: Dennys Ramos
 *Date: Oct/8th/2023
 *Brief: Através de Um Timer periódico, Leia a temperatura da CPU e a envie via serial para o computador;
*/

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

/* Choose 'C' for Celsius or 'F' for Fahrenheit. */
#define TEMPERATURE_UNITS 'C'

volatile bool led_status = false; 
const uint led_pin = 25;

/* References for this implementation:
 * raspberry-pi-pico-c-sdk.pdf, Section '4.1.1. hardware_adc'
 * pico-examples/adc/adc_console/adc_console.c */
float read_onboard_temperature(const char unit) {
    
    /* 12-bit conversion, assume max value == ADC_VREF == 3.3 V */
    const float conversionFactor = 3.3f / (1 << 12);

    float adc = (float)adc_read() * conversionFactor;
    float tempC = 27.0f - (adc - 0.706f) / 0.001721f;

    if (unit == 'C') {
        return tempC;
    } else if (unit == 'F') {
        return tempC * 9 / 5 + 32;
    }

    return -1.0f;
}

bool per_timer_callback(struct repeating_timer *t) {

    float temperature = read_onboard_temperature(TEMPERATURE_UNITS);
    printf("Onboard temperature = %.02f %c\n", temperature, TEMPERATURE_UNITS);

    led_status = !led_status;
    gpio_put(led_pin, led_status);

    return true;
}


int main() {

    // Initialize chosen serial port
    stdio_init_all();

    // Initialize LED pin
    gpio_init(led_pin);
    gpio_set_dir(led_pin, GPIO_OUT);

    //Create periodic timer
    struct repeating_timer timer;

    /* Initialize hardware AD converter, enable onboard temperature sensor and
     *   select its channel */
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4);

    //Wait to be able to open terminal
    sleep_ms(5000);

    // Loop forever
    while (true) {

        gpio_put(led_pin, false);
        int tempo = 0;


        do {
            printf("Insira o Tempo para o timer em Milissegundos, para cancelar digite -1\n");
        } while (scanf("%d", &tempo) < 0 && tempo == 0);

        if (tempo == -1) {
            cancel_repeating_timer(&timer);
            continue;
        }


        add_repeating_timer_ms(tempo, &per_timer_callback, NULL, &timer);

    }
    return 0;
}