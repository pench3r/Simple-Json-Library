#include "simplejson.h"
#include <string.h>
#include <assert.h> /* assert */
#include <stdlib.h> /* strtod */
#include <math.h> /* HUGE_VAL */
#include <errno.h> /* errno */


#define IS_DIGIT1_9(ch) (((ch)>='1') && ((ch)<='9'))
#define IS_DIGIT(ch) (((ch)>='0') && ((ch)<='9'))


const char *sj_parse_str[5] = {
  "SIMPLEJ_PARSE_OK", "SIMPLEJ_PARSE_EXPECT_VALUE",
  "SIMPLEJ_PARSE_INVALID_VALUE", "SIMPLEJ_PARSE_ROOT_NOT_SINGULAR",
	"SIMPLEJ_PARSE_NUMBER_TOO_BIG"
};

const char *sj_type_str[7] = {
	"SIMPLEJ_NULL", "SIMPLEJ_FALSE", "SIMPLEJ_TRUE",
	"SIMPLEJ_NUMBER", "SIMPLEJ_STRING", "SIMPLEJ_ARRAY",
	"SIMPLEJ_OBJECT"
};

SIMPLEJ_TYPE get_simplejson_type(const SIMPLEJ_VALUE *sj_value) {
	assert(sj_value != NULL);
	return sj_value->sj_type;
}

double get_simplejson_number(const SIMPLEJ_VALUE *sj_value) {
	assert(sj_value != NULL && sj_value->sj_type == SIMPLEJ_NUMBER);	
	return sj_value->u.number;
}

const char * get_simplejson_string(const SIMPLEJ_VALUE *sj_value) {
	assert(sj_value != NULL && sj_value->sj_type == SIMPLEJ_STRING);	
	return sj_value->u.s.s;
}

SIMPLEJ_PARSE_RESULT simplejson_parse_string(SIMPLEJ_VALUE *sj_value, const char *str) {
	return SIMPLEJ_PARSE_OK;
}

SIMPLEJ_PARSE_RESULT simplejson_parse_literal(SIMPLEJ_VALUE *sj_value, SIMPLEJ_CONTEXT *sj_context, const char *except, SIMPLEJ_TYPE type) {
	if (is_except_str(sj_context->json, except)) {
		sj_value->sj_type = type;
		sj_context->json += strlen(except);
		return SIMPLEJ_PARSE_OK;
	}
	return SIMPLEJ_PARSE_INVALID_VALUE;
}

SIMPLEJ_PARSE_RESULT simplejson_parse_number(SIMPLEJ_VALUE *sj_value, SIMPLEJ_CONTEXT *sj_context) {
	double tmp_number = 0.0;
	char *endPtr;
	const char *str = sj_context->json;
	errno = 0;
	tmp_number = strtod(str, &endPtr);
	if ((tmp_number == HUGE_VAL || tmp_number == -HUGE_VAL) && errno == ERANGE) return SIMPLEJ_PARSE_NUMBER_TOO_BIG;
	/* parse first part */
	if (*str == '-')
		str++;

	/* parse second part */
	if (*str == '0') {
		str++;
	} else {
		if (!IS_DIGIT1_9(*str)) return SIMPLEJ_PARSE_INVALID_VALUE;
		str++;
		while(IS_DIGIT(*str))
			str++;
	}

	/* parse third part */
	if (*str == '.') {
		str++;
		if (!IS_DIGIT(*str)) return SIMPLEJ_PARSE_INVALID_VALUE;
		str++;	
		while(IS_DIGIT(*str))
			str++;
	}

	/* parse four part */
	if (*str == 'e' || *str == 'E')	{
		str++;
		if (*str == '-' || *str == '+')
			str++;
		if (!IS_DIGIT(*str)) return SIMPLEJ_PARSE_INVALID_VALUE;
		str++;	
		while(IS_DIGIT(*str))
			str++;
	}
	
	/* last parse */
	if (str == endPtr ) {
		sj_value->u.number = tmp_number;
		sj_value->sj_type = SIMPLEJ_NUMBER;
		sj_context->json = str;
		return SIMPLEJ_PARSE_OK;
	}
	
	return SIMPLEJ_PARSE_INVALID_VALUE;
}

SIMPLEJ_PARSE_RESULT simplejson_parse_value(SIMPLEJ_VALUE *sj_value, SIMPLEJ_CONTEXT *sj_context) {
	char first_char = *(sj_context->json);
	switch(first_char) {
		case 'n': return simplejson_parse_literal(sj_value, sj_context, "null", SIMPLEJ_NULL);
		case 't': return simplejson_parse_literal(sj_value, sj_context, "true", SIMPLEJ_TRUE);
		case 'f': return simplejson_parse_literal(sj_value, sj_context, "false", SIMPLEJ_FALSE);
		case '\0': return SIMPLEJ_PARSE_EXPECT_VALUE;
		default: return simplejson_parse_number(sj_value, sj_context);
	}	
}

SIMPLEJ_PARSE_RESULT simplejson_parse(SIMPLEJ_VALUE *sj_value, const char *input_str) {
	int ret;
	SIMPLEJ_CONTEXT sj_context;
	sj_context.json = input_str;
	strip_space(&sj_context);
	if ((ret = simplejson_parse_value(sj_value, &sj_context)) == SIMPLEJ_PARSE_OK) {
		strip_space(&sj_context);
		if (*sj_context.json != '\0') {
			sj_value->sj_type = SIMPLEJ_NULL;
			ret = SIMPLEJ_PARSE_ROOT_NOT_SINGULAR;
		}
	}
	return ret;
}

void strip_space(SIMPLEJ_CONTEXT *sj_context) {
	const char *tmp_str = sj_context->json;
	while ( *tmp_str == ' ' || *tmp_str == '\t' || *tmp_str == '\n' || *tmp_str == '\r')
		tmp_str++;
	sj_context->json = tmp_str;
}

int is_except_str(const char *input_str,const char *except_str) {
	int except_len = strlen(except_str);
	size_t index;
	for (index = 0; index < except_len; index++) {
		if (input_str[index] != except_str[index])
			return 0;	
	}	
	return 1;
}
