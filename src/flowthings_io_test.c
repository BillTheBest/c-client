
#include "mraa.h"

#include <stdio.h>
#include <unistd.h>

#include "flowthings_io_services.h"

#ifdef  __cplusplus
extern "C" {
#endif

struct my_flow {
	char id[100];
	char path[100];
	char description[100];
};

struct my_drop {
	int num;
};

BOOL decode_my_flow(cJSON *json_in, void *obj_out)
{
	if (!obj_out || !json_in)
		return FALSE;

	struct my_flow *mf = (struct my_flow *)obj_out;

	cJSON* jp = cJSON_GetObjectItem(json_in, "path");

	char *id = cJSON_GetObjectItem(json_in, "id")->valuestring;
	strcpy(mf->id, id);

	char *path = cJSON_GetObjectItem(json_in, "path")->valuestring;
	strcpy(mf->path, path);

	char *description = cJSON_GetObjectItem(json_in, "description")->valuestring;
	strcpy(mf->description, description);

	return TRUE;
}

BOOL encode_my_flow(void *obj_in, cJSON *json_out)
{
	if (!obj_in || !json_out)
		return FALSE;

	struct my_flow *mf = (struct my_flow *)obj_in;

	cJSON_AddStringToObject(json_out, "path", mf->path);
	cJSON_AddStringToObject(json_out, "description", mf->description);

	return TRUE;
}

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

int main()
{
	flowthings_io_token creds;
	flowthings_io_result_code code;

	creds.account = "myaccountname";
	creds.token = "mytoken";

	/* TEST 1 - request */

	flowthings_io_string *s = flowthings_io_string_init();
	flowthings_io_http *fhttp = flowthings_io_http_init(FLOWTHINGS_IO_VERSION,
			FLOWTHINGS_IO_HOST, FALSE, &creds);

	char base_path[] = "/flow";
	fhttp->base_path = base_path;

	flowthings_io_http_request(fhttp, "GET", "/f55252d860cf228002a19fd04", NULL, s);

	printf("test 1 result: %s\n", s->ptr);

	flowthings_io_http_cleanup(fhttp);
	flowthings_io_string_cleanup(s);

	/* TEST 2 - read */

	struct my_flow result;
	flowthings_io_api *api = flowthings_io_api_init(FLOWTHINGS_IO_VERSION, FLOWTHINGS_IO_HOST, FALSE, &creds);

	flowthings_io_service_read(FLOWTHINGS_IO_SERVICE_TYPE_FLOW, NULL, api, "f55252d860cf228002a19fd04", NULL, decode_my_flow, &result);

	printf("test 2 result: %s, %s\n", result.description, result.path);

	/* TEST 3 - create */

	strcpy(result.description, "the newly created flow");
	strcpy(result.path, "/raj/ctesta12346");

	code = flowthings_io_service_create(FLOWTHINGS_IO_SERVICE_TYPE_FLOW, NULL, api, NULL, encode_my_flow, decode_my_flow, &result);

	if (code != FLOWTHINGS_IO_OK) {
		printf("Failed create");
		exit(1);
	}

	printf("test 3 result %d: %s, %s, %s\n", code, result.id, result.description, result.path);

	/* TEST 4 - update */

	strcpy(result.description, "the newly created flow renamed");

	code = flowthings_io_service_update(FLOWTHINGS_IO_SERVICE_TYPE_FLOW, NULL, api, result.id, NULL, encode_my_flow, decode_my_flow, &result);

	if (code != FLOWTHINGS_IO_OK) {
		printf("Failed update");
		exit(1);
	}

	printf("test 4 result %d: %s, %s, %s\n", code, result.id, result.description, result.path);

	/* TEST 5 - delete */

	// code = flowthings_io_service_delete(FLOWTHINGS_IO_SERVICE_TYPE_FLOW, api, result.id, NULL);
	code = flowthings_io_flow_delete(api, result.id, NULL);

	if (code != FLOWTHINGS_IO_OK) {
		printf("Failed delete");
		exit(1);
	}

	printf("test 5 delete ok\n");

	/* TEST 6 - find a drop */

	struct my_drop drop_result;
	struct my_drop multiple_drops[10];
	int result_count = 10;
	int *rc = &result_count;
	flowthings_io_drop_find("f552a87090cf2afb329f31f37", api, "elems.num>3", NULL, decode_my_drop, &multiple_drops, rc);

	printf("test 6 returned %d results: %d\n", result_count, multiple_drops[0].num);

	/* TEST 7 - read a drop */

	flowthings_io_drop_read("f552a87090cf2afb329f31f37", api, "d5565c4a168056d6bd8c4c4be", NULL, decode_my_drop, &drop_result);
	printf("test 7 result: %d\n", drop_result.num);

	/* TEST 8 - find multiple */

	*rc = 10;
	flowthings_io_idlist *idlist = flowthings_io_idlist_init();
	flowthings_io_params *params = flowthings_io_params_init();
	flowthings_io_params_add(params, "filter", "elems.num > 3");
	flowthings_io_idlist_add(idlist, "f552a87090cf2afb329f31f37", params);

	// flatten=flat isn't working
	//flowthings_io_drop_find_many(api, decode_my_drop, idlist, &multiple_drops, rc);
	//printf("test 8 returned %d results: %d\n", result_count, multiple_drops[0].num);

	flowthings_io_params_cleanup(params);
	flowthings_io_idlist_cleanup(idlist);

	struct my_drop new_drop;
	new_drop.num = 99;

	code = flowthings_io_drop_create("f552a87090cf2afb329f31f37", api, NULL, encode_my_drop, decode_my_drop, &new_drop);

	flowthings_io_api_cleanup(api);

	return 0;
}


#ifdef  __cplusplus
}
#endif
