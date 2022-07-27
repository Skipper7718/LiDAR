#ifndef __STEPPER_H__
#define __STEPPER_H__

typedef struct {
    int gpio_dir;
    int gpio_step;
    int gpio_dir_default;
    int step_delay_ms;
} stepper_t;

stepper_t stepper_configure( int dir_pin, int step_pin );
void stepper_init( stepper_t *config );
void stepper_step( stepper_t *config, int steps, bool reverse );

#endif