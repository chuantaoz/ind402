/*
IND402
Chuantao Zhang, XJTLU
2022
*/

#include <stdio.h>
#include <string.h>
#include <pico/platform.h>
#include "pico/stdlib.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"
#include "hardware/sync.h"
#include "hardware/uart.h"
#include "pico/sleep.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "blink.pio.h"
//#include "hardware/rct.h"

// Sounds
#include "sounds.h"

// Buttons GPIO
#define BTN1 8
#define BTN2 9
#define BTN3 10
#define BTN4 11

// Audio GPIO
#define AUD1 12

// Radio Switch GPIO
#define RAD1 13

// Light Switch GPIO
#define LIG1 14

// UART GPS GPIO
#define EN0 15
#define UART_TX_PIN 16
#define UART_RX_PIN 17
#define UART_ID uart0
#define BAUD_RATE 9600

// LED Indicator GPIO
#define LED1 18

// Weak Up GPIO
#define WAKEUP 19

// Max Inputs + 1
#define MAXI 13

// Inputs interval (ms)
#define INTERVAL 100

// Input variables
bool key[4];
bool keyRead[4];
int input[MAXI];

// Sounds variables
int audio_pin_slice;
const uint8_t *wav_play;
int wav_index = 0;
int wav_count = 0;
int wav_length;
int wav_position = 0;
const uint8_t *wav[19] = {
    WAV_0,
    WAV_1,
    WAV_2,
    WAV_3,
    WAV_4,
    WAV_5,
    WAV_6,
    WAV_7,
    WAV_8,
    WAV_9,
    WAV_POINT,
    WAV_WEST,
    WAV_EAST,
    WAV_NORTH,
    WAV_SOUTH,
    WAV_SOS,
    WAV_POSITION,
    WAV_UNKNOWN,
    WAV_EMPTY
};
 int wav_l[19] = {
    WAV_0_LENGTH,
    WAV_1_LENGTH,
    WAV_2_LENGTH,
    WAV_3_LENGTH,
    WAV_4_LENGTH,
    WAV_5_LENGTH,
    WAV_6_LENGTH,
    WAV_7_LENGTH,
    WAV_8_LENGTH,
    WAV_9_LENGTH,
    WAV_POINT_LENGTH,
    WAV_WEST_LENGTH,
    WAV_EAST_LENGTH,
    WAV_NORTH_LENGTH,
    WAV_SOUTH_LENGTH,
    WAV_SOS_LENGTH,
    WAV_POSITION_LENGTH,
    WAV_UNKNOWN_LENGTH,
    WAV_EMPTY_LENGTH
};

//int wav_list[23] = {15, 16, 3, 6, 1, 1, 10, 2, 4, 8, 1, 13, 1, 1, 7, 0, 8, 10, 5, 5, 3, 1, 12};
int wav_list[23];

// GPS variables
//char buffer[1024] = "$GNGGA,095528.000,2318.1133,N,11319.7210,E,1,06,3.7,55.1,M,-5.4,M,,0000*69";
char buffer[1024];
char *gpsstr[7];

// PIO variables
PIO pio = pio0;
uint offset;

void read_gps() {
    do {
        scanf("%1024s", buffer);
    }
    while (buffer[3] != 'G' || buffer[4] != 'G' || buffer[5] != 'A');
    printf( "%s\n", buffer);
    char *h = buffer;
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < strlen(h); j++) {
            if(*(h + j) == ',') {
                gpsstr[i] = h;
                h = h + j + 1;
                *(gpsstr[i]+ j) = '\0';
                break;
            }
        }
    }
    /*
    0   $**GGA
    1   UTC time    hhmmss.ss
    2   Latitude    ddmm.mmmm
    3   LatiDir     N/S
    4   Longitude   dddmm.mmmm
    5   LongDir     W/E
    6   Condition   0=Not,1=Differential,2=Undifferential
    */

    //TEST
    /*
    *gpsstr[0] = "$GNGGA";
    *gpsstr[1] = "0400.0000";
    *gpsstr[2] = "3611.2481";
    *gpsstr[3] = "N";
    *gpsstr[4] = "11708.5531";
    *gpsstr[5] = "E";
    *gpsstr[6] = "1";
    */
    /*
    for (int i = 0; i < 7; i++) {
        printf( "%s\n", gpsstr[i]);
    }
    */
    if(*gpsstr[6] == '0') {
        wav_list[0] = 15;//SOS
        wav_list[1] = 16;//Position
        wav_list[2] = 17;//Unknown
        wav_list[3] = 18;
        wav_list[4] = 18;
        wav_list[5] = 18;
        wav_list[6] = 18;
        wav_list[7] = 18;
        wav_list[8] = 18;
        wav_list[9] = 18;
        wav_list[10] = 18;
        wav_list[11] = 18;
        wav_list[12] = 18;
        wav_list[13] = 18;
        wav_list[14] = 18;
        wav_list[15] = 18;
        wav_list[16] = 18;
        wav_list[17] = 18;
        wav_list[18] = 18;
        wav_list[19] = 18;
        wav_list[20] = 18;
        wav_list[21] = 18;
        wav_list[22] = 18;
    }
    else {
        wav_list[0] = 15;
        wav_list[1] = 16;
        wav_list[2] = *gpsstr[2] - '0';
        wav_list[3] = *(gpsstr[2] + 1) - '0';
        wav_list[4] = *(gpsstr[2] + 2) - '0';
        wav_list[5] = *(gpsstr[2] + 3) - '0';
        wav_list[6] = 10;
        wav_list[7] = *(gpsstr[2] + 5) - '0';
        wav_list[8] = *(gpsstr[2] + 6) - '0';
        wav_list[9] = *(gpsstr[2] + 7) - '0';
        wav_list[10] = *(gpsstr[2] + 8) - '0';
        if (*gpsstr[3] == 'N') {
            wav_list[11] = 13;
        }
        else {
            wav_list[11] = 14;
        }
        wav_list[12] = *gpsstr[4] - '0';
        wav_list[13] = *(gpsstr[4] + 1) - '0';
        wav_list[14] = *(gpsstr[4] + 2) - '0';
        wav_list[15] = *(gpsstr[4] + 3) - '0';
        wav_list[16] = *(gpsstr[4] + 4) - '0';
        wav_list[17] = 10;
        wav_list[18] = *(gpsstr[4] + 6) - '0';
        wav_list[19] = *(gpsstr[4] + 7) - '0';
        wav_list[20] = *(gpsstr[4] + 8) - '0';
        wav_list[21] = *(gpsstr[4] + 9) - '0';
        if (*gpsstr[5] == 'W') {
            wav_list[22] = 11;
        }
        else {
            wav_list[22] = 12;
        }
    }
}

// PWM audio handler
void pwm_interrupt_handler() {
    wav_play = wav[wav_list[wav_index]];
    wav_length = wav_l[wav_list[wav_index]];
    pwm_clear_irq(pwm_gpio_to_slice_num(AUD1));    
    if (wav_count > 6) {
        if (wav_position < wav_length - 1) { 
            pwm_set_gpio_level(AUD1, *(wav_play + wav_position));  
            wav_position++;
            wav_count = 0;
        } 
        else {
            wav_position = 0;
            wav_count = 0;
            if (wav_index < 22){
                wav_index++;
            }
            else {
                wav_index = 0;
                read_gps();
            }
        }
    }
    else {
        wav_count++;
    }
}

void read_key(int k) 
{
    int n;
    switch (k) {
    case 1:
        n = BTN1;
        break;
    case 2:
        n = BTN2;
        break;
    case 3:
        n = BTN3;
        break;
    case 4:
        n = BTN4;
        break;
    default:
        break;
    }

    // State changed
    if (gpio_get(n) != key[k - 1]) {
        // Push
        if (key[k - 1]) {
            key[k - 1] = false;
            keyRead[k - 1] = true;
        }
        // Release
        else {
            key[k - 1] = true;
        }
    }
    // State unchanged
    else {
        // Hold
        if (!key[k - 1] && keyRead[k - 1] && input[0] < (MAXI - 1)) {
            input[input[0] + 1] = k;
            input[0]++;
            keyRead[k - 1] = false;
        }
        else {

        }
    }
}

void blink(PIO pio, uint sm, uint offset, uint pin, uint freq) {
    blink_program_init(pio, sm, offset, pin);
    pio_sm_set_enabled(pio, sm, true);
    // PIO counter program takes 3 more cycles in total than we pass as
    // input (wait for n + 1; mov; jmp)
    pio->txf[sm] = (clock_get_hz(clk_sys) / 65535 / (2 * freq)) - 3;
}

void light_switch() {
    if (gpio_get(LIG1)) {
        gpio_put(LIG1, 0);
        //printf("LIGHT OFF\n");
    }
    else {
        gpio_put(LIG1, 1);
        //printf("LIGHT ON\n");
    }
}

void emg_switch() {
    if (gpio_get(RAD1)) {
        gpio_put(RAD1, 0);
        //printf("EMG OFF\n");
        pio_sm_set_enabled(pio, 0, false);
        pio_sm_set_pins(pio, 0, 0);
        pwm_set_gpio_level(AUD1, 0);
        pwm_set_enabled(audio_pin_slice, false);
    }
    else {
        gpio_put(RAD1, 1);
        //printf("EMG ON\n");
        blink(pio, 0, offset, 25, 1);
        read_gps();
        wav_position = 0;
        wav_index = 0;
        pwm_set_enabled(audio_pin_slice, true);
    }
}

int main() 
{
    stdio_init_all();

    // UART init
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // Buttons init
    gpio_init(BTN1);
    gpio_set_dir(BTN1, GPIO_IN);
    gpio_pull_up(BTN1);
    gpio_init(BTN2);
    gpio_set_dir(BTN2, GPIO_IN);
    gpio_pull_up(BTN2);
    gpio_init(BTN3);
    gpio_set_dir(BTN4, GPIO_IN);
    gpio_pull_up(BTN3);
    gpio_init(BTN4);
    gpio_set_dir(BTN4, GPIO_IN);
    gpio_pull_up(BTN4);

    // Audio init
    gpio_set_function(AUD1, GPIO_FUNC_PWM);
    audio_pin_slice = pwm_gpio_to_slice_num(AUD1);
    pwm_clear_irq(audio_pin_slice);
    pwm_set_irq_enabled(audio_pin_slice, true);
    irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_interrupt_handler); 
    irq_set_enabled(PWM_IRQ_WRAP, true);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 8.0f); 
    pwm_config_set_wrap(&config, 250); 
    pwm_init(audio_pin_slice, &config, false);
    
    // Radio Switch init
    gpio_init(RAD1);
    gpio_set_dir(RAD1, GPIO_OUT);
    gpio_put(RAD1, 0);

    // Light Switch init
    gpio_init(LIG1);
    gpio_set_dir(LIG1, GPIO_OUT);
    gpio_put(LIG1, 0);

    // UART GPS init
    gpio_init(EN0);
    gpio_set_dir(EN0, GPIO_OUT);
    gpio_put(EN0, 0);

    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // LED Indicator init
    gpio_init(LED1);
    gpio_set_dir(LED1, GPIO_OUT);
    gpio_put(LED1, 0);

    // WAKEUP init
    gpio_init(WAKEUP);
    gpio_set_dir(WAKEUP, GPIO_IN);

    // PIO init
    PIO pio = pio0;
    offset = pio_add_program(pio, &blink_program);

    // Input variables init
    for (int i = 0; i < 4; i++) {
        key[i] =  true;
    }
    for (int i = 0; i < 4; i++) {
        keyRead[i] =  false;
    }
    for (int i = 0; i < MAXI; i++) {
        input[i] =  0;
    }

    printf("Initialization Finished\n");

    while(1) {
        // Go sleep until WAKEUP is high
        //sleep_run_from_xosc();
        //sleep_goto_dormant_until_edge_high(WAKEUP);
        
        // Start input
        int idle = 0;
        while (1) {
            int lastInput0 = input[0];
            for (int k = 1; k <= 4; k++) {
                read_key(k);
            }

            if (input[0] == 0) {
                
            }
            else if (input[0] == lastInput0) {
                idle++;
            }
            else {
                idle = 0;
            }

            // End input
            if (idle >= 5) {
                if (input[0] == 4 && input[1] == 1 && input[2] == 2 && input[3] == 3 && input[4] == 4) {
                    light_switch();
                    break;
                }
                else if (input[0] == 4 && input[1] == 4 && input[2] == 3 && input[3] == 2 && input[4] == 1) {
                    light_switch();
                    break;
                }
                 else if (input[0] == 7 && input[1] == 1 && input[2] == 2 && input[3] == 3 && input[4] == 4 && input[5] == 3 && input[6] == 2 && input[7] == 1) {
                    emg_switch();
                    break;
                }
                else if (input[0] == 7 && input[1] == 4 && input[2] == 3 && input[3] == 2 && input[4] == 1 && input[5] == 2 && input[6] == 3 && input[7] == 4) {
                    emg_switch();
                    break;
                }
                else {
                    break;
                }
            }   

            sleep_ms(INTERVAL);
        }
        
        // Reset input variables
        for (int i = 0; i < 4; i++) {
            key[i] =  true;
        }
        for (int i = 0; i < 4; i++) {
            keyRead[i] =  false;
        }
        for (int i = 0; i < MAXI; i++) {
            input[i] =  0;
        }
    }

    return 0;
}