#include <stdio.h>
#include <stdlib.h>
#include "csr.h"
#include "hart.h"
#include "handler.h"
#include "core_v_mini_mcu.h"
#include "rv_plic.h"
#include "rv_plic_regs.h"
#include "gpio.h"
#include "pad_control.h"
#include "pad_control_regs.h"  // Generated.
#include "x-heep.h"

#include "i2c.h"

#include "i2c_structs.h"

void test_config();
void test_set_watermark();
void test_reset();
void test_override();
void test_write_byte();
// void test_read_byte();

int main()
{
    
    printf("\nStarting i2c test!\n");

    test_config();
    test_set_watermark();
    test_reset();
    test_override();
    test_write_byte();
    // test_read_byte();

    printf("Done!\n");

    return 1;
}


void test_config()
{
    printf(">> Config...\n");

    // i2c_config_t config;

    // i2c_timing_config_t time_config = {
    //     .lowest_target_device_speed = 5,
    //     .clock_period_nanos = 1000,
    //     .sda_rise_nanos = 2,
    //     .sda_fall_nanos = 3,
    //     .scl_period_nanos = 1
    // };


    // i2c_compute_timing(time_config, &config);


    /* This is the hardcoded version of doing it
       it allows to directly see the numbers in the
       register when pringing */

    i2c_config_t config = {
        // TIMING 0
        .scl_time_high_cycles = 1,
        .scl_time_low_cycles = 1,
        
        // TIMING 1
        .rise_cycles = 1,
        .fall_cycles = 1,

        // TIMING 2
        .start_signal_setup_cycles = 1,
        .start_signal_hold_cycles = 1,

        // TIMING 3
        .data_signal_setup_cycles = 1,
        .data_signal_hold_cycles = 1,

        // TIMING 4
        .stop_signal_setup_cycles = 1,
        .stop_signal_hold_cycles = 1
    };

    i2c_configure(config);

    printf("TIMING0:\t%d\n", i2c_peri->TIMING0);
    printf("TIMING1:\t%d\n", i2c_peri->TIMING1);
    printf("TIMING2:\t%d\n", i2c_peri->TIMING2);
    printf("TIMING3:\t%d\n", i2c_peri->TIMING3);
    printf("TIMING4:\t%d\n", i2c_peri->TIMING4);
}

void test_set_watermark()
{
    printf(">> Set watermark...\n");

    i2c_level_t rx_level = kI2cLevel4Byte;

    i2c_level_t fmt_level = kI2cLevel16Byte;

    i2c_set_watermarks(rx_level, fmt_level);

    printf("FIFO_CTRL:\t%d\n", i2c_peri->FIFO_CTRL);

}

void test_reset()
{   
    printf(">> Reset...\n");

    i2c_reset_rx_fifo();
    i2c_reset_fmt_fifo();

    printf("FIFO_CTRL:\t%d\n", i2c_peri->FIFO_CTRL);
}

void test_override()
{
    printf(">> Test override...\n");

    /* This test is to check that I cannot
       modify the pins if first I don't enable 
       the override mode */
    i2c_override_drive_pins(1, 1);
    printf("OVRD:\t%d\n", i2c_peri->OVRD);

    /* This is the one in which I alse enable the override */
    i2c_override_set_enabled(kI2cToggleEnabled);
    i2c_override_drive_pins(1, 1);
    printf("OVRD:\t%d\n", i2c_peri->OVRD);

}

void test_write_byte()
{
    printf(">> Write byte...\n");

    i2c_fmt_t code = kDifI2cFmtStart;
    uint8_t fmt_lvl = 0;
    uint8_t rx_lvl = 0;

    i2c_get_fifo_levels(&fmt_lvl, &rx_lvl);

    printf("fmt_lvl:\t%d\n", fmt_lvl);
    printf("rx_lvl:\t\t%d\n", rx_lvl);

    i2c_result_t res = i2c_write_byte(42, code, false);

    if(res != kDifI2cOk){
        printf("--|| ERROR!\n ||--\n");
    } else {
        printf("Byte written!\n");
    }

    i2c_get_fifo_levels(&fmt_lvl, &rx_lvl);

    printf("fmt_lvl:\t%d\n", fmt_lvl);
    printf("rx_lvl:\t\t%d\n", rx_lvl);

}


// void test_read_byte()
// {
//     printf(">> Read byte...\n");

//     uint8_t byte;

//     i2c_read_byte(&byte);

//     printf("Read byte:\t%d\n", byte);
// }