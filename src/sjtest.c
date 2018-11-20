#include "simplejson.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int testTotalNum = 0;
static int testPassNum = 0;

#define CHECK_FOR_EQUAL(equality, expect, actual, check_type) \
	do {\
		testTotalNum++;\
		if (equality) {\
			testPassNum++;\
		} else {\
			if (check_type == SIMPLEJ_PARSE_ERROR_STR) {\
					fprintf(stderr, "%s:%d: expect: %s, actual: %s.\n", __FILE__, __LINE__, sj_parse_str[(int)expect], sj_parse_str[(int)actual]);\
			} else if (check_type == SIMPLEJ_TYPE_ERROR_STR) {\
					fprintf(stderr, "%s:%d: expect: %s, actual: %s.\n", __FILE__, __LINE__, sj_type_str[(int)expect], sj_type_str[(int)actual]);\
			} else if (check_type == SIMPLEJ_NUMBER_ERROR_STR) {\
					fprintf(stderr, "%s:%d: expect: %g, actual: %g.\n", __FILE__, __LINE__, (double)expect, (double)actual);\
			} else {\
					fprintf(stderr, "%s:%d: expect: %d, actual: %d.\n", __FILE__, __LINE__, (int)expect, (int)actual);\
			}\
		}\
	} while(0)

#define EXPECT_EQ_BASE(equality, expect, actual, format) \
	do {\
		testTotalNum++;\
		if (equality) {\
			testPassNum++;\
		} else {\
			fprintf(stderr, "%s:%d: expect:" format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
		}\
	} while(0) 
	
#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect)==(actual), expect, actual, "%d")
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect)==(actual), expect, actual, "%g")
#define EXPECT_EQ_STRING(expect, actual, len) \
	EXPECT_EQ_BASE(sizeof(expect) -1 == len && memcmp(expect, actual, len) == 0, expect, actual, "%s")

#define CHECK_ERROR(expect_parse, expect_type, json) \
	do {\
		SIMPLEJ_VALUE sj_value;\
		sj_value.sj_type = SIMPLEJ_NULL;\
		EXPECT_EQ_INT(expect_parse, simplejson_parse(&sj_value, json));\
		EXPECT_EQ_INT(expect_type, get_simplejson_type(&sj_value));\
	} while(0)

#define CHECK_NUMBER(expect, json) \
	do {\
		SIMPLEJ_VALUE sj_value;\
		sj_value.sj_type = SIMPLEJ_NULL;\
		EXPECT_EQ_INT(SIMPLEJ_PARSE_OK, simplejson_parse(&sj_value,json));\
		EXPECT_EQ_INT(SIMPLEJ_NUMBER, get_simplejson_type(&sj_value));\
		EXPECT_EQ_DOUBLE(expect, get_simplejson_number(&sj_value));\
	} while(0)

#define CHECK_STRING(expect, json) \
	do {\
		SIMPLEJ_VALUE sj_value;\
		sj_value.sj_type = SIMPLEJ_NULL;\
		EXPECT_EQ_INT(SIMPLEJ_PARSE_OK, simplejson_parse(&sj_value,json));\
		EXPECT_EQ_INT(SIMPLEJ_STRING, get_simplejson_type(&sj_value));\
		EXPECT_EQ_STRING(expect, get_simplejson_string(&sj_value), get_simplejson_string_length(&sj_value));\
	} while(0)

static void test_parse_string() {
	CHECK_STRING("hello", "\"hello\"");
}

static void test_parse_number() {
#if 0
	CHECK_ERROR(SIMPLEJ_PARSE_OK, SIMPLEJ_NUMBER, "0");
	CHECK_ERROR(SIMPLEJ_PARSE_OK, SIMPLEJ_NUMBER, "-0");
	CHECK_ERROR(SIMPLEJ_PARSE_OK, SIMPLEJ_NUMBER, "-0.0");
	CHECK_ERROR(SIMPLEJ_PARSE_OK, SIMPLEJ_NUMBER, "1");
	CHECK_ERROR(SIMPLEJ_PARSE_OK, SIMPLEJ_NUMBER, "1.5");
	CHECK_ERROR(SIMPLEJ_PARSE_OK, SIMPLEJ_NUMBER, "-1");
	CHECK_ERROR(SIMPLEJ_PARSE_OK, SIMPLEJ_NUMBER, "-1.23");
	CHECK_ERROR(SIMPLEJ_PARSE_OK, SIMPLEJ_NUMBER, "3.1416");
	CHECK_ERROR(SIMPLEJ_PARSE_OK, SIMPLEJ_NUMBER, "1E+10");
	CHECK_ERROR(SIMPLEJ_PARSE_OK, SIMPLEJ_NUMBER, "1E-10");
	CHECK_ERROR(SIMPLEJ_PARSE_OK, SIMPLEJ_NUMBER, "-1E10");
	CHECK_ERROR(SIMPLEJ_PARSE_OK, SIMPLEJ_NUMBER, "-1e10");
	CHECK_ERROR(SIMPLEJ_PARSE_OK, SIMPLEJ_NUMBER, "-1E+10");
	CHECK_ERROR(SIMPLEJ_PARSE_OK, SIMPLEJ_NUMBER, "-1E-10");
	CHECK_ERROR(SIMPLEJ_PARSE_OK, SIMPLEJ_NUMBER, "1.234E+10");
	CHECK_ERROR(SIMPLEJ_PARSE_OK, SIMPLEJ_NUMBER, "1.234E-10");
	CHECK_ERROR(SIMPLEJ_PARSE_OK, SIMPLEJ_NUMBER, "1e-10000");
#endif
	CHECK_NUMBER(0.0, "0");
	CHECK_NUMBER(0.0, "-0");
	CHECK_NUMBER(0.0, "-0.0");
	CHECK_NUMBER(1.0, "1");
	CHECK_NUMBER(-1.0, "-1");
	CHECK_NUMBER(1.5, "1.5");
	CHECK_NUMBER(-1.5, "-1.5");
	CHECK_NUMBER(3.1416, "3.1416");
	CHECK_NUMBER(1E10, "1E10");
	CHECK_NUMBER(1e10, "1e10");
	CHECK_NUMBER(1E+10, "1E+10");
	CHECK_NUMBER(1E-10, "1E-10");
	CHECK_NUMBER(-1E10, "-1E10");
	CHECK_NUMBER(-1e10, "-1e10");
	CHECK_NUMBER(-1E+10, "-1E+10");
	CHECK_NUMBER(-1E-10, "-1E-10");
	CHECK_NUMBER(1.234E+10, "1.234E+10");
	CHECK_NUMBER(1.234E-10, "1.234E-10");
	CHECK_NUMBER(0.0, "1e-10000"); /* must underflow */
}

static void test_parse_null() {
	CHECK_ERROR(SIMPLEJ_PARSE_OK, SIMPLEJ_NULL, "null");
	CHECK_ERROR(SIMPLEJ_PARSE_OK, SIMPLEJ_NULL, " 	null");
	CHECK_ERROR(SIMPLEJ_PARSE_OK, SIMPLEJ_NULL, " 	null 	");
}

static void test_parse_false() {
	CHECK_ERROR(SIMPLEJ_PARSE_OK, SIMPLEJ_FALSE, "false");
	CHECK_ERROR(SIMPLEJ_PARSE_OK, SIMPLEJ_FALSE, " 	false");
	CHECK_ERROR(SIMPLEJ_PARSE_OK, SIMPLEJ_FALSE, " 	false 	");
}

static void test_parse_true() {
	CHECK_ERROR(SIMPLEJ_PARSE_OK, SIMPLEJ_TRUE, "true");
	CHECK_ERROR(SIMPLEJ_PARSE_OK, SIMPLEJ_TRUE, " 	true");
	CHECK_ERROR(SIMPLEJ_PARSE_OK, SIMPLEJ_TRUE, " 	true 	");
}

static void test_parse_expect_value() {
	CHECK_ERROR(SIMPLEJ_PARSE_EXPECT_VALUE, SIMPLEJ_NULL, "");
	CHECK_ERROR(SIMPLEJ_PARSE_EXPECT_VALUE, SIMPLEJ_NULL, " ");
}

static void test_parse_invalid_value() {
	CHECK_ERROR(SIMPLEJ_PARSE_INVALID_VALUE, SIMPLEJ_NULL, " fals");
	CHECK_ERROR(SIMPLEJ_PARSE_INVALID_VALUE, SIMPLEJ_NULL, "nul ");
#if 1
	CHECK_ERROR(SIMPLEJ_PARSE_INVALID_VALUE, SIMPLEJ_NULL, "+0");	
	CHECK_ERROR(SIMPLEJ_PARSE_INVALID_VALUE, SIMPLEJ_NULL, "+1");	
	CHECK_ERROR(SIMPLEJ_PARSE_INVALID_VALUE, SIMPLEJ_NULL, ".123");	
	CHECK_ERROR(SIMPLEJ_PARSE_INVALID_VALUE, SIMPLEJ_NULL, "1.");	
	CHECK_ERROR(SIMPLEJ_PARSE_INVALID_VALUE, SIMPLEJ_NULL, "INF");	
	CHECK_ERROR(SIMPLEJ_PARSE_INVALID_VALUE, SIMPLEJ_NULL, "inf");	
	CHECK_ERROR(SIMPLEJ_PARSE_INVALID_VALUE, SIMPLEJ_NULL, "NAN");	
	CHECK_ERROR(SIMPLEJ_PARSE_INVALID_VALUE, SIMPLEJ_NULL, "nan");	
#endif
}

static void test_parse_root_not_singular() {
	CHECK_ERROR(SIMPLEJ_PARSE_ROOT_NOT_SINGULAR, SIMPLEJ_NULL, " 	null x");
	CHECK_ERROR(SIMPLEJ_PARSE_ROOT_NOT_SINGULAR, SIMPLEJ_NULL, "	false x");
}

static void test_parse_number_too_big() {
#if 1
	CHECK_ERROR(SIMPLEJ_PARSE_NUMBER_TOO_BIG, SIMPLEJ_NULL, "1e309");
	CHECK_ERROR(SIMPLEJ_PARSE_NUMBER_TOO_BIG, SIMPLEJ_NULL, "-1e309");
#endif
}
		

static void test_parse() {
	test_parse_string();
	test_parse_null();
	test_parse_false();
	test_parse_true();
	test_parse_invalid_value();
	test_parse_expect_value();
	test_parse_root_not_singular();
	test_parse_number();
	test_parse_number_too_big();
}

int main(int argc, char *argv[]) {
	test_parse();
	printf("totalTest: %d, passTest: %d, %d/%d (%3.2f%%) passed\n", testTotalNum, testPassNum, testPassNum, testTotalNum, testPassNum * 100.0 / testTotalNum);
	return 0;
}
