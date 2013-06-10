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


static const mapreduce_json_t doc1 = {
    .json = "{\"value\": 1}",
    .length = sizeof("{\"value\": 1}") - 1
};
static const mapreduce_json_t meta1 = {
    .json = "{\"id\":\"doc1\"}",
    .length = sizeof("{\"id\":\"doc1\"}") - 1
};

static const mapreduce_json_t doc2 = {
    .json = "{\"value\": 2}",
    .length = sizeof("{\"value\": 2}") - 1
};
static const mapreduce_json_t meta2 = {
    .json = "{\"id\":\"doc2\"}",
    .length = sizeof("{\"id\":\"doc2\"}") - 1
};

static const mapreduce_json_t doc3 = {
    .json = "{\"value\": 3}",
    .length = sizeof("{\"value\": 3}") - 1
};
static const mapreduce_json_t meta3 = {
    .json = "{\"id\":\"doc3\"}",
    .length = sizeof("{\"id\":\"doc3\"}") - 1
};

static void test_bad_syntax_functions(void);
static void test_runtime_exception(void);
static void test_runtime_error(void);
static void test_map_no_emit(void);
static void test_map_single_emit(void);
static void test_map_multiple_emits(void);
static void test_timeout(void);


void map_tests(void)
{
    TPRINT("Running map tests\n");

    mapreduce_set_timeout(1);
    test_timeout();

    for (int i = 0; i < 100; ++i) {
        test_bad_syntax_functions();
        test_runtime_exception();
        test_runtime_error();
        test_map_no_emit();
        test_map_single_emit();
        test_map_multiple_emits();
    }

    test_timeout();
}


static void test_bad_syntax_functions(void)
{
    void *context = NULL;
    char *error_msg = NULL;
    mapreduce_error_t ret;
    const char *functions[] = {
        "function(doc, meta) { emit(meta.id, null); }",
        "function(doc, meta { emit(doc.field, meta.id); }"
    };

    ret = mapreduce_start_map_context(functions, 2, &context, &error_msg);
    assert(ret == MAPREDUCE_SYNTAX_ERROR);
    assert(error_msg != NULL);
    assert(strlen(error_msg) > 0);
    assert(context == NULL);

    mapreduce_free_error_msg(error_msg);
}


static void test_runtime_exception(void)
{
    void *context = NULL;
    char *error_msg = NULL;
    mapreduce_error_t ret;
    const char *functions[] = {
        "function(doc, meta) { throw('foobar'); }"
    };
    mapreduce_map_result_list_t *result = NULL;

    ret = mapreduce_start_map_context(functions, 1, &context, &error_msg);
    assert(ret == MAPREDUCE_SUCCESS);
    assert(error_msg == NULL);
    assert(context != NULL);

    ret = mapreduce_map(context, &doc1, &meta1, &result);
    assert(ret == MAPREDUCE_SUCCESS);
    assert(result != NULL);
    assert(result->length == 1);
    assert(result->list != NULL);

    assert(result->list[0].error == MAPREDUCE_RUNTIME_ERROR);
    assert(result->list[0].result.error_msg != NULL);
    assert(strcmp("foobar", result->list[0].result.error_msg) == 0);

    mapreduce_free_map_result_list(result);
    mapreduce_free_context(context);
}


static void test_runtime_error(void)
{
    void *context = NULL;
    char *error_msg = NULL;
    mapreduce_error_t ret;
    const char *functions[] = {
        "function(doc, meta) { emit(doc.foo.bar, meta.id); }"
    };
    mapreduce_map_result_list_t *result = NULL;

    ret = mapreduce_start_map_context(functions, 1, &context, &error_msg);
    assert(ret == MAPREDUCE_SUCCESS);
    assert(error_msg == NULL);
    assert(context != NULL);

    ret = mapreduce_map(context, &doc1, &meta1, &result);
    assert(ret == MAPREDUCE_SUCCESS);
    assert(result != NULL);
    assert(result->length == 1);
    assert(result->list != NULL);

    assert(result->list[0].error == MAPREDUCE_RUNTIME_ERROR);
    assert(result->list[0].result.error_msg != NULL);
    assert(strcmp("TypeError: Cannot read property 'bar' of undefined",
                  result->list[0].result.error_msg) == 0);

    mapreduce_free_map_result_list(result);
    mapreduce_free_context(context);
}


static void test_map_no_emit(void)
{
    void *context = NULL;
    char *error_msg = NULL;
    mapreduce_error_t ret;
    const char *functions[] = {
        "function(doc, meta) { }",
        "function(doc, meta) { if (doc.value > 12345) { emit(meta.id, null); } }"
    };
    mapreduce_map_result_list_t *result = NULL;

    ret = mapreduce_start_map_context(functions, 2, &context, &error_msg);
    assert(ret == MAPREDUCE_SUCCESS);
    assert(error_msg == NULL);
    assert(context != NULL);

    ret = mapreduce_map(context, &doc1, &meta1, &result);
    assert(ret == MAPREDUCE_SUCCESS);
    assert(result != NULL);
    assert(result->length == 2);
    assert(result->list != NULL);

    assert(result->list[0].error == MAPREDUCE_SUCCESS);
    assert(result->list[0].result.kvs.length == 0);

    assert(result->list[1].error == MAPREDUCE_SUCCESS);
    assert(result->list[1].result.kvs.length == 0);

    mapreduce_free_map_result_list(result);
    mapreduce_free_context(context);
}


static void test_map_single_emit(void)
{
    void *context = NULL;
    char *error_msg = NULL;
    mapreduce_error_t ret;
    const char *functions[] = {
        "function(doc, meta) { emit(meta.id, doc.value); }",
        "function(doc, meta) { emit(doc.value, meta.id); }"
    };
    mapreduce_map_result_list_t *result = NULL;

    ret = mapreduce_start_map_context(functions, 2, &context, &error_msg);
    assert(ret == MAPREDUCE_SUCCESS);
    assert(error_msg == NULL);
    assert(context != NULL);

    ret = mapreduce_map(context, &doc1, &meta1, &result);
    assert(ret == MAPREDUCE_SUCCESS);
    assert(result != NULL);
    assert(result->length == 2);
    assert(result->list != NULL);

    assert(result->list[0].error == MAPREDUCE_SUCCESS);
    assert(result->list[0].result.kvs.length == 1);
    assert(result->list[0].result.kvs.kvs[0].key.length == (sizeof("\"doc1\"") - 1));
    assert(memcmp(result->list[0].result.kvs.kvs[0].key.json,
                  "\"doc1\"",
                  (sizeof("\"doc1\"") - 1)) == 0);
    assert(result->list[0].result.kvs.kvs[0].value.length == (sizeof("1") - 1));
    assert(memcmp(result->list[0].result.kvs.kvs[0].value.json,
                  "1",
                  (sizeof("1") - 1)) == 0);

    assert(result->list[1].error == MAPREDUCE_SUCCESS);
    assert(result->list[1].result.kvs.length == 1);
    assert(result->list[1].result.kvs.kvs[0].key.length == (sizeof("1") - 1));
    assert(memcmp(result->list[1].result.kvs.kvs[0].key.json,
                  "1",
                  (sizeof("1") - 1)) == 0);
    assert(result->list[1].result.kvs.kvs[0].value.length == (sizeof("\"doc1\"") - 1));
    assert(memcmp(result->list[1].result.kvs.kvs[0].value.json,
                  "\"doc1\"",
                  (sizeof("\"doc1\"") - 1)) == 0);

    mapreduce_free_map_result_list(result);

    ret = mapreduce_map(context, &doc2, &meta2, &result);
    assert(ret == MAPREDUCE_SUCCESS);
    assert(result != NULL);
    assert(result->length == 2);
    assert(result->list != NULL);

    assert(result->list[0].error == MAPREDUCE_SUCCESS);
    assert(result->list[0].result.kvs.length == 1);
    assert(result->list[0].result.kvs.kvs[0].key.length == (sizeof("\"doc2\"") - 1));
    assert(memcmp(result->list[0].result.kvs.kvs[0].key.json,
                  "\"doc2\"",
                  (sizeof("\"doc2\"") - 1)) == 0);
    assert(result->list[0].result.kvs.kvs[0].value.length == (sizeof("2") - 1));
    assert(memcmp(result->list[0].result.kvs.kvs[0].value.json,
                  "2",
                  (sizeof("2") - 1)) == 0);

    assert(result->list[1].error == MAPREDUCE_SUCCESS);
    assert(result->list[1].result.kvs.length == 1);
    assert(result->list[1].result.kvs.kvs[0].key.length == (sizeof("2") - 1));
    assert(memcmp(result->list[1].result.kvs.kvs[0].key.json,
                  "2",
                  (sizeof("2") - 1)) == 0);
    assert(result->list[1].result.kvs.kvs[0].value.length == (sizeof("\"doc2\"") - 1));
    assert(memcmp(result->list[1].result.kvs.kvs[0].value.json,
                  "\"doc2\"",
                  (sizeof("\"doc2\"") - 1)) == 0);

    mapreduce_free_map_result_list(result);

    mapreduce_free_context(context);
}


static void test_map_multiple_emits(void)
{
    void *context = NULL;
    char *error_msg = NULL;
    mapreduce_error_t ret;
    const char *functions[] = {
        "function(doc, meta) {\n"
        "  if (doc.value != 1) { throw('foobar'); } else { emit(meta.id, doc.value); }\n"
        "}\n",
        "function(doc, meta) {\n"
        "  emit(doc.value, meta.id);\n"
        "  emit([meta.id, doc.value], null);\n"
        "  emit(doc.value * 5, -doc.value);\n"
        "}\n",
        "function(doc, meta) {\n"
        "  if (doc.value != 3) { emit(doc.value, 0); } else { emit(meta.id, doc.value.f.z); }\n"
        "}\n"
    };
    mapreduce_map_result_list_t *result = NULL;

    ret = mapreduce_start_map_context(functions, 3, &context, &error_msg);
    assert(ret == MAPREDUCE_SUCCESS);
    assert(error_msg == NULL);
    assert(context != NULL);

    /* map doc1 */
    ret = mapreduce_map(context, &doc1, &meta1, &result);
    assert(ret == MAPREDUCE_SUCCESS);
    assert(result != NULL);
    assert(result->length == 3);
    assert(result->list != NULL);

    /* function 1 */
    assert(result->list[0].error == MAPREDUCE_SUCCESS);
    assert(result->list[0].result.kvs.length == 1);
    assert(result->list[0].result.kvs.kvs[0].key.length == (sizeof("\"doc1\"") - 1));
    assert(memcmp(result->list[0].result.kvs.kvs[0].key.json,
                  "\"doc1\"",
                  (sizeof("\"doc1\"") - 1)) == 0);
    assert(result->list[0].result.kvs.kvs[0].value.length == (sizeof("1") - 1));
    assert(memcmp(result->list[0].result.kvs.kvs[0].value.json,
                  "1",
                  (sizeof("1") - 1)) == 0);

    /* function 2 */
    assert(result->list[1].error == MAPREDUCE_SUCCESS);
    assert(result->list[1].result.kvs.length == 3);
    assert(result->list[1].result.kvs.kvs[0].key.length == (sizeof("1") - 1));
    assert(memcmp(result->list[1].result.kvs.kvs[0].key.json,
                  "1",
                  (sizeof("1") - 1)) == 0);
    assert(result->list[1].result.kvs.kvs[0].value.length == (sizeof("\"doc1\"") - 1));
    assert(memcmp(result->list[1].result.kvs.kvs[0].value.json,
                  "\"doc1\"",
                  (sizeof("\"doc1\"") - 1)) == 0);
    assert(result->list[1].result.kvs.kvs[1].key.length == (sizeof("[\"doc1\",1]") - 1));
    assert(memcmp(result->list[1].result.kvs.kvs[1].key.json,
                  "[\"doc1\",1]",
                  (sizeof("[\"doc1\",1]") - 1)) == 0);
    assert(result->list[1].result.kvs.kvs[1].value.length == (sizeof("null") - 1));
    assert(memcmp(result->list[1].result.kvs.kvs[1].value.json,
                  "null",
                  (sizeof("null") - 1)) == 0);
    assert(result->list[1].result.kvs.kvs[2].key.length == (sizeof("5") - 1));
    assert(memcmp(result->list[1].result.kvs.kvs[2].key.json,
                  "5",
                  (sizeof("5") - 1)) == 0);
    assert(result->list[1].result.kvs.kvs[2].value.length == (sizeof("-1") - 1));
    assert(memcmp(result->list[1].result.kvs.kvs[2].value.json,
                  "-1",
                  (sizeof("-1") - 1)) == 0);

    /* function 3 */
    assert(result->list[2].error == MAPREDUCE_SUCCESS);
    assert(result->list[2].result.kvs.length == 1);
    assert(result->list[2].result.kvs.kvs[0].key.length == (sizeof("1") - 1));
    assert(memcmp(result->list[2].result.kvs.kvs[0].key.json,
                  "1",
                  (sizeof("1") - 1)) == 0);
    assert(result->list[2].result.kvs.kvs[0].value.length == (sizeof("0") - 1));
    assert(memcmp(result->list[2].result.kvs.kvs[0].value.json,
                  "0",
                  (sizeof("0") - 1)) == 0);

    mapreduce_free_map_result_list(result);

    /* map doc2 */
    ret = mapreduce_map(context, &doc2, &meta2, &result);
    assert(ret == MAPREDUCE_SUCCESS);
    assert(result != NULL);
    assert(result->length == 3);
    assert(result->list != NULL);

    /* function 1 */
    assert(result->list[0].error == MAPREDUCE_RUNTIME_ERROR);
    assert(result->list[0].result.error_msg != NULL);
    assert(strcmp("foobar", result->list[0].result.error_msg) == 0);

    /* function 2 */
    assert(result->list[1].error == MAPREDUCE_SUCCESS);
    assert(result->list[1].result.kvs.length == 3);
    assert(result->list[1].result.kvs.kvs[0].key.length == (sizeof("2") - 1));
    assert(memcmp(result->list[1].result.kvs.kvs[0].key.json,
                  "2",
                  (sizeof("2") - 1)) == 0);
    assert(result->list[1].result.kvs.kvs[0].value.length == (sizeof("\"doc2\"") - 1));
    assert(memcmp(result->list[1].result.kvs.kvs[0].value.json,
                  "\"doc2\"",
                  (sizeof("\"doc2\"") - 1)) == 0);
    assert(result->list[1].result.kvs.kvs[1].key.length == (sizeof("[\"doc2\",2]") - 1));
    assert(memcmp(result->list[1].result.kvs.kvs[1].key.json,
                  "[\"doc2\",2]",
                  (sizeof("[\"doc2\",2]") - 1)) == 0);
    assert(result->list[1].result.kvs.kvs[1].value.length == (sizeof("null") - 1));
    assert(memcmp(result->list[1].result.kvs.kvs[1].value.json,
                  "null",
                  (sizeof("null") - 1)) == 0);
    assert(result->list[1].result.kvs.kvs[2].key.length == (sizeof("10") - 1));
    assert(memcmp(result->list[1].result.kvs.kvs[2].key.json,
                  "10",
                  (sizeof("10") - 1)) == 0);
    assert(result->list[1].result.kvs.kvs[2].value.length == (sizeof("-2") - 1));
    assert(memcmp(result->list[1].result.kvs.kvs[2].value.json,
                  "-2",
                  (sizeof("-2") - 1)) == 0);

    /* function 3 */
    assert(result->list[2].error == MAPREDUCE_SUCCESS);
    assert(result->list[2].result.kvs.length == 1);
    assert(result->list[2].result.kvs.kvs[0].key.length == (sizeof("2") - 1));
    assert(memcmp(result->list[2].result.kvs.kvs[0].key.json,
                  "2",
                  (sizeof("2") - 1)) == 0);
    assert(result->list[2].result.kvs.kvs[0].value.length == (sizeof("0") - 1));
    assert(memcmp(result->list[2].result.kvs.kvs[0].value.json,
                  "0",
                  (sizeof("0") - 1)) == 0);

    mapreduce_free_map_result_list(result);

    /* map doc3 */
    ret = mapreduce_map(context, &doc3, &meta3, &result);
    assert(ret == MAPREDUCE_SUCCESS);
    assert(result != NULL);
    assert(result->length == 3);
    assert(result->list != NULL);

    /* function 1 */
    assert(result->list[0].error == MAPREDUCE_RUNTIME_ERROR);
    assert(result->list[0].result.error_msg != NULL);
    assert(strcmp("foobar", result->list[0].result.error_msg) == 0);

    /* function 2 */
    assert(result->list[1].error == MAPREDUCE_SUCCESS);
    assert(result->list[1].result.kvs.length == 3);
    assert(result->list[1].result.kvs.kvs[0].key.length == (sizeof("3") - 1));
    assert(memcmp(result->list[1].result.kvs.kvs[0].key.json,
                  "3",
                  (sizeof("3") - 1)) == 0);
    assert(result->list[1].result.kvs.kvs[0].value.length == (sizeof("\"doc3\"") - 1));
    assert(memcmp(result->list[1].result.kvs.kvs[0].value.json,
                  "\"doc3\"",
                  (sizeof("\"doc3\"") - 1)) == 0);
    assert(result->list[1].result.kvs.kvs[1].key.length == (sizeof("[\"doc3\",3]") - 1));
    assert(memcmp(result->list[1].result.kvs.kvs[1].key.json,
                  "[\"doc3\",3]",
                  (sizeof("[\"doc3\",3]") - 1)) == 0);
    assert(result->list[1].result.kvs.kvs[1].value.length == (sizeof("null") - 1));
    assert(memcmp(result->list[1].result.kvs.kvs[1].value.json,
                  "null",
                  (sizeof("null") - 1)) == 0);
    assert(result->list[1].result.kvs.kvs[2].key.length == (sizeof("15") - 1));
    assert(memcmp(result->list[1].result.kvs.kvs[2].key.json,
                  "15",
                  (sizeof("15") - 1)) == 0);
    assert(result->list[1].result.kvs.kvs[2].value.length == (sizeof("-3") - 1));
    assert(memcmp(result->list[1].result.kvs.kvs[2].value.json,
                  "-3",
                  (sizeof("-3") - 1)) == 0);

    /* function 3 */
    assert(result->list[2].error == MAPREDUCE_RUNTIME_ERROR);
    assert(result->list[2].result.error_msg != NULL);
    assert(strcmp("TypeError: Cannot read property 'z' of undefined",
                  result->list[2].result.error_msg) == 0);

    mapreduce_free_map_result_list(result);

    mapreduce_free_context(context);
}


static void test_timeout(void)
{
    void *context = NULL;
    char *error_msg = NULL;
    mapreduce_error_t ret;
    const char *functions[] = {
        "function(doc, meta) {"
        "  if (doc.value === 1) {"
        "    while (true) { };"
        "  } else {"
        "    emit(meta.id, doc.value);"
        "  }"
        "}"
    };
    mapreduce_map_result_list_t *result = NULL;

    ret = mapreduce_start_map_context(functions, 1, &context, &error_msg);
    assert(ret == MAPREDUCE_SUCCESS);
    assert(error_msg == NULL);
    assert(context != NULL);

    ret = mapreduce_map(context, &doc1, &meta1, &result);
    assert(ret == MAPREDUCE_TIMEOUT);
    assert(result == NULL);

    ret = mapreduce_map(context, &doc2, &meta2, &result);
    assert(ret == MAPREDUCE_SUCCESS);
    assert(result != NULL);
    assert(result->length == 1);
    assert(result->list != NULL);

    assert(result->list[0].error == MAPREDUCE_SUCCESS);
    assert(result->list[0].result.kvs.length == 1);
    assert(result->list[0].result.kvs.kvs[0].key.length == (sizeof("\"doc2\"") - 1));
    assert(memcmp(result->list[0].result.kvs.kvs[0].key.json,
                  "\"doc2\"",
                  (sizeof("\"doc2\"") - 1)) == 0);
    assert(result->list[0].result.kvs.kvs[0].value.length == (sizeof("2") - 1));
    assert(memcmp(result->list[0].result.kvs.kvs[0].value.json,
                  "2",
                  (sizeof("2") - 1)) == 0);

    mapreduce_free_map_result_list(result);
    mapreduce_free_context(context);
}
