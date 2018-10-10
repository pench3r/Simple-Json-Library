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

### 中间遇到的一些问题

- 在宏中想尝试根据不同的type来返回不同的字符串,这时由于参数的类型不固定,导致编译不通过,必须通过强制类型转换才能保证编译通过,因为C是强类型的语言
