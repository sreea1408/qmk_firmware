#include "color.h"
#include "quantum.h"
#include "indicator.h"

#ifndef BAT_LEVEL_SHOW_INTERVAL
#    define BAT_LEVEL_SHOW_INTERVAL 5000
#endif

#ifdef LED_MATRIX_ENABLE
#    define LED_DRIVER_IS_ENABLED led_matrix_is_enabled
#endif

#ifdef RGB_MATRIX_ENABLE
#    define LED_DRIVER_IS_ENABLED rgb_matrix_is_enabled
#endif

enum Display {
    BAT_LVL_STAT_NONE,
    BAT_LVL_STAT_CLEAR,
    BAT_LVL_STAT_SHOW,
};

static enum Display display_state = BAT_LVL_STAT_NONE;
static uint8_t      bat_percentage;
static uint32_t     bat_lvl_stat_timer_buffer = 0;

extern indicator_config_t indicator_config;

void bat_level_static_start(const uint8_t percentage) {
    /* Turn on backlight mode for indicator */
    indicator_enable();

    display_state             = BAT_LVL_STAT_SHOW;
    bat_percentage            = percentage;
    bat_lvl_stat_timer_buffer = sync_timer_read32();
}

bool bat_level_static_actived(void) {
    return display_state;
}

void bat_level_static_indicate(void) {
#ifdef LED_MATRIX_ENABLE
    for (uint8_t i = 0; i <= LED_MATRIX_LED_COUNT; i++) {
        led_matrix_set_value(i, 0);
    }

    if (display_state == BAT_LVL_STAT_SHOW) {
        uint8_t bat_lvl_led_list[10] = BAT_LEVEL_LED_LIST;
        for (uint8_t i = 0; i < bat_percentage / 10; i++) {
            led_matrix_set_value(bat_lvl_led_list[i], 255);
        }
    }
#endif

#ifdef RGB_MATRIX_ENABLE
    for (uint8_t i = 0; i <= RGB_MATRIX_LED_COUNT; i++) {
        rgb_matrix_set_color(i, 0, 0, 0);
    }

    if (display_state == BAT_LVL_STAT_SHOW) {
        uint8_t       bat_lvl_led_list[10] = BAT_LEVEL_LED_LIST;
        const uint8_t green_blocks         = bat_percentage / 10;
        for (uint8_t i = 0; i < 10; i++) {
            if (i < green_blocks) {
                rgb_matrix_set_color(bat_lvl_led_list[i], RGB_GREEN);
            } else {
                rgb_matrix_set_color(bat_lvl_led_list[i], RGB_RED);
            }
        }
    }
#endif
}

static void bat_level_static_update(void) {
    switch (display_state) {
        case BAT_LVL_STAT_SHOW:
            display_state = BAT_LVL_STAT_CLEAR;
            break;
        case BAT_LVL_STAT_CLEAR:
            display_state = BAT_LVL_STAT_NONE;
            indicator_eeconfig_reload();
            if (indicator_config.value == 0 && !LED_DRIVER_IS_ENABLED()) {
                indicator_disable();
            }
            break;
        default:
            break;
    }
}

void bat_level_static_task(void) {
    if (display_state && sync_timer_elapsed32(bat_lvl_stat_timer_buffer) > BAT_LEVEL_SHOW_INTERVAL) {
        bat_level_static_update();
    }
}
