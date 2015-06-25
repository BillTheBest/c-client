/*
 * flowthings_io.h
 *
 * This file must be included to use the flowthings.io C library
 *
 *  Created on: May 20, 2015
 */

#ifndef FLOWTHINGS_IO_H_
#define FLOWTHINGS_IO_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#ifdef  __cplusplus
extern "C" {
#endif

/***********************************************************************
 * Host and version defaults
 ***********************************************************************/

#define FLOWTHINGS_IO_VERSION "0.1"
#define FLOWTHINGS_IO_HOST "api.flowthings.io"
#define FLOWTHINGS_IO_ID_LEN 40

/***********************************************************************
 * Define BOOL and FAIL if it doesn't exist
 ***********************************************************************/

#ifndef BOOL
typedef enum BOOL {
	FALSE,
	TRUE
} BOOL;
#endif

#ifndef FAIL
#define FAIL do { fprintf(stderr, "failure at %s %d", __FILE__, __LINE__); exit(1); } while (0)
#endif

/***********************************************************************
 * Error codes from flowthings functions
 ***********************************************************************/

/*
 * NAME: flowthings_io_result_code
 *
 * Possible result codes from the flowthings_io_service functions.
 */
typedef enum flowthings_io_result_code {
	FLOWTHINGS_IO_OK,
	FLOWTHINGS_IO_ERROR_COULDNT_DECODE,
	FLOWTHINGS_IO_ERROR_COULDNT_ENCODE,
	FLOWTHINGS_IO_ERROR_NOT_INITIALIZED,
	FLOWTHINGS_IO_ERROR_MALFORMED_RESPONSE,
	FLOWTHINGS_IO_ERROR_NOT_FOUND,
	FLOWTHINGS_IO_ERROR_FORBIDDEN,
	FLOWTHINGS_IO_ERROR_BAD_REQUEST,
	FLOWTHINGS_IO_ERROR_SERVER_ERROR,
	FLOWTHINGS_IO_ERROR_UNKNOWN,
} flowthings_io_result_code;

/*
 * NAME: flowthings_io_token
 *
 * A token to be passed to the platform.
 */
typedef struct flowthings_io_token {

	const char *account;
	const char *token;

} flowthings_io_token;


/***********************************************************************
 * flowthings_io_string - a resizeable string object
 ***********************************************************************/

typedef struct flowthings_io_string {
	char *ptr;
	size_t len;
} flowthings_io_string;

/*
 * NAME: flowthings_io_string_init
 *
 * Initialize a flowthings_io_string object, the caller is responsible for calling
 * flowthings_io_string_cleanup when done.
 */
flowthings_io_string *flowthings_io_string_init();

/*
 * NAME: flowthings_io_string_cleanup
 *
 * Cleans up the object.
 *
 * PARAMS:
 * s - the object to free
 */
void flowthings_io_string_cleanup(flowthings_io_string *s);

/*
 * NAME: flowthings_io_string_strcat
 *
 * This is a length-save strcat for flowthings_io_strings, which ensures that there's
 * no buffer overrun.
 */
void flowthings_io_string_strcat(flowthings_io_string *dest, const char *src);

/*
 * NAME: flowthings_io_strcat
 *
 * This is a length-save strcat for char* strings, which ensures that there's
 * no buffer overrun.
 */
void flowthings_io_strcat(char *dest, const char *src, int max_length);


/***********************************************************************
 * flowthings_io_params - a linked list of key => value objects.
 ***********************************************************************/

typedef struct flowthings_io_param {
	flowthings_io_string *key;
	flowthings_io_string *value;
	struct flowthings_io_param *next;
} flowthings_io_param;

typedef struct flowthings_io_params {
	flowthings_io_param *start;
} flowthings_io_params;

/*
 * NAME: flowthings_io_params_init
 *
 * Initialize a params object, the caller is responsible for calling
 * flowthings_io_params_cleanup when done.
 */
flowthings_io_params *flowthings_io_params_init();

/*
 * NAME: flowthings_io_params_cleanup
 *
 * Cleans up the object. This will cleanup the flowthings_io_string objects
 * it uses.
 *
 * PARAMS:
 * params - the object to free
 */
void flowthings_io_params_cleanup(flowthings_io_params *params);

/*
 * NAME: flowthings_io_params_add
 *
 * Adds a param (new_key = new_value) to the params object.
 *
 * PARAMS:
 * new_key - the new key to add
 * new_value - the corresponding value
 */
void flowthings_io_params_add(flowthings_io_params *params, const char *key, const char *value);

/*
 * NAME: flowthings_io_params_to_url
 *
 * Adds the params to the passed URL.  For example, if params is [{ id=3 }, { name=bb }]
 * it will add ?id=3&name=bb to the URL.
 *
 * PARAMS:
 * params - the params object
 * url - the URL string, which must be allocated and < max_length
 * max_length - the max length allocated for the url; this function will fail if the
 *     params are too long
 */
void flowthings_io_params_to_url(flowthings_io_params *params,
		char *url,
		int max_length);


/***********************************************************************
 * flowthings_io_idlist - A linked list of id => object.
 ***********************************************************************/

typedef struct flowthings_io_idlistitem {
	char id[FLOWTHINGS_IO_ID_LEN];
	void *item;
	struct flowthings_io_idlistitem *next;
} flowthings_io_idlistitem;

typedef struct flowthings_io_idlist {
	flowthings_io_idlistitem *start;
} flowthings_io_idlist;


/*
 * NAME: flowthings_io_idlist_init
 *
 * Initialize an ID list, the caller is responsible for calling
 * flowthings_io_idlist_cleanup when done.
 */
flowthings_io_idlist *flowthings_io_idlist_init();

/*
 * NAME: flowthings_io_idlist_cleanup
 *
 * Cleans up an ID list. NOTE: This function will not clean up the void* in
 * item; client is responsible for that.
 *
 * PARAMS:
 * idlist - the ID list item
 */
void flowthings_io_idlist_cleanup(flowthings_io_idlist *idlist);

/*
 * NAME: flowthings_io_idlist_add
 *
 * Add an item pointer to an ID list
 *
 * PARAMS:
 * idlist - the ID list item
 * id - the ID of the item, cannot be NULL
 * item - a pointer to the item, or NULL to just have a list of IDs
 */
void flowthings_io_idlist_add(flowthings_io_idlist *idlist, const char *key, void *item);


#ifdef  __cplusplus
}
#endif


#endif /* FLOWTHINGS_IO_H_ */
