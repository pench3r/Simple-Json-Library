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
		

void test_parse() {
	SIMPLEJ_VALUE sj_value;
	sj_value.sj_type = SIMPLEJ_NULL;
	CHECK_FOR_EQUAL(SIMPLEJ_PARSE_OK, simplejson_parse(&sj_value,"  	xnull"), SIMPLEJ_PARSE_ERROR_STR);
	CHECK_FOR_EQUAL(SIMPLEJ_NULL, get_simplejson_type(&sj_value), SIMPLEJ_TYPE_ERROR_STR);

	sj_value.sj_type = SIMPLEJ_NULL;
	CHECK_FOR_EQUAL(SIMPLEJ_PARSE_OK, simplejson_parse(&sj_value," 	true"), SIMPLEJ_PARSE_ERROR_STR);
	CHECK_FOR_EQUAL(SIMPLEJ_TRUE, get_simplejson_type(&sj_value), SIMPLEJ_TYPE_ERROR_STR);

	sj_value.sj_type = SIMPLEJ_NULL;
	CHECK_FOR_EQUAL(SIMPLEJ_PARSE_OK, simplejson_parse(&sj_value,"false"), SIMPLEJ_PARSE_ERROR_STR);
	CHECK_FOR_EQUAL(SIMPLEJ_FALSE, get_simplejson_type(&sj_value), SIMPLEJ_TYPE_ERROR_STR);
}

int main(int argc, char *argv[]) {
	test_parse();
	printf("totalTest: %d, passTest: %d, %d/%d (%3.2f%%) passed\n", testTotalNum, testPassNum, testPassNum, testTotalNum, testPassNum * 100.0 / testTotalNum);
	return 0;
}
