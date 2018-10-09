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

#define CHECK_ERROR(expect_parse, expect_type, json) \
	do {\
		SIMPLEJ_VALUE sj_value;\
		sj_value.sj_type = SIMPLEJ_NULL;\
		CHECK_FOR_EQUAL(expect_parse, simplejson_parse(&sj_value,json), SIMPLEJ_PARSE_ERROR_STR);\
		CHECK_FOR_EQUAL(expect_type, get_simplejson_type(&sj_value), SIMPLEJ_TYPE_ERROR_STR);\
	} while(0)

static void test_parse_number() {
	CHECK_ERROR(SIMPLEJ_PARSE_OK, SIMPLEJ_NUMBER, "-1.23");
}

#if 0
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
}

static void test_parse_root_not_singular() {
	CHECK_ERROR(SIMPLEJ_PARSE_ROOT_NOT_SINGULAR, SIMPLEJ_NULL, " 	null x");
	CHECK_ERROR(SIMPLEJ_PARSE_ROOT_NOT_SINGULAR, SIMPLEJ_NULL, "	false x");
}
#endif
		

static void test_parse() {
#if 0
	test_parse_null();
	test_parse_false();
	test_parse_true();
	test_parse_invalid_value();
	test_parse_expect_value();
	test_parse_root_not_singular();
#endif
	test_parse_number();
}

int main(int argc, char *argv[]) {
	test_parse();
	printf("totalTest: %d, passTest: %d, %d/%d (%3.2f%%) passed\n", testTotalNum, testPassNum, testPassNum, testTotalNum, testPassNum * 100.0 / testTotalNum);
	return 0;
}
