#ifndef EEPROM_H
# define EEPROM_H

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <error_led.h>
#include <log.h>
#include <stdbool.h>
#include <utils.h>

#ifndef CPU_FREQ
 #define CPU_FREQ 16000000
#endif

#define ID_STR 1

#define MAGIC_NUMBER_VALUE 0x8F59CFED5100ED11
#define MAGIC_NUMBER_SIZE 8

#define EEPROM_MAX_ADDRESS 0x03FF
#define EEPROM_START_ADDRESS (0x00 + MAGIC_NUMBER_SIZE)
#define EEPROM_MAX_BLOCK (EEPROM_MAX_ADDRESS - 2 - 2 - 2 - 1)
#define EEPROM_EOM_VALUE 14897 //magic number where start memory fragmentation

#define EEPROM_ERROR_UNINTIALIZED 1
#define EEPROM_ERROR_BAD_ADDRESS 2 //Address < MNSize or > (max address - 7) 
#define EEPROM_ERROR_ID_NOT_FOUND 3
#define EEPROM_ERROR_BAD_ID 4 //ID can't be 0x00
#define EEPROM_ERROR_BAD_SIZE 5 //Size of block must be >= 3 (ID >= 2 && value >= 1)
#define EEPROM_ERROR_MAX_ADDRESS 6 //Getting next block will overflow eeprom
#define EEPROM_ERROR_NULL 7
#define EEPROM_ERROR_BUFFER_OVERFLOW 8
#define EEPROM_ERROR_FULL 9

// uint8_t	safeRead(uint16_t address, void* dest, uint16_t size);
// uint8_t	safeWrite(uint16_t address, void* data, uint16_t size);

// bool	eeprom_safe_write(void *buffer, size_t offset, size_t length);
// bool	eeprom_safe_read(void * buffer, size_t offset, size_t length);

/*

MAGIC_NUMBER [SIZE OF DATA] [USED] [ID] [VALUE] [0x00]...
8B			 2B              2B    [NB        ]     1B

*/

/// @brief Check if id exist and overwrite it so.
/// Else allocate it using an available memory block with the closest size possible
/// @param id 
/// @param buffer 
/// @param length 
/// @return ```error``` if no space was left or if id is incorrect, if buffer is NULL
/// or size was 0 
uint8_t	eeprom_alloc_write(char* id, char* buffer);

/// @brief 
/// @param id 
/// @param buffer 
/// @param size 
uint8_t	eeprom_alloc_read(char* id, char* buffer);
uint8_t	eeprom_alloc_free(char* id);

/// @brief Check for existence of MN at start of EEPROM, and init EEPROM to 0
/// @param  
/// @return 
uint8_t		eeprom_init(void);

/// @brief!!!!!!!!!  Clear memory content. DANGEROUS !!!!!!!!!
/// @param  
void		eeprom_clear_DANGEROUS(bool confirm);

void	eeprom_print_blocks(void);

#endif
