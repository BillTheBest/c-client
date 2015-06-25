/*
 * flowthings_io_api.c
 *
 *  Created on: May 21, 2015
 */


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

#include "flowthings_io.h"
#include "flowthings_io_http.h"
#include "flowthings_io_api.h"

/***********************************************************************
 * The API functions
 ***********************************************************************/

/*
 * NAME: flowthings_io_api_init
 *
 * Initializes the flowthings_io api object and returns it
 *
 * PARAMS:
 * version - the flowthings platform version
 * host - the domain/IP address
 * secure - true if this is a secure connection; depending on the HTTP library, this may require
 *    extra configuration
 * creds - a flowthings_io_token object with the user's credentials
 */
flowthings_io_api *flowthings_io_api_init(const char *version,
		const char *host, BOOL secure, flowthings_io_token *creds)
{
	flowthings_io_api *api = malloc(sizeof(flowthings_io_api));
	if (!api) FAIL;

	api->fhttp = flowthings_io_http_init(version, host, secure, creds);

	return api;
}

/*
 * NAME: flowthings_io_api_cleanup
 *
 * Cleans up the flowthings.io API, must be called for all flowthings_io_api
 * objects.
 */
void flowthings_io_api_cleanup(flowthings_io_api *api)
{
	if (api) {
		if (api->fhttp) flowthings_io_http_cleanup(api->fhttp);

		free(api);
	}
}


#ifdef  __cplusplus
}
#endif
