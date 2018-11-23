# Simple-Json-Library

主要通过[https://github.com/miloyip/json-tutorial](https://github.com/miloyip/json-tutorial)中的文档进行指引开发的

### 目标：

- 熟悉一个独立项目的构建过程
- 学习测试驱动开发(test driven development, TDD)
- API设计
- 数据结构设计

### 1. 解析true，false，null语法

	json-text = ws value ws
	ws = *(%x20 / %x09 / %x0a / %x0d)
	value = null / true / false
	null = "null"
	true = "true"
	false = "false"

其中`%xh`代表十六禁止,`/`代表或的意思,`*`代表0至多,`()`代表为一组

第一行代表由3部分组成第一部分ws,第二部分value,第三部分为ws

第二行代表什么为ws,有0至多个空格符（space U+0020）、制表符（tab U+0009）、换行符（LF U+000A）、回车符（CR U+000D）所组成

第三行表示value目前的表示为null、true、false，而剩下的内容则是表明它们都是依次通过对应的`literal`来表示

### 2. 解析number语法

	number = [ "-" ] int [ frac ] [ exp ]
	int = "0" / digit1-9 *digit
	frac = "." 1*digit
	exp = ("e" / "E") ["-" / "+"] 1*digit

number主要是由四部分组成，第一部分符号位是否为负数,缺省为整数,第二部分为int代表的数字,第三部分为小数点部分的值,第四部分代表科学计数法的表示

第二行表示int的组成为2种形式开头如果为0,那么只有一个0;另外一种形式为非零开头后面带着`0至多`个数字

第三行表示小数点部分的组成为以`.`开头并附带`1至多`的数字

第四行表示科学计数法的组成方式,第一部分为必须存在的`e`或者`E`,第二部分为可存在的`-`或者`+`,第三部分为固定的`1至多`个数字

同时添加了SIMPLEJ_PARSE_NUMBER_TOO_BIG的解析错误,主要是依靠`strtod`转换时的返回值来判断,超出范围的话会返回`HUGE_VAL`或者`-HUGE_VAL`,并且会将`errno`设置为`ERANGE`

### 3. 解析string语法

	string = quotation-mark *char quotation-mark
	char = unescaped /
	   escape (
	       %x22 /          ; "    quotation mark  U+0022
	       %x5C /          ; \    reverse solidus U+005C
	       %x2F /          ; /    solidus         U+002F
	       %x62 /          ; b    backspace       U+0008
	       %x66 /          ; f    form feed       U+000C
	       %x6E /          ; n    line feed       U+000A
	       %x72 /          ; r    carriage return U+000D
	       %x74 /          ; t    tab             U+0009
	       %x75 4HEXDIG )  ; uXXXX                U+XXXX
	escape = %x5C          ; \
	quotation-mark = %x22  ; "
	unescaped = %x20-21 / %x23-5B / %x5D-10FFFF

string是由双引号所包围的,在测试的时候传入json测试字符串时为`"\"hello\""`格式,期望解析到的为`"hello"`.

对于char的值主要分为转义和非转义的,非法的字符为小于0x20的.

还有针对与\uxxxx格式的需要先转化成为对应的码点,然后使用utf8的编码进行保存.

涉及到的数据结构,在接收解析结果的数据结构`SIMPLEJ_VALUE`中添加保存字符串的字段,这里为了节省内存,使用union添加字符串保存的字段,定义如下:

	typedef struct simplej_value {
		struct union {
			struct union { char *s; size_t len; }s;
			double number;
		}u;
		SIMPLEJ_TYPE sj_type;
	}

在解析字符串的时候,使用了类似栈的方式进行字符串每个字符的解析结果存储,思路是通过判断每个字符然后依次压栈,并更新相应的长度;当遇到`\"`时则根据长度将字符串弹出栈;通过`simplejson_context_push`将字符串保存到`SIMPLEJ_VALUE`结构体中;在`SIMPLEJ_CONTEXT`中新增了`stack`和`top`进行栈的功能实现.

新增的解析错误类型: `SIMPLEJ_PARSE_INVALID_STRING_CHAR`、`SIMPLEJ_PARSE_INVALID_MISS_QUOTATION_MARK`、`SIMPLEJ_PARSE_INVALID_ESCAPE`.

### 4. 解析JSON数组语法

JSON数组是一种复合类型的结构,语法如下:

	array = %x5B ws [ value *( ws %x2C ws value ) ] ws %x5D

包含的value为null、false、true、string、number、array等类型,每个value以`,`间隔;

首先了解我们如何去解析和保存array.这里依然使用解析string时使用的动态栈,由于array中的每个元素都是可以使用`SIMPLEJ_VALUE`来保存,因此在解析的过程中使用stack(这里使用`simplejson_context_push`来进行保存,函数的定义可以满足任何类型数据的保存,以为需要自己传入`len`这样的设计复用性很高)来依次保存解析出来的每个`SIMPLEJ_VALUE`;在保存的时候根据解析出来的`VALUE`个数再去动态创建`SIMPLEJ_VALUE`数组,这样就节省空间以及提升效率;

涉及的数据结构:

typedef struct {
	union {
		struct { SIMPLEJ_VALUE *element; size_t size;}a;
		struct { char* s; size_t len;}s;
		double number;
	}u;
}

对于`context`中的结构不需要变动即可满足需求,在解析array时使用递归的方式调用`simplejson_parse_value`来识别各种类型的值.

错误类型添加了: `SIMPLEJ_PARSE_MISS_COMMA_OR_SQUARE_BRACKET`

### 5. 解析object语法

object主要是由`{`和`}`进行包围的键值对,键为字符串,值为之前对应的各种类型,语法如下:

	member = string ws %x3A ws value
	object = %x7B ws [ member *( ws %x2C ws member ) ] ws %x7D

这里解析的思路与数组相同,也是在解析的过程以每个`member`进行解析,分为两部分先解析key的字符串,接着解析对应的value并判断是否以`:`为间隔符;

相关的数据结构:

	struct SIMPLEJ_VALUE {
		union {
			struct {SIMPLEJ_MEMBER *member; size_t size;}o;
			...	
		}u;
		...
	};
	
	stuct SIMPLEJ_MEMBER {
		char *key; size_t klen;
		SIMPLEJ_VALUE value;
	}

添加的错误类型: `SIMPLEJ_PARSE_MISS_KEY`、`SIMPLEJ_PARSE_MISS_COLON`、`SIMPLEJ_PARSE_MISS_COMMA_OR_CURLY_BRACKET`

### 中间遇到的一些问题

- 在宏中想尝试根据不同的`type`来返回不同的字符串,这时由于参数的类型不固定,导致编译不通过,必须通过强制类型转换才能保证编译通过,因为C是强类型的语言
- 在解析字符串时,最后需要使用`utf8`的编码函数将对应的码点解析成相应的十六进制然后每个字节再进行保存
- 在调试array中碰到的一个`bug`,解析`string`时,在调用`set_simplejson_string`时引用的参数地址没有符合函数的需求,导致在`test`时出现了诡异的错误
- 只有c99以上的标准才支持`size_t`的格式化字符输出`%zu`
