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

/*
	基本的调试宏,接收表达式和输出格式,可以在此基础上扩展各种类型的输出
	功能和使用方法上都很容易进行各种扩展
*/
#define EXPECT_EQ_BASE(equality, expect, actual, format) \
	do {\
		testTotalNum++;\
		if (equality) {\
			testPassNum++;\
		} else {\
			fprintf(stderr, "%s:%d: expect:" format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
		}\
	} while(0) 
	
/*
	封装的整数调试宏
*/
#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect)==(actual), expect, actual, "%d")
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect)==(actual), expect, actual, "%g")
/*
	字符串类型比较特殊因为它的比较需要使用memcmp,这里就很好的凸显出基本调试宏的设计
	同样可以传入%s的格式化字符进行输出
*/
#define EXPECT_EQ_STRING(expect, actual, len) \
	EXPECT_EQ_BASE(sizeof(expect) -1 == len && memcmp(expect, actual, len) == 0, expect, actual, "%s")
/*
	通过接受1或0来判断输出错误信息
*/
#define EXPECT_TRUE(actual) EXPECT_EQ_BASE((actual) != 0, "true", "false", "%s")
#define EXPECT_FALSE(actual) EXPECT_EQ_BASE((actual) != 1, "false", "true", "%s")

#define EXPECT_EQ_SIZE_T(expect, actual) EXPECT_EQ_BASE((expect)==(actual), (size_t)expect, (size_t)actual, "%zu")

/*
	这里同样是基于基本调试宏,判断解析的结果(int)和解析出的数据类型(int)
	都可以使用封装的整数调试宏,达到重用的效果
*/
#define CHECK_ERROR(expect_parse, expect_type, json) \
	do {\
		SIMPLEJ_VALUE sj_value;\
		sj_value.sj_type = SIMPLEJ_NULL;\
		EXPECT_EQ_INT(expect_parse, simplejson_parse(&sj_value, json));\
		EXPECT_EQ_INT(expect_type, get_simplejson_type(&sj_value));\
	} while(0)

/*
	唯一不同的在与最后判断解析出来的double的值的判断,使用封装的double调试宏
*/
#define CHECK_NUMBER(expect, json) \
	do {\
		SIMPLEJ_VALUE sj_value;\
		sj_value.sj_type = SIMPLEJ_NULL;\
		EXPECT_EQ_INT(SIMPLEJ_PARSE_OK, simplejson_parse(&sj_value,json));\
		EXPECT_EQ_INT(SIMPLEJ_NUMBER, get_simplejson_type(&sj_value));\
		EXPECT_EQ_DOUBLE(expect, get_simplejson_number(&sj_value));\
	} while(0)

/*
	效果同上也是单独使用了封装的string调试宏,进行了string值的比较
*/
#define CHECK_STRING(expect, json) \
	do {\
		SIMPLEJ_VALUE sj_value;\
		sj_value.sj_type = SIMPLEJ_NULL;\
		EXPECT_EQ_INT(SIMPLEJ_PARSE_OK, simplejson_parse(&sj_value,json));\
		EXPECT_EQ_INT(SIMPLEJ_STRING, get_simplejson_type(&sj_value));\
		EXPECT_EQ_STRING(expect, get_simplejson_string(&sj_value), get_simplejson_string_length(&sj_value));\
		sj_free(&sj_value);\
	} while(0)

/*
	这里需要在添加一个sj_value的参数可以在外面进行array元素的访问
*/
#define CHECK_ARRAY(sj_value, expect, json) \
	do {\
		EXPECT_EQ_INT(SIMPLEJ_PARSE_OK, simplejson_parse(sj_value,json));\
		EXPECT_EQ_INT(SIMPLEJ_ARRAY, get_simplejson_type(sj_value));\
		EXPECT_EQ_SIZE_T(expect, get_simplejson_array_size(sj_value));\
	} while(0)

static void test_parse_array() {
	SIMPLEJ_VALUE sj_value;
	sj_value.sj_type = SIMPLEJ_NULL;
#if 1
	CHECK_ARRAY(&sj_value, 0, "[ ]");
	sj_free(&sj_value);
#endif
	CHECK_ARRAY(&sj_value, 5, "[ null , false , true , 123 , \"abc\" ]");
	EXPECT_EQ_INT(SIMPLEJ_NULL, get_simplejson_type(get_simplejson_array_element(&sj_value, 0)));
	EXPECT_EQ_INT(SIMPLEJ_FALSE, get_simplejson_type(get_simplejson_array_element(&sj_value, 1)));
	EXPECT_EQ_INT(SIMPLEJ_TRUE, get_simplejson_type(get_simplejson_array_element(&sj_value, 2)));
	EXPECT_EQ_INT(SIMPLEJ_NUMBER, get_simplejson_type(get_simplejson_array_element(&sj_value, 3)));
	EXPECT_EQ_INT(SIMPLEJ_STRING, get_simplejson_type(get_simplejson_array_element(&sj_value, 4)));
	sj_free(&sj_value);
	CHECK_ARRAY(&sj_value, 4, "[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]");
	sj_free(&sj_value);

	EXPECT_EQ_INT(SIMPLEJ_PARSE_OK, simplejson_parse(&sj_value, "[ [ ] , [ 123 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]"));
	EXPECT_EQ_INT(SIMPLEJ_ARRAY, get_simplejson_type(&sj_value));
	EXPECT_EQ_SIZE_T(4, get_simplejson_array_size(&sj_value));
	for (int i=0; i<4; ++i) {
		SIMPLEJ_VALUE *tmp_sj_value = get_simplejson_array_element(&sj_value, i);
		EXPECT_EQ_INT(SIMPLEJ_ARRAY, get_simplejson_type(tmp_sj_value));
		EXPECT_EQ_SIZE_T(i, get_simplejson_array_size(tmp_sj_value));
		for (int j=0; j<i; ++j) {
			SIMPLEJ_VALUE *value = get_simplejson_array_element(tmp_sj_value, j);
			if (i == 1) {
				printf("element index: %d, type: %d\n", j, get_simplejson_type(value));
			}
			EXPECT_EQ_INT(SIMPLEJ_NUMBER, get_simplejson_type(value));
		}
	}
}

static void test_parse_string() {
	CHECK_STRING("hello", "\"hello\"");
	CHECK_STRING("123", "\"123\"");
	CHECK_STRING("hello\nworld", "\"hello\\nworld\"");
	CHECK_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
}

static void test_parse_miss_quotation_mark() {
	CHECK_ERROR(SIMPLEJ_PARSE_MISS_QUOTATION_MARK, SIMPLEJ_NULL, "\"");
	CHECK_ERROR(SIMPLEJ_PARSE_MISS_QUOTATION_MARK, SIMPLEJ_NULL, "\"h");
}

static void test_parse_invalid_string_escape() {
	CHECK_ERROR(SIMPLEJ_PARSE_INVALID_STRING_ESCAPE, SIMPLEJ_NULL, "\"\\v\"");
	CHECK_ERROR(SIMPLEJ_PARSE_INVALID_STRING_ESCAPE, SIMPLEJ_NULL,"\"\\'\"");
	CHECK_ERROR(SIMPLEJ_PARSE_INVALID_STRING_ESCAPE, SIMPLEJ_NULL, "\"\\0\"");
	CHECK_ERROR(SIMPLEJ_PARSE_INVALID_STRING_ESCAPE, SIMPLEJ_NULL, "\"\\x12\"");
}

static void test_parse_invalid_string_char() {
	CHECK_ERROR(SIMPLEJ_PARSE_INVALID_STRING_CHAR, SIMPLEJ_NULL, "\"\x01\"");
	CHECK_ERROR(SIMPLEJ_PARSE_INVALID_STRING_CHAR, SIMPLEJ_NULL, "\"\x1f\"");
}

static void test_parse_invalid_unicode_hex() {
	CHECK_ERROR(SIMPLEJ_PARSE_INVALID_UNICODE_HEX, SIMPLEJ_NULL, "\"\\u\"");
	CHECK_ERROR(SIMPLEJ_PARSE_INVALID_UNICODE_HEX, SIMPLEJ_NULL, "\"\\u0\"");
	CHECK_ERROR(SIMPLEJ_PARSE_INVALID_UNICODE_HEX, SIMPLEJ_NULL, "\"\\u01\"");
	CHECK_ERROR(SIMPLEJ_PARSE_INVALID_UNICODE_HEX, SIMPLEJ_NULL, "\"\\u012\"");
	CHECK_ERROR(SIMPLEJ_PARSE_INVALID_UNICODE_HEX, SIMPLEJ_NULL, "\"\\u/000\"");
	CHECK_ERROR(SIMPLEJ_PARSE_INVALID_UNICODE_HEX, SIMPLEJ_NULL, "\"\\uG000\"");
	CHECK_ERROR(SIMPLEJ_PARSE_INVALID_UNICODE_HEX, SIMPLEJ_NULL, "\"\\u0/00\"");
	CHECK_ERROR(SIMPLEJ_PARSE_INVALID_UNICODE_HEX, SIMPLEJ_NULL, "\"\\u00G0\"");
	CHECK_ERROR(SIMPLEJ_PARSE_INVALID_UNICODE_HEX, SIMPLEJ_NULL, "\"\\u000/\"");
	CHECK_ERROR(SIMPLEJ_PARSE_INVALID_UNICODE_HEX, SIMPLEJ_NULL, "\"\\u000G\"");
}

static void test_parse_invalid_unicode_surrogate() {
	CHECK_ERROR(SIMPLEJ_PARSE_INVALID_UNICODE_SURROGATE, SIMPLEJ_NULL, "\"\\uD800\"");
	CHECK_ERROR(SIMPLEJ_PARSE_INVALID_UNICODE_SURROGATE, SIMPLEJ_NULL, "\"\\uDBFF\"");
	CHECK_ERROR(SIMPLEJ_PARSE_INVALID_UNICODE_SURROGATE, SIMPLEJ_NULL, "\"\\uD800\\\\\"");
	CHECK_ERROR(SIMPLEJ_PARSE_INVALID_UNICODE_SURROGATE, SIMPLEJ_NULL, "\"\\uD800\\uDBFF\"");
	CHECK_ERROR(SIMPLEJ_PARSE_INVALID_UNICODE_SURROGATE, SIMPLEJ_NULL, "\"\\uD800\\uE000\"");
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
#if 0
	CHECK_ERROR(SIMPLEJ_PARSE_OK, SIMPLEJ_NULL, "null");
	CHECK_ERROR(SIMPLEJ_PARSE_OK, SIMPLEJ_NULL, " 	null");
	CHECK_ERROR(SIMPLEJ_PARSE_OK, SIMPLEJ_NULL, " 	null 	");
#endif
	SIMPLEJ_VALUE sj_value;
	sj_value.sj_type = SIMPLEJ_NULL;
	set_simplejson_boolean(&sj_value, 0);
	EXPECT_EQ_INT(SIMPLEJ_PARSE_OK, simplejson_parse(&sj_value, "null"));
	EXPECT_EQ_INT(SIMPLEJ_NULL, get_simplejson_type(&sj_value));
	sj_free(&sj_value);
}

static void test_parse_false() {
#if 0
	CHECK_ERROR(SIMPLEJ_PARSE_OK, SIMPLEJ_FALSE, "false");
	CHECK_ERROR(SIMPLEJ_PARSE_OK, SIMPLEJ_FALSE, " 	false");
	CHECK_ERROR(SIMPLEJ_PARSE_OK, SIMPLEJ_FALSE, " 	false 	");
#endif
	SIMPLEJ_VALUE sj_value;
	sj_value.sj_type = SIMPLEJ_NULL;
	set_simplejson_boolean(&sj_value, 0);
	EXPECT_EQ_INT(SIMPLEJ_PARSE_OK, simplejson_parse(&sj_value, "false"));
	EXPECT_EQ_INT(SIMPLEJ_FALSE, get_simplejson_type(&sj_value));
	sj_free(&sj_value);
}

static void test_parse_true() {
#if 0
	CHECK_ERROR(SIMPLEJ_PARSE_OK, SIMPLEJ_TRUE, "true");
	CHECK_ERROR(SIMPLEJ_PARSE_OK, SIMPLEJ_TRUE, " 	true");
	CHECK_ERROR(SIMPLEJ_PARSE_OK, SIMPLEJ_TRUE, " 	true 	");
#endif
	SIMPLEJ_VALUE sj_value;
	sj_value.sj_type = SIMPLEJ_NULL;
	set_simplejson_boolean(&sj_value, 1);
	EXPECT_EQ_INT(SIMPLEJ_PARSE_OK, simplejson_parse(&sj_value, "true"));
	EXPECT_EQ_INT(SIMPLEJ_TRUE, get_simplejson_type(&sj_value));
	sj_free(&sj_value);
}

static void test_access_boolean() {
	SIMPLEJ_VALUE sj_value;
	sj_value.sj_type = SIMPLEJ_NULL;
	set_simplejson_boolean(&sj_value, 1);
	EXPECT_TRUE(get_simplejson_boolean(&sj_value));
	sj_free(&sj_value);
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
	test_parse_array();
	test_parse_string();
	test_parse_miss_quotation_mark();
	test_parse_invalid_string_escape();
	test_parse_invalid_string_char();
	test_parse_invalid_unicode_hex();
	test_parse_invalid_unicode_surrogate();
	test_parse_null();
	test_parse_false();
	test_parse_true();
	test_access_boolean();
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
