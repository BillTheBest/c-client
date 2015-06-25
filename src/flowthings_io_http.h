/*
 * flowthings_io_http.h
 *
 *  Created on: May 20, 2015
 */

#ifndef FLOWTHINGS_IO_HTTP_H_
#define FLOWTHINGS_IO_HTTP_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#ifdef  __cplusplus
extern "C" {
#endif

#define USING_HTTP_LIBRARY_CURL

/***********************************************************************
 * Flowthings includes
 ***********************************************************************/

#include "flowthings_io.h"


/***********************************************************************
 * HTTP definitions
 ***********************************************************************/

#define FLOWTHINGS_IO_MAX_URL_SIZE 1000
#define FLOWTHINGS_IO_MAX_PATH_SIZE 500

#define FLOWTHINGS_IO_HTTP_METHOD_GET "GET"
#define FLOWTHINGS_IO_HTTP_METHOD_MGET "MGET"
#define FLOWTHINGS_IO_HTTP_METHOD_POST "POST"
#define FLOWTHINGS_IO_HTTP_METHOD_PUT "PUT"
#define FLOWTHINGS_IO_HTTP_METHOD_DELETE "DELETE"


/***********************************************************************
 * The flowthings HTTP object, used for all HTTP calls
 ***********************************************************************/

typedef struct flowthings_io_http {

	flowthings_io_token *creds;
	const char *version;
	const char *host;
	const char *base_path;
	BOOL secure;

#ifdef USING_HTTP_LIBRARY_CURL
	CURL* curl;
#endif

} flowthings_io_http;


/***********************************************************************
 * Flowthings HTTP functions
 ***********************************************************************/

/*
 * NAME: flowthings_io_http_init
 *
 * Initializes the flowthings_io_http object and returns it. Shouldn't be called directly from
 * outside this library.
 *
 * PARAMS:
 * version - the flowthings platform version
 * host - the domain/IP address
 * secure - true if this is a secure connection; depending on the HTTP library, this may require
 *    extra configuration
 * creds - a flowthings_io_token object with the user's credentials
 */
flowthings_io_http *flowthings_io_http_init(const char *version,
		const char *host, BOOL secure, flowthings_io_token *creds);


/*
 * NAME: flowthings_io_http_cleanup
 *
 * Cleans up an HTTP object
 */
void flowthings_io_http_cleanup(flowthings_io_http *fhttp);


/*
 * NAME: flowthings_io_http_writefunc
 *
 * A callback for reading info from the HTTP library
 */
size_t flowthings_io_http_writefunc(void *ptr, size_t size, size_t nmemb,
		flowthings_io_string *s);


/*
 * NAME: flowthings_io_http_request
 *
 * Make an HTTP request to the flowthings server.
 *
 * PARAMS:
 * fhttp - the HTTP object
 * method - one of FLOWTHINGS_IO_HTTP_METHOD_*
 * path - the path on the flowthings platform, starting with a /
 * data - the post data string
 * response - a flowthigns_io_string that must previously allocated
 *
 * RETURN:
 * Returns a HTTP response code, or 0 for an unknown error.
 */
int flowthings_io_http_request(flowthings_io_http *fhttp, const char *method,
		const char *path, const char *data, flowthings_io_string *response);

/*
 * NAME: flowthings_io_http_urlencode
 *
 * URLencodes an input and puts it in a flowthings_io_string
 *
 * PARAMS:
 * input - the input value to url encode
 * output - the flowthings_io_string (must be pre-allocated) where the encoded value will go
 */
void flowthings_io_http_urlencode(const char *input, flowthings_io_string *output);

#ifdef  __cplusplus
}
#endif



#endif /* FLOWTHINGS_IO_HTTP_H_ */
