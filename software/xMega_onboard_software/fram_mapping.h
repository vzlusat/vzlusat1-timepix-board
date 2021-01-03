/*
 * fram_mapping.h
 *
 * Created: 25.6.2015 15:13:09
 *  Author: klaxalk
 */ 


#ifndef FRAM_MAPPING_H_
#define FRAM_MAPPING_H_

// define address spaces for images and working space
#define IMAGE_MEMORY_OFFSET				((unsigned long) 1024)
#define RAW_IMAGE_START_ADDRESS			IMAGE_MEMORY_OFFSET
#define FILTERED_IMAGE_START_ADDRESS	((unsigned long) 65536) + IMAGE_MEMORY_OFFSET
#define WORKING_SPACE_START_ADDRESS		((unsigned long) 131072) + IMAGE_MEMORY_OFFSET

// define additional addresses
#define BOOT_COUNT_ADDRESS				((unsigned long) 0)	// uint16t
#define IMAGE_ID_ADDRESS				((unsigned long) 2)	// uint16t
#define FRAM_TEST_ADDRESS				((unsigned long) 4) // uint8_t
#define IMAGE_PARAMETERS_ADDRESS		((unsigned long) 20)  // imageParameters_t
#define MEDIPIX_BOOTUP_MESSAGE			((unsigned long) 84) // bootup message from medipix is stored here
#define ENERGY_HISTOGRAM_ADRESS			((unsigned long) 256)

#endif /* FRAM_MAPPING_H_ */