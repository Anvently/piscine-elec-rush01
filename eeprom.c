#include <eeprom.h>

char		id_buffer[256] = {0}, value_buffer[256] = {0};
const uint64_t	MAGIC_NUMBER = MAGIC_NUMBER_VALUE;

static uint8_t	checkBlockInvalid(uint16_t* address)
{
	if (address == NULL)
		return (EEPROM_ERROR_NULL);
	if ((uint16_t)address >= EEPROM_MAX_ADDRESS || (uint16_t)address < EEPROM_START_ADDRESS) // > 8 && < 1023
		return (EEPROM_ERROR_BAD_ADDRESS);
	return (0);
}


static uint16_t	getBlockSize(uint16_t* addr)
{
	return (2 + 2 + eeprom_read_word(addr));
}

static uint16_t	getDataSize(uint16_t* addr)
{
	return (eeprom_read_word(addr));
}

static void	updateDataSize(uint16_t* address, uint16_t value)
{
	eeprom_update_word(address, value);
}

static uint16_t	getDataUsed(uint16_t* addr)
{
	return (eeprom_read_word(addr + 1));
}

static void	updateDataUsed(uint16_t* address, uint16_t value)
{
	eeprom_update_word(address + 1, value);
}

static uint16_t	updateBlockID(uint16_t* addr, const void *id, uint16_t size)
{
	eeprom_update_block(id, ((uint8_t*)addr) + 4, size);
}

uint16_t	updateData(uint16_t* addr, const void* id, uint16_t idSize, const void *value, uint16_t valueSize)
{
	updateBlockID(addr, id, idSize);
	eeprom_update_block(value, ((uint8_t*)addr) + 4 + idSize, valueSize);
}

/// @brief Return 2 (which is ID size), and assign it to ID
/// @param buffer 
/// @return 
static uint16_t	getID(char* buffer, void* id)
{
#if ID_STR == 0
	*((uint16_t*)id) = *((uint16_t*)buffer);
	return (2);
#elif ID_STR == 1
	uint16_t i;
	for (i = 0; i < 254 && buffer[i]; i++)
		((char*)id)[i] = buffer[i];
	((char*)id)[i++] = '\0'; //We want to count '\0' in buffer size
	return (i);
#endif
}

static uint16_t	compID(const char* id1, const char* id2)
{
#if ID_STR == 0
	return (ft_strncmp(id1, id2, 2));
#elif ID_STR == 1
	return (ft_strcmp(id1, id2));
#endif
}

static uint8_t	readBlock(uint16_t* address, void* id, void* value)
{
	uint8_t		isInvalid;
	uint16_t	dataUsed, dataSize;
	char		buffer[256] = {0};

	if ((isInvalid = checkBlockInvalid(address)))
		return (isInvalid);
	dataUsed = getDataUsed(address);
	if (dataUsed == 0x0000) //If empty block
	{
		if (id)
			ft_memcpy(id, buffer, 2);
		if (value)
			ft_memcpy(value, buffer, 2);
		return (0);
	}
	eeprom_read_block(buffer, ((uint8_t*)address) + 2 + 2, dataUsed);
	uint16_t idSize = getID(buffer, id);
	if (value)
	{
		ft_memcpy(value, buffer + idSize, dataUsed - idSize);
		((uint8_t*)value)[dataUsed - idSize] = '\0';
	}
	return (0);
}

static uint8_t	getNextBlock(uint16_t** cursor)
{
	uint16_t	blockSize;
	uint8_t		isInvalid;

	if (cursor == NULL)
		return (EEPROM_ERROR_NULL);
	if ((isInvalid = checkBlockInvalid(*cursor)))
		return (isInvalid);
	blockSize = getBlockSize(*cursor);
	if ((uint16_t)*cursor + blockSize >= EEPROM_MAX_ADDRESS) //If it was the last block.
		return (EEPROM_ERROR_MAX_ADDRESS);
	*((uint16_t*)cursor) += blockSize;
	return (0);
}

static uint16_t*	findID(void* id, uint16_t size)
{
	uint16_t*	cursor = (uint16_t*)EEPROM_START_ADDRESS;
	uint8_t		ret = 0;

	// ft_memset(value_buffer, 0, 256);
	// ft_memset(id_buffer, 0, 256);
	do
	{
		ret = readBlock(cursor, id_buffer, value_buffer);
		if (!ret && compID(id, id_buffer) == 0)
			break;
		ret = getNextBlock(&cursor);
	} while (ret == 0);
	if (ret == EEPROM_ERROR_MAX_ADDRESS)
		return (0);
	return (cursor);
}

static uint16_t*	findFirstFreeBlock(uint16_t blockSize)
{
	uint16_t*	cursor = (uint16_t*)EEPROM_START_ADDRESS;
	uint8_t		ret = 0;
	const char	emptyID[2] = {0};
	uint16_t	size;
	do
	{
		ret = readBlock(cursor, id_buffer, value_buffer);
		if (!ret && compID(emptyID, id_buffer) == 0) //If it is a free block
		{
			size = getDataSize(cursor);
			//If it's an appropriate match
			if (size >= blockSize && size <= (blockSize * 2))
				break;
		}
		ret = getNextBlock(&cursor);
	} while (ret == 0);
	if (ret == EEPROM_ERROR_MAX_ADDRESS)
		return (0);
	return (cursor);
}

static uint16_t*	fragmentFirstFreeBlock(uint16_t dataSize)
{
	uint16_t*	cursor = (uint16_t*)EEPROM_START_ADDRESS, *newBlockAddr;
	uint8_t		ret = 0;
	const char	emptyID[2] = {0};
	uint16_t	size;
	do
	{
		ret = readBlock(cursor, id_buffer, value_buffer);
		if (!ret && compID(emptyID, id_buffer) == 0) //If it is a free block
		{
			size = getDataSize(cursor);
			//Make sure there will be enough place for splitting the block in 2 different valid bocks
			if (size > dataSize + 2 + 2 + 3) //3 is the minimum data size
				break;
		}
		ret = getNextBlock(&cursor);
	} while (ret == 0);
	if (ret)
		return (0);
	updateDataSize(cursor, dataSize); //Update block data size
	newBlockAddr = (uint16_t*)((uint16_t)cursor + 2 + 2 + dataSize); //update new block
	updateDataSize(newBlockAddr, size - dataSize - 2 -2);
	updateDataUsed(newBlockAddr, 0x00);
	updateBlockID(newBlockAddr, emptyID, 2);
	return (cursor);
}

uint8_t	eeprom_alloc_read(char* id, char* buffer)
{
	uint16_t*	address;
	uint8_t		ret = 0;
	uint16_t	dataSize = 0;
	uint16_t	idSize = ft_strlen(id) + 1;

	address = findID(id, idSize);
	if (address == 0)
		return (EEPROM_ERROR_ID_NOT_FOUND);
	dataSize = getBlockSize(address);
	ft_memcpy(buffer, value_buffer, dataSize - idSize);
	buffer[dataSize - idSize] = '\0';
	if (255 < dataSize - idSize) //Need to change this for strings id
		return (EEPROM_ERROR_BUFFER_OVERFLOW);
	return (0);
}

uint8_t	eeprom_alloc_free(char* id)
{
	const uint16_t	nullID = 0x0000; //No very usefull
	
	uint16_t*	address = findID(id, ft_strlen(id) + 1);
	if (address == 0)
		return (EEPROM_ERROR_ID_NOT_FOUND);
	updateDataUsed(address, 0x0000);
	updateBlockID(address, &nullID, 2); //Not usefull
	//May want to implement defragmentation
	return (0);
}

uint8_t	eeprom_alloc_write(char* id, char* buffer)
{
	uint16_t*	address;
	uint16_t	dataSize;
	uint16_t	bufferSize = ft_strlen(buffer) + 1;
	uint16_t	idSize = ft_strlen(id) + 1;
	uint8_t		ret = 0;

	if (idSize < 2 || idSize > 254 || bufferSize < 2 || bufferSize > 254 || bufferSize + idSize > 256)
		return (EEPROM_ERROR_BAD_SIZE);
	if (id == 0x0000)
		return (EEPROM_ERROR_BAD_ID);
	address = findID(id, idSize);
	if (address != 0)
	{
		dataSize = getDataSize(address);
		if (dataSize - idSize < bufferSize)
		{
			if (ret = eeprom_alloc_free(id))
				return (ret);
			if (ret = eeprom_alloc_write(id, buffer))
				return (ret);
			return (0);
		}
	}
	else
	{
		address = findFirstFreeBlock(idSize + bufferSize);
		if (address == 0) //If no appropriate match
			address = fragmentFirstFreeBlock(idSize + bufferSize);
		if (address == 0) //If no block could be fragmented to given size
			return (EEPROM_ERROR_FULL);
	}
	updateData(address, id, idSize, buffer, bufferSize);
	updateDataUsed(address, bufferSize + idSize);
	return (0);
}

uint8_t	eeprom_init(void)
{
	const uint8_t	buffer[5] = {0}; 
	uint16_t		word;
	uint8_t	magic_buffer[MAGIC_NUMBER_SIZE];

	eeprom_read_block(magic_buffer, 0x00, MAGIC_NUMBER_SIZE);
	if (ft_memcmp(magic_buffer, &MAGIC_NUMBER, MAGIC_NUMBER_SIZE) != 0)
	{
		eeprom_write_block(&MAGIC_NUMBER, 0x00, sizeof(MAGIC_NUMBER)); //write MAGIC number
		//Write the first free block which is he full meomory, size_max = 1011 = 1023 - 8 - 4
		eeprom_write_word((uint16_t*)EEPROM_START_ADDRESS, EEPROM_MAX_ADDRESS - EEPROM_START_ADDRESS - 2 - 2);
		//Initialize to 0 block usage, ID and first byte of value
		eeprom_write_block(buffer, (uint8_t*)(EEPROM_START_ADDRESS + 2), sizeof(buffer));
		return (EEPROM_ERROR_UNINTIALIZED);
	}
	return (0);
}

void	eeprom_print_blocks(void)
{
	uint16_t*	cursor = (uint16_t*)EEPROM_START_ADDRESS;
	uint8_t		ret = 0;
	uint16_t	dataUsed = 0;

	do
	{
		ret = readBlock(cursor, id_buffer, value_buffer);
		printHexa16((uint16_t)cursor);
		if (ret)
			break;	
		dataUsed = getDataUsed(cursor);
		print(" [ ", 0);
		printHexa16(getDataSize(cursor));
		print(" ", 0);
		printHexa16(dataUsed);
		print(" ", 0);
		if (dataUsed)
		{
			print(id_buffer, 0);
			print(" ", 0);
			print(value_buffer, 0);
		}
		else
			print("0x00", 0);
		print(" ", 0);
		print("]\n\r", 0);
		ret = getNextBlock(&cursor);
	} while (ret == 0);
}

