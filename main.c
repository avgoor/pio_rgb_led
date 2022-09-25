#include "stdio.h"

#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/pwm.h"
#include "pwm.pio.h"

const uint16_t period = 381u;

#define LEDS_COUNT  9

struct _rgb_led {
    uint8_t cmn;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} leds[LEDS_COUNT] = {
    {15, 7, 8, 9},
    {14, 7, 8, 9},
    {13, 7, 8, 9},
    {12, 7, 8, 9},
    {11, 7, 8, 9},
    {10, 13, 14, 15},
    {9, 13, 14, 15},
    {8, 13, 14, 15},
    {7, 13, 14, 15}
};

struct _led_brightness {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} leds_brightness[LEDS_COUNT] = {
    {126, 0, 0},
    {0, 126, 0},
    {0, 0, 126},
    {1, 0, 1},
    {0, 0, 0},
    {1, 5, 5},
    {0, 0, 0},
    {1, 1, 1},
    {0,0,0}
};

static volatile int position = 0;
static PIO pio = pio0;
static int sm = 0;


void pio_pwm_set_levels(PIO pio, uint sm, uint32_t red, uint32_t green, uint32_t blue) {
    uint32_t levels = (period - (red + green + blue)) | (red << 25) | (green << 18) | (blue << 11);
    pio_sm_put_blocking(pio, sm, levels);
}

void gpio_callback(uint gpio, uint32_t events) {
    printf("Button pushed, reboot to bootloader.\n");
    reset_usb_boot(0, 0);
}

void draw_next_led(){
    //pio_sm_set_enabled(pio, sm, false);

    //gpio_set_dir_all_bits(0);
    // gpio_set_function(leds[position].red, GPIO_FUNC_SIO);
    // gpio_set_function(leds[position].green, GPIO_FUNC_SIO);
    // gpio_set_function(leds[position].blue, GPIO_FUNC_SIO);
    // gpio_set_function(leds[position].cmn, GPIO_FUNC_SIO);
    //gpio_set_dir(leds[position].cmn, false);
    gpio_set_dir_in_masked(0xFF80);
    gpio_clr_mask(0xFF80);


    if (++position == LEDS_COUNT)
        position = 0;
    
    gpio_set_function(leds[position].red, GPIO_FUNC_PIO0);
    gpio_set_function(leds[position].green, GPIO_FUNC_PIO0);
    gpio_set_function(leds[position].blue, GPIO_FUNC_PIO0);
    gpio_set_function(leds[position].cmn, GPIO_FUNC_SIO);
    gpio_set_dir(leds[position].cmn, true);
    gpio_put(leds[position].cmn, true);

    //pio_sm_set_enabled(pio, sm, true);
    pio_sm_set_set_pins(pio, sm, leds[position].red, 3);

    pio_pwm_set_levels(pio, sm, leds_brightness[position].red,
        leds_brightness[position].green, leds_brightness[position].blue);

};

bool refresh_callback(struct repeating_timer *t) {
    draw_next_led();
    return true;
}

int main(){
    stdio_init_all();


    gpio_set_pulls(0, true, false);
    gpio_set_irq_enabled_with_callback(0, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
        true, &gpio_callback);



    uint8_t r_level = 0;
    uint8_t g_level = 0;
    uint8_t b_level = 0;
    uint offset = pio_add_program(pio, &pwmrgb_program);
    //printf("Loaded program at %d\n", offset);
    gpio_init_mask(0xFFFFFF);
    gpio_set_dir_all_bits(0);

    gpio_set_pulls(15, false, false);
    gpio_set_pulls(14, false, false);
    gpio_set_pulls(13, false, false);
    gpio_set_pulls(12, false, false);
    gpio_set_pulls(11, false, false);
    gpio_set_pulls(10, false, false);
    gpio_set_pulls(9, false, false);
    gpio_set_pulls(8, false, false);
    gpio_set_pulls(7, false, false);
    
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, true);
    
    bool state = true;

    pwm_program_init(pio, sm, offset, 13);
    pio_pwm_set_levels(pio, sm, 0, 0, 0);
    
    struct repeating_timer timer;
    add_repeating_timer_us(-200, refresh_callback, NULL, &timer);

    while (true) {
        sleep_ms(50);
        // r_level = (r_level + 1) % 127;
        // g_level = (g_level + 1) % 127;
        // b_level = (b_level + 1) % 127;
        // leds_brightness[0].red = r_level;
        // leds_brightness[0].green = 0;
        // leds_brightness[0].blue = 0;
        // leds_brightness[1].green = g_level;
        // leds_brightness[2].blue = b_level;
        // leds_brightness[3].red = r_level;
        // leds_brightness[4].green = g_level;
        // leds_brightness[5].blue = b_level;
        // leds_brightness[6].red = r_level;
        // leds_brightness[7].green = g_level;
        // leds_brightness[8].blue = b_level;
        gpio_put(PICO_DEFAULT_LED_PIN, state);
        state = !state;
    }
	
}
