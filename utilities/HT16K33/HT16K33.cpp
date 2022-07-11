/* C Includes */
#include <memory.h>
#include <stdio.h>

/* Pico SDK Includes */

/* Project Includes */
#include "HT16K33.h"

/****************************************************************
Function:   HT16K33 (Constructor)
Arguments:  (i2c_inst_t) *i2c_instance
            (uint16_t)   clock_pin
            (uint16_t)   data_pin
            (uint32_t)   baudrate (max: 400000)
Return:     void

Constructs an instance of the HT16K33 class and initialises the
provided i2c bus according to specified clock and data pins and 
baudrate. Enables pull up resisitors on the chosen pins and 
enables the internal osscilator of the display. The internal memory 
of the HT16K33 boots up with random data so it is zeroed.
****************************************************************/
HT16K33::HT16K33(){}

HT16K33::HT16K33(i2c_inst_t *i2c_instance, uint8_t i2c_address)
{
    this->i2c_instance = i2c_instance;       
    this->i2c_address  = i2c_address;
    
    clear_buffer();
    update();
}

/****************************************************************
Function:  enable_oscillator
Arguments: (bool) status
Return:    void

Enables or disables the internal osscilator of the display. This
is called in the constructor but is provided as a standalone
function for more granular control if desired.
****************************************************************/
void HT16K33::set_oscillator(bool status)
{
    /* Write the command code for oscillator control into the control buffer */
    control_buffer = OSCILLATE << 4;

    /* If status is true perform bitwise OR with ENABLE mask */
    if(status)
    {
       control_buffer |= ENABLE;
    }
    
    i2c_write_blocking(i2c_instance, i2c_address, &control_buffer, 1, false);
}


/****************************************************************
Function:  update
Arguments: none
Return:    void

Writes the contents of buffer[] to the display starting from memory 
location 0x0000, equivalent to writing a single command byte, and 
8 bytes of character data (2 bytes per character) starting at memory 
address 0b0000 (first character). 

Starting address of ADDR_1 is hardcoded to prevent any other value 
from being used.

This function is provided to enable load_char and load_string to
be used to assemble a buffer piecemeal before writing it out to
the display.
****************************************************************/
void HT16K33::update(void)
{
    data_buffer[0] = (SET_ADDRESS_PTR << 4) | ADDR_1;
    i2c_write_blocking(i2c_instance, i2c_address, (uint8_t*)&data_buffer, sizeof(data_buffer), false);
}


/****************************************************************
Function:  load_char
Arguments: (const char*) character
           (uint8_t)     position
Return:    void

Updates buffer[position] with a new character without writing the
buffer to the display. Provides granular control allowing a set of 
characters to be constructed piecemeal without updating each time.
****************************************************************/
void HT16K33::load_char(char character, uint8_t position)
{
    uint16_t found_char;

    if(position <= 3)
    {
        /* each "position" requires 2 bytes, so position 0 = buffer[1-2] */
        position *= 2;

        /* check for valid character */
        found_char = character_lookup(character);
        
        /* casting character to uint8_t yields least significant byte */
        position++;
        data_buffer[position] = (uint8_t) found_char;

        /* shifting character 8 bits to the right and casting to uint8_t yields most significant byte */
        position++;
        data_buffer[position] = (uint8_t) (found_char >> 8);
    }
}


void HT16K33::load_character_mask(uint16_t mask, uint8_t position)
{
    if(position <= 3)
    {
        /* each "position" requires 2 bytes, so position 0 = buffer[1-2] */
        position *= 2;

        position++;

        data_buffer[position] = (uint8_t) mask;
        position++;
        data_buffer[position] = (uint8_t) (mask >> 8);
    }
}

/****************************************************************
Function:  load_string 
Arguments: (const char*)
Return:    void

Loads buffer[] with LED data representing a 4 character string.
Each character is converted from ASCII value to a 16Bit map by
referencing the font table by index.
****************************************************************/
void HT16K33::load_string(char *str)
{
    uint16_t found_char;
    
    /* for each character in the string perform character lookup */
    for(uint8_t i = 0; i < 4; i++)
    {
        found_char = character_lookup((char)str[i]);

        /* write the LS Byte and MS Byte to the current buffer position */
        data_buffer[(i * 2) + 1] = (uint8_t) found_char;
        data_buffer[(i * 2) + 2] = (uint8_t) (found_char >> 8);
    }
}


/****************************************************************
Function:    write_character
Arguments:   (const char*) character value
             (uint8_t)     character position 0-3
Return:      void

High-level function to immediately write a given character to the 
given display position, indexed from 0. So 0 is first character... 
1 is second... If an invalid position is given the command is 
ignored.
****************************************************************/
void HT16K33::write_character(char character, uint8_t position)
{
    load_char(character, position);
    update();
}


/****************************************************************
Function:  write_string
Arguments: const char *string
Return:    void

High-level function which interprets a given 4 character string 
and writes it immediately to the display.Any invalid characters 
will be replaced with a period.
****************************************************************/
void HT16K33::write_string(char *str)
{
   load_string(str);
   update();
}

/****************************************************************
Function:  output_enable
Arguments: (bool) status
Return:    void 

Set the output status of the display to enabled(true) or disabled.

E.g. to enable display, option nibble is set to 0b0001:

Command nibble 0b1000 shifted right by 4 = 0b1000 0000 
  OR option nibble with 0b0001 (1) yields: 0b1000 0001
                                                     ^
****************************************************************/
void HT16K33::output_enable(bool status)
{
    /* Write command code for setting output status to the buffer */
    control_buffer = SET_OUTPUT_STS << 4;

    /* If status is true perform bitwise OR to append the value of ENABLE mask (0b0001) */
    if(status)
    {
        control_buffer |= ENABLE;
    }

    i2c_write_blocking(i2c_instance, i2c_address, &control_buffer, 1, false);
}

/****************************************************************
Function:  dimming_control
Arguments: (uint8_t) level
Return:    void

Sets the PWM output level of the display to a value between 0 and
15. Any value given above this limit will be floored down.

E.g. for dimming level 6:
Command nibble 0b1110 shifted right by 4 = 0b1110 0000 
  OR with option nibble 0b0100 (6) yields: 0b1110 0100
                                                   ^
****************************************************************/
void HT16K33::set_dimming(u_int8_t level)
{
    if(level > 15)
    {
        level = 15;
    }

    /* Load the command nibble for dimming control into MSByte of control_command
    and perform bitwise logical OR with the given level */
    control_buffer = (DIMMING_SET << 4) | level;

    i2c_write_blocking(i2c_instance, i2c_address, &control_buffer, 1, false);
}


/****************************************************************
Function:  character_lookup
Arguments: (const char*)
Return:    void

Converts a given ASCI character into a 16Bit LED segment map. If
any invalid value is given it is replaced with a period.
****************************************************************/
uint16_t HT16K33::character_lookup(char character)
{
    /* check if the character is within valid range */
    if(character >= 32 && character <= 126)
    {
        return font_table[character];
    }
    else
    {
        return font_table[46];
    }
}

void HT16K33::clear_buffer(void)
{
    memset(data_buffer, 0, sizeof(data_buffer));
}