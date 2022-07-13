/* C Includes */
#include <string.h>
#include <stdio.h>

/* Pico Includes */
#include "pico/stdlib.h"
#include "hardware/sync.h"

/* Project Includes */
#include "debug.h"
#include "gpio_defs.h"
#include "storage_manager.h"
#include "state_manager.h"

const char* PATCH_DEFAULT_TITLE = "New Patch %d";

StorageManager::StorageManager(i2c_inst_t *i2c_instance, uint8_t i2c_address)
{
    eeprom = CAT24C32(i2c_instance, i2c_address);
}

void StorageManager::factory_reset(void)
{
    eeprom.erase();
}

void StorageManager::initialise(StateManager *pStateManager)
{
    this->pStateManager = pStateManager;
}

void StorageManager::read_system_data(void)
{
    #ifdef DEBUG
    printf("StorageManager::read_system_data()\n");
    #endif

    eeprom.read_multiple_bytes(SYSTEM_INFO_OFFSET, SYSTEM_INFO_SIZE, read_buffer);

    #ifdef DEBUG
    printf(" - Read EEPROM\n");
    fflush(stdout);
    #endif

    /* Read and decode system data flags */
    pStateManager->set_mode((read_buffer[FLAGS_OFFSET]) & MODE_FLAG_MASK);

    #ifdef DEBUG
    printf(" - Mode Read As: %d\n", pStateManager->get_mode());
    fflush(stdout);
    #endif

    //TODO:
    pStateManager->set_ext_ctrl_a_type((read_buffer[FLAGS_OFFSET] & EXT_CTRL_A_MASK) >> 1);
    pStateManager->set_ext_ctrl_b_type((read_buffer[FLAGS_OFFSET] & EXT_CTRL_B_MASK) >> 2);

    /* Read Last Bank/Patch */
    pStateManager->set_active_bank(read_buffer[LAST_BANK_OFFSET]);
    pStateManager->set_active_patch(read_buffer[LAST_PATCH_OFFSET]);
}


BANK_DATA_X StorageManager::read_bank(uint8_t bank)
{
    #ifdef DEBUG
    printf("StorageManager::read_bank()\n");
    #endif

    BANK_DATA_X read_bank;

    read_bank.patch_array[0] = read_patch(bank, 0);
    read_bank.patch_array[1] = read_patch(bank, 1);
    read_bank.patch_array[2] = read_patch(bank, 2);
    read_bank.patch_array[3] = read_patch(bank, 3);
    read_bank.patch_array[4] = read_patch(bank, 4);

    return read_bank;
}

PATCH_DATA_X StorageManager::read_patch(uint8_t bank, uint8_t patch)
{
    #ifdef DEBUG
    printf("StorageManager::read_patch()\n");
    #endif

    PATCH_DATA_X   read_patch;
    uint16_t offset;
    uint16_t location = PATCH_DATA_OFFSET + (BANK_DATA_SIZE * bank) + (PATCH_DATA_SIZE * patch);

    #ifdef DEBUG
    printf(" -Reading Bank %d, Patch %d\n", bank, patch);
    #endif

    /* Title */
    #ifdef DEBUG
    printf(" - Reading Title\n");
    #endif
    eeprom.read_multiple_bytes(location, PATCH_TITLE_SIZE, read_buffer);
    memcpy(&read_patch.title, read_buffer, PATCH_TITLE_SIZE);
    memset(read_buffer, 0, sizeof(read_buffer));

    #ifdef DEBUG
    printf(" - Reading General\n");
    #endif

    /* General Data */
    location+= PATCH_GENERAL_OFFSET;
    eeprom.read_multiple_bytes(location, PATCH_GENERAL_SIZE, read_buffer);

    read_patch.amp_ctrl_a_enable = (read_buffer[PATCH_CTRL_FLAGS_OFFSET] & PATCH_AMP_A_ENABLE_MASK);
    read_patch.amp_ctrl_a_value  = (read_buffer[PATCH_CTRL_FLAGS_OFFSET] & PATCH_AMP_A_VALUE_MASK);
    read_patch.amp_ctrl_b_enable = (read_buffer[PATCH_CTRL_FLAGS_OFFSET] & PATCH_AMP_B_ENABLE_MASK);
    read_patch.amp_ctrl_b_value  = (read_buffer[PATCH_CTRL_FLAGS_OFFSET] & PATCH_AMP_B_VALUE_MASK);
    read_patch.ext_ctrl_a_enable = (read_buffer[PATCH_CTRL_FLAGS_OFFSET] & PATCH_EXT_CTRL_A_ENABLE_MASK);
    read_patch.ext_ctrl_a_value  = (read_buffer[PATCH_CTRL_FLAGS_OFFSET] & PATCH_EXT_CTRL_A_VALUE_MASK);
    read_patch.ext_ctrl_b_enable = (read_buffer[PATCH_CTRL_FLAGS_OFFSET] & PATCH_EXT_CTRL_B_ENABLE_MASK);
    read_patch.ext_ctrl_b_value  = (read_buffer[PATCH_CTRL_FLAGS_OFFSET] & PATCH_EXT_CTRL_B_VALUE_MASK);

    read_patch.output_mask       = read_buffer[OUTPUT_BITMASK_OFFSET];
    read_patch.num_midi_pc       = read_buffer[PATCH_NUM_MIDI_PC_OFFSET];
    read_patch.num_midi_cc       = read_buffer[PATCH_NUM_MIDI_CC_OFFSET];


    
    /* MIDI Program Changes */
    #ifdef DEBUG
    printf(" - Reading MIDI PC\n");
    #endif

    location+= PATCH_MIDI_PC_OFFSET;

    if(read_patch.num_midi_pc > 0)
    {
        memset(read_buffer, 0, sizeof(read_buffer));

        offset = location; // copy current byte offset for the purpose of optimising our loop without affecting the current data index - space for MIDI data is pre-allocated in the ROM, so we must always add its size to our offset, regardless of how many PC/CC messages have been saved

        for(int i = 0; i < read_patch.num_midi_pc; i++)
        {
            eeprom.read_multiple_bytes(offset, MIDI_PC_SIZE, read_buffer);
            read_patch.midi_program_changes[i].channel = read_buffer[0];
            read_patch.midi_program_changes[i].data    = read_buffer[1];
            offset+= MIDI_PC_SIZE;
        }
    }

    /* MIDI Control Changes */
    #ifdef DEBUG
    printf(" - Reading MIDI CC\n");
    #endif
    location+= PATCH_MIDI_PC_DATA_SIZE; // no need to copy as nothing to read after program change messages

    if(read_patch.num_midi_cc > 0)
    {
        memset(read_buffer, 0, sizeof(read_buffer));

        for(int i = 0; i < read_patch.num_midi_cc; i++)
        {
            eeprom.read_multiple_bytes(location, MIDI_CC_SIZE, read_buffer);
            read_patch.midi_control_changes[i].channel = read_buffer[0];
            read_patch.midi_control_changes[i].controller = read_buffer[1];
            read_patch.midi_control_changes[i].value = read_buffer[2];
            location+= MIDI_CC_SIZE;
        }
    }
    return read_patch;
}

uint8_t StorageManager::write_active_bank(uint8_t bank)
{
    uint8_t result = eeprom.write_byte(bank, SYSTEM_INFO_OFFSET + LAST_BANK_OFFSET);
    return result;
}

uint8_t StorageManager::write_active_patch(uint8_t patch)
{
    uint8_t result = eeprom.write_byte(patch, SYSTEM_INFO_OFFSET + LAST_PATCH_OFFSET);
    return result;
}

uint8_t StorageManager::write_system_flags(uint8_t mode, uint8_t ctrl_a, uint8_t ctrl_b)
{
    uint8_t mask = 0;
    uint8_t readback;

    mask |= mode;
    mask |= ctrl_a << 1;
    mask |= ctrl_b << 2;

    eeprom.write_byte(mask, SYSTEM_INFO_OFFSET);

    readback = eeprom.read_byte(SYSTEM_INFO_OFFSET);

    if(readback == mask)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

uint8_t StorageManager::write_patch_title(uint8_t bank, uint8_t patch, uint8_t* title)
{
    uint16_t byte_address; //TODO: Make this a lookup table instead for robustness?
    uint8_t result;

    byte_address = PATCH_DATA_OFFSET + (bank * BANK_DATA_SIZE) + (patch * PATCH_DATA_SIZE);

    #ifdef DEBUG
    printf("Write Path Title Calculated Offset: %d\n", byte_address);
    #endif

    memset(read_buffer, 0, sizeof(read_buffer));

    eeprom.write_multiple_bytes(title, byte_address, PATCH_TITLE_SIZE);

    eeprom.read_multiple_bytes(byte_address, PATCH_TITLE_SIZE, read_buffer);
    result = memcmp(read_buffer, write_buffer, PATCH_TITLE_SIZE);
    return result;
}

uint8_t StorageManager::write_patch_switch_data(void)
{
    uint8_t result;
    result = write_patch_output_mask();
    //TODO:ext ctrl

    return result;
}

uint8_t StorageManager::write_patch_output_mask(void)
{
    uint8_t result;
    uint8_t byte_offset = PATCH_DATA_OFFSET + ((pStateManager->get_active_bank()) * BANK_DATA_SIZE) + ((pStateManager->get_write_location()) * PATCH_DATA_SIZE) + PATCH_GENERAL_OFFSET + OUTPUT_BITMASK_OFFSET;

    #ifdef DEBUG
    printf("StorageManager::write_patch_output_mask()\n");
    printf(" - Bank: %d, Patch: %d, Mask: %02x\n", pStateManager->get_active_bank(), pStateManager->get_write_location(), pStateManager->get_output_mask());
    #endif

    result = eeprom.write_byte(pStateManager->get_output_mask(), byte_offset);
    
    #ifdef DEBUG
    printf("Readback: %02x\n", eeprom.read_byte(byte_offset));
    #endif

    return result;
}

uint8_t StorageManager::validate_eeprom(void)
{
    bool header_not_found;
    bool footer_not_found;
    int result = 0;
    int address;
    uint8_t offset;
    char string_buffer[33];

    memset(write_buffer, 0, CAT24C32_PAGE_SIZE);
    memset(read_buffer, 0x00, CAT24C32_PAGE_SIZE);

    /* Check first 4 bytes of EEPROM for the boot flag */
    eeprom.read_multiple_bytes(BOOT_FLAG_OFFSET, BOOT_FLAG_SIZE, read_buffer);

    #ifdef DEBUG
    //sleep_ms(5000);
    printf("Checking Boot Header:\n");
    print_buff(read_buffer, BOOT_FLAG_SIZE);
    fflush(stdout);
    #endif

    write_buffer[0] = (uint8_t)(BOOT_FLAG >> 24);
    write_buffer[1] = (uint8_t)(BOOT_FLAG >> 16);
    write_buffer[2] = (uint8_t)(BOOT_FLAG >> 8);
    write_buffer[3] = (uint8_t)(BOOT_FLAG);

    header_not_found = memcmp(read_buffer, write_buffer, BOOT_FLAG_SIZE);

    #ifdef DEBUG
    printf("Header Result: %d\n", header_not_found);
    #endif

    memset(read_buffer, 0, CAT24C32_PAGE_SIZE);
    eeprom.read_multiple_bytes(BOOT_FLAG_END_OFFSET, BOOT_FLAG_SIZE, read_buffer);

    #ifdef DEBUG
    printf("Checking Boot Footer:\n");
    print_buff(read_buffer, BOOT_FLAG_SIZE);
    fflush(stdout);
    #endif

    footer_not_found = memcmp(read_buffer, write_buffer, BOOT_FLAG_SIZE);
    
    #ifdef DEBUG
    printf("Footer Result %d\n", footer_not_found);
    #endif

    if(1)
    //if(header_not_found || footer_not_found)
    {
        // TODO: maybe put something in here to inform that the pedal is doing some "setup" or whatever
        
        #ifdef DEBUG
        printf("First Boot!\n");
        printf("Erasing...\n");
        #endif

        /* Erase all 4096 bytes of EEPROM */
        eeprom.erase();

        #ifdef DEBUG
        printf("All Erased, Readback:\n"); 
        for(int i = 0; i < CAT24C32_PAGE_COUNT; i++)
        {
            address = i * CAT24C32_PAGE_SIZE;
            eeprom.read_multiple_bytes(address, CAT24C32_PAGE_SIZE, read_buffer);
            printf("Reading Page: %d\n", i);
            print_buff(read_buffer, CAT24C32_PAGE_SIZE);
            fflush(stdout);
        }
        
        #endif

        /* First boot, so prepare buffer with boot flag and write it to page 0 */
        write_buffer[0] = (uint8_t)(BOOT_FLAG >> 24);
        write_buffer[1] = (uint8_t)(BOOT_FLAG >> 16);
        write_buffer[2] = (uint8_t)(BOOT_FLAG >> 8);
        write_buffer[3] = (uint8_t)(BOOT_FLAG);
        
        #ifdef DEBUG
        printf("Buffer to be written:\n");
        print_buff(write_buffer, BOOT_FLAG_SIZE);
        #endif

        eeprom.write_multiple_bytes(write_buffer, BOOT_FLAG_OFFSET, BOOT_FLAG_SIZE);

        memset(read_buffer, 0, CAT24C32_PAGE_SIZE);
        eeprom.read_multiple_bytes(BOOT_FLAG_OFFSET, BOOT_FLAG_SIZE, read_buffer);

        #ifdef DEBUG
        printf("\n");
        printf("Readback:\n");
        print_buff(read_buffer, BOOT_FLAG_SIZE);
        #endif

        result = memcmp(write_buffer, read_buffer, BOOT_FLAG_SIZE);

        if(result)
        {   
            #ifdef DEBUG
            printf("WRITE FAILED\n");
            return result;
            #endif
        }
        #ifdef DEBUG
        else
        {
            printf("Boot flag written and readback successfully!\n");
        }
        #endif

        /* Write default system data */
        #ifdef DEBUG
        printf("Writing System Info Defaults...\n");
        #endif

        memset(write_buffer, 0, SYSTEM_INFO_SIZE);

        write_buffer[0] = 0b00000000; // setting defaults for mode/ext ctrl A/B type
        write_buffer[1] = 0;          // default bank: 1
        write_buffer[2] = 0;          // default patch: 1

        #ifdef DEBUG
        printf("Sys Info Buffer:\n");
        print_buff(write_buffer, SYSTEM_INFO_SIZE); // remaining bytes reserved for IP address/other data in future
        #endif

        /* Readback the data to check */
        eeprom.write_multiple_bytes(write_buffer, SYSTEM_INFO_OFFSET, SYSTEM_INFO_SIZE);

        memset(read_buffer, 0, CAT24C32_PAGE_SIZE);
        eeprom.read_multiple_bytes(SYSTEM_INFO_OFFSET, SYSTEM_INFO_SIZE, read_buffer);

        #ifdef DEBUG
        printf("Sys Info Readback:\n");
        print_buff(read_buffer, SYSTEM_INFO_SIZE);
        #endif

        result = memcmp(read_buffer, write_buffer, SYSTEM_INFO_SIZE);

        if(result)
        {
            //TODO: Different return values to indicate different failure?
            #ifdef DEBUG
            printf("ERROR WRITING SYS INFO\n");
            #endif
            return result;
        }    


        for(int bank = 0; bank < NUM_BANKS; bank++)
        {
            for(int patch = 0; patch < NUM_PATCHES; patch++)
            {
                memset(string_buffer, 0, sizeof(string_buffer));
                memset(write_buffer, 0, sizeof(write_buffer));

                sprintf(string_buffer, PATCH_DEFAULT_TITLE, patch);
                #ifdef DEBUG
                printf("Formatted String: [%s]\n", string_buffer);
                fflush(stdout);
                #endif

                memcpy(write_buffer, string_buffer, strlen(string_buffer));
                write_patch_title(bank, patch, write_buffer);

                memset(read_buffer, 0, CAT24C32_PAGE_SIZE);

                #ifdef DEBUG
                eeprom.read_multiple_bytes(PATCH_DATA_OFFSET + (bank * BANK_DATA_SIZE) + (patch * PATCH_DATA_SIZE), PATCH_TITLE_SIZE, read_buffer);
                memset(string_buffer, 0, sizeof(string_buffer));
                memcpy(string_buffer, read_buffer, sizeof(string_buffer));
                printf("Bank: %d Patch: %d [%s]\n", bank, patch, string_buffer);
                fflush(stdout);
                #endif
            }
        }
        
        // ----------------------------------------------------------------

        memset(write_buffer, 0xFF, CAT24C32_PAGE_SIZE);
        eeprom.write_multiple_bytes(write_buffer, 168, CAT24C32_PAGE_SIZE);

        write_buffer[0] = (uint8_t)(BOOT_FLAG >> 24);
        write_buffer[1] = (uint8_t)(BOOT_FLAG >> 16);
        write_buffer[2] = (uint8_t)(BOOT_FLAG >> 8);
        write_buffer[3] = (uint8_t)(BOOT_FLAG);

        eeprom.write_multiple_bytes(write_buffer, BOOT_FLAG_END_OFFSET, BOOT_FLAG_SIZE);

        #ifdef DEBUG
        printf("All Readback:\n"); 

        for(int i = 0; i < CAT24C32_PAGE_COUNT; i++)
        {
            address = i * CAT24C32_PAGE_SIZE;
            memset(read_buffer, 0, CAT24C32_PAGE_SIZE);
            eeprom.read_multiple_bytes(address, CAT24C32_PAGE_SIZE, read_buffer);
            printf("Reading Page: %d\n", i);
            print_buff(read_buffer, CAT24C32_PAGE_SIZE);
            fflush(stdout);
        }
        #endif
    }
    #ifdef DEBUG
    else
    {
        printf("Boot Flags Detected, Pass Validation\n");
    }
    #endif
    return result;
}

void StorageManager::test(void)
{
    uint16_t address;
    uint8_t test_buffer[64];

    eeprom.erase();

    for(int i = 0; i < 8; i++)
    {
        address = i * CAT24C32_PAGE_SIZE;
        memset(read_buffer, 0, CAT24C32_PAGE_SIZE);
        eeprom.read_multiple_bytes(address, CAT24C32_PAGE_SIZE, read_buffer);
        printf("Reading Page: %d\n", i);
        print_buff(read_buffer, CAT24C32_PAGE_SIZE);
        fflush(stdout);
    }   

    memset(test_buffer, 0xAA, 32);
    memset(&test_buffer[32], 0xBB, 32);

    eeprom.write_multiple_bytes(test_buffer, 0, 64);

    printf("All Readback:\n"); 

    for(int i = 0; i < 8; i++)
    {
        address = i * CAT24C32_PAGE_SIZE;
        memset(read_buffer, 0, CAT24C32_PAGE_SIZE);
        eeprom.read_multiple_bytes(address, CAT24C32_PAGE_SIZE, read_buffer);
        printf("Reading Page: %d\n", i);
        print_buff(read_buffer, CAT24C32_PAGE_SIZE);
        fflush(stdout);
    }    
}

