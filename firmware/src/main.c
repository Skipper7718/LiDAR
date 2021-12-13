#include "pico/bootrom.h"
#include "pico/stdlib.h"
#include "servo.h"
#include "hardware/i2c.h"
#include "lidar_lite_v3hp.h"
#include <stdbool.h>
#include <stdio.h>

int main()
{
    // initialize PWM, I2C and Serial
    stdio_usb_init();
    // wait one second, I don't know why but without this the code does not work :(
    sleep_ms(1000);
    printf("PROGRAM START\n");
    servo_init(18, 50);
    servo_init(19, 50);
    init_lidar(16, 17);
    // servo_put(18, 180, true);
    // reset_usb_boot(0,0);

    int start_x;
    int stop_x;
    int start_y;
    int stop_y;
    int command;

    for ( ;; ) {
        // home to middle position
        servo_put(18, 90, true);
        servo_put(19, 90, true);
        // read commend from serial
        scanf("%d %d %d %d %d", &command, &start_x, &stop_x, &start_y, &stop_y);
        switch( command ) {
            // normal run mode, run one measurement per angle
            case 1:
                for(int y = start_y; y <= stop_y; y++ ) {
                    servo_put(19, y, false);
                    servo_put(18, start_x, true);
                    for(int x = start_x; x <= stop_x; x++ ) {
                        servo_put(18, x, false);
                        // sleep_ms(500);
                        printf("%d\n", lidar_get_measurement());
                    }
                }
                break;

            // quad measurement run mode, run 4 measurements with small delays per angle
            case 2:
                for(int y = start_y; y <= stop_y; y++ ) {
                    servo_put(19, y, false);
                    servo_put(18, start_x, true);
                    for(int x = start_x; x <= stop_x; x++ ) {
                        servo_put(18, x, false);
                        // sleep_ms(500);
                        for(int i = 0; i < 4; i++) {
                            printf("%d\n", lidar_get_measurement());
                            sleep_ms(2);
                        }
                    }
                }
                break;
            
            // go scale to see where the laser will approximately scan
            case 3:
                servo_put(19, start_y, true);
                servo_put(18, start_x, true);
                sleep_ms(500);
                servo_put(18, stop_x, true);
                sleep_ms(500);
                servo_put(19, stop_y, true);
                sleep_ms(500);
                servo_put(18, start_x, true);
                break;

            // park position, ignore the other arguments
            case 4:
                servo_put(19, 50, true);
                servo_put(18, 10, true);
                reset_usb_boot(0,0);
                break; //unnecessary, but looks ok

            
            default:
                break;

        }
    }

    reset_usb_boot(0,0); // reset to BOOTSEL for another flash
}
