#include "simplejson.h"
#include <string.h>
#include <assert.h> /* assert */
#include <stdlib.h> /* strtod */
#include <math.h> /* HUGE_VAL */
#include <errno.h> /* errno */

#ifndef SJSON_PARSE_STACK_INIT_SIZE
#define SJSON_PARSE_STACK_INIT_SIZE 256
#endif

#define IS_DIGIT1_9(ch) (((ch)>='1') && ((ch)<='9'))
#define IS_DIGIT(ch) (((ch)>='0') && ((ch)<='9'))

#define EXCEPT_CH(str,ch) do { assert(*str == ch); str++; } while(0)
#define PUTC(context, ch) do {*(char *)simplejson_context_push(context, sizeof(char)) = (ch);} while(0)


const char *sj_parse_str[6] = {
  "SIMPLEJ_PARSE_OK", "SIMPLEJ_PARSE_EXPECT_VALUE",
  "SIMPLEJ_PARSE_INVALID_VALUE", "SIMPLEJ_PARSE_ROOT_NOT_SINGULAR",
	"SIMPLEJ_PARSE_NUMBER_TOO_BIG", "SIMPLEJ_PARSE_MISS_QUOTATION_MASK"
};

const char *sj_type_str[7] = {
	"SIMPLEJ_NULL", "SIMPLEJ_FALSE", "SIMPLEJ_TRUE",
	"SIMPLEJ_NUMBER", "SIMPLEJ_STRING", "SIMPLEJ_ARRAY",
	"SIMPLEJ_OBJECT"
};

static void * simplejson_context_push(SIMPLEJ_CONTEXT *sj_context, size_t size) {
	void * ret;
	assert(size > 0);
	/* 这里通过条件将需要更新stack的情况进行处理 */
	if (sj_context->top + size >= sj_context->size) {
		/* 首先判断是否已经初始化过stack */
		if (sj_context->size == 0) {
			sj_context->size = SJSON_PARSE_STACK_INIT_SIZE;
		}
		/* 这里使用while可以确保更新后的内存是否能够满足空间的需求 */
		while (sj_context->top + size >= sj_context->size) {
			sj_context->size += sj_context->size >> 1;  /* size * 1.5 */
		}
		sj_context->stack = (char *)realloc(sj_context->stack, sj_context->size);
	}
	ret = sj_context->stack + sj_context->top;	
	sj_context->top += size;
	return ret;
}

SIMPLEJ_TYPE get_simplejson_type(const SIMPLEJ_VALUE *sj_value) {
	assert(sj_value != NULL);
	return sj_value->sj_type;
}

double get_simplejson_number(const SIMPLEJ_VALUE *sj_value) {
	assert(sj_value != NULL && sj_value->sj_type == SIMPLEJ_NUMBER);	
	return sj_value->u.number;
}

int get_simplejson_boolean(const SIMPLEJ_VALUE *sj_value) {
	assert(sj_value != NULL && (sj_value->sj_type == SIMPLEJ_TRUE || sj_value->sj_type == SIMPLEJ_FALSE));	
	return sj_value->sj_type == SIMPLEJ_TRUE;
}

void set_simplejson_boolean(SIMPLEJ_VALUE *sj_value, int value) {
	sj_free(sj_value);
	sj_value->sj_type = value ? SIMPLEJ_TRUE : SIMPLEJ_FALSE;
}

const char * get_simplejson_string(const SIMPLEJ_VALUE *sj_value) {
	assert(sj_value != NULL && sj_value->sj_type == SIMPLEJ_STRING);	
	return sj_value->u.s.s;
}

size_t get_simplejson_string_length(const SIMPLEJ_VALUE *sj_value) {
	assert(sj_value != NULL && sj_value->sj_type == SIMPLEJ_STRING);	
	return sj_value->u.s.len;	
}

void set_simplejson_string(SIMPLEJ_VALUE *sj_value, const char* str, size_t len) {
	/* 先判断输入非空 */
	assert(sj_value != NULL && (str != NULL || len != 0));
	/* 先free相关的结构体 */
	sj_free(sj_value);
	/* 需要申请len+1个空间,最后一位置为\0 */
	sj_value->u.s.s = (char *)malloc(len + 1);
	sj_value->u.s.s[len] = '\0';
	/* copy */
	memcpy(sj_value->u.s.s, str, len);	
	sj_value->u.s.len = len;
	sj_value->sj_type = SIMPLEJ_STRING;
}

SIMPLEJ_PARSE_RESULT simplejson_parse_string(SIMPLEJ_VALUE *sj_value, SIMPLEJ_CONTEXT *sj_context) {
	const char * tmp_str;
	char current_ch;
	tmp_str = sj_context->json;
	EXCEPT_CH(tmp_str, '\"');	
	while (1) {
		current_ch = *tmp_str++;
		switch(current_ch) {
			case '\"':
				set_simplejson_string(sj_value, sj_context->stack, sj_context->top);
				sj_context->json = tmp_str;
				return SIMPLEJ_PARSE_OK;
			case '\0':
				sj_context->top = 0;
				return SIMPLEJ_PARSE_MISS_QUOTATION_MARK;
			default:
				PUTC(sj_context, current_ch);	
		}
	}
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
		case '"': return simplejson_parse_string(sj_value, sj_context); 
	}	
}

SIMPLEJ_PARSE_RESULT simplejson_parse(SIMPLEJ_VALUE *sj_value, const char *input_str) {
	int ret;
	SIMPLEJ_CONTEXT sj_context;
	sj_context.json = input_str;
	strip_space(&sj_context);
	sj_context.stack = NULL;
	sj_context.top = sj_context.size = 0;
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

void sj_free(SIMPLEJ_VALUE *sj_value) {
  assert(sj_value != NULL);
  /* 当保存的为string时,free掉其用来保存内容的变量 */
  if (sj_value->sj_type == SIMPLEJ_STRING) {
    free(sj_value->u.s.s);  
  }
  sj_value->sj_type = SIMPLEJ_NULL;
}

