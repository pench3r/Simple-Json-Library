#ifndef SIMPLEJSON_H__
#define SIMPLEJSON_H__

#include <stdio.h>

/* parse resutl type */
typedef enum {
	SIMPLEJ_PARSE_OK = 0,
	SIMPLEJ_PARSE_EXPECT_VALUE,
	SIMPLEJ_PARSE_INVALID_VALUE,
	SIMPLEJ_PARSE_ROOT_NOT_SINGULAR
} SIMPLEJ_PARSE_RESULT;

/* error str type */
typedef enum {
	SIMPLEJ_PARSE_ERROR_STR = 0,
	SIMPLEJ_TYPE_ERROR_STR
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
	SIMPLEJ_TYPE sj_type;
} SIMPLEJ_VALUE;

const char *sj_parse_str[4];
const char *sj_type_str[7];

/* parse function
param: SIMPLEJ_VALUE to recv parse value type
			 str that present input string
return: parse result type			 
*/
SIMPLEJ_PARSE_RESULT simplejson_parse(SIMPLEJ_VALUE *sj_value, const char *str);

/* get value type */
SIMPLEJ_TYPE get_simplejson_type(const SIMPLEJ_VALUE *sj_value);

SIMPLEJ_PARSE_RESULT simplejson_parse_value(SIMPLEJ_VALUE *sj_value, const char *str);

SIMPLEJ_PARSE_RESULT simplejson_parse_null(SIMPLEJ_VALUE *sj_value, const char *str);

SIMPLEJ_PARSE_RESULT simplejson_parse_true(SIMPLEJ_VALUE *sj_value, const char *str);

SIMPLEJ_PARSE_RESULT simplejson_parse_false(SIMPLEJ_VALUE *sj_value, const char *str);

/* this argument can use some struct to store user input str address that easy to update input str */
void strip_space(const char **input_str);

int is_except_str(const char *input_str, char *except_str);

#endif /* SIMPLEJSON_H__ */
