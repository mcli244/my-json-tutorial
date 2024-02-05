#include "my_leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL */
#include <errno.h>   /* errno, ERANGE */
#include <math.h>    /* HUGE_VAL */

#define ISDIGIT(ch) ((ch) > '0' && (ch) < '9')
#define ISDIGIT1TO9(ch) ((ch) > '1' && (ch) < '9')

typedef struct
{
    const char *json;
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

lept_type lept_get_type(const lept_value *v)
{
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->number;
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
    int i = 0;
    for(i = 0; literal[i]; i++) /* 确保传入的是字符串，字符末尾有'\0'存在 (C89 注释方式)*/ 
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

    tmp = strtod(c->json, &p);
    if (errno == ERANGE && (tmp == HUGE_VAL || tmp == -HUGE_VAL))   /* HUGE_VAL为 double类型的最大值*/
        return LEPT_PARSE_NUMBER_TOO_BIG;

    if(*p != '\0')
        return LEPT_PARSE_INVALID_VALUE;

    v->number = tmp;
    v->type = LEPT_NUMBER;
    c->json = p;
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
    case '"':   break;
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

    lept_whitespace_skip(&c);
    if((ret = lept_parse_value(&c,v)) == LEPT_PARSE_OK)
    {
        lept_whitespace_skip(&c);
        if(*c.json != '\0')
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
    }   
    return ret;
}

