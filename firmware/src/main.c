#include "pico/bootrom.h"
#include "pico/stdlib.h"
#include "servo.h"
#include "hardware/i2c.h"
#include "lidar_lite_v3hp.h"
#include "stepper.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define STEP 0.225 // 1.8° stepper, 1/8 microstepping

void stepper_angle( stepper_t *config, float *position, float new_position) {
    float diff = new_position - *position;
    int steps = diff / STEP;
    stepper_step( config, abs(steps), steps >= 0 ? false : true );
    *position = new_position;
}

int main()
{
    // initialize PWM, I2C and Serial
    stdio_usb_init();
    // wait one second, I don't know why but without this the code does not work :(
    sleep_ms(1000);
    printf("PROGRAM START\n");
    servo_init(19, 50);
    init_lidar(16, 17);
    // servo_put(18, 180, true);
    stepper_t stepper = stepper_configure( 15, 14 );
    stepper.step_delay_ms = 5;
    stepper_init( &stepper );
    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);
    // stepper_step(&stepper, 30, true);
    // stepper_step(&stepper, 60, false);
    // stepper_step(&stepper, 30, true);
    // reset_usb_boot(0,0);

    int start_x;
    int stop_x;
    int start_y;
    int stop_y;
    int command;

    float position = 90;
    bool dir;

    stepper.step_delay_ms = 1;
    stepper_angle(&stepper, &position, 45);
    sleep_ms(500);
    stepper_angle(&stepper, &position, 135);
    sleep_ms(500);
    stepper_angle(&stepper, &position, 90);
    sleep_ms(100);
    stepper_angle(&stepper, &position, 80);
    sleep_ms(100);
    stepper_angle(&stepper, &position, 100);
    sleep_ms(100);
    stepper_angle(&stepper, &position, 90);


    for ( ;; ) {
        // home to middle position
        stepper_angle(&stepper, &position, 90);
        servo_put(19, 90, true);
        // read commend from serial
	gpio_put(25, true);
        scanf("%d %d %d %d %d", &command, &start_x, &stop_x, &start_y, &stop_y);
	gpio_put(25, false);
	sleep_ms(1000);
        switch( command ) {
            // normal run mode, run one measurement per angle
            case 1:
                dir = true;
                stepper_angle(&stepper, &position, start_x);
                for(int y = start_y; y <= stop_y; y++ ) {
                    servo_put(19, y, false);
                    for(float x = 0; x <= stop_x - start_x; x+=STEP ) {
                        stepper_step(&stepper, 1, dir ? false : true);
                        // sleep_ms(3);
                        printf("%d\n", lidar_get_measurement());
                        fflush(stdout);
                    }
                    dir = !dir;
                    sleep_ms(20);
                }
                position = stop_x;
                break;

            // quad measurement run mode, run 4 measurements with small delays per angle
            case 2:
                dir = true;
                stepper_angle(&stepper, &position, start_x);
                for(int y = start_y; y <= stop_y; y++ ) {
                    servo_put(19, y, false);
                    for(float x = 0; x <= stop_x - start_x; x+=STEP ) {
                        stepper_step(&stepper, 1, dir ? false : true);
                        // sleep_ms(10);
                        for(int i = 0; i < 4; i++) {
                            printf("%d\n", lidar_get_measurement());
                            fflush(stdout);
                            sleep_ms(1);
                        }
                    }
                    dir = !dir;
                    sleep_ms(30);
                }
                position = stop_x;
                break;

            // go scale to see where the laser will approximately scan
            case 3:
                servo_put(19, start_y, true);
                stepper_angle(&stepper, &position, start_x);
                sleep_ms(500);
                stepper_angle(&stepper, &position, stop_x);
                sleep_ms(500);
                servo_put(19, stop_y, true);
                sleep_ms(500);
                stepper_angle(&stepper, &position, start_x);
                break;

            // park position, ignore the other arguments
            case 4:
                servo_put(19, 50, true);
                stepper_angle(&stepper, &position, 0);
                reset_usb_boot(0,0);
                break; //unnecessary, but looks ok

            
            default:
                break;

        }
    }

    reset_usb_boot(0,0); // reset to BOOTSEL for another flash
}
