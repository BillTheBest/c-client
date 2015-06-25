/*
 * flowthings_io.c
 *
 *  Created on: May 20, 2015
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#ifdef  __cplusplus
extern "C" {
#endif

#include "flowthings_io.h"
#include "flowthings_io_http.h"


/***********************************************************************
 * The flowthings_io_string functions
 ***********************************************************************/

/*
 * NAME: flowthings_io_string_init
 *
 * Initialize a flowthings_io_string object, the caller is responsible for calling
 * flowthings_io_string_cleanup when done.
 */
flowthings_io_string *flowthings_io_string_init()
{
	flowthings_io_string *s = malloc(sizeof(flowthings_io_string));

	if (s == NULL) {
		FAIL;
	}

	s->len = 0;
	s->ptr = malloc(s->len + 1);

	if (s->ptr == NULL) {
		FAIL;
	}

	s->ptr[0] = '\0';

	return s;
}

/*
 * NAME: flowthings_io_string_cleanup
 *
 * Cleans up the object.
 *
 * PARAMS:
 * s - the object to free
 */
void flowthings_io_string_cleanup(flowthings_io_string *s)
{
	free(s->ptr);
	free(s);
}

/*
 * NAME: flowthings_io_strcat
 *
 * This is a length-save strcat for char* strings, which ensures that there's
 * no buffer overrun.
 */
void flowthings_io_strcat(char *dest, const char *src, int max_length)
{
	if (!src || !dest) FAIL;

	if (strlen(src) + strlen(dest) + 1 > max_length) {
		FAIL;
	}
	else {
		strcat(dest, src);
	}
}

/*
 * NAME: flowthings_io_string_strcat
 *
 * This is a length-save strcat for flowthings_io_strings, which ensures that there's
 * no buffer overrun.
 */
void flowthings_io_string_strcat(flowthings_io_string *dest, const char *src)
{
	if (!src || !dest) FAIL;

	size_t new_len = dest->len + (sizeof(char) * strlen(src));
	dest->ptr = realloc(dest->ptr, new_len+1);
	if (!dest->ptr) FAIL;

	memcpy(dest->ptr + dest->len, src, new_len);
	dest->ptr[new_len] = '\0';
	dest->len = new_len;
}


/***********************************************************************
 * The flowthings_io_params functions
 ***********************************************************************/

/*
 * NAME: flowthings_io_params_init
 *
 * Initialize a params object, the caller is responsible for calling
 * flowthings_io_params_cleanup when done.
 */
flowthings_io_params *flowthings_io_params_init()
{
	flowthings_io_params *params = malloc(sizeof(flowthings_io_params));

	if (!params) {
		FAIL;
	}

	params->start = NULL;

	return params;
}

/*
 * NAME: flowthings_io_params_cleanup
 *
 * Cleans up the object. This will cleanup the flowthings_io_string objects
 * it uses.
 *
 * PARAMS:
 * params - the object to free
 */
void flowthings_io_params_cleanup(flowthings_io_params *params)
{
	if (!params) {
		FAIL;
	}

	while (params->start != NULL) {

		flowthings_io_param *param = params->start;
		params->start = param->next;

		if (param->key)
			flowthings_io_string_cleanup(param->key);
		if (param->value)
			flowthings_io_string_cleanup(param->value);

		free(param);
	}

	free(params);
}

/*
 * NAME: flowthings_io_params_add
 *
 * Adds a param (new_key = new_value) to the params object.
 *
 * PARAMS:
 * new_key - the new key to add
 * new_value - the corresponding value
 */
void flowthings_io_params_add(flowthings_io_params *params, const char *new_key, const char *new_value)
{
	if (!params || !new_key || !new_value) {
		FAIL;
	}

	flowthings_io_param *old_start = params->start;
	flowthings_io_param *new_param = malloc(sizeof(flowthings_io_param));

	if (!new_param) {
		FAIL;
	}

	flowthings_io_string *key = flowthings_io_string_init();
	flowthings_io_string *value = flowthings_io_string_init();
	flowthings_io_string_strcat(key, new_key);
	flowthings_io_string_strcat(value, new_value);

	new_param->key = key;
	new_param->value = value;
	new_param->next = old_start;

	params->start = new_param;
}

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
		int max_length)
{
	if (!params || !url) {
		FAIL;
	}

	flowthings_io_param *param = params->start;

	if (param)
		flowthings_io_strcat(url, "?", max_length);

	while (param != NULL) {

		if (!param->key || !param->value) {
			FAIL;
		}

		flowthings_io_string *encoded_value = flowthings_io_string_init();
		flowthings_io_http_urlencode(param->value->ptr, encoded_value);

		flowthings_io_strcat(url, param->key->ptr, max_length);
		flowthings_io_strcat(url, "=", max_length);
		flowthings_io_strcat(url, encoded_value->ptr, max_length);
		flowthings_io_strcat(url, "&", max_length);

		flowthings_io_string_cleanup(encoded_value);

		param = param->next;
	}
}


/***********************************************************************
 * The flowthings_io_idlist functions
 ***********************************************************************/

/*
 * NAME: flowthings_io_idlist_init
 *
 * Initialize an ID list, the caller is responsible for calling
 * flowthings_io_idlist_cleanup when done.
 */
flowthings_io_idlist *flowthings_io_idlist_init()
{
	flowthings_io_idlist *idlist = malloc(sizeof(flowthings_io_idlist));

	if (!idlist) {
		FAIL;
	}

	idlist->start = NULL;

	return idlist;
}

/*
 * NAME: flowthings_io_idlist_cleanup
 *
 * Cleans up an ID list. NOTE: This function will not clean up the void* in
 * item; client is responsible for that.
 *
 * PARAMS:
 * idlist - the ID list item
 */
void flowthings_io_idlist_cleanup(flowthings_io_idlist *idlist)
{
	if (!idlist) {
		FAIL;
	}

	while (idlist->start != NULL) {

		flowthings_io_idlistitem *idlistitem = idlist->start;
		idlist->start = idlistitem->next;

		// the client is responsible for freeing this
		idlistitem->item = NULL;

		free(idlistitem);
	}

	free(idlist);
}


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
void flowthings_io_idlist_add(flowthings_io_idlist *idlist, const char *id, void *item)
{
	if (!idlist || !id) {
		FAIL;
	}

	flowthings_io_idlistitem *old_start = idlist->start;
	flowthings_io_idlistitem *new_idlistitem = malloc(sizeof(flowthings_io_idlistitem));

	if (!new_idlistitem) {
		FAIL;
	}

	strcpy(new_idlistitem->id, "");
	flowthings_io_strcat(new_idlistitem->id, id, FLOWTHINGS_IO_ID_LEN);
	new_idlistitem->item = item;
	new_idlistitem->next = old_start;

	idlist->start = new_idlistitem;
}

#ifdef  __cplusplus
}
#endif
