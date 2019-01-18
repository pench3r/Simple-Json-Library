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

#define EXPECT_CH(str,ch) do { assert(*str == ch); str++; } while(0)
#define PUTC(context, ch) do {*(char *)simplejson_context_push(context, sizeof(char)) = (ch);} while(0)
#define PUT_SJ_VALUE(dest, value) \
	do {\
		memcpy(dest, value, sizeof(SIMPLEJ_VALUE));\
	} while(0)
#define PUT_SJ_MEMBER(dest, value) \
	do {\
		memcpy(dest, value, sizeof(SIMPLEJ_MEMBER));\
	} while(0)


const char *sj_parse_str[8] = {
  "SIMPLEJ_PARSE_OK", "SIMPLEJ_PARSE_EXPECT_VALUE",
  "SIMPLEJ_PARSE_INVALID_VALUE", "SIMPLEJ_PARSE_ROOT_NOT_SINGULAR",
	"SIMPLEJ_PARSE_NUMBER_TOO_BIG", "SIMPLEJ_PARSE_MISS_QUOTATION_MASK",
	"SIMPLEJ_PARSE_INVALID_STRING_ESCAPE", "SIMPLEJ_PARSE_INVALID_STRING_CHAR"
};

const char *sj_type_str[7] = {
	"SIMPLEJ_NULL", "SIMPLEJ_FALSE", "SIMPLEJ_TRUE",
	"SIMPLEJ_NUMBER", "SIMPLEJ_STRING", "SIMPLEJ_ARRAY",
	"SIMPLEJ_OBJECT"
};

/* 函数的主要功能: 返回可用的地址指针(stack + top),用于后续存储数据,并更新top值(代表可用地址的起始值)
*/
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

static void* simplejson_context_pop(SIMPLEJ_CONTEXT *sj_context, size_t size) {
	assert(sj_context->top >= size);
	return sj_context->stack + (sj_context->top -= size);
}

static const char* simplejson_parse_hex4(const char* str, unsigned* uval) {
	char ch;
	int i = 0;
	*uval = 0;
	for (i = 0; i < 4; ++i) {
		ch = *str++;
		*uval <<= 4;
		if ( ch >= '0' && ch <= '9' ) *uval |= ch - '0';	
		else if ( ch >= 'A' && ch <= 'F' ) *uval |= ch - ('A' - 10);
		else if ( ch >= 'a' && ch <= 'f' ) *uval |= ch - ('a' - 10);
		else return NULL;
	}
	return str;
}

static void simplejson_encode_utf8(SIMPLEJ_CONTEXT *sj_context, unsigned uval) {
    if (uval <= 0x7F) 
        PUTC(sj_context, uval & 0xFF);
    else if (uval <= 0x7FF) {
        PUTC(sj_context, 0xC0 | ((uval >> 6) & 0xFF));
        PUTC(sj_context, 0x80 | ( uval       & 0x3F));
    }
    else if (uval <= 0xFFFF) {
        PUTC(sj_context, 0xE0 | ((uval >> 12) & 0xFF));
        PUTC(sj_context, 0x80 | ((uval >>  6) & 0x3F));
        PUTC(sj_context, 0x80 | ( uval        & 0x3F));
    }
    else {
        assert(uval <= 0x10FFFF);
        PUTC(sj_context, 0xF0 | ((uval >> 18) & 0xFF));
        PUTC(sj_context, 0x80 | ((uval >> 12) & 0x3F));
        PUTC(sj_context, 0x80 | ((uval >>  6) & 0x3F));
        PUTC(sj_context, 0x80 | ( uval        & 0x3F));
    }
}


SIMPLEJ_TYPE get_simplejson_type(const SIMPLEJ_VALUE *sj_value) {
	assert(sj_value != NULL);
	return sj_value->sj_type;
}

double get_simplejson_number(const SIMPLEJ_VALUE *sj_value) {
	assert(sj_value != NULL && sj_value->sj_type == SIMPLEJ_NUMBER);	
	return sj_value->u.number;
}

void set_simplejson_number(SIMPLEJ_VALUE *sj_value, double value) {
	sj_free(sj_value);
	sj_value->sj_type = SIMPLEJ_NUMBER;
	sj_value->u.number = value;	
}

int get_simplejson_boolean(const SIMPLEJ_VALUE *sj_value) {
	/* 这里还需要判断类型是否为boolean中的之一 */
	assert(sj_value != NULL && (sj_value->sj_type == SIMPLEJ_TRUE || sj_value->sj_type == SIMPLEJ_FALSE));	
	return sj_value->sj_type == SIMPLEJ_TRUE;
}

/* 函数设置1类型为true,设置0类型为false */
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

size_t get_simplejson_array_size(const SIMPLEJ_VALUE *sj_value) {
	assert(sj_value != NULL && sj_value->sj_type == SIMPLEJ_ARRAY);
	return sj_value->u.a.size;
}

SIMPLEJ_VALUE* get_simplejson_array_element(const SIMPLEJ_VALUE *sj_value, size_t index) {
	assert(sj_value != NULL && sj_value->sj_type == SIMPLEJ_ARRAY);
	/* size_t 为非负整数类型 */
	assert(sj_value->u.a.size > index);
	return &sj_value->u.a.element[index];
}

void set_simplejson_array(SIMPLEJ_VALUE *sj_value, SIMPLEJ_VALUE *src_value, size_t len) {
	assert(sj_value != NULL && src_value != NULL && len > 0);
	/* 清空sj_value结构体 */
	sj_free(sj_value);
	/* 申请array存放的内容地址 */
	sj_value->u.a.element = (SIMPLEJ_VALUE *)malloc(len);
	/* copy */
	memcpy(sj_value->u.a.element, src_value, len);
	sj_value->u.a.size = len / sizeof(SIMPLEJ_VALUE);
	sj_value->sj_type = SIMPLEJ_ARRAY; 
}

SIMPLEJ_PARSE_RESULT simplejson_parse_array(SIMPLEJ_VALUE *sj_value, SIMPLEJ_CONTEXT *sj_context) {
	size_t head = sj_context->top, len, index;
	int result;
	void *p;
	EXPECT_CH(sj_context->json, '[');
	strip_space(sj_context);	
	if (*sj_context->json == ']') {
		sj_context->json++;
		sj_value->sj_type = SIMPLEJ_ARRAY;
		sj_value->u.a.size = 0;
		sj_value->u.a.element = NULL;
		return SIMPLEJ_PARSE_OK;
	}
	for (;;) {
		SIMPLEJ_VALUE tmp_value;
		tmp_value.sj_type = SIMPLEJ_NULL;
		if ((result = simplejson_parse_value(&tmp_value, sj_context)) != SIMPLEJ_PARSE_OK) {
			break;
		}
		strip_space(sj_context);	
		PUT_SJ_VALUE(simplejson_context_push(sj_context, sizeof(SIMPLEJ_VALUE)), &tmp_value);
		if (*sj_context->json == ',') {
			sj_context->json++;
			strip_space(sj_context);	
		} else if (*sj_context->json == ']') {
			sj_context->json++;
			len = sj_context->top - head;
			p = simplejson_context_pop(sj_context, len);
			set_simplejson_array(sj_value, (SIMPLEJ_VALUE *)p, len);
			/* 这里需要再处理一下遗留在stack上的SIMPLEJ_VALUE结构体 */
			/* 这里存在一个BUG，由于stack上面我保存的并非是指针 */
			/* 这里进行free导致了非预期性的执行错误 */
			/* for (index=0; index<len/sizeof(SIMPLEJ_VALUE); ++index) {
				sj_free((SIMPLEJ_VALUE *)p+index);
			}
			*/
			return SIMPLEJ_PARSE_OK;
		} else {
			result = SIMPLEJ_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
			break;
		}
	}
	/* 获取到已经压栈的value的字节数 */
	len = sj_context->top - head;
	/* 计算对应的value个数依次进行free */
	len /= sizeof(SIMPLEJ_VALUE);
	/* 当解析失败时,需要处理遗留在stack上的结构体 */
	/* 并且将他们弹出stack */
	for (index=0; index<len; ++index) {
		sj_free((SIMPLEJ_VALUE *)simplejson_context_pop(sj_context, sizeof(SIMPLEJ_VALUE)));
	}
	sj_context->top = head;
	return result; 
}

SIMPLEJ_PARSE_RESULT simplejson_parse_string(SIMPLEJ_VALUE *sj_value, SIMPLEJ_CONTEXT *sj_context) {
	size_t recvLen;
	char *recvStr;
	SIMPLEJ_PARSE_RESULT ret;
	if ((ret = simplejson_parse_string_raw(sj_context, &recvStr, &recvLen)) == SIMPLEJ_PARSE_OK) {
		set_simplejson_string(sj_value, recvStr, recvLen);	
	}
	return ret;
}

/* 
	对string解析函数进行重构,发现该函数对于sj_value参数的依赖
	只有针对set_string时才会进行sj_value的操作,因此思路将解析字符串的功能
	和set_string的功能分开,这样就可以在解析object中使用解析key
*/
SIMPLEJ_PARSE_RESULT simplejson_parse_string_raw(SIMPLEJ_CONTEXT *sj_context, char **outStr, size_t *outLen) {
	/* 保存stack的top信息,用于回滚操作 */
	size_t head = sj_context->top, len;
	const char * tmp_str;
	const char *tmp_pos;
	char current_ch;
	unsigned u1, u2;
	tmp_str = sj_context->json;
	EXPECT_CH(tmp_str, '\"');	
	while (1) {
		current_ch = *tmp_str++;
		switch(current_ch) {
			/* 该分支处理转义字符 */
			/* \" 为一个字节 */
			case '\\':
				current_ch = *tmp_str++;
				switch(current_ch) {
					case 'u':
						tmp_pos = simplejson_parse_hex4(tmp_str, &u1);
						if (tmp_pos == NULL) return SIMPLEJ_PARSE_INVALID_UNICODE_HEX;
						/* 高位置的代理对 */
						if ( u1 >= 0xD800 && u1 <= 0xDBFF ) {
							if ( *tmp_pos++ != '\\' ) return SIMPLEJ_PARSE_INVALID_UNICODE_SURROGATE;
							if ( *tmp_pos++ != 'u' ) return SIMPLEJ_PARSE_INVALID_UNICODE_SURROGATE; 
							tmp_pos = simplejson_parse_hex4(tmp_pos, &u2);
							if (tmp_pos == NULL) return SIMPLEJ_PARSE_INVALID_UNICODE_HEX;
							if ( u2 < 0xDC00 || u2 > 0xdfff ) return SIMPLEJ_PARSE_INVALID_UNICODE_SURROGATE;
							/* 计算对应的码点 */
							u1 = (((u1 - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
						}
						/* 最后编码成为utf-8进行保存 */
						simplejson_encode_utf8(sj_context, u1);
						sj_context->json = tmp_pos;
						break;
					case 'b':
						PUTC(sj_context, '\b');
						break;
					case 'f':
						PUTC(sj_context, '\f');
						break;
					case 'n':
						PUTC(sj_context, '\n');
						break;
					case 'r':
						PUTC(sj_context, '\r');
						break;
					case 't':
						PUTC(sj_context, '\t');
						break;
					case '/':
						PUTC(sj_context, '/');
						break;
					case '\\':
						PUTC(sj_context, '\\');
						break;
					case '\"':
						PUTC(sj_context, '\"');
						break;
					default:
						/* 发现非法转义字符 */
						/* 将stack的top恢复 */
						sj_context->top = head;
						return SIMPLEJ_PARSE_INVALID_STRING_ESCAPE;
				}
				break;
			case '\"':
				/* 获取目前得到的字符总数 */
				len = sj_context->top - head;
				/* 设置value中的string内容 */
				/* 这里之前存在一个BUG,只有在特定的条件下才能触发 */
				/* set_simplejson_string(sj_value, simplejson_context_pop(sj_context, len), len); */
				*outStr = simplejson_context_pop(sj_context, len);
				*outLen = len;
				/* 同时更新保存的字符串的地址,为了后续字符的处理 */
				sj_context->json = tmp_str;
				return SIMPLEJ_PARSE_OK;
			case '\0':
				/* 恢复初始状态 */
				sj_context->top = head;
				return SIMPLEJ_PARSE_MISS_QUOTATION_MARK;
			default:
				/* 判断是否为非法字符 */
				/* 这里的输入对于\x10为一个字符 */
				if ((unsigned char)current_ch < 0x20) {
					sj_context->top = head;
					return SIMPLEJ_PARSE_INVALID_STRING_CHAR;
				}
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

SIMPLEJ_PARSE_RESULT simplejson_parse_object(SIMPLEJ_VALUE *sj_value, SIMPLEJ_CONTEXT *sj_context) {
	SIMPLEJ_PARSE_RESULT ret;
	size_t head = sj_context->top;
	EXPECT_CH(sj_context->json, '{');
	strip_space(sj_context);
	/* 首先解析为空的情况 */
	if (*sj_context->json == '}') {
		sj_context->json++;
		sj_value->sj_type = SIMPLEJ_OBJECT;
		return SIMPLEJ_PARSE_OK;		
	}
	for (;;) {
		char *keyStr;
		size_t keyLen;
		/* 用以保存object中的每个成员 */
		SIMPLEJ_MEMBER sj_member;
		SIMPLEJ_VALUE tmp_value;
		tmp_value.sj_type = SIMPLEJ_NULL;
		/* 处理"{1:1,"情况 */
		if (*sj_context->json == '\0') {
			ret = SIMPLEJ_PARSE_MISS_KEY;
			break;
		}
		/* 第一部分先解析key部分,并保存到member结构体中 */
		if ((ret = simplejson_parse_string_raw(sj_context, &keyStr, &keyLen)) != SIMPLEJ_PARSE_OK) {
			break;
		}	
		sj_member.key = (char *)malloc(keyLen+1);
		sj_member.key[keyLen] = '\0';
		sj_member.klen = keyLen;
		memcpy(sj_member.key, keyStr, keyLen);
		strip_space(sj_context);
		if (*sj_context->json != ':') {
			return SIMPLEJ_PARSE_MISS_COLON;
		}
		sj_context->json++;
		strip_space(sj_context);
		/* 第二部分解析value部分 */
		if ((ret = simplejson_parse_value(&tmp_value, sj_context)) != SIMPLEJ_PARSE_OK) {
			break;
		}
		/* 保存value */
		memcpy(&sj_member.val, &tmp_value, sizeof(SIMPLEJ_VALUE));
		/* 将member压栈保存 */
		PUT_SJ_MEMBER(simplejson_context_push(sj_context, sizeof(SIMPLEJ_MEMBER)), &sj_member);
		strip_space(sj_context);
		if ( *sj_context->json == '}' ) {
			sj_context->json++;
			return SIMPLEJ_PARSE_OK;
		}
		if ( *sj_context->json != ',' || *sj_context->json == '\0') {
			ret = SIMPLEJ_PARSE_MISS_COMMA_OR_CURLY_BRACKET;
			break;
		}
		sj_context->json++;
		strip_space(sj_context);
	}
	return ret;
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
		case '[': return simplejson_parse_array(sj_value, sj_context);
		case '{': return simplejson_parse_object(sj_value, sj_context);
	}	
}

/* 通过context来保存一些临时状态信息,value来保存解析出来的信息 */
/* 这样的好处是逻辑清晰,可以随时在结构体中添加新的字段满足需求 */
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
	/* 回收内存 */
	free(sj_context.stack);
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
	/* 当保存的为array时,free掉其申请的内容 */
	/* 因为这里保存的SIMPLEJ_VALUE都是值复制 */
	/* 函数这里存在内存泄漏，需要解决嵌套数组中的申请的内存回收问题 */
	if (sj_value->sj_type == SIMPLEJ_ARRAY) {
		free(sj_value->u.a.element);
	}
  sj_value->sj_type = SIMPLEJ_NULL;
}

