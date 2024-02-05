#include "my_leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL */
#include <errno.h>   /* errno, ERANGE */
#include <math.h>    /* HUGE_VAL */
#include <string.h> /* memcpy*/
#include <stdio.h>

#ifndef LEPT_PARSE_STACK_INIT_SIZE
#define LEPT_PARSE_STACK_INIT_SIZE 256
#endif

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT(ch) ((ch) > '0' && (ch) < '9')
#define ISDIGIT1TO9(ch) ((ch) > '1' && (ch) < '9')

typedef struct
{
    const char *json;

    struct{
        char *pbuf;
        size_t size, top;
    }stack;
    
}lept_context;

/*
{
  "name": "John",
  "age": 30,
  "city": "New York",
  "friends": ["Tom", "Lisa", "Mike"],
  "isMarried": false,
  "address": {
    "street": "123 Street",
    "city": "New York",
    "zipcode": "12345"
  }
}
*/

/**
 * @brief 压栈操作
 * 
 * @note 压栈动作，一次压入一个数据，当栈空间不足时，自动按1.5倍扩充
 * 
 * @param [in] c	栈操作对象
 * @param [in] ch	待压入的数据
 * @return int 
 */
int lept_context_stack_push(lept_context *c, const char ch)
{
    if(c->stack.top + sizeof(char) > c->stack.size)
    {
        /* 重新申请内存 */
        if(0 == c->stack.size)
            c->stack.size = LEPT_PARSE_STACK_INIT_SIZE;
        while(c->stack.top + sizeof(char) > c->stack.size)
            c->stack.size += c->stack.size >> 1;  /* 扩大1.5倍 PS：让我想起了TCP中的超时重试时间为2倍增加 kcp项目中 将其设置为1.5倍 */
        
        c->stack.pbuf = realloc(c->stack.pbuf, c->stack.size); /* realooc 会拷贝之前的数据到新的内存中 */
        if(!c->stack.pbuf)
            return LEPT_PARSE_NOMEM;
    }

    c->stack.pbuf[c->stack.top++] = ch;
    
    return LEPT_PARSE_OK;
}

/**
 * @brief 出栈操作
 * 
 * @note 出栈动作，一次出一个数据，当栈空时，返回错误码
 * 
 * @param [in] c	栈操作对象
 * @param [in] ch	待出栈的数据
 * @return int
 */
int lept_context_stack_pop(lept_context *c, char *dat)
{
    if(c->stack.top == 0)
        *dat = c->stack.pbuf[0];

    *dat = c->stack.pbuf[c->stack.top--];
    
    return LEPT_PARSE_OK;
}

int lept_context_stack_top_set(lept_context *c, int top)
{
    if(top > c->stack.size)
        return LEPT_PARSE_NOMEM;
    
    c->stack.top = top;

    return LEPT_PARSE_OK;
}

char * lept_context_stack_pbuf_get(lept_context *c)
{
    return c->stack.pbuf;
}

int lept_context_stack_top_get(lept_context *c)
{
    return c->stack.top;
}

lept_type lept_get_type(const lept_value *v)
{
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->data.number;
}

char* lept_get_string(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_STRING);
    return v->data.string.dat;
}

int lept_get_string_len(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_STRING);
    return v->data.string.len;
}

void lept_whitespace_skip(lept_context *c)
{
    const char *p = c->json;
    assert(c != NULL);

    while(*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
        p++;
    
    c->json = p;
}

int lept_parse_literal(lept_context *c, lept_value *v, const char *literal, lept_type type)
{
    size_t i = 0;
    for(i = 0; literal[i]; i++) /* 确保传入的是字符串，字符末尾有'\0'存在 (C89 注释方式) PS:如果传入一个异常指针 这里直接访问会造成程序崩溃*/ 
        if(c->json[i] != literal[i])
            return LEPT_PARSE_INVALID_VALUE;
    
    c->json += i;   /* 解析完了 就把指针执行字符串末 */
    v->type = type;
    return LEPT_PARSE_OK;
}



int lept_parse_number(lept_context *c, lept_value *v)
{
    char *p;
    double tmp;
    errno = 0;

    /* 与原项目不同，个人认为这里直接使用strtod去做转换即可，不需要去判断是否有符号 是否满足条件 
        strtod内部带有空格跳过，转换遇到非数字时，将指针传出，可以在后面判断是否到了'\0' 效率高一点
    */
    tmp = strtod(c->json, &p);
    if (errno == ERANGE && (tmp == HUGE_VAL || tmp == -HUGE_VAL))   /* HUGE_VAL为 double类型的最大值*/
        return LEPT_PARSE_NUMBER_TOO_BIG;
    
    c->json = p;
    lept_whitespace_skip(c);
    if(*c->json != '\0')
        return LEPT_PARSE_INVALID_VALUE;

    v->data.number = tmp;
    v->type = LEPT_NUMBER;
    
    return LEPT_PARSE_OK;
}

int lept_parse_string(lept_context *c, lept_value *v)
{
    /* \"string\" */
    int cnt = 0;
    char dat;
    const char *p = c->json;

    if(*p++ != '\"')
        return LEPT_PARSE_MISS_QUOTATION_MARK;

    while(*p != '\"'){
        if(*p == '\0')
            return LEPT_PARSE_MISS_QUOTATION_MARK;
        else
            lept_context_stack_push(c, *p);
        p++;
    }

    /* TODO: 释放内存 */

    v->data.string.dat = (char *)malloc(lept_context_stack_top_get(c) + sizeof(char));
    v->data.string.len = lept_context_stack_top_get(c);

    if(v->data.string.len)
    {
        cnt = v->data.string.len;
        while(cnt >= 0)
        {
            lept_context_stack_pop(c, &dat); /* TODO: 出错处理 */
            v->data.string.dat[cnt--] = dat;
        }
    }

    v->data.string.dat[v->data.string.len] = '\0';
    v->type = LEPT_STRING;
    c->json = p + 1;
    
    return LEPT_PARSE_OK;
}

int lept_parse_value(lept_context *c, lept_value *v)
{
    const char *p = c->json;

    int ret = LEPT_PARSE_INVALID_VALUE;
    switch (*p)
    {
    case 'n':  ret = lept_parse_literal(c, v, "null", LEPT_NULL); break;
    case 't':  ret = lept_parse_literal(c, v, "true", LEPT_TRUE); break;
    case 'f':  ret = lept_parse_literal(c, v, "false", LEPT_FALSE);  break;
    case '"': ret= lept_parse_string(c, v);  break;
    case '[':   break;
    case '{':   break;
    case '\0': ret = LEPT_PARSE_EXPECT_VALUE; break;
    default:    ret = lept_parse_number(c, v); break;
    }
    return ret;
}

/**
 * @brief 解析json字符串
 * 
 * @param [out] v	解析后的数据
 * @param [in] json	待解析的json字符串
 * @return int 
 */
int lept_parse(lept_value *v, const char *json)
{
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    c.stack.pbuf = NULL;
    c.stack.top = 0;
    c.stack.size = 0;

    lept_whitespace_skip(&c);
    if((ret = lept_parse_value(&c,v)) == LEPT_PARSE_OK)
    {
        lept_whitespace_skip(&c);
        if(*c.json != '\0')
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
    }   
    return ret;
}

