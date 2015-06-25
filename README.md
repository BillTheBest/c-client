# flowthings.io C Library

This is a C library for flowthings.io.  This version was compiled with Edison, but it should compile on most POSIX-compliant operating systems.

### Installing

To install this library, you will need to get the source from Github and compile it into your project.  See Compiling and Building below.

### Documentation

The function headers contain API documentation.  Additional how-to-use documentation is in this README file.

### Basics

This is a basic C library for flowthings.io.  It provides most of the functionality of the python library, and is intended to function as similarly as possible (considering the major differences between the two languages).  You should be able to refer to the python docs for more information on specific functions. 

### API Initialization

To use this library, you must include the following:
```c
#include "flowthings_io_services.h"
```
The cJSON library contains JSON functions, which you will need to use for converting your own objects into JSON for the platform.  The `flowthings_io.h` header defines all of the basic types and functions for flowthings.io.  The `flowthings_io_services.h` header defines the service functions that you will use to communicate with the flowthings.io platform.

All service functions you will need to call require a `flowthings_io_api` pointer.  This can be created by calling the flowthings_io_api_init function and passing the version, host, secure boolean, and credentials.  The creds parameter is a pointer to a struct of type `flowthings_io_token` which can be initialized like this:
```c
flowthings_io_token creds;
creds.account = "myaccountname";
creds.token = "mytoken";
```

Then the API can be initialized:
```c
flowthings_io_api *api = flowthings_io_api_init(FLOWTHINGS_IO_VERSION, FLOWTHINGS_IO_HOST, TRUE, &creds);
```

When you're done with the API, make sure to call:
```c
flowthings_io_api_cleanup(api);
```

### Service Functions

After you have initialized the API, you can start making service calls.  All calls are of the type flowthings_io_<object type>_<action>.  For instance, to create a drop, you would call `flowthings_io_drop_create(...)`.  

#### Encoding/Decoding

Each call to one of these functions requires either an encoder or decoder callback function.  
```c
typedef BOOL (*flowthings_io_cb_encode_object)(void *obj_in, cJSON *json_out);
typedef BOOL (*flowthings_io_cb_decode_object)(cJSON *json_in, void *obj_out);
```

You will need to create functions to translate your objects into JSON objects (using cJSON) which the platform can then act on.  These functions can be very simple:
```c
BOOL decode_my_drop(cJSON *json_in, void *obj_out)
{
	struct my_drop *md = (struct my_drop *)obj_out;
	md->num = cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json_in, "elems"), "num"), "value")->valueint;

	return TRUE;
}

BOOL encode_my_drop(void *obj_in, cJSON *json_out) {
	struct my_drop *md = (struct my_drop *)obj_in;

	cJSON *cjo = cJSON_CreateObject();
	cJSON_AddNumberToObject(cjo, "num", md->num);
	cJSON_AddItemToObject(json_out, "elems", cjo);

	return TRUE;
}
```

As you can see, the encode function takes a user-defined object (which you will pass to the service functions) and a cJSON object.  Your callback needs to add the fields in the user-defined object into the cJSON object.  Documentation for cJSON is here: https://github.com/kbranigan/cJSON

The decode object takes a cJSON object and fills out a pre-allocated user-defined object with its values.  In the example above, the object `obj_out` is cast as a `my_drop` and then the `num` field is set from the platform drop element.

These functions should return TRUE if they were successful, and FALSE if there was some kind of failure.

#### Service Function Return Values

Each service function has a return value of type `flowthings_io_result_code`:
```c
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
```

If the function completes successfully, it returns `FLOWTHINGS_IO_OK`, otherwise it returns one of the error codes.

#### Additional Parameters

Most of the following functions accept a `param` argument, which will be passed along to the platform in the query string.  To create and fill this object do the following:
```c
flowthings_io_params *params = flowthings_io_params_init();
flowthings_io_params_add(params, "sort", "asc");
```

Make sure to call `flowthings_io_params_cleanup(params);` when done.

#### Calling Service Functions

The following is information on each service function:

#### Read

To read an item from the platform, you can call one of these functions:
```c
flowthings_io_flow_read(...) 
flowthings_io_identity_read(...) 
flowthings_io_group_read(...) 
flowthings_io_track_read(...) 
flowthings_io_api_task_read(...) 
flowthings_io_mqtt_task_read(...) 
flowthings_io_token_read(...) 
flowthings_io_share_read(...) 
```

Each requires the following parameters:

 * api - the API object
 * id - the ID of the object to delete
 * params - any additional query string parameters to be passed to the platform
 * decoder - the object decoder (see flowthings_io_cb_decode_object)
 * result - must be preallocated; will be filled with the output of decode(return from platform) 

For drops, you can use `flowthings_io_drop_read(...)`.  The only difference is that it takes an additional parameter at the beginning:

 * flow_id - the ID of the parent flow
 * api - the API object
 * id - the ID of the object to delete
 * params - any additional query string parameters to be passed to the platform
 * decoder - the object decoder (see flowthings_io_cb_decode_object)
 * result - must be preallocated; will be filled with the output of decode(return from platform)

#### Create

To create an item on the platform, you can call one of these functions:
```c
flowthings_io_flow_create(...)
flowthings_io_group_create(...) 
flowthings_io_track_create(...) 
flowthings_io_api_task_create(...) 
flowthings_io_mqtt_task_create(...)
flowthings_io_token_create(...) 
flowthings_io_share_create(...) 
```

Each requires the following parameters:

 * api - the API object
 * params - any additional query string parameters to be passed to the platform
 * encoder - the object encoder, which will be called on void *object (see flowthings_io_cb_encode_object)
 * decoder - the object decoder (see flowthings_io_cb_decode_object)
 * object - the object with the fields to create; encoder will be called on this object and the result will be passed to the platform

After this function completes, the `object` parameter will contain the resulting object from the platform.

For drops, you can use `flowthings_io_drop_create(...)`.  The only difference is that it takes an additional parameter at the beginning:

 * flow_id - the ID of the parent flow
 * api - the API object
 * params - any additional query string parameters to be passed to the platform
 * encoder - the object encoder, which will be called on void *object (see flowthings_io_cb_encode_object)
 * decoder - the object decoder (see flowthings_io_cb_decode_object)
 * object - the object with the fields to create; encoder will be called on this object and the result will be passed to the platform

#### Update

To update an item on the platform, you can call one of these functions:
```c
flowthings_io_flow_update(...) 
flowthings_io_identity_update(...)
flowthings_io_group_update(...) 
flowthings_io_track_update(...) 
flowthings_io_api_task_update(...) 
flowthings_io_mqtt_task_update(...) 
```

Each requires the following parameters:

 * api - the API object
 * id - the ID of the object to delete
 * params - any additional query string parameters to be passed to the platform
 * encoder - the object encoder, which will be called on void *object (see flowthings_io_cb_encode_object)
 * decoder - the object decoder (see flowthings_io_cb_decode_object)
 * object - the object with the fields to update; encoder will be called on this object and the result will be passed to the platform

After this function completes, the `object` parameter will contain the full updated object from the platform.

For drops, you can use `flowthings_io_drop_update(...)`.  The only difference is that it takes an additional parameter at the beginning:

 * flow_id - the ID of the parent flow
 * api - the API object
 * id - the ID of the object to delete
 * params - any additional query string parameters to be passed to the platform
 * encoder - the object encoder, which will be called on void *object (see flowthings_io_cb_encode_object)
 * decoder - the object decoder (see flowthings_io_cb_decode_object)
 * object - the object with the fields to update; encoder will be called on this object and the result will be passed to the platform

#### Delete

To delete an item on the platform, you can call one of these functions:
```c
flowthings_io_flow_delete(...)
flowthings_io_group_delete(...)
flowthings_io_track_delete(...)
flowthings_io_api_task_delete(...)
flowthings_io_mqtt_task_delete(...)
flowthings_io_token_delete(...)
flowthings_io_share_delete(...) 
```

Each requires the following parameters:

 * api - the API object
 * id - the ID of the object to delete
 * params - any additional query string parameters to be passed to the platform

For drops, you can use `flowthings_io_drop_delete(...)`.  The only difference is that it takes an additional parameter at the beginning:

 * flow_id - the ID of the parent flow
 * api - the API object
 * id - the ID of the object to delete
 * params - any additional query string parameters to be passed to the platform

### Find (Drops Only)

Find `flowthings_io_drop_find(...)` requires the following parameters:

 * flow_id - the flow ID
 * api - the API object
 * filter - a filter string for drops (see https://flowthings.io/docs/flow-filter-language)
 * params - any additional query string parameters to be passed to the platform
 * decoder - the object decoder (see flowthings_io_cb_decode_object)
 * result - the array of result pointers; this array and all result objects must be pre-allocated and initialized; decode will be performed on all platform results and they will be placed in the result array
 * result_count - must initially be set to the allocated size of the result array; when this function completes, it will be set to the number of items in the array

Before calling this function, the result array must be allocated.  The size of the array should be passed as the result_count.  For example:
```c
struct my_drop multiple_drops[10];
int result_count = 10;
int *rc = &result_count;
flowthings_io_drop_find("f552a87090cf2afb329f31f37", api, "elems.num>3", NULL, decode_my_drop, &multiple_drops, rc);
```

Once this function completes, `result_count` will be the number of elements in `multiple_drops`.

### Compiling and Building

When compiling, make sure you have included the required headers above.  In order to build the flowthing_io_c library, you will need the HTTP library and the standard C math library.  Depending on the port, the flowthing_io_c library will use different HTTP libraries.  Currently, it only supports libcurl, so you will have to link that when building.

### Porting

This library uses standard C99 so it should be portable to most devices/OSes.  If the target OS does not support libcurl, you can easily modify this library to use any other HTTP library.  Simply remove the `#define USING_HTTP_LIBRARY_CURL` line from `flowthings_io_http.h` and add another define.  For instance `#define USING_HTTP_MYLIB`.  Then add the corresponding `#ifdef` sections into `flowthings_io_http.c`
