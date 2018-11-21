#ifndef SIMPLEJSON_H__
#define SIMPLEJSON_H__

#include <stdio.h>

/* parse resutl type */
typedef enum {
	SIMPLEJ_PARSE_OK = 0,
	SIMPLEJ_PARSE_EXPECT_VALUE,
	SIMPLEJ_PARSE_INVALID_VALUE,
	SIMPLEJ_PARSE_ROOT_NOT_SINGULAR,
	SIMPLEJ_PARSE_NUMBER_TOO_BIG,
	SIMPLEJ_PARSE_MISS_QUOTATION_MARK,
	SIMPLEJ_PARSE_INVALID_STRING_ESCAPE,
	SIMPLEJ_PARSE_INVALID_STRING_CHAR
} SIMPLEJ_PARSE_RESULT;

/* error str type */
typedef enum {
	SIMPLEJ_PARSE_ERROR_STR = 0,
	SIMPLEJ_TYPE_ERROR_STR,
	SIMPLEJ_NUMBER_ERROR_STR,
	SIMPLEJ_STRING_ERROR_STR
} SIMPLEJ_ERROR_STR;

/* json type */
typedef enum {
	SIMPLEJ_NULL = 0,
	SIMPLEJ_FALSE,
	SIMPLEJ_TRUE,
	SIMPLEJ_NUMBER,
	SIMPLEJ_STRING,
	SIMPLEJ_ARRAY,
	SIMPLEJ_OBJECT
} SIMPLEJ_TYPE;

/* basic parse node struct */
typedef struct {
	union {
		struct {char* s; size_t len;}s;	/* string */
		double number;	/* number */
	}u;
	SIMPLEJ_TYPE sj_type;
} SIMPLEJ_VALUE;

/* basic parse context struct */
typedef struct {
	const char * json;
	char * stack;
	size_t top, size;
} SIMPLEJ_CONTEXT;

const char *sj_parse_str[8];
const char *sj_type_str[7];

/* parse function
param: SIMPLEJ_VALUE to recv parse value type
			 str that present input string
return: parse result type			 
*/
SIMPLEJ_PARSE_RESULT simplejson_parse(SIMPLEJ_VALUE *sj_value, const char *str);

/* get value type */
SIMPLEJ_TYPE get_simplejson_type(const SIMPLEJ_VALUE *sj_value);

/* get number */
double get_simplejson_number(const SIMPLEJ_VALUE *sj_value);

/* set number */
void set_simplejson_number(SIMPLEJ_VALUE *sj_value, double value);

/* get boolean */
int get_simplejson_boolean(const SIMPLEJ_VALUE *sj_value);

/* set boolean */
void set_simplejson_boolean(SIMPLEJ_VALUE *sj_value, int value);

/* get string */
const char * get_simplejson_string(const SIMPLEJ_VALUE *sj_value);

/* get string len */
size_t get_simplejson_string_length(const SIMPLEJ_VALUE *sj_value);

/* set string */
void set_simplejson_string(SIMPLEJ_VALUE *sj_value, const char* str, size_t len);

SIMPLEJ_PARSE_RESULT simplejson_parse_value(SIMPLEJ_VALUE *sj_value, SIMPLEJ_CONTEXT *sj_context);

SIMPLEJ_PARSE_RESULT simplejson_parse_literal(SIMPLEJ_VALUE *sj_value, SIMPLEJ_CONTEXT *sj_context, const char *except, SIMPLEJ_TYPE type);

SIMPLEJ_PARSE_RESULT simplejson_parse_number(SIMPLEJ_VALUE *sj_value, SIMPLEJ_CONTEXT *sj_context);

SIMPLEJ_PARSE_RESULT simplejson_parse_string(SIMPLEJ_VALUE *sj_value, SIMPLEJ_CONTEXT *sj_context);

/* this argument can use some struct to store user input str address that easy to update input str */
void strip_space(SIMPLEJ_CONTEXT *sj_context);

int is_except_str(const char *input_str, const char *except_str);

void sj_free(SIMPLEJ_VALUE *sj_value);

#endif /* SIMPLEJSON_H__ */
