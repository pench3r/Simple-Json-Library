#include "simplejson.h"

static int testTotalNum = 0;
static int testPassNum = 0;

#define CHECK_FOR_EQUAL(expect, actual, check_type) \
	do {\
		testTotalNum++;\
		if (expect == actual) {\
			testPassNum++;\
		} else {\
			if (check_type == 0) {\
				fprintf(stderr, "%s:%d: expect: %s, actual: %s.\n", __FILE__, __LINE__, sj_parse_str[expect], sj_parse_str[actual]);\
			} else {\
				fprintf(stderr, "%s:%d: expect: %s, actual: %s.\n", __FILE__, __LINE__, sj_type_str[expect], sj_type_str[actual]);\
			}\
		}\
	} while(0)

static void test_parse_null() {
	SIMPLEJ_VALUE sj_value;
	sj_value.sj_type = SIMPLEJ_NULL;
	CHECK_FOR_EQUAL(SIMPLEJ_PARSE_OK, simplejson_parse(&sj_value,"null"), SIMPLEJ_PARSE_ERROR_STR);
	CHECK_FOR_EQUAL(SIMPLEJ_NULL, get_simplejson_type(&sj_value), SIMPLEJ_TYPE_ERROR_STR);

	sj_value.sj_type = SIMPLEJ_NULL;
	CHECK_FOR_EQUAL(SIMPLEJ_PARSE_OK, simplejson_parse(&sj_value," 	null"), SIMPLEJ_PARSE_ERROR_STR);
	CHECK_FOR_EQUAL(SIMPLEJ_NULL, get_simplejson_type(&sj_value), SIMPLEJ_TYPE_ERROR_STR);

	sj_value.sj_type = SIMPLEJ_NULL;
	CHECK_FOR_EQUAL(SIMPLEJ_PARSE_OK, simplejson_parse(&sj_value,"	 null	 "), SIMPLEJ_PARSE_ERROR_STR);
	CHECK_FOR_EQUAL(SIMPLEJ_NULL, get_simplejson_type(&sj_value), SIMPLEJ_TYPE_ERROR_STR);
}

static void test_parse_false() {
	SIMPLEJ_VALUE sj_value;
	sj_value.sj_type = SIMPLEJ_NULL;
	CHECK_FOR_EQUAL(SIMPLEJ_PARSE_OK, simplejson_parse(&sj_value,"false"), SIMPLEJ_PARSE_ERROR_STR);
	CHECK_FOR_EQUAL(SIMPLEJ_FALSE, get_simplejson_type(&sj_value), SIMPLEJ_TYPE_ERROR_STR);

	sj_value.sj_type = SIMPLEJ_NULL;
	CHECK_FOR_EQUAL(SIMPLEJ_PARSE_OK, simplejson_parse(&sj_value," 	false"), SIMPLEJ_PARSE_ERROR_STR);
	CHECK_FOR_EQUAL(SIMPLEJ_FALSE, get_simplejson_type(&sj_value), SIMPLEJ_TYPE_ERROR_STR);

	sj_value.sj_type = SIMPLEJ_NULL;
	CHECK_FOR_EQUAL(SIMPLEJ_PARSE_OK, simplejson_parse(&sj_value," 	false	 "), SIMPLEJ_PARSE_ERROR_STR);
	CHECK_FOR_EQUAL(SIMPLEJ_FALSE, get_simplejson_type(&sj_value), SIMPLEJ_TYPE_ERROR_STR);
}

static void test_parse_true() {
	SIMPLEJ_VALUE sj_value;
	sj_value.sj_type = SIMPLEJ_NULL;
	CHECK_FOR_EQUAL(SIMPLEJ_PARSE_OK, simplejson_parse(&sj_value,"true"), SIMPLEJ_PARSE_ERROR_STR);
	CHECK_FOR_EQUAL(SIMPLEJ_TRUE, get_simplejson_type(&sj_value), SIMPLEJ_TYPE_ERROR_STR);

	sj_value.sj_type = SIMPLEJ_NULL;
	CHECK_FOR_EQUAL(SIMPLEJ_PARSE_OK, simplejson_parse(&sj_value," 	true"), SIMPLEJ_PARSE_ERROR_STR);
	CHECK_FOR_EQUAL(SIMPLEJ_TRUE, get_simplejson_type(&sj_value), SIMPLEJ_TYPE_ERROR_STR);

	sj_value.sj_type = SIMPLEJ_NULL;
	CHECK_FOR_EQUAL(SIMPLEJ_PARSE_OK, simplejson_parse(&sj_value," 	true	 "), SIMPLEJ_PARSE_ERROR_STR);
	CHECK_FOR_EQUAL(SIMPLEJ_TRUE, get_simplejson_type(&sj_value), SIMPLEJ_TYPE_ERROR_STR);
}

static void test_parse_expect_value() {
	SIMPLEJ_VALUE sj_value;
	sj_value.sj_type = SIMPLEJ_NULL;
	CHECK_FOR_EQUAL(SIMPLEJ_PARSE_EXPECT_VALUE, simplejson_parse(&sj_value,""), SIMPLEJ_PARSE_ERROR_STR);
	CHECK_FOR_EQUAL(SIMPLEJ_NULL, get_simplejson_type(&sj_value), SIMPLEJ_TYPE_ERROR_STR);

	sj_value.sj_type = SIMPLEJ_NULL;
	CHECK_FOR_EQUAL(SIMPLEJ_PARSE_EXPECT_VALUE, simplejson_parse(&sj_value," 	"), SIMPLEJ_PARSE_ERROR_STR);
	CHECK_FOR_EQUAL(SIMPLEJ_NULL, get_simplejson_type(&sj_value), SIMPLEJ_TYPE_ERROR_STR);
}

static void test_parse_invalid_value() {
	SIMPLEJ_VALUE sj_value;
	sj_value.sj_type = SIMPLEJ_NULL;
	CHECK_FOR_EQUAL(SIMPLEJ_PARSE_INVALID_VALUE, simplejson_parse(&sj_value," 	fals"), SIMPLEJ_PARSE_ERROR_STR);
	CHECK_FOR_EQUAL(SIMPLEJ_NULL, get_simplejson_type(&sj_value), SIMPLEJ_TYPE_ERROR_STR);

	sj_value.sj_type = SIMPLEJ_NULL;
	CHECK_FOR_EQUAL(SIMPLEJ_PARSE_INVALID_VALUE, simplejson_parse(&sj_value,"nul "), SIMPLEJ_PARSE_ERROR_STR);
	CHECK_FOR_EQUAL(SIMPLEJ_NULL, get_simplejson_type(&sj_value), SIMPLEJ_TYPE_ERROR_STR);
}

static void test_parse_root_not_singular() {
	SIMPLEJ_VALUE sj_value;
	sj_value.sj_type = SIMPLEJ_NULL;
	CHECK_FOR_EQUAL(SIMPLEJ_PARSE_ROOT_NOT_SINGULAR, simplejson_parse(&sj_value," 	null 	x"), SIMPLEJ_PARSE_ERROR_STR);
	CHECK_FOR_EQUAL(SIMPLEJ_NULL, get_simplejson_type(&sj_value), SIMPLEJ_TYPE_ERROR_STR);

	sj_value.sj_type = SIMPLEJ_NULL;
	CHECK_FOR_EQUAL(SIMPLEJ_PARSE_ROOT_NOT_SINGULAR, simplejson_parse(&sj_value,"null 	x"), SIMPLEJ_PARSE_ERROR_STR);
	CHECK_FOR_EQUAL(SIMPLEJ_NULL, get_simplejson_type(&sj_value), SIMPLEJ_TYPE_ERROR_STR);
}
		

static void test_parse() {
	test_parse_null();
	test_parse_false();
	test_parse_true();
	test_parse_invalid_value();
	test_parse_expect_value();
	test_parse_root_not_singular();
}

int main(int argc, char *argv[]) {
	test_parse();
	printf("totalTest: %d, passTest: %d, %d/%d (%3.2f%%) passed\n", testTotalNum, testPassNum, testPassNum, testTotalNum, testPassNum * 100.0 / testTotalNum);
	return 0;
}
