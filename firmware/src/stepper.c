#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "stepper.h"

stepper_t stepper_configure( int dir_pin, int step_pin ) {
    stepper_t config = {
        .gpio_dir = dir_pin,
        .gpio_step = step_pin,
        .gpio_dir_default = true,
        .step_delay_ms = 1
    };
    return config;
}

void stepper_init( stepper_t *config ) {
    gpio_init( config->gpio_dir );
    gpio_init( config->gpio_step );
    gpio_set_dir( config->gpio_dir, GPIO_OUT );
    gpio_set_dir( config->gpio_step, GPIO_OUT );
    gpio_pull_down( config->gpio_dir );
    gpio_pull_down( config->gpio_step );
}

void stepper_step( stepper_t *config, int steps, bool reverse ) {
    gpio_put(
        config->gpio_dir,
        reverse ? !config->gpio_dir_default : config->gpio_dir_default
    );
    for( int i = 0; i < steps; i++ ) {
        gpio_put( config->gpio_step, true );
        sleep_ms( config->step_delay_ms );
        gpio_put( config->gpio_step, false );
        sleep_ms( config->step_delay_ms );
    }
}