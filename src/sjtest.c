#include "simplejson.h"

static int testTotalNum = 0;
static int testPassNum = 0;

#define CHECK_FOR_EQUAL(expect, actual) \
	do {\
		testTotalNum++;\
		if (expect == actual) {\
			testPassNum++;\
		} else {\
			fprintf(stderr, "%s:%d: expect: %s, actual: %s.\n", __FILE__, __LINE__, sj_parse_str[expect], sj_parse_str[actual]);\
		}\
	} while(0)
		

void test_parse() {
	CHECK_FOR_EQUAL(SIMPLEJ_PARSE_INVALID_VALUE, simplejson_parse("begin"));
}

int main(int argc, char *argv[]) {
	test_parse();
	printf("totalTest: %d, passTest: %d, %d/%d (%3.2f%%) passed\n", testTotalNum, testPassNum, testPassNum, testTotalNum, testPassNum * 100.0 / testTotalNum);
	return 0;
}
