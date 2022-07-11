/* C Includes */

/* Pico SDK Includes */
#include "pico/stdlib.h"

/* Project Includes */
#include "display_manager.h"

// DisplayManager::DisplayManager(i2c_inst_t *i2c,
//                                      uint8_t an_address, 
//                                      uint16_t an_data_pin, 
//                                      uint16_t an_clock_pin, 
//                                      uint32_t an_baudrate
//                                      )
// {
//     this->itsAlphaNumeric = new HT16K33(i2c, 
//                                         an_address, 
//                                         an_data_pin, 
//                                         an_clock_pin, 
//                                         an_baudrate);
// }

DisplayManager::DisplayManager(i2c_inst_t *i2c_instance,
                                    uint8_t an_address)
{
    itsAlphaNumeric = HT16K33(i2c_instance, 
                              an_address);
}

void DisplayManager::initialise(StateManager *pStateManager)
{
    this->pStateManager = pStateManager;
}

void DisplayManager::update(void)
{
    uint8_t mode = pStateManager->get_mode();
    uint8_t loc;

    clear();

    switch(mode)
    {
        case MANUAL:
            //maybe other stuff later?
            break;

        case PROGRAM:
            write_character('-', 1);
            update_bank(pStateManager->get_active_bank());
            update_patch(pStateManager->get_active_patch());
            break;

        case WRITE:
            update_bank(pStateManager->get_active_bank());
            loc = pStateManager->get_write_location();
            if(loc < 6)
            {
                update_patch(loc);
            }
            else
            {
                write_character('_', 3);
            }
            break;

        case MENU:
            // do stuff
            break;

        default:
            // no-op
            break;
    }
    update_mode(mode);
    itsAlphaNumeric.update();
}

void DisplayManager::clear(void)
{
    itsAlphaNumeric.clear_buffer();
    itsAlphaNumeric.update();
}

void DisplayManager::update_patch(uint8_t value)
{
    value += 49;
    
    #ifdef DEBUG
    printf("DisplayManager::update_patch\n");
    printf(" - Setting Patch To: %c\n", (char)value);
    #endif

    itsAlphaNumeric.write_character(value, 3);
}

void DisplayManager::update_bank(uint8_t value)
{
    uint8_t bank_code = bank_lookup[value];
    uint16_t found_value = itsAlphaNumeric.character_lookup((char)bank_code);

    found_value |= itsAlphaNumeric.character_lookup('.');

    itsAlphaNumeric.load_character_mask(found_value, 2);
}

void DisplayManager::update_mode(uint8_t value)
{
    char lookup;
    lookup = mode_lookup[value];
    write_character(lookup, 0);
    
    switch(value)
    {
        case PROGRAM:
            write_character('-', 1);

            break;
        case WRITE:
            write_character('-', 1);

            break;
    }

}

void DisplayManager::write_string(char *str)
{
    itsAlphaNumeric.write_string(str);
}

void DisplayManager::write_character(char character, uint8_t position)
{
    itsAlphaNumeric.write_character(character, position);
}

void DisplayManager::test()
{
    itsAlphaNumeric.set_oscillator(true);
    itsAlphaNumeric.output_enable(true);
    itsAlphaNumeric.clear_buffer();
    itsAlphaNumeric.update();
    itsAlphaNumeric.write_character('C', 0);
}

//TODO: Implement blinking for underscore when waiting for a selection of write location or new patch after changing bank
