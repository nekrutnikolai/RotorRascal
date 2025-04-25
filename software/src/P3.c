#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "pico/binary_info.h"
#include "../lib/MPU9250/mpu9250.h"
#include "../lib/ESC/esc.h"
#include "../lib/Filter/leaky_LP.h"
#include "../lib/Control/controller.h"
#include "../lib/Radio/radio.h"
#include "pico/cyw43_arch.h"
#include <math.h>
#include <time.h>

// IMU Connection (same)
#define PIN_MISO 4
#define PIN_CS 5
#define PIN_SCK 6
#define PIN_MOSI 7

// ESC
#define PWM_WRAP 100000
#define PWM_FREQ 50
#define MAX_DUTY 0.1
#define MIN_DUTY 0.05

#define PIN_PWM1 21
#define PIN_PWM2 20
#define PIN_PWM3 19
#define PIN_PWM4 18

// RADIO
#define RADIO_TX 8
#define RADIO_RX 9

// PID Defaults
#define Default_Kp 15
#define Default_Ki 0
#define Default_Kd 0

#define Default_yaw_Kp 0
#define Default_yaw_Ki 0
#define Default_yaw_Kd 0

#define INIT_THROTTLE 5.0

static mpu9250 imu;
static ESC esc;
static leaky_lp w_filter;
static leaky_lp a_filter;
static controller fc;
static radio rdo;

// control cycle timing
#define dT_ms 1
#define dT_s (dT_ms / 1000.0)

volatile bool new_input = false;

char serial_buffer[64];
int serial_buffer_index = 0;

void print_pid_values() {
    printf("Current PID values:\n");
    printf("Kp = %.3f, Ki = %.3f, Kd = %.3f\n", fc.Kp, fc.Ki, fc.Kd);
}

void handle_serial_input() {
    int c;
    while ((c = getchar_timeout_us(0)) != PICO_ERROR_TIMEOUT) {
        if (c == '\n' || c == '\r') {
            if (serial_buffer_index > 0) {
                serial_buffer[serial_buffer_index] = '\0';

                if (strncmp(serial_buffer, "kp=", 3) == 0) {
                    fc.Kp = atof(serial_buffer + 3);
                    printf("Updated Kp to %.3f\n", fc.Kp);
                } else if (strncmp(serial_buffer, "ki=", 3) == 0) {
                    fc.Ki = atof(serial_buffer + 3);
                    printf("Updated Ki to %.3f\n", fc.Ki);
                } else if (strncmp(serial_buffer, "kd=", 3) == 0) {
                    fc.Kd = atof(serial_buffer + 3);
                    printf("Updated Kd to %.3f\n", fc.Kd);
                } else {
                    printf("Invalid command. Use 'kp=<value>', 'ki=<value>', or 'kd=<value>'.\n");
                }
                print_pid_values();
                serial_buffer_index = 0;
                new_input = true;
            }
        } else {
            if (serial_buffer_index < sizeof(serial_buffer) - 1) {
                serial_buffer[serial_buffer_index++] = (char)c;
            }
        }
    }
}

int main() {
    stdio_init_all();  // USB serial init

    // Wait for USB connection if needed
    sleep_ms(2000);

    // Radio (still on UART1 GPIO8/9)
    radio_setup(&rdo, RADIO_RX, RADIO_TX);

    // IMU
    mpu9250_setup(&imu, PIN_CS, PIN_MISO, PIN_SCK, PIN_MOSI);
    printf("Calibrating Gyro....Keep it Still...\n");
    gyro_cal(&imu, 50);
    acc_cal(&imu, 50);
    printf("Done. Offsets: w_x: %.5f, w_y:%.5f, w_z:%.5f\n", imu.w_offsets[0], imu.w_offsets[1], imu.w_offsets[2]);

    // ESC
    uint32_t PWM_PIN[4] = {PIN_PWM1, PIN_PWM2, PIN_PWM3, PIN_PWM4};
    esc_setup(&esc, PWM_PIN, PWM_FREQ, PWM_WRAP, MIN_DUTY, MAX_DUTY);

    printf("Calibrating ESC...\n");
    cali_motor(&esc);
    printf("Done.\n");

    printf("Arming ESC...\n");
    arm_motor(&esc);
    printf("Done.\n");

    leaky_init(&w_filter, 0.9f);
    leaky_init(&a_filter, 0.9f);
    init_controller(&fc, 0.98f, dT_s, Default_Kp, Default_Ki, Default_Kd, Default_yaw_Kp, Default_yaw_Ki, Default_yaw_Kd);

    // Wi-Fi init (for LED control)
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed\n");
        return -1;
    }
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

    // Blink ready
    sleep_ms(100);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    sleep_ms(100);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

    while (1) {
        absolute_time_t startTime = get_absolute_time();

        handle_serial_input();

        mpu9250_update(&imu);
        float wf[3];
        float af[3];
        leaky_update(&w_filter, imu.w, wf);
        leaky_update(&a_filter, imu.a, af);

        update_controller(&fc, wf, af);

        if (new_input) {
            new_input = false;
            printf("roll: %.3f, pitch: %.3f, yaw: %.3f\n", fc.x.roll, fc.x.pitch, fc.x.yaw);
            printf("Kp = %.3f, Ki = %.3f, Kd = %.3f, throttle = %.3f, %s\n", 
                    fc.Kp, fc.Ki, fc.Kd, rdo.throttle, 
                    rdo.controller_armed ? "ARMed" : "NOT-ARMed");
        }

        if (rdo.controller_armed) {
            float base_throttle = INIT_THROTTLE + rdo.throttle * ((100.0 - INIT_THROTTLE) / 100.0);

            float motor1_throttle = saturate(base_throttle + fc.u.t1 / 10.0, INIT_THROTTLE, 100.0);
            float motor2_throttle = saturate(base_throttle + fc.u.t2 / 10.0, INIT_THROTTLE, 100.0);
            float motor3_throttle = saturate(base_throttle + fc.u.t3 / 10.0, INIT_THROTTLE, 100.0);
            float motor4_throttle = saturate(base_throttle + fc.u.t4 / 10.0, INIT_THROTTLE, 100.0);

            motor_control(&esc, motor1_throttle, 1);
            motor_control(&esc, motor2_throttle, 2);
            motor_control(&esc, motor3_throttle, 3);
            motor_control(&esc, motor4_throttle, 4);
        } else {
            motor_control(&esc, 0, 1);
            motor_control(&esc, 0, 2);
            motor_control(&esc, 0, 3);
            motor_control(&esc, 0, 4);
        }

        absolute_time_t endTime = get_absolute_time();
        int64_t time_diff_ms = absolute_time_diff_us(startTime, endTime) / 1000;
        uint32_t sleep_time_ms = dT_ms > time_diff_ms ? (dT_ms - time_diff_ms) : 0;
        sleep_ms(sleep_time_ms);
    }
}