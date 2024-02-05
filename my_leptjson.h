#ifndef __MY_LEPTJSON_H__
#define __MY_LEPTJSON_H__

typedef enum { LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT } lept_type;

typedef struct 
{
    double number;
    lept_type type;
}lept_value;

enum {
    LEPT_PARSE_OK = 0,
    LEPT_PARSE_EXPECT_VALUE,
    LEPT_PARSE_INVALID_VALUE,
    LEPT_PARSE_ROOT_NOT_SINGULAR,
    LEPT_PARSE_NUMBER_TOO_BIG
};

lept_type lept_get_type(const lept_value *v);
int lept_parse(lept_value *v, const char *json);
double lept_get_number(const lept_value* v);

#endif /*__MY_LEPTJSON_H__*/