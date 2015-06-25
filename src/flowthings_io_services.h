/*
 * flowthings_io_services.h
 *
 * This file must be included to use the flowthings.io C library
 *
 *  Created on: May 21, 2015
 */

#ifndef FLOWTHINGS_IO_SERVICES_H_
#define FLOWTHINGS_IO_SERVICES_H_

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
#include "flowthings_io_api.h"


/***********************************************************************
 * Flowthings service types and service type definitions
 ***********************************************************************/

#define FLOWTHINGS_IO_SERVICE_TYPE_FLOW 0
#define FLOWTHINGS_IO_SERVICE_TYPE_DROP 1
#define FLOWTHINGS_IO_SERVICE_TYPE_IDENTITY 2
#define FLOWTHINGS_IO_SERVICE_TYPE_GROUP 3
#define FLOWTHINGS_IO_SERVICE_TYPE_TRACK 4
#define FLOWTHINGS_IO_SERVICE_TYPE_API_TASK 5
#define FLOWTHINGS_IO_SERVICE_TYPE_MQTT_TASK 6
#define FLOWTHINGS_IO_SERVICE_TYPE_TOKEN 7
#define FLOWTHINGS_IO_SERVICE_TYPE_SHARE 8

#define FLOWTHINGS_IO_ACTION_READ 0x01
#define FLOWTHINGS_IO_ACTION_CREATE 0x02
#define FLOWTHINGS_IO_ACTION_UPDATE 0x04
#define FLOWTHINGS_IO_ACTION_DELETE 0x08

/*
 * NAME: flowthings_io_service_type
 *
 * A service type - must be one of the FLOWTHINGS_IO_SERVICE_TYPE_* values
 */
typedef int flowthings_io_service_type;

struct __flowthings_io_service_info_item {
	int id;
	char *name;
	char *base_path;
	int actions;
};

/*
 * NAME: __flowthings_io_service_info
 *
 * A structure with service definitions; should not be used outside this library
 */
static struct __flowthings_io_service_info_item __flowthings_io_service_info[] = {
	{ FLOWTHINGS_IO_SERVICE_TYPE_FLOW, "flow", "/flow", FLOWTHINGS_IO_ACTION_READ | FLOWTHINGS_IO_ACTION_CREATE | FLOWTHINGS_IO_ACTION_UPDATE | FLOWTHINGS_IO_ACTION_DELETE },
	{ FLOWTHINGS_IO_SERVICE_TYPE_DROP, "drop", "/drop", FLOWTHINGS_IO_ACTION_READ | FLOWTHINGS_IO_ACTION_CREATE | FLOWTHINGS_IO_ACTION_UPDATE | FLOWTHINGS_IO_ACTION_DELETE },
	{ FLOWTHINGS_IO_SERVICE_TYPE_IDENTITY, "identity", "/identity", FLOWTHINGS_IO_ACTION_READ | FLOWTHINGS_IO_ACTION_UPDATE },
	{ FLOWTHINGS_IO_SERVICE_TYPE_GROUP, "group", "/group", FLOWTHINGS_IO_ACTION_READ | FLOWTHINGS_IO_ACTION_CREATE | FLOWTHINGS_IO_ACTION_UPDATE | FLOWTHINGS_IO_ACTION_DELETE },
	{ FLOWTHINGS_IO_SERVICE_TYPE_TRACK, "track", "/track", FLOWTHINGS_IO_ACTION_READ | FLOWTHINGS_IO_ACTION_CREATE | FLOWTHINGS_IO_ACTION_UPDATE | FLOWTHINGS_IO_ACTION_DELETE },
	{ FLOWTHINGS_IO_SERVICE_TYPE_API_TASK, "api-task", "/api-task", FLOWTHINGS_IO_ACTION_READ | FLOWTHINGS_IO_ACTION_CREATE | FLOWTHINGS_IO_ACTION_UPDATE | FLOWTHINGS_IO_ACTION_DELETE },
	{ FLOWTHINGS_IO_SERVICE_TYPE_MQTT_TASK, "mqtt", "/mqtt", FLOWTHINGS_IO_ACTION_READ | FLOWTHINGS_IO_ACTION_CREATE | FLOWTHINGS_IO_ACTION_UPDATE | FLOWTHINGS_IO_ACTION_DELETE },
	{ FLOWTHINGS_IO_SERVICE_TYPE_TOKEN, "token", "/token", FLOWTHINGS_IO_ACTION_READ | FLOWTHINGS_IO_ACTION_CREATE | FLOWTHINGS_IO_ACTION_DELETE },
	{ FLOWTHINGS_IO_SERVICE_TYPE_SHARE, "share", "/share", FLOWTHINGS_IO_ACTION_READ | FLOWTHINGS_IO_ACTION_CREATE | FLOWTHINGS_IO_ACTION_DELETE }
};


/***********************************************************************
 * Encode/decode callbacks
 ***********************************************************************/

/*
 * NAME: flowthings_io_cb_encode_object
 *
 * This callback function takes a user-defined object (obj_in) and is responsible for converting its
 * fields into a cJSON compatible format, and adding the resulting fields to the json_out object.
 *
 * obj_in - this is the object of whatever type was passed to the service* functions
 * json_out - a cJSON * object which must be filled in by this callback function
 */
typedef BOOL (*flowthings_io_cb_encode_object)(void *obj_in, cJSON *json_out);

/*
 * NAME: flowthings_io_cb_decode_object
 *
 * This callback function takes a cJSON object and is responsible for converting it back into the
 * user defined object expected as a result of the service* functions.  It will be called on every
 * object returned from the platform, and the result will be returned from the service* functions.
 *
 * json_in - the cJSON * object that contains the fields to decode
 * obj_out - a preallocated object of the type passed to the service* functions which this callback
 *     must fill in with the fields returned in the JSON
 */
typedef BOOL (*flowthings_io_cb_decode_object)(cJSON *json_in, void *obj_out);


/***********************************************************************
 * The generic service functions
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
		flowthings_io_cb_decode_object decoder, void *result);

#define flowthings_io_drop_read(...) flowthings_io_service_read(FLOWTHINGS_IO_SERVICE_TYPE_DROP, __VA_ARGS__)
#define flowthings_io_flow_read(...) flowthings_io_service_read(FLOWTHINGS_IO_SERVICE_TYPE_FLOW, NULL, __VA_ARGS__)
#define flowthings_io_identity_read(...) flowthings_io_service_read(FLOWTHINGS_IO_SERVICE_TYPE_IDENTITY, NULL, __VA_ARGS__)
#define flowthings_io_group_read(...) flowthings_io_service_read(FLOWTHINGS_IO_SERVICE_TYPE_GROUP, NULL, __VA_ARGS__)
#define flowthings_io_track_read(...) flowthings_io_service_read(FLOWTHINGS_IO_SERVICE_TYPE_TRACK, NULL, __VA_ARGS__)
#define flowthings_io_api_task_read(...) flowthings_io_service_read(FLOWTHINGS_IO_SERVICE_TYPE_API_TASK, NULL, __VA_ARGS__)
#define flowthings_io_mqtt_task_read(...) flowthings_io_service_read(FLOWTHINGS_IO_SERVICE_TYPE_MQTT_TASK, NULL, __VA_ARGS__)
#define flowthings_io_token_read(...) flowthings_io_service_read(FLOWTHINGS_IO_SERVICE_TYPE_TOKEN, NULL, __VA_ARGS__)
#define flowthings_io_share_read(...) flowthings_io_service_read(FLOWTHINGS_IO_SERVICE_TYPE_SHARE, NULL, __VA_ARGS__)

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
		flowthings_io_cb_decode_object decoder, void *object);

#define flowthings_io_drop_create(...) flowthings_io_service_create(FLOWTHINGS_IO_SERVICE_TYPE_DROP, __VA_ARGS__)
#define flowthings_io_flow_create(...) flowthings_io_service_create(FLOWTHINGS_IO_SERVICE_TYPE_FLOW, NULL, __VA_ARGS__)
#define flowthings_io_group_create(...) flowthings_io_service_create(FLOWTHINGS_IO_SERVICE_TYPE_GROUP, NULL, __VA_ARGS__)
#define flowthings_io_track_create(...) flowthings_io_service_create(FLOWTHINGS_IO_SERVICE_TYPE_TRACK, NULL, __VA_ARGS__)
#define flowthings_io_api_task_create(...) flowthings_io_service_create(FLOWTHINGS_IO_SERVICE_TYPE_API_TASK, NULL, __VA_ARGS__)
#define flowthings_io_mqtt_task_create(...) flowthings_io_service_create(FLOWTHINGS_IO_SERVICE_TYPE_MQTT_TASK, NULL, __VA_ARGS__)
#define flowthings_io_token_create(...) flowthings_io_service_create(FLOWTHINGS_IO_SERVICE_TYPE_TOKEN, NULL, __VA_ARGS__)
#define flowthings_io_share_create(...) flowthings_io_service_create(FLOWTHINGS_IO_SERVICE_TYPE_SHARE, NULL, __VA_ARGS__)

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
		flowthings_io_cb_decode_object decoder, void *object);

#define flowthings_io_drop_update(...) flowthings_io_service_update(FLOWTHINGS_IO_SERVICE_TYPE_DROP, __VA_ARGS__)
#define flowthings_io_flow_update(...) flowthings_io_service_update(FLOWTHINGS_IO_SERVICE_TYPE_FLOW, NULL, __VA_ARGS__)
#define flowthings_io_identity_update(...) flowthings_io_service_update(FLOWTHINGS_IO_SERVICE_TYPE_IDENTITY, NULL, __VA_ARGS__)
#define flowthings_io_group_update(...) flowthings_io_service_update(FLOWTHINGS_IO_SERVICE_TYPE_GROUP, NULL, __VA_ARGS__)
#define flowthings_io_track_update(...) flowthings_io_service_update(FLOWTHINGS_IO_SERVICE_TYPE_TRACK, NULL, __VA_ARGS__)
#define flowthings_io_api_task_update(...) flowthings_io_service_update(FLOWTHINGS_IO_SERVICE_TYPE_API_TASK, NULL, __VA_ARGS__)
#define flowthings_io_mqtt_task_update(...) flowthings_io_service_update(FLOWTHINGS_IO_SERVICE_TYPE_MQTT_TASK, NULL, __VA_ARGS__)

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
		flowthings_io_api *api, const char *id, flowthings_io_params *params);

#define flowthings_io_drop_delete(...) flowthings_io_service_delete(FLOWTHINGS_IO_SERVICE_TYPE_DROP, __VA_ARGS__)
#define flowthings_io_flow_delete(...) flowthings_io_service_delete(FLOWTHINGS_IO_SERVICE_TYPE_FLOW, NULL, __VA_ARGS__)
#define flowthings_io_group_delete(...) flowthings_io_service_delete(FLOWTHINGS_IO_SERVICE_TYPE_GROUP, NULL, __VA_ARGS__)
#define flowthings_io_track_delete(...) flowthings_io_service_delete(FLOWTHINGS_IO_SERVICE_TYPE_TRACK, NULL, __VA_ARGS__)
#define flowthings_io_api_task_delete(...) flowthings_io_service_delete(FLOWTHINGS_IO_SERVICE_TYPE_API_TASK, NULL, __VA_ARGS__)
#define flowthings_io_mqtt_task_delete(...) flowthings_io_service_delete(FLOWTHINGS_IO_SERVICE_TYPE_MQTT_TASK, NULL, __VA_ARGS__)
#define flowthings_io_token_delete(...) flowthings_io_service_delete(FLOWTHINGS_IO_SERVICE_TYPE_TOKEN, NULL, __VA_ARGS__)
#define flowthings_io_share_delete(...) flowthings_io_service_delete(FLOWTHINGS_IO_SERVICE_TYPE_SHARE, NULL, __VA_ARGS__)


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
		int *result_count);

#define flowthings_io_drop_find(...) flowthings_io_service_find(FLOWTHINGS_IO_SERVICE_TYPE_DROP, __VA_ARGS__)

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
		int *result_count);

#define flowthings_io_drop_find_many(...) flowthings_io_service_find_many(FLOWTHINGS_IO_SERVICE_TYPE_DROP, NULL, __VA_ARGS__)

#ifdef  __cplusplus
}
#endif

#endif /* FLOWTHINGS_IO_SERVICES_H_ */
