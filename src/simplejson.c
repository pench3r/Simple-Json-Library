#include "simplejson.h"

const char *sj_parse_str[4] = {
  "SIMPLEJ_PARSE_OK", "SIMPLEJ_PARSE_EXPECT_VALUE",
  "SIMPLEJ_PARSE_INVALID_VALUE", "SIMPLEJ_PARSE_ROOT_NOT_SINGULAR"
};


SIMPLEJ_PARSE_RESULT simplejson_parse(const char *input_str) {
	printf("%s: parse ok\n", input_str);
	return SIMPLEJ_PARSE_OK;
}
