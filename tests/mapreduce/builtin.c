/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */

/**
 * @copyright 2013 Couchbase, Inc.
 *
 * @author Filipe Manana  <filipe@couchbase.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 **/

#include "mapreduce_tests.h"
#include <string.h>


#define DOC_BODY "{" \
    "  \"values\": [10, -7, 20, 1]," \
    "  \"bin\": \"aGVsbG8gd29ybGQh\"," \
    "  \"date\":\"+033658-09-27T01:46:40.000Z\"" \
    "}"

static const mapreduce_json_t doc = {
    .json = DOC_BODY,
    .length = sizeof(DOC_BODY) - 1
};
static const mapreduce_json_t meta = {
    .json = "{\"id\":\"doc1\"}",
    .length = sizeof("{\"id\":\"doc1\"}") - 1
};

static void test_sum_function(void);
static void test_b64decode_function(void);
static void test_date_to_array_function(void);


void builtin_tests(void)
{
    TPRINT("Running mapreduce builtin tests\n");

    for (int i = 0; i < 100; ++i) {
        test_sum_function();
        test_b64decode_function();
        test_date_to_array_function();
    }
}


static void test_sum_function(void)
{
    void *context = NULL;
    char *error_msg = NULL;
    mapreduce_error_t ret;
    const char *functions[] = {
        "function(doc, meta) { emit(meta.id, sum(doc.values)); }"
    };
    mapreduce_map_result_list_t *result = NULL;

    ret = mapreduce_start_map_context(functions, 1, &context, &error_msg);
    assert(ret == MAPREDUCE_SUCCESS);
    assert(error_msg == NULL);
    assert(context != NULL);

    ret = mapreduce_map(context, &doc, &meta, &result);
    assert(ret == MAPREDUCE_SUCCESS);
    assert(result != NULL);
    assert(result->length == 1);
    assert(result->list != NULL);

    assert(result->list[0].error == MAPREDUCE_SUCCESS);
    assert(result->list[0].result.kvs.length == 1);
    assert(result->list[0].result.kvs.kvs[0].key.length == (sizeof("\"doc1\"") - 1));
    assert(memcmp(result->list[0].result.kvs.kvs[0].key.json,
                  "\"doc1\"",
                  (sizeof("\"doc1\"") - 1)) == 0);
    assert(result->list[0].result.kvs.kvs[0].value.length == (sizeof("24") - 1));
    assert(memcmp(result->list[0].result.kvs.kvs[0].value.json,
                  "24",
                  (sizeof("24") - 1)) == 0);

    mapreduce_free_map_result_list(result);
    mapreduce_free_context(context);
}


static void test_b64decode_function(void)
{
    void *context = NULL;
    char *error_msg = NULL;
    mapreduce_error_t ret;
    const char *functions[] = {
        "function(doc, meta) {"
        "  emit(meta.id, String.fromCharCode.apply(this, decodeBase64(doc.bin)));"
        "}"
    };
    mapreduce_map_result_list_t *result = NULL;

    ret = mapreduce_start_map_context(functions, 1, &context, &error_msg);
    assert(ret == MAPREDUCE_SUCCESS);
    assert(error_msg == NULL);
    assert(context != NULL);

    ret = mapreduce_map(context, &doc, &meta, &result);
    assert(ret == MAPREDUCE_SUCCESS);
    assert(result != NULL);
    assert(result->length == 1);
    assert(result->list != NULL);

    assert(result->list[0].error == MAPREDUCE_SUCCESS);
    assert(result->list[0].result.kvs.length == 1);
    assert(result->list[0].result.kvs.kvs[0].key.length == (sizeof("\"doc1\"") - 1));
    assert(memcmp(result->list[0].result.kvs.kvs[0].key.json,
                  "\"doc1\"",
                  (sizeof("\"doc1\"") - 1)) == 0);
    assert(result->list[0].result.kvs.kvs[0].value.length == (sizeof("\"hello world!\"") - 1));
    assert(memcmp(result->list[0].result.kvs.kvs[0].value.json,
                  "\"hello world!\"",
                  (sizeof("\"hello world!\"") - 1)) == 0);

    mapreduce_free_map_result_list(result);
    mapreduce_free_context(context);
}


static void test_date_to_array_function(void)
{
    void *context = NULL;
    char *error_msg = NULL;
    mapreduce_error_t ret;
    const char *functions[] = {
        "function(doc, meta) { emit(meta.id, dateToArray(doc.date)); }"
    };
    mapreduce_map_result_list_t *result = NULL;

    ret = mapreduce_start_map_context(functions, 1, &context, &error_msg);
    assert(ret == MAPREDUCE_SUCCESS);
    assert(error_msg == NULL);
    assert(context != NULL);

    ret = mapreduce_map(context, &doc, &meta, &result);
    assert(ret == MAPREDUCE_SUCCESS);
    assert(result != NULL);
    assert(result->length == 1);
    assert(result->list != NULL);

    assert(result->list[0].error == MAPREDUCE_SUCCESS);
    assert(result->list[0].result.kvs.length == 1);
    assert(result->list[0].result.kvs.kvs[0].key.length == (sizeof("\"doc1\"") - 1));
    assert(memcmp(result->list[0].result.kvs.kvs[0].key.json,
                  "\"doc1\"",
                  (sizeof("\"doc1\"") - 1)) == 0);
    assert(result->list[0].result.kvs.kvs[0].value.length == (sizeof("[33658,9,27,1,46,40]") - 1));
    assert(memcmp(result->list[0].result.kvs.kvs[0].value.json,
                  "[33658,9,27,1,46,40]",
                  (sizeof("[33658,9,27,1,46,40]") - 1)) == 0);

    mapreduce_free_map_result_list(result);
    mapreduce_free_context(context);
}
