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
#define IMAGE_ID_ADDRESS				((unsigned long) 2)	// uint16t
#define BOOT_COUNT_ADDRESS				((unsigned long) 0)	// uint16t
#define IMAGE_PARAMETERS_ADDRESS		((unsigned long) 20)  // imageParameters_t

#endif /* FRAM_MAPPING_H_ */