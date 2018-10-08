#include "simplejson.h"
#include <string.h>

const char *sj_parse_str[4] = {
  "SIMPLEJ_PARSE_OK", "SIMPLEJ_PARSE_EXPECT_VALUE",
  "SIMPLEJ_PARSE_INVALID_VALUE", "SIMPLEJ_PARSE_ROOT_NOT_SINGULAR"
};

const char *sj_type_str[7] = {
	"SIMPLEJ_NULL", "SIMPLEJ_FALSE", "SIMPLEJ_TRUE",
	"SIMPLEJ_NUMBER", "SIMPLEJ_STRING", "SIMPLEJ_ARRAY",
	"SIMPLEJ_OBJECT"
};

SIMPLEJ_TYPE get_simplejson_type(const SIMPLEJ_VALUE *sj_value) {
	return sj_value->sj_type;
}

SIMPLEJ_PARSE_RESULT simplejson_parse_literal(SIMPLEJ_VALUE *sj_value, const char *str, const char *except, SIMPLEJ_TYPE type) {
	if (is_except_str(str, except)) {
		sj_value->sj_type = type;
		return SIMPLEJ_PARSE_OK;
	}
	return SIMPLEJ_PARSE_INVALID_VALUE;
}

SIMPLEJ_PARSE_RESULT simplejson_parse_value(SIMPLEJ_VALUE *sj_value, const char *str) {
	char first_char = *str;
	if(is_single_word(str) == 0)
		return SIMPLEJ_PARSE_ROOT_NOT_SINGULAR;
	switch(first_char) {
		case 'n': return simplejson_parse_literal(sj_value, str, "null", SIMPLEJ_NULL);
		case 't': return simplejson_parse_literal(sj_value, str, "true", SIMPLEJ_TRUE);
		case 'f': return simplejson_parse_literal(sj_value, str, "false", SIMPLEJ_FALSE);
		case '\0': return SIMPLEJ_PARSE_EXPECT_VALUE;
		default: return SIMPLEJ_PARSE_INVALID_VALUE;
	}	
}

SIMPLEJ_PARSE_RESULT simplejson_parse(SIMPLEJ_VALUE *sj_value, const char *input_str) {
	strip_space(&input_str);
	return simplejson_parse_value(sj_value, input_str);
}

void strip_space(const char **input_str) {
	const char *tmp_str = *input_str;
	while ( *tmp_str == ' ' || *tmp_str == '\t' || *tmp_str == '\n' || *tmp_str == '\r')
		tmp_str++;
	*input_str = tmp_str;
}

int is_except_str(const char *input_str,const char *except_str) {
	int except_len = strlen(except_str);
	int index;
	for (index = 0; index < except_len; index++) {
		if (input_str[index] != except_str[index])
			return 0;	
	}	
	return 1;
}

int is_single_word(const char *input_str) {
	while(*input_str != '\0' && *input_str !=	' ' && *input_str != '\t' && *input_str != '\n' && *input_str != '\r') {
		input_str++;
	}
	strip_space(&input_str);
	if (*input_str != '\0')
		return 0;
	return 1;
}
