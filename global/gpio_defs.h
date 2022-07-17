#ifndef GPIO_DEFS_H
#define GPIO_DEFS_H

#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/flash.h"

/* This file defines GPIO macros, structures and data storage offsets */
#define DEBUG

/* CTRL Types */
#define MOMENTARY 0
#define LATCHING 1

/* LED Indicators */
#define PICO_LED           25

#define RELAY_ONE          0
#define RELAY_TWO          1
#define RELAY_THREE        2
#define RELAY_FOUR         3
#define RELAY_FIVE         4
#define RELAY_AMP_SW_A     5
#define RELAY_AMP_SW_B     6
#define EXT_CTRL_A         7
#define EXT_CTRL_B         8
#define MUTE_OPTORELAY     9

/* Most significant 7 bits are 5x pedal relays and 2 amp F/SW relays */
#define OUTPUT_MASK       0b11111111
#define RELAY_MASK        0b00011111
#define OUTPUT_SHIFT_MASK 0b00000001
#define AMP_SW_A_MASK     0b01000000
#define AMP_SW_B_MASK     0b10000000

/* Number of times and cadence which an LED should blink during rapid_blink() */
#define BLINK_REPEATS       6
#define RAPID_BLINK_TIME    50
#define SLOW_BLINK_TIME     300

/* Momentary Switches */
#define SW_ONE        16
#define SW_TWO        17
#define SW_THREE      18
#define SW_FOUR       19
#define SW_FIVE       20
#define SW_MODE       21
#define SW_WRITE      22

#define NUM_LOOPS     5
#define NUM_BANKS     5
#define NUM_PATCHES   5
#define TOTAL_PATCHES 25

/* Bit Mask for decoding gpio_get_all() bit mask into commands */
#define GPIO_SWITCH_MASK 0b000000000011111110000000000000000                        
#define SW_1_MASK        0x10000 /* 65,536 */ 
#define SW_2_MASK        0x20000 /* 131,072 */
#define SW_3_MASK        0x40000 /* 262,144 */
#define SW_4_MASK        0x80000 /* 524,288 */
#define SW_5_MASK        0x100000 /* 1,048,576 */

#define SW_MODE_MASK     0x200000 /* 2,097,152 */
#define SW_WRITE_MASK    0x400000 /* 4,194,304 */

#define PATCH_INC_MASK   0x60000 /* 393,216 */
#define PATCH_DEC_MASK   0x30000 /* 196,608 */

#define CHAR_A 0x41
#define CHAR_B 0x42
#define CHAR_C 0x43
#define CHAR_D 0x44
#define CHAR_E 0x45

#define INVALID 0

/* State Macros */
#define HIGH         1
#define LOW          0 

/* Modes */
#define MANUAL       0
#define PROGRAM      1
#define WRITE        2
#define MENU         3

/* Lookup Tables - Enables index based lookup of GPIO values, input bit masks and character codes. 
Pins in use may not always be consecutive, so cannot be accessed by looping with a fixed offset */

/* Indicators/Relay Outputs */ //TODO: can probably get rid of this now...
const uint8_t output_lookup[] = {RELAY_ONE, RELAY_TWO, RELAY_THREE, RELAY_FOUR, RELAY_FIVE};

/* 32Bit masks for decoding input */
const uint32_t input_lookup[] = {SW_1_MASK, SW_2_MASK, SW_3_MASK, SW_4_MASK, SW_5_MASK};

/* Bank Position to ASCII */
const char bank_lookup[] = {CHAR_A, CHAR_B, CHAR_C, CHAR_D, CHAR_E};

/* Array to provide ASCII character lookup for modes... e.g. MANUAL = 0, mode_lookup[0] = ASCII value for M - 77 */
const char mode_lookup[] = {77, 80, 87};

/* Multiple Input Command Codes */
#define BANK_DOWN  (SW_ONE + SW_TWO)
#define BANK_UP    (SW_TWO + SW_THREE)
#define OPEN_MENU  (SW_MODE + SW_WRITE)


/* Command Buffer */
#define COM_BUF_CAP 2
typedef struct com_buffer_x
{
    uint32_t command;
    bool command_flag;
} COM_BUFFER_X;

/* 2 Bytes: Channel, Data */
typedef struct midi_pc_data_x
{
    uint8_t channel;
    uint8_t data;
} MIDI_PC_DATA_X;

/* 3 Bytes: Channel, Controller, Value */
typedef struct midi_cc_data_x
{
    uint8_t channel;
    uint8_t controller;
    uint8_t value;
} MIDI_CC_DATA_X;

typedef struct patch_data_x
{
    char title[33]; //33 to allow 32 + null character

    uint8_t amp_ctrl_a_enable;
    uint8_t amp_ctrl_a_value;

    uint8_t amp_ctrl_b_enable;
    uint8_t amp_ctrl_b_value;

    uint8_t ext_ctrl_a_enable;
    uint8_t ext_ctrl_b_enable;

    uint8_t ext_ctrl_a_value;
    uint8_t ext_ctrl_b_value;

    uint8_t num_midi_pc;
    uint8_t num_midi_cc;

    uint8_t output_mask;

    MIDI_PC_DATA_X midi_program_changes[10];

    MIDI_CC_DATA_X midi_control_changes[10];

} PATCH_DATA_X;

typedef struct bank_data_x
{
    PATCH_DATA_X patch_array[NUM_PATCHES];
} BANK_DATA_X;

/* Storage Defines - Keep in case need for any "write once/read many" data e.g. graphics data... */
#define FLASH_TARGET_OFFSET   262144
#define FLASH_OFFSET          XIP_BASE + FLASH_TARGET_OFFSET

#define BOOT_FLAG  0xACC01ADE

/* Bitmasks for decoding System Data Flags */
#define MODE_FLAG_MASK  0b00000001
#define EXT_CTRL_A_MASK 0b00000010
#define EXT_CTRL_B_MASK 0b00000100

/* Bitmasks for decoding Patch Data Flags */
#define PATCH_AMP_A_ENABLE_MASK       0b00000001
#define PATCH_AMP_A_VALUE_MASK        0b00000010
#define PATCH_AMP_B_ENABLE_MASK       0b00000100
#define PATCH_AMP_B_VALUE_MASK        0b00001000
#define PATCH_EXT_CTRL_A_ENABLE_MASK  0b00010000
#define PATCH_EXT_CTRL_A_VALUE_MASK   0b00100000
#define PATCH_EXT_CTRL_B_ENABLE_MASK  0b01000000
#define PATCH_EXT_CTRL_B_VALUE_MASK   0b10000000

/* Midi Message Sizes in Bytes */
#define MIDI_PC_SIZE 2
#define MIDI_CC_SIZE 3

/* Offsets within System Info */
#define BOOT_FLAG_OFFSET         0
#define BOOT_FLAG_END_OFFSET     4092
#define BOOT_FLAG_SIZE           4

#define SYSTEM_INFO_OFFSET       (BOOT_FLAG_SIZE + BOOT_FLAG_OFFSET)
#define FLAGS_OFFSET             0   // bitmask for Startup Mode, Ex Ctrl A type, Ext Ctrl B Type
#define LAST_BANK_OFFSET         1
#define LAST_PATCH_OFFSET        2
#define IP_ADDRESS_OFFSET        3
#define IP_ADDRESS_SIZE          12
#define SYSTEM_INFO_SIZE         15 // (IP_ADDRESS_OFFSET + IP_ADDRESS_SIZE + 1)

#define PATCH_DATA_OFFSET (SYSTEM_INFO_OFFSET + SYSTEM_INFO_SIZE)
/* Offsets Within Patch Data */
#define PATCH_TITLE_OFFSET 0
#define PATCH_TITLE_SIZE 32

#define PATCH_GENERAL_OFFSET (PATCH_TITLE_SIZE)

#define PATCH_CTRL_FLAGS_OFFSET      0 //whether to modify amp ctrl state values or not

#define OUTPUT_BITMASK_OFFSET        1
#define PATCH_NUM_MIDI_PC_OFFSET     2
#define PATCH_NUM_MIDI_CC_OFFSET     3
#define PATCH_GENERAL_SIZE           4

#define PATCH_MIDI_PC_OFFSET         (PATCH_GENERAL_OFFSET + PATCH_GENERAL_SIZE)
#define PATCH_MIDI_PC_DATA_SIZE      20

#define PATCH_MIDI_CC_DATA_OFFSET    (PATCH_MIDI_PC_OFFSET)
#define PATCH_MIDI_CC_DATA_SIZE      30

#define PATCH_DATA_SIZE              (PATCH_TITLE_SIZE + PATCH_GENERAL_SIZE + PATCH_MIDI_PC_DATA_SIZE + PATCH_MIDI_CC_DATA_SIZE)
#define BANK_DATA_SIZE               (PATCH_DATA_SIZE * NUM_PATCHES)


/* i2c */
#define I2C0_DATA    4
#define I2C0_CLOCK   5
#define I2C1_DATA    14
#define I2C1_CLOCK   15
#define QUAD_ADDR    0x70
#define EEPROM_ADDR  0x50
#define OUTPUT_PORT_I2C_ADDR 0x20

#endif