/*
 * flowthings_io_services.c
 *
 *  Created on: May 21, 2015
 */

#ifndef FLOWTHINGS_IO_SERVICES_C_
#define FLOWTHINGS_IO_SERVICES_C_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#ifdef  __cplusplus
extern "C" {
#endif

/***********************************************************************
 * Flowthings includes
 ***********************************************************************/

#include "cJSON.h"
#include "flowthings_io.h"
#include "flowthings_io_http.h"
#include "flowthings_io_api.h"
#include "flowthings_io_services.h"


/***********************************************************************
 * Helper functions
 ***********************************************************************/

void __flowthings_io_add_path_ext(char *path, const char *path_ext)
{
	if (path_ext) {
		flowthings_io_strcat(path, "/", FLOWTHINGS_IO_MAX_PATH_SIZE);
		flowthings_io_strcat(path, path_ext, FLOWTHINGS_IO_MAX_PATH_SIZE);
	}
}


/***********************************************************************
 * The Service functions
 ***********************************************************************/

/*
 * NAME: flowthings_io_service_read
 *
 * Perform a create on valid objects from the platform.  This function should not be called directly --
 * one of the defines below should be called depending on the object type.  For example, to create a
 * drop, call flowthings_io_drop_create(...)
 *
 * PARAMS:
 * svc - the service type
 * path_ext - any path extension to add in creating the URL
 * api - the API object
 * id - the ID of the object to delete
 * params - any additional query string parameters to be passed to the platform
 * decoder - the object decoder (see flowthings_io_cb_decode_object)
 * result - must be preallocated; will be filled with the output of decode(return from platform)
 */
flowthings_io_result_code flowthings_io_service_read(
		flowthings_io_service_type svc, const char *path_ext,
		flowthings_io_api *api, const char *id, flowthings_io_params *params,
		flowthings_io_cb_decode_object decoder, void *result)
{
	if (!api || !api->fhttp)
		return FLOWTHINGS_IO_ERROR_NOT_INITIALIZED;

	if (!decoder || !result)
		return FLOWTHINGS_IO_ERROR_COULDNT_DECODE;

	if (!id) {
		FAIL
		;
	}

	char path[FLOWTHINGS_IO_MAX_PATH_SIZE] = "";
	flowthings_io_string *response = flowthings_io_string_init();

	struct __flowthings_io_service_info_item *service_item =
			&__flowthings_io_service_info[svc];

	api->fhttp->base_path = service_item->base_path;

	__flowthings_io_add_path_ext(path, path_ext);
	flowthings_io_strcat(path, "/", FLOWTHINGS_IO_MAX_PATH_SIZE);
	flowthings_io_strcat(path, id, FLOWTHINGS_IO_MAX_PATH_SIZE);

	if (params)
		flowthings_io_params_to_url(params, path, FLOWTHINGS_IO_MAX_PATH_SIZE);

	int http_response_code = flowthings_io_http_request(api->fhttp,
			FLOWTHINGS_IO_HTTP_METHOD_GET, path, NULL, response);

	if (http_response_code < 200 || http_response_code >= 400) {
		switch (http_response_code) {
		case 400:
			return FLOWTHINGS_IO_ERROR_BAD_REQUEST;
		case 403:
			return FLOWTHINGS_IO_ERROR_FORBIDDEN;
		case 404:
			return FLOWTHINGS_IO_ERROR_NOT_FOUND;
		case 500:
			return FLOWTHINGS_IO_ERROR_SERVER_ERROR;
		default:
			return FLOWTHINGS_IO_ERROR_UNKNOWN;
		}
	}

	cJSON *root = cJSON_Parse(response->ptr);
	flowthings_io_string_cleanup(response);

	cJSON *body = cJSON_GetObjectItem(root, "body");
	if (!body)
		return FLOWTHINGS_IO_ERROR_MALFORMED_RESPONSE;

	flowthings_io_result_code code =
			decoder(body, result) ?
					FLOWTHINGS_IO_OK : FLOWTHINGS_IO_ERROR_COULDNT_DECODE;

	cJSON_Delete(root);

	return code;
}

/*
 * NAME: flowthings_io_service_create
 *
 * Perform a create on valid objects from the platform.  This function should not be called directly --
 * one of the defines below should be called depending on the object type.  For example, to create a
 * drop, call flowthings_io_drop_create(...)
 *
 * PARAMS:
 * svc - the service type
 * path_ext - any path extension to add in creating the URL
 * api - the API object
 * params - any additional query string parameters to be passed to the platform
 * encoder - the object encoder, which will be called on void *object (see flowthings_io_cb_encode_object)
 * decoder - the object decoder (see flowthings_io_cb_decode_object)
 * object - the object with the fields to create; encoder will be called on this object and the result
 *     will be passed to the platform; when this function returns, object will be filled with the resulting
 *     object from the platform
 */
flowthings_io_result_code flowthings_io_service_create(
		flowthings_io_service_type svc, const char *path_ext,
		flowthings_io_api *api, flowthings_io_params *params,
		flowthings_io_cb_encode_object encoder,
		flowthings_io_cb_decode_object decoder, void *object)
{
	if (!api || !api->fhttp)
		return FLOWTHINGS_IO_ERROR_NOT_INITIALIZED;

	char path[FLOWTHINGS_IO_MAX_PATH_SIZE] = "";
	flowthings_io_result_code code;
	flowthings_io_string *response = flowthings_io_string_init();
	cJSON *in_root, *out_root;
	struct __flowthings_io_service_info_item *service_item =
			&__flowthings_io_service_info[svc];

	api->fhttp->base_path = service_item->base_path;

	__flowthings_io_add_path_ext(path, path_ext);

	if (!encoder || !object)
		return FLOWTHINGS_IO_ERROR_COULDNT_ENCODE;

	in_root = cJSON_CreateObject();
	encoder(object, in_root);

	int http_response_code = flowthings_io_http_request(api->fhttp,
			FLOWTHINGS_IO_HTTP_METHOD_POST, path, cJSON_Print(in_root), response);

	cJSON_Delete(in_root);

	if (http_response_code < 200 || http_response_code >= 400) {
		switch (http_response_code) {
		case 400:
			return FLOWTHINGS_IO_ERROR_BAD_REQUEST;
		case 403:
			return FLOWTHINGS_IO_ERROR_FORBIDDEN;
		case 404:
			return FLOWTHINGS_IO_ERROR_NOT_FOUND;
		case 500:
			return FLOWTHINGS_IO_ERROR_SERVER_ERROR;
		default:
			return FLOWTHINGS_IO_ERROR_UNKNOWN;
		}
	}

	out_root = cJSON_Parse(response->ptr);
	flowthings_io_string_cleanup(response);

	cJSON *body = cJSON_GetObjectItem(out_root, "body");
	if (!body)
		return FLOWTHINGS_IO_ERROR_MALFORMED_RESPONSE;

	if (decoder)
		code = decoder(body, object) ?
				FLOWTHINGS_IO_OK : FLOWTHINGS_IO_ERROR_COULDNT_DECODE;
	else
		code = FLOWTHINGS_IO_OK;

	cJSON_Delete(out_root);

	return code;
}

/*
 * NAME: flowthings_io_service_update
 *
 * Perform an update on valid objects from the platform.  This function should not be called directly --
 * one of the defines below should be called depending on the object type.  For example, to update a
 * drop, call flowthings_io_drop_update(...)
 *
 * PARAMS:
 * svc - the service type
 * path_ext - any path extension to add in creating the URL
 * api - the API object
 * id - the ID of the object to delete
 * params - any additional query string parameters to be passed to the platform
 * encoder - the object encoder, which will be called on void *object (see flowthings_io_cb_encode_object)
 * decoder - the object decoder (see flowthings_io_cb_decode_object)
 * object - the object with the fields to update; encoder will be called on this object and the result
 *     will be passed to the platform; when this function returns, this will be filled with the updated
 *     object from the platform
 */
flowthings_io_result_code flowthings_io_service_update(
		flowthings_io_service_type svc, const char *path_ext,
		flowthings_io_api *api, const char *id, flowthings_io_params *params,
		flowthings_io_cb_encode_object encoder,
		flowthings_io_cb_decode_object decoder, void *object)
{
	if (!api || !api->fhttp)
		return FLOWTHINGS_IO_ERROR_NOT_INITIALIZED;

	char path[FLOWTHINGS_IO_MAX_PATH_SIZE] = "/";
	flowthings_io_result_code code;
	flowthings_io_string *response = flowthings_io_string_init();
	cJSON *in_root, *out_root;

	struct __flowthings_io_service_info_item *service_item =
			&__flowthings_io_service_info[svc];

	api->fhttp->base_path = service_item->base_path;

	__flowthings_io_add_path_ext(path, path_ext);
	flowthings_io_strcat(path, "/", FLOWTHINGS_IO_MAX_PATH_SIZE);
	flowthings_io_strcat(path, id, FLOWTHINGS_IO_MAX_PATH_SIZE);

	if (!encoder || !object)
		return FLOWTHINGS_IO_ERROR_COULDNT_ENCODE;

	in_root = cJSON_CreateObject();
	encoder(object, in_root);

	int http_response_code = flowthings_io_http_request(api->fhttp,
			FLOWTHINGS_IO_HTTP_METHOD_PUT, path, cJSON_Print(in_root),
			response);

	cJSON_Delete(in_root);

	if (http_response_code < 200 || http_response_code >= 400) {
		switch (http_response_code) {
		case 400:
			return FLOWTHINGS_IO_ERROR_BAD_REQUEST;
		case 403:
			return FLOWTHINGS_IO_ERROR_FORBIDDEN;
		case 404:
			return FLOWTHINGS_IO_ERROR_NOT_FOUND;
		case 500:
			return FLOWTHINGS_IO_ERROR_SERVER_ERROR;
		default:
			return FLOWTHINGS_IO_ERROR_UNKNOWN;
		}
	}

	out_root = cJSON_Parse(response->ptr);
	flowthings_io_string_cleanup(response);

	cJSON *body = cJSON_GetObjectItem(out_root, "body");
	if (!body)
		return FLOWTHINGS_IO_ERROR_MALFORMED_RESPONSE;

	if (decoder)
		code = decoder(body, object) ?
				FLOWTHINGS_IO_OK : FLOWTHINGS_IO_ERROR_COULDNT_DECODE;
	else
		code = FLOWTHINGS_IO_OK;

	cJSON_Delete(out_root);

	return code;
}

/*
 * NAME: flowthings_io_service_delete
 *
 * Perform a delete on valid objects from the platform.  This function should not be called directly --
 * one of the defines below should be called depending on the object type.  For example, to delete a
 * drop, call flowthings_io_drop_delete(...)
 *
 * PARAMS:
 * svc - the service type
 * path_ext - any path extension to add in creating the URL
 * api - the API object
 * id - the ID of the object to delete
 * params - any additional query string parameters to be passed to the platform
 */
flowthings_io_result_code flowthings_io_service_delete(
		flowthings_io_service_type svc, const char *path_ext,
		flowthings_io_api *api, const char *id, flowthings_io_params *params)
{
	if (!api || !api->fhttp)
		return FLOWTHINGS_IO_ERROR_NOT_INITIALIZED;

	char path[FLOWTHINGS_IO_MAX_PATH_SIZE] = "/";
	flowthings_io_string *response = flowthings_io_string_init();

	struct __flowthings_io_service_info_item *service_item =
			&__flowthings_io_service_info[svc];

	api->fhttp->base_path = service_item->base_path;
	__flowthings_io_add_path_ext(path, path_ext);
	flowthings_io_strcat(path, id, FLOWTHINGS_IO_MAX_PATH_SIZE);

	int http_response_code = flowthings_io_http_request(api->fhttp,
			FLOWTHINGS_IO_HTTP_METHOD_DELETE, path, NULL, response);

	if (http_response_code < 200 || http_response_code >= 400) {
		switch (http_response_code) {
		case 400:
			return FLOWTHINGS_IO_ERROR_BAD_REQUEST;
		case 403:
			return FLOWTHINGS_IO_ERROR_FORBIDDEN;
		case 404:
			return FLOWTHINGS_IO_ERROR_NOT_FOUND;
		case 500:
			return FLOWTHINGS_IO_ERROR_SERVER_ERROR;
		default:
			return FLOWTHINGS_IO_ERROR_UNKNOWN;
		}
	}

	flowthings_io_string_cleanup(response);

	return FLOWTHINGS_IO_OK;
}


/***********************************************************************
 * The drop-only service functions
 ***********************************************************************/

/*
 * NAME: flowthings_io_service_find
 *
 * Perform a find on drops from the platform.  This function should not be called directly --
 * one of the defines below should be called depending on the object type.
 *
 * PARAMS:
 * svc - the service type
 * path_ext - any path extension to add in creating the URL
 * api - the API object
 * filter - a filter string for drops (see https://flowthings.io/docs/flow-filter-language)
 * params - any additional query string parameters to be passed to the platform
 * decoder - the object decoder (see flowthings_io_cb_decode_object)
 * result - the array of result pointers; this array and all result objects must be pre-allocated
 *     and initialized; decode will be performed on all platform results and they will be placed
 *     in the result array
 * result_count - must initially be set to the allocated size of the result array; when this
 *     function completes, it will be set to the number of items in the array
 */
flowthings_io_result_code flowthings_io_service_find(
		flowthings_io_service_type svc, const char *path_ext,
		flowthings_io_api *api, const char *filter,
		flowthings_io_params *params, flowthings_io_cb_decode_object decoder,
		void *result[],
		int *result_count)
{
	flowthings_io_params *my_params = params;
	flowthings_io_result_code code;
	int i;

	if (!my_params) {
		my_params = flowthings_io_params_init();
	}

	flowthings_io_params_add(my_params, "filter", filter);

	if (!api || !api->fhttp)
		return FLOWTHINGS_IO_ERROR_NOT_INITIALIZED;

	if (!decoder || !result)
		return FLOWTHINGS_IO_ERROR_COULDNT_DECODE;

	char path[FLOWTHINGS_IO_MAX_PATH_SIZE] = "";
	flowthings_io_string *response = flowthings_io_string_init();

	struct __flowthings_io_service_info_item *service_item =
			&__flowthings_io_service_info[svc];

	api->fhttp->base_path = service_item->base_path;

	__flowthings_io_add_path_ext(path, path_ext);

	if (my_params)
		flowthings_io_params_to_url(my_params, path, FLOWTHINGS_IO_MAX_PATH_SIZE);

	int http_response_code = flowthings_io_http_request(api->fhttp,
			FLOWTHINGS_IO_HTTP_METHOD_GET, path, NULL, response);

	if (http_response_code < 200 || http_response_code >= 400) {
		switch (http_response_code) {
		case 400:
			return FLOWTHINGS_IO_ERROR_BAD_REQUEST;
		case 403:
			return FLOWTHINGS_IO_ERROR_FORBIDDEN;
		case 404:
			return FLOWTHINGS_IO_ERROR_NOT_FOUND;
		case 500:
			return FLOWTHINGS_IO_ERROR_SERVER_ERROR;
		default:
			return FLOWTHINGS_IO_ERROR_UNKNOWN;
		}
	}

	cJSON *root = cJSON_Parse(response->ptr);
	flowthings_io_string_cleanup(response);

	cJSON *body = cJSON_GetObjectItem(root, "body");
	if (!body)
		return FLOWTHINGS_IO_ERROR_MALFORMED_RESPONSE;

	for (i = 0; i < *result_count && body->type == cJSON_Array && i < cJSON_GetArraySize(body); i++) {

		code = decoder(cJSON_GetArrayItem(body, i), &result[i]) ? FLOWTHINGS_IO_OK : FLOWTHINGS_IO_ERROR_COULDNT_DECODE;
		if (code != FLOWTHINGS_IO_OK) break;

	}

	*result_count = i;

	cJSON_Delete(root);

	if (!params) {
		flowthings_io_params_cleanup(my_params);
	}

	return code;
}

/*
 * NAME: flowthings_io_service_find_many
 *
 * Perform a find_many on drops from the platform.  This function should not be called directly --
 * one of the defines below should be called depending on the object type.
 *
 * PARAMS:
 * svc - the service type
 * path_ext - any path extension to add in creating the URL
 * api - the API object
 * decoder - the object decoder (see flowthings_io_cb_decode_object)
 * idlist - the list of [ flowId => { param1=value, param2=value, ... } ]; idlist must
 *     have flowthings_io_param as the void* item value
 * result - the array of result pointers; this array and all result objects must be pre-allocated
 *     and initialized; decode will be performed on all platform results and they will be placed
 *     in the result array
 * result_count - must initially be set to the allocated size of the result array; when this
 *     function completes, it will be set to the number of items in the array
 */
flowthings_io_result_code flowthings_io_service_find_many(
		flowthings_io_service_type svc, const char *path_ext,
		flowthings_io_api *api, flowthings_io_cb_decode_object decoder,
		flowthings_io_idlist *idlist,
		void *result[],
		int *result_count)
{
	cJSON *in_root, *jitem, *jsubitem;
	flowthings_io_result_code code = FLOWTHINGS_IO_ERROR_UNKNOWN;
	int i;

	if (!api || !api->fhttp)
		return FLOWTHINGS_IO_ERROR_NOT_INITIALIZED;

	if (!decoder)
		return FLOWTHINGS_IO_ERROR_COULDNT_DECODE;

	char path[FLOWTHINGS_IO_MAX_PATH_SIZE] = "";
	flowthings_io_string *response = flowthings_io_string_init();

	struct __flowthings_io_service_info_item *service_item =
			&__flowthings_io_service_info[svc];

	api->fhttp->base_path = service_item->base_path;

	__flowthings_io_add_path_ext(path, path_ext);
	flowthings_io_strcat(path, "?flatten=flat", FLOWTHINGS_IO_MAX_PATH_SIZE);

	if (!idlist)
		return FLOWTHINGS_IO_ERROR_BAD_REQUEST;

	in_root = cJSON_CreateArray();

	flowthings_io_idlistitem *item = idlist->start;
	flowthings_io_params *params;
	flowthings_io_param *param;

	while (item != NULL) {

		if (!item->id) {
			FAIL;
		}

		jitem = cJSON_CreateObject();
		cJSON_AddStringToObject(jitem, "flowId", item->id);

		jsubitem = cJSON_CreateObject();
		params = (flowthings_io_params *)item->item;
		param = params->start;

		while (param != NULL) {

			cJSON_AddStringToObject(jsubitem, param->key->ptr, param->value->ptr);
			param = param->next;

		}

		cJSON_AddItemToObject(jitem, "params", jsubitem);
		cJSON_AddItemToArray(in_root, jitem);

		item = item->next;
	}

	int http_response_code = flowthings_io_http_request(api->fhttp,
			FLOWTHINGS_IO_HTTP_METHOD_MGET, path, cJSON_Print(in_root), response);

	if (http_response_code < 200 || http_response_code >= 400) {
		switch (http_response_code) {
		case 400:
			return FLOWTHINGS_IO_ERROR_BAD_REQUEST;
		case 403:
			return FLOWTHINGS_IO_ERROR_FORBIDDEN;
		case 404:
			return FLOWTHINGS_IO_ERROR_NOT_FOUND;
		case 500:
			return FLOWTHINGS_IO_ERROR_SERVER_ERROR;
		default:
			return FLOWTHINGS_IO_ERROR_UNKNOWN;
		}
	}

	cJSON *root = cJSON_Parse(response->ptr);
	flowthings_io_string_cleanup(response);

	cJSON *body = cJSON_GetObjectItem(root, "body");
	if (!body)
		return FLOWTHINGS_IO_ERROR_MALFORMED_RESPONSE;

	for (i = 0; i < *result_count && body->type == cJSON_Array && i < cJSON_GetArraySize(body); i++) {

		code = decoder(cJSON_GetArrayItem(body, i), &result[i]) ? FLOWTHINGS_IO_OK : FLOWTHINGS_IO_ERROR_COULDNT_DECODE;
		if (code != FLOWTHINGS_IO_OK) break;

	}

	*result_count = i;

	cJSON_Delete(root);

	return code;
}

#ifdef  __cplusplus
}
#endif

#endif /* FLOWTHINGS_IO_SERVICES_C_ */
