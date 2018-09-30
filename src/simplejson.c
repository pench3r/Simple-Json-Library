#include "simplejson.h"

const char *sj_parse_str[4] = {
  "SIMPLEJ_PARSE_OK", "SIMPLEJ_PARSE_EXPECT_VALUE",
  "SIMPLEJ_PARSE_INVALID_VALUE", "SIMPLEJ_PARSE_ROOT_NOT_SINGULAR"
};

const char *sj_type_str[7] = {
	"SIMPLEJ_NULL", "SIMPLEJ_FALSE", "SIMPLEJ_TRUE",
	"SIMPLEJ_NUMBER", "SIMPLEJ_STRING", "SIMPLEJ_ARRAY",
	"SIMPLEJ_OBJECT"
}

SIMPLEJ_TYPE get_simplejson_type(const SIMPLEJ_VALUE *sj_value) {
	return sj_value->sj_type;
}

SIMPLEJ_PARSE_RESULT simple_parse_null(SIMPLEJ_VALUE *sj_value, const char *str) {
	char *tmp_str = str;
	tmp_str++;
	if (tmp_str[0] == 'u' && tmp_str[1] == 'l' && tmp_str[2] == 'l' && tmp_str[3] == '\0')
		sj_value->sj_type = SIMPLEJ_NULL;
		return SIMPLEJ_PARSE_OK;	
	return SIMPLEJ_PARSE_INVALID_VALUE;
}

SIMPLEJ_PARSE_RESULT simplejson_parse_value(SIMPLEJ_VALUE *sj_value, const char *str) {
	char first_char = *str;
	switch(first_char) {
		case 'n': simplejson_parse_null(sj_value, str);
		default: return SIMPLEJ_PARSE_INVALID_VALUE;
	}	
}

SIMPLEJ_PARSE_RESULT simplejson_parse(SIMPLEJ_VALUE *sj_value, const char *input_str) {
	return simplejson_parse_value(&sj_value, input_str);
}
