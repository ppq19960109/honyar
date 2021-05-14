#ifndef _CONFIG_LINE_ITEM_H_
#define _CONFIG_LINE_ITEM_H_

#include "error_no.h"

#define CONFIG_DELIMITER "="

#define CONFIG_LINE_MAX_LEN 				1024
#define CONFIG_KEY_MAX_LEN 					63
#define CONFIG_ELT_MAX_NB 					32
#define CONFIG_ELE_MAX_LEN					64


extern int 
config_read_items (const char *filename,
				const char * const labels[],
				char *values[], 
				const int buffers_len[], 
				unsigned int count);

extern int 
config_read_item (const char *filename, 
				const char *label, 
				char *value,
				int buffer_len);

extern int
config_write_items (const char *filename,
				const char * const labels[],
				const char * const values[],
				unsigned int count);

extern int 
config_write_item (const char *filename,
				const char *label, 
				const char *value);


extern int
config_read_line_repetitive (const char *filename,
						const char* delimiters, 
						char *key,
						unsigned int *elt_number, 
						char *elt[],
						char *buffer, 
						unsigned int buffer_len,
						void ** user_data);

extern int
config_read_line (const char *filename,
				const char* delimiters, 
				char *key, 
				unsigned int *elt_number,
				char *elt[], 
				char *buffer, 
				unsigned int buffer_len);

extern int
config_write_line (const char *filename, 
				const char delimiter, 
				const char *key, 
				unsigned int elt_number,
				char *elt[]);

extern int
config_remove_line (const char *filename,
				const char *delimiters,
				const char *key);
				
#endif	/* _CONFIG_LINE_ITEM_H_ */
