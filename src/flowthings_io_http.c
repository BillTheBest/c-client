/*
 * flowthings_io_http.c
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


/***********************************************************************
 * Flowthings includes
 ***********************************************************************/

#include "flowthings_io_http.h"


/***********************************************************************
 * HTTP utillity functions
 ***********************************************************************/

/*
 * NAME: __flowthings_io_makeurl
 *
 * Create a URL for the flowthings platform.
 *
 * PARAMS:
 * fhttp - the HTTP object
 * url - a URL string to be filled, must be FLOWTHINGS_IO_MAX_URL_SIZE
 * path - the path on the server
 */
static void __flowthings_io_makeurl(flowthings_io_http *fhttp,
		char *url,
		const char *path)
{
	if (!url || !path) FAIL;

	if (fhttp->secure)
		strcpy(url, "https://");
	else
		strcpy(url, "http://");

	flowthings_io_strcat(url, fhttp->host, FLOWTHINGS_IO_MAX_URL_SIZE);
	flowthings_io_strcat(url, "/v", FLOWTHINGS_IO_MAX_URL_SIZE);
	flowthings_io_strcat(url, fhttp->version, FLOWTHINGS_IO_MAX_URL_SIZE);
	flowthings_io_strcat(url, "/", FLOWTHINGS_IO_MAX_URL_SIZE);
	flowthings_io_strcat(url, fhttp->creds->account, FLOWTHINGS_IO_MAX_URL_SIZE);
	flowthings_io_strcat(url, fhttp->base_path, FLOWTHINGS_IO_MAX_URL_SIZE);
	flowthings_io_strcat(url, path, FLOWTHINGS_IO_MAX_URL_SIZE);
}



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
		const char *host, BOOL secure, flowthings_io_token *creds)
{
	flowthings_io_http *fhttp = malloc(sizeof(flowthings_io_http));
	if (!fhttp) FAIL;

#ifdef USING_HTTP_LIBRARY_CURL
	CURL *curl = curl_easy_init();
	fhttp->curl = curl;
#endif

	fhttp->host = host;
	fhttp->version = version;
	fhttp->secure = secure;
	fhttp->creds = creds;

	return fhttp;
}


/*
 * NAME: flowthings_io_http_writefunc
 *
 * A callback for reading info from the HTTP library
 */
size_t flowthings_io_http_writefunc(void *ptr,
		size_t size,
		size_t nmemb,
		flowthings_io_string *s)
{
	size_t new_len = s->len + size * nmemb;
	s->ptr = realloc(s->ptr, new_len+1);
	if (!s->ptr) FAIL;

	memcpy(s->ptr + s->len, ptr, size * nmemb);
	s->ptr[new_len] = '\0';
	s->len = new_len;

	return size * nmemb;
}


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
int flowthings_io_http_request(flowthings_io_http *fhttp,
		const char *method,
		const char *path,
		const char *data,
		flowthings_io_string *response)
{

#ifdef USING_HTTP_LIBRARY_CURL

	CURLcode res;
	char url[FLOWTHINGS_IO_MAX_URL_SIZE] = "";

	if (!method || !path || !fhttp || !response) return 0;

	struct curl_slist *headers = NULL;
	char x_auth_account[100] = "x-auth-account: ";
	char x_auth_token[100] = "x-auth-token: ";

	__flowthings_io_makeurl(fhttp, url, path);

	flowthings_io_strcat(x_auth_account, fhttp->creds->account, 100);
	flowthings_io_strcat(x_auth_token, fhttp->creds->token, 100);

	headers = curl_slist_append(headers, "Content-Type: application/json");
	headers = curl_slist_append(headers, x_auth_account);
	headers = curl_slist_append(headers, x_auth_token);

    curl_easy_setopt(fhttp->curl, CURLOPT_URL, url);
    curl_easy_setopt(fhttp->curl, CURLOPT_WRITEFUNCTION, flowthings_io_http_writefunc);
    curl_easy_setopt(fhttp->curl, CURLOPT_WRITEDATA, response);
	curl_easy_setopt(fhttp->curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(fhttp->curl, CURLOPT_CUSTOMREQUEST, method);

	/* if this isn't a get, add post data */
	if (strcmp(method, FLOWTHINGS_IO_HTTP_METHOD_DELETE) == 0) {
	}
	else if (strcmp(method, FLOWTHINGS_IO_HTTP_METHOD_GET)) {
		curl_easy_setopt(fhttp->curl, CURLOPT_POSTFIELDS, data);
	}

    res = curl_easy_perform(fhttp->curl);

    curl_slist_free_all(headers);

    if (res == CURLE_OK) {
    	long rc;
    	curl_easy_getinfo(fhttp->curl, CURLINFO_RESPONSE_CODE, &rc);

    	return (int)rc;
    }

#endif

    return 0;
}

/*
 * NAME: flowthings_io_http_urlencode
 *
 * URLencodes an input and puts it in a flowthings_io_string
 *
 * PARAMS:
 * input - the input value to url encode
 * output - the flowthings_io_string (must be pre-allocated) where the encoded value will go
 */
void flowthings_io_http_urlencode(const char *input, flowthings_io_string *output)
{

#ifdef USING_HTTP_LIBRARY_CURL

	CURL *curl = curl_easy_init();

	char *str = curl_easy_escape(curl, input, 0);
	flowthings_io_string_strcat(output, str);
	curl_free(str);

	curl_easy_cleanup(curl);

#endif

}

/*
 * NAME: flowthings_io_http_cleanup
 *
 * Cleans up an HTTP object
 */
void flowthings_io_http_cleanup(flowthings_io_http *fhttp)
{
	if (fhttp) {

#ifdef USING_HTTP_LIBRARY_CURL
		if (fhttp->curl) {
			curl_easy_cleanup(fhttp->curl);
			fhttp->curl = NULL;
		}
#endif

		free(fhttp);
	}
}


#ifdef  __cplusplus
}
#endif
