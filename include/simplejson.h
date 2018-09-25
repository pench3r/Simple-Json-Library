#ifndef SIMPLEJSON_H__
#define SIMPLEJSON_H__

#include <stdio.h>

typedef enum {
	SIMPLEJ_PARSE_OK = 0,
	SIMPLEJ_PARSE_EXPECT_VALUE,
	SIMPLEJ_PARSE_INVALID_VALUE,
	SIMPLEJ_PARSE_ROOT_NOT_SINGULAR
} SIMPLEJ_PARSE_RESULT;

const char *sj_parse_str[4];

SIMPLEJ_PARSE_RESULT simplejson_parse(const char *str);

#endif /* SIMPLEJSON_H__ */
