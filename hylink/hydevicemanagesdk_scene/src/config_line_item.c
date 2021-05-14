
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "config_line_item.h"
#include "error_no.h"


/**
 * Read a configuration line from a file.<BR>
 * A configuration line is composed of a unique key, which
 * is playing the role of index, and several elements. All are
 * separated by a delimiter and terminated by a carriage
 * return.<BR>
 * When line is read, it is put inside a provided buffer, and
 * then processed. If the searched key is found, the provided
 * element table is filled with pointers to each found line
 * elements.If the key is an empty string, the key of the
 * first line is returned into the key buffer.<BR>
 * Key buffer must not be smaller than ML_MAX_KEY_LEN.<BR>
 * When line elements are returned, the key buffer if filled
 * with the next found key in the next line. If there is no more
 * line to read, the key buffer contains a empty string. The
 * element number parameter is updated with the number of
 * elements filled into the element table.
 *
 * \param filename full path filename of a configuration file
 * \param delimiters string containing all accepted delimiters
 * (usually space and tab characters)
 * \param key key of the line to read; updated with the key of
 * the next line; key buffer size must be ML_CONFIG_KEY_MAX_LEN
 * \param elt_number maximum number of elements to read, updated
 * with the real number of read elements if less is found
 * \param elt array of pointers where are stored pointers to
 * elements
 * \param buffer whole line working buffer
 * \param buffer_len working buffer length
 * \param user_data pointer to open file. if this pointer is different from
 * zero, after successfull reading it points to open file. If the end of file
 * is reached pointer is zero. If some error occur during reading file is
 * closed and pointer is nullified. When this pointer is zero, file is open
 * and read from the beginning. It is used to repetitively read whole file
 * line by line. Usefull when file contains a lot of entries.
 * \return error type (ERROR_SUCCESS if success)
 * \return ERROR_PARAM: bad input parameters
 * \return ERROR_NOT_FOUND: key not found or no more
 * line to read
 * \return ERROR_NO_SPACE: no enough space in buffer or
 * to much elements to put into element table
 * \return ERROR_SYSTEM: system error, see errno
 */

extern int
config_read_line_repetitive (
	const char *filename, 
	const char* delimiters, 
	char *key, 
	unsigned int *elt_number,
	char *elt[], 
	char *buffer,
	unsigned int buffer_len,
	void ** user_data
)
{
    FILE *fp = NULL;
    char line_buffer[CONFIG_LINE_MAX_LEN];
    char line2_buffer[CONFIG_LINE_MAX_LEN];
    char *ptr, *strtok_ctx;
#ifdef USE_LOCK
    struct flock lock;
#endif /* USE_LOCK */
    unsigned int max_elt_number;
    int is_key_found;
    int status = ERROR_PARAM;

    /* check input parameters */
    if ((filename == NULL) || (delimiters == NULL) || (key == NULL) || (elt_number == NULL)
            || (elt == NULL) || (buffer == NULL))
    {
        goto read_cleanup;
    }

    if ((strlen (delimiters) <= 0) || (*elt_number > CONFIG_ELT_MAX_NB)
            || (buffer_len <= 0))
    {
        goto read_cleanup;
    }

    /* check delimiter validity */
    if (strchr (delimiters, '\n') || strchr (delimiters, '\r'))
    {
        goto read_cleanup;
    }

    /* check key validity */
    if (strlen (key) > CONFIG_KEY_MAX_LEN)
    {
        goto read_cleanup;
    }

    if (NULL != strpbrk (key, delimiters))
    {
        goto read_cleanup;
    }

    /* User already opened file. */
    if (user_data && *user_data)
    {
        fp = (FILE *)*user_data;
    }
    else
    {
        /* open config file */
        if ((fp = fopen (filename, "r")) == NULL)
        {
            return ERROR_SYSTEM;
        }
#ifdef USE_LOCK
        /* set the read lock */
        lock.l_type = F_RDLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = 0;
        lock.l_len = 0;
        if (fcntl (fileno(fp), F_SETLKW, &lock) < 0)
        {
            fclose (fp);
            return ERROR_SYSTEM;
        }
#endif /* USE_LOCK */
        if (user_data)
            *user_data = fp;
    }
    max_elt_number = *elt_number;
    *elt_number = 0;
    is_key_found = 0;

    if (strlen (key) > 0)
    {
        while (NULL != fgets (buffer, buffer_len, fp))
        {
            /* remove the newline character*/
            if (buffer[strlen (buffer) - 1] == '\n')
            {
                if (buffer[strlen (buffer) - 2] == '\r')
                {
                    /* DOS format */
                    buffer[strlen (buffer) - 2] = '\0';
                }
                else
                {
                    /* Unix format */
                    buffer[strlen (buffer) - 1] = '\0';
                }
            }
            /* chops chars until end of line */
            else if (NULL == fgets (line_buffer, CONFIG_LINE_MAX_LEN - 1, fp))
            {
#ifdef USE_LOCK
              /* close lock */
              fcntl (fileno (fp), F_UNLCK, &lock);
#endif /* USE_LOCK */
              /* close file */
              fclose (fp);
              if (user_data)
                  *user_data = NULL;
              return ERROR_SYSTEM;
            }
            /* get the index */
            ptr = strtok_r (buffer, delimiters, &strtok_ctx);
            /* check if it is looked key */
            if ((ptr == NULL) || (strlen (ptr) <= 0) || (*ptr == '#') || strcmp (ptr, key))
                continue;

            is_key_found = 1;

            /* get all elements */
            while ((ptr = strtok_r (NULL, delimiters, &strtok_ctx)) && (*elt_number < max_elt_number))
            {
                elt[*elt_number] = ptr;
                *elt_number += 1;
            }
            break;
        }
    }
    else
    {
        /* just find the first key */
        is_key_found = 1;
    }
    /* check if key has been found */
    if (!is_key_found)
    {
#ifdef USE_LOCK
        /* close lock */
        fcntl (fileno (fp), F_UNLCK, &lock);
#endif /* USE_LOCK */
        /* close file */
        fclose (fp);
        if (user_data)
            *user_data = NULL;
        return ERROR_NOT_FOUND;
    }
    /* get next key */
    *key = '\0';
    long pos = ftell(fp);
    while (NULL != fgets (line2_buffer, CONFIG_LINE_MAX_LEN, fp))
    {
        /* remove the newline */
        if (line2_buffer[strlen (line2_buffer) - 1] == '\n')
        {
            line2_buffer[strlen (line2_buffer) - 1] = '\0';
        }
        /* get the index */
        ptr = strtok_r (line2_buffer, delimiters, &strtok_ctx);
        /* check if it is looked key */
        if ((ptr == NULL) || (strlen(ptr) <= 0) || (*ptr == '#')
            || (strlen(ptr) >= CONFIG_KEY_MAX_LEN))
            continue;
        strcpy (key, ptr);
        break;
    }
    fseek (fp, pos, SEEK_SET);

    status = ERROR_SUCCESS;

    read_cleanup:
    /* On error close the open file and clear pointer. */
    if (!user_data || status != ERROR_SUCCESS)
    {
        if (user_data && *user_data)
        {
            fp = (FILE *)*user_data;
            *user_data = NULL;
        }
#ifdef USE_LOCK
        /* close lock */
        if (fp)
            fcntl (fileno (fp), F_UNLCK, &lock);
#endif /* USE_LOCK */
        /* close file */
        if (fp)
            fclose (fp);
    }

    return status;
}

/**
 * Read a configuration line from a file.<BR>
 * A configuration line is composed of a unique key, which
 * is playing the role of index, and several elements. All are
 * separated by a delimiter and terminated by a carriage
 * return.<BR>
 * When line is read, it is put inside a provided buffer, and
 * then processed. If the searched key is found, the provided
 * element table is filled with pointers to each found line
 * elements.If the key is an empty string, the key of the
 * first line is returned into the key buffer.<BR>
 * Key buffer must not be smaller than ML_MAX_KEY_LEN.<BR>
 * When line elements are returned, the key buffer if filled
 * with the next found key in the next line. If there is no more
 * line to read, the key buffer contains a empty string. The
 * element number parameter is updated with the number of
 * elements filled into the element table.
 *
 * \param filename full path filename of a configuration file
 * \param delimiters string containing all accepted delimiters
 * (usually space and tab characters)
 * \param key key of the line to read; updated with the key of
 * the next line
 * \param elt_number maximum number of elements to read, updated
 * with the real number of read elements if less is found
 * \param elt array of pointers where are stored pointers to
 * elements
 * \param buffer whole line working buffer
 * \param buffer_len working buffer length
 * \param user_data pointer to open file. if this pointer is different from
 * zero, after successfull reading it points to open file. If the end of file
 * is reached pointer is zero. If some error occur during reading file is
 * closed and pointer is nullified. When this pointer is zero, file is open
 * and read from the beginning. It is used to repetitively read whole file
 * line by line. Usefull when file contains a lot of entries.
 * \return error type (ERROR_SUCCESS if success)
 * \return ERROR_PARAM: bad input parameters
 * \return ERROR_NOT_FOUND: key not found or no more
 * line to read
 * \return ERROR_NO_SPACE: no enough space in buffer or
 * to much elements to put into element table
 * \return ERROR_SYSTEM: system error, see errno
 */
extern int
config_read_line (
	const char *filename, 
	const char* delimiters,
	char *key,
	unsigned int *elt_number, 
	char *elt[], 
	char *buffer, 
	unsigned int buffer_len
)
{
    return config_read_line_repetitive (
				filename,
				delimiters, 
				key, 
				elt_number,
				elt, 
				buffer,
				buffer_len,
				0
			);
}

/**
 * Write a configuration line into a configuration file.<BR>
 * A configuration line is made of a key and several elements,
 * separated by a delimiter.<BR>
 * A key item is used as unique line reference. If the key
 * already exists, the line is updated with new elements.
 * If the key does not exist, the line is added to the end of
 * the file.<BR>
 * The key length must not be longer than ML_KEY_MAX_LEN.
 *
 * \param filename full path filename of a configuration file
 * \param delimiter char used for delimiting key and elements
 * \param key the key (index) of the line
 * \param elt_number number of elements to write
 * \param elt list of elements to write
 * \result error type (ERROR_SUCCESS if success)
 * \return ERROR_PARAM: bad input parameters
 * \return ERROR_NO_SPACE: no enough space in buffer or
 * to much elements to put into element table
 * \return ERROR_SYSTEM: system error, see errno
 */

extern int
config_write_line (
	const char *filename, 
	const char delimiter,
	const char *key,
	unsigned int elt_number,
	char *elt[]
	)
{
    char buffer[CONFIG_LINE_MAX_LEN], key_buffer[CONFIG_KEY_MAX_LEN + 1];
    char out_filename[64];
    char *ptr, *strtok_ctx;
    FILE *fp_in, *fp_out;
    int fd_in;
    unsigned int index = 0;
    int is_key_found = 0;
    char delimiters[8] = CONFIG_DELIMITER " \t";
#ifdef USE_LOCK
    struct flock lock;
#endif /* USE_LOCK */
    char key_temp[CONFIG_KEY_MAX_LEN + 1];

    /* add given delimiter to the set of delimiters */
    strncat (delimiters, &delimiter, 1);

    /* check input parameters */
    if ((filename == NULL) || (key == NULL)  || (elt == NULL) || (elt_number > CONFIG_ELT_MAX_NB))
    {
        return ERROR_PARAM;
    }

    /* check sizes */
    if ((strlen (key) == 0))
    {
        return ERROR_PARAM;
    }

    /* check delimiter validity */
    if ((delimiter == '\n') || (delimiter == '\r'))
    {
        return ERROR_PARAM;
    }

    /* check key validity */
    if (strchr (key, delimiter) != NULL)
    {
        return ERROR_PARAM;
    }
    if (strlen (key) > CONFIG_KEY_MAX_LEN)
    {
        return ERROR_PARAM;
    }

    strcpy (key_temp, key);
	
    /* open config file */
    if ((fd_in = open (filename, O_RDWR | O_CREAT, 0666)) < 0)
    {
        return ERROR_SYSTEM;
    }
    fp_in = fdopen (fd_in, "r+");
    /* create the modified config file */
    sprintf (out_filename, "%sXXXXXX", filename);
    if ((fp_out = fdopen (mkstemp (out_filename), "w")) == NULL)
    {
        fclose (fp_in);
        close (fd_in);
        return ERROR_SYSTEM;
    }

#ifdef USE_LOCK
    /* set the write lock */
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    if (fcntl (fd_in, F_SETLKW, &lock) < 0)
    {
        fclose (fp_out);
        fclose (fp_in);
        close (fd_in);
        return ERROR_SYSTEM;
    }
#endif /* USE_LOCK */
    /* find the label to modify */
    while (NULL != fgets (buffer, CONFIG_LINE_MAX_LEN - 1, fp_in))
    {
        /* fill working copy for key */
        memset(key_buffer, '\0', CONFIG_KEY_MAX_LEN + 1);
        strncpy (key_buffer, buffer, CONFIG_KEY_MAX_LEN);
		/* remove the newline character*/
        if (key_buffer[strlen (key_buffer) - 1] == '\n')
        {
            if (key_buffer[strlen (key_buffer) - 2] == '\r')
            {
                /* DOS format */
                key_buffer[strlen (key_buffer) - 2] = '\0';
            }
            else
            {
                /* Unix format */
                key_buffer[strlen (key_buffer) - 1] = '\0';
            }
        }
		
        /* get the index */
        ptr = strtok_r (key_buffer, delimiters, &strtok_ctx);
        if ((ptr == NULL) || (strlen (ptr) <= 0) || (*ptr == '#') || strcmp (ptr, key_temp))
        {
            fprintf (fp_out, "%s", buffer);
            continue;
        }

        is_key_found = 1;
        /* create the new line */
        sprintf (buffer, "%s%c", key_temp, delimiter);
        for (index = 0; index < elt_number; index++)
        {
            /* check for buffer overflow */
            if ((strlen (buffer) + 1 /*delimiter*/ + strlen (elt[index]) + 1 /*'\0'*/ ) > CONFIG_LINE_MAX_LEN)
            {
                fclose (fp_out);
                fclose (fp_in);
#ifdef USE_LOCK
                /* close lock */
                fcntl (fd_in, F_UNLCK, &lock);
#endif /* USE_LOCK */
                close (fd_in);
                return ERROR_NO_SPACE;
            }
            sprintf (buffer + strlen (buffer), "%s%c", elt[index],  delimiter);
        }
        /* string should end with last element, so erase last delimiter and finish the line */
        buffer[strlen (buffer) - 1] = '\n';
        fprintf (fp_out, "%s", buffer);
    }
    if (!is_key_found)
    {
        /* add new key + element at the end of file */
        sprintf (buffer, "%s%c", key_temp, delimiter);
        for (index = 0; index < elt_number; index++)
        {
            /* check for buffer overflow */
            if ((strlen (buffer) + 1 /*delimiter*/ + strlen (elt[index]) + 1 /*'\0'*/ ) > CONFIG_LINE_MAX_LEN)
            {
                fclose (fp_out);
                fclose (fp_in);
#ifdef USE_LOCK
                /* close lock */
                fcntl (fd_in, F_UNLCK, &lock);
#endif /* USE_LOCK */
                close (fd_in);
                return ERROR_NO_SPACE;
            }
            sprintf (buffer + strlen (buffer), "%s%c", elt[index], delimiter );
        }
        /* string should end with last element, so erase last delimiter and finish the line */
        buffer[strlen (buffer) - 1] = '\n';
        fprintf (fp_out, "%s", buffer);
    }
    fclose (fp_out);
    fclose (fp_in);
#ifdef USE_LOCK
    /* close lock */
    fcntl (fd_in, F_UNLCK, &lock);
#endif /* USE_LOCK */
    close (fd_in);
    if (rename (out_filename, filename) < 0)
    {
    }
	
	chmod(filename, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
	
    return ERROR_SUCCESS;
}

/**
 * Remove a line from a configuration file.<BR>
 * Line to remove is referred according to a provided key. A
 * line is composed of a key and several elements, separated by
 * a common delimiter.<BR>
 * The provided key must not be longer than ML_MAX_KEY_LEN.
 *
 * \param filename full path filename of a configuration file
 * \param delimiters chars used for delimiting key and elements
 * \param key key (index) of the line to remove
 * \return error type (ERROR_SUCCESS if success)
 * \return ERROR_PARAM: bad input parameters
 * \return ERROR_NOT_FOUND: key not found
 * \return ERROR_SYSTEM: system error, see errno
 */

extern int
config_remove_line (

	const char *filename, 
	const char *delimiters,
	const char *key
)
{
    char buffer[CONFIG_LINE_MAX_LEN], key_buffer[CONFIG_KEY_MAX_LEN + 1];
    char out_filename[64];
    char *ptr, *strtok_ctx;
    FILE *fp_in, *fp_out;
    int fd_in;
    int is_key_found = 0;
#ifdef USE_LOCK
    struct flock lock;
#endif /* USE_LOCK */
    char key_temp[CONFIG_KEY_MAX_LEN + 1];

    /* check input parameters */
    if ((filename == NULL) || (delimiters == NULL) || (key == NULL))
    {
        return ERROR_PARAM;
    }

    /* check sizes */
    if ((strlen (key) == 0) || (strlen (delimiters) == 0))
    {
        return ERROR_PARAM;
    }

    /* check delimiter validity */
    if (strchr (delimiters, '\n') || strchr (delimiters, '\r'))
    {
        return ERROR_PARAM;
    }

    /* check key validity */
    if (NULL != strpbrk (key, delimiters))
    {
        return ERROR_PARAM;
    }
    if (strlen (key) > CONFIG_KEY_MAX_LEN)
    {
        return ERROR_PARAM;
    }

    strcpy (key_temp, key);

    /* open config file */
    if ((fd_in = open (filename, O_RDWR | O_CREAT, 0666)) < 0)
    {
        return ERROR_SYSTEM;
    }
    fp_in = fdopen (fd_in, "r");
    /* create the modified config file */
    sprintf (out_filename, "%sXXXXXX", filename);
    if ((fp_out = fdopen (mkstemp (out_filename), "w")) == NULL)
    {
        fclose (fp_in);
        close (fd_in);
        return ERROR_SYSTEM;
    }

#ifdef USE_LOCK
    /* set the write lock */
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    if (fcntl(fd_in, F_SETLKW, &lock) < 0)
    {
        fclose (fp_out);
        fclose (fp_in);
        close (fd_in);
        return ERROR_SYSTEM;
    }
#endif /* USE_LOCK */
    /* find the label to modify */
    while (NULL != fgets (buffer, CONFIG_LINE_MAX_LEN - 1, fp_in))
    {
        /* fill working copy for key */
        memset (key_buffer, '\0', CONFIG_KEY_MAX_LEN + 1);
        strncpy (key_buffer, buffer, CONFIG_KEY_MAX_LEN);
		/* remove the newline character*/
        if (key_buffer[strlen (key_buffer) - 1] == '\n')
        {
            if (key_buffer[strlen (key_buffer) - 2] == '\r')
            {
                /* DOS format */
                key_buffer[strlen (key_buffer) - 2] = '\0';
            }
            else
            {
                /* Unix format */
                key_buffer[strlen (key_buffer) - 1] = '\0';
            }
        }
		
        /* get the index */
        ptr = strtok_r (key_buffer, delimiters, &strtok_ctx);

        if ((ptr == NULL) || (strlen (ptr) <= 0) || (*ptr == '#') || strcmp (ptr, key_temp))
        {
            fprintf (fp_out, "%s", buffer);
            continue;
        }

        is_key_found = 1;
    }
    fclose (fp_out);
    fclose (fp_in);
#ifdef USE_LOCK
    /* close lock */
    fcntl (fd_in, F_UNLCK, &lock);
#endif /* USE_LOCK */
    close (fd_in);
    if (is_key_found)
    {
        if (rename (out_filename, filename) < 0)
        {
        }
    }
    else
    {
        remove (out_filename);
        return ERROR_NOT_FOUND;
    }
	
	chmod(filename, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
	
    return ERROR_SUCCESS;
}

/**
 * Return the values of items from a configuration file.
 *
 * \param filename full path filename of a configuration file.
 * \param labels the labels of the items to read.
 * \param values the buffers where to store the the read values.
 * \param buffers_len the lengths of the buffers.
 * \param count the number of items' values to read.
 * \return error type (ERROR_SUCCESS if success)
 * \return ERROR_PARAM: bad input parameters
 * \return ERROR_NOT_FOUND: item has not been found
 * \return ERROR_SYSTEM: system error, see errno
 * \return ERROR_NO_SPACE: not enough space in buffer
 */
extern int
config_read_items (
	const char *filename, 
	const char * const labels[], 
	char *values[], 
	const int buffers_len[],
	unsigned int count
)
{
    char buffer[CONFIG_LINE_MAX_LEN];
    FILE *fp;
    char *strtok_ctx, *ptr, *blank_ptr;

    /* check input parameters */
    if((filename == NULL) || (labels == NULL) || (values == NULL)
       ||(buffers_len == NULL) || (count == 0))
    {
        return ERROR_PARAM;
    }

    unsigned int i;
    for (i = 0; i < count; i++)
    {
        if ((labels[i] == NULL) || (values[i] == NULL) || (buffers_len[i] <= 0))
        {
            return ERROR_PARAM;
        }

        /* check key validity */
        if ( strstr(labels[i], CONFIG_DELIMITER)
             || strchr(labels[i], ' ')
             || strchr(labels[i], '\t') )
        {
            return ERROR_PARAM;
        }
    }

    /* open config file */
    if ((fp = fopen (filename, "r")) == NULL)
    {
        return ERROR_SYSTEM;
    }

    char read[count]; /* whether each label was read or not in the file */
    memset (read, 0, count);
    unsigned int nb_read = 0; /* number of labels read from the file */

    while (fgets(buffer, CONFIG_LINE_MAX_LEN - 1, fp)
           && (nb_read < count))
    {
        /* get the label */
        ptr = strtok_r (buffer, CONFIG_DELIMITER " \t", &strtok_ctx);
        /* check if it is the wanted label */
        if((ptr == NULL) || (strlen(ptr) <= 0) || (*ptr == '#'))
            continue;

        for (i = 0; i < count; i++)
        {
            if (read[i])
            {
                continue;
            }

            if ( ! strcmp(ptr, labels[i]))
            {
                break;
            }
        }

        if (i >= count)
        {
            continue;
        }

        /* remove space and '=' before the value */
        ptr += strlen (ptr) + 1;
        while (isblank (*ptr) || (*ptr == *CONFIG_DELIMITER))
            ptr++;
        ptr = strtok_r (ptr, "\n\r", &strtok_ctx);
        /* init value to empty */
        *(values[i]) = '\0';
        if(ptr != NULL)
        {
            /* remove spaces at the end of value */
            blank_ptr = ptr + strlen (ptr) - 1;
            while ((blank_ptr >= ptr) && isblank (*blank_ptr))
            {
                *blank_ptr = '\0';
                blank_ptr--;
            }
            /* check for buffer space to copy the value */
            if ((int) strlen (ptr) >= buffers_len[i])
            {
                fclose (fp);
                return ERROR_NO_SPACE;
            }
            strcpy(values[i], ptr);
        }

        read[i] = 1;
        nb_read++;
    }

    fclose(fp);

    if (nb_read < count)
    {
        /* some labels were not found */
        return ERROR_NOT_FOUND;
    }

    return ERROR_SUCCESS;
}

/**
 * Read an item value inside a configuration file; put the value
 * into a provided buffer
 *
 * \param filename full path filename of a configuration file
 * \param label label of wanted item to read
 * \param value pointer to buffer where read value is written
 * \param buffer_len length of value buffer
 * \return error type (ERROR_SUCCESS if success)
 * \return ERROR_PARAM: bad input parameters
 * \return ERROR_NOT_FOUND: item has not been found
 * \return ERROR_SYSTEM: system error, see errno
 * \return ERROR_NO_SPACE: not enough space in buffer
 */
extern int
config_read_item (
	const char *filename, 
	const char *label,
	char *value,
	int buffer_len
)
{
    return config_read_items (filename, &label, &value, &buffer_len, 1);
}

/**
 * Write values of items into a configuration file
 *
 * \param filename full path filename of a configuration file
 * \param labels labels of item values to write; if item does not already exists, it will be added into the configuration file
 * \param values values of items to write
 * \param count number of items to write
 * \return error type (ERROR_SUCCESS if success)
 * \return ERROR_PARAM: bad input parameters
 * \return ERROR_SYSTEM: system error, see errno
 * \return ERROR_NO_SPACE: not enough space in buffer
 */
extern int
config_write_items (const char *filename, const char * const labels[], const char * const values[], unsigned int count)
{
    char buffer[CONFIG_LINE_MAX_LEN];
    char out_filename[64];
    char *ptr, *strtok_ctx;
    FILE *fp_in, *fp_out;
    int fd_in;
    char *tmp_line;

    /* check input parameters */
    if ((filename == NULL) || (labels == NULL) || (values == NULL)
        || (count == 0))
    {
        return ERROR_PARAM;
    }

    unsigned int i;
    for (i = 0; i < count; i++)
    {
        /* check key validity */
        if ((labels[i] == NULL) || (strlen(labels[i]) <= 0)
            || strstr(labels[i], CONFIG_DELIMITER)
            || strchr(labels[i], ' ') || strchr(labels[i], '\t')
            || (values[i] == NULL))
        {
            return ERROR_PARAM;
        }
    }
	
	chmod(out_filename, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
	
    /* open config file */
    if ((fd_in = open (filename, O_RDONLY | O_CREAT, 0666)) < 0)
    {
        return ERROR_SYSTEM;
    }
    fp_in = fdopen (fd_in, "r");
    /* create the modified config file */
    sprintf (out_filename, "%sXXXXXX", filename);
    if ((fp_out = fdopen (mkstemp (out_filename), "w")) == NULL)
    {
        fclose (fp_in);
        close (fd_in);
        return ERROR_SYSTEM;
    }

    char written[count]; /* keep track of written items */
    memset (written, 0, count);
    unsigned int nb_written = 0; /* number of written items */

    /* find the label to modify */
    while (NULL != fgets (buffer, CONFIG_LINE_MAX_LEN - 1, fp_in))
    {
        tmp_line = strdup (buffer);

        /* get the label */
        ptr = strtok_r(buffer, CONFIG_DELIMITER " \t", &strtok_ctx);

        int copy_line = 0;
        if ( (ptr == NULL) || (strlen(ptr) <= 0) || (*ptr == '#') )
        {
            copy_line = 1;
        }
        else
        {
            /* check if it is the wanted label */
            for (i = 0; i < count; i++)
            {
                if (written[i])
                {
                    continue;
                }

                if ( ! strcmp(ptr, labels[i]))
                {
                    break;
                }
            }
            copy_line = (i >= count);
        }
        if (copy_line)
        {
            /* print line unchanged back to file */
            fprintf (fp_out, "%s", tmp_line);
            free (tmp_line);
            continue;
        }

        free (tmp_line);

        /* check for buffer overload */
        if ( CONFIG_LINE_MAX_LEN - 1/*'\0'*/ - 1 /*'\n'*/ - ( strlen(ptr) + 1/*' '*/ + 1/*'='*/ + 1/*' '*/ ) < strlen(values[i]) )
        {
            fclose(fp_out);
            fclose(fp_in);
            return ERROR_NO_SPACE;
        }
        /* write the new value */
        fprintf(fp_out, "%s %s %s\n", ptr, CONFIG_DELIMITER, values[i]);  /* make line in format "label = value" */
        written[i] = 1;
        nb_written++;
    }

    if (nb_written < count)
    {
        for (i = 0; i < count; i++)
        {
            if (! written[i])
            {
                /* add new 'label = value' at the end of file */
                fprintf(fp_out, "%s %s %s\n", labels[i],
                        CONFIG_DELIMITER, values[i]);
            }
        }
    }

    fclose(fp_out);
    fclose(fp_in);
    if ( rename(out_filename, filename) < 0 )
    {
    }
	
	chmod(filename, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
	
    return ERROR_SUCCESS;
}

/**
 * Write a value of an item into a configuration file
 *
 * \param filename full path filename of a configuration file
 * \param label label of item value to write; if item does not already exists, it will be added into the configuration file
 * \param value value of item to write
 * \return error type (ERROR_SUCCESS if success)
 * \return ERROR_PARAM: bad input parameters
 * \return ERROR_SYSTEM: system error, see errno
 * \return ERROR_NO_SPACE: not enough space in buffer
 */
extern int
config_write_item (
	const char *filename,
	const char *label,
	const char *value
)
{
    return config_write_items (filename, &label, &value, 1);
}
