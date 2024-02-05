#include <stdio.h>
#include <string.h>
#include "my_leptjson.h"

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ_BASE(eauality, expect, actual, format) \
    do{ \
        test_count ++; \
        if(eauality) \
            test_pass ++; \
        else{ \
            fprintf(stderr, "%s:%d expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual); \
            main_ret = 1;\
        } \
    }while(0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")
#define EXPECT_EQ_STRING(expect, actual, alength) \
    EXPECT_EQ_BASE(sizeof(expect) - 1 == (alength) && memcmp(expect, actual, alength) == 0, expect, actual, "[%s]")

#define TEST_NUMBER(expect, actual) \
    do{ \
        lept_value v;\
        EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, actual));\
        EXPECT_EQ_INT(LEPT_NUMBER, lept_get_type(&v));\
        EXPECT_EQ_DOUBLE(expect, lept_get_number(&v));\
    }while(0)

#define TEST_STRING(expect, actual) \
    do{ \
        lept_value v;\
        EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, actual));\
        EXPECT_EQ_INT(LEPT_STRING, lept_get_type(&v));\
        EXPECT_EQ_STRING(expect, lept_get_string(&v), lept_get_string_len(&v));\
    }while(0)

void test_parse(void)
{
    lept_value v;
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "null"));
    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));

    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "true"));
    EXPECT_EQ_INT(LEPT_TRUE, lept_get_type(&v));

    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "false"));
    EXPECT_EQ_INT(LEPT_FALSE, lept_get_type(&v));

    EXPECT_EQ_INT(LEPT_PARSE_ROOT_NOT_SINGULAR, lept_parse(&v, "false ,"));
    EXPECT_EQ_INT(LEPT_FALSE, lept_get_type(&v));

    TEST_NUMBER(0.0, "0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(0.0, "-0.0");
    TEST_NUMBER(1.0, "1");
    TEST_NUMBER(-1.0, "-1");
    TEST_NUMBER(1.5, "1.5");
    TEST_NUMBER(-1.5, "-1.5");
    TEST_NUMBER(3.1416, "3.1416");
    TEST_NUMBER(1E10, "1E10");
    TEST_NUMBER(1e10, "1e10");
    TEST_NUMBER(1E+10, "1E+10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(-1E10, "-1E10");
    TEST_NUMBER(-1e10, "-1e10");
    TEST_NUMBER(-1E+10, "-1E+10");
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E+10, "1.234E+10");
    TEST_NUMBER(1.234E-10, "1.234E-10");
    TEST_NUMBER(0.0, "1e-10000"); /* must underflow */

    TEST_NUMBER(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
    TEST_NUMBER( 4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
    TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    TEST_NUMBER( 2.2250738585072009e-308, "2.2250738585072009e-308");  /* Max subnormal double */
    TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    TEST_NUMBER( 2.2250738585072014e-308, "2.2250738585072014e-308");  /* Min normal positive double */
    TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    TEST_NUMBER( 1.7976931348623157e+308, "1.7976931348623157e+308");  /* Max double */
    TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");

    /* 允许出现空格制表符等分割符号 */
    TEST_NUMBER(-1e10, "-1e10 ");
    TEST_NUMBER(-1e10, "-1e10 \r");
    TEST_NUMBER(-1e10, "-1e10 \n");
    TEST_NUMBER(-1e10, "-1e10 \t");
    
    /* 不合法数字 */
    EXPECT_EQ_INT(LEPT_PARSE_INVALID_VALUE, lept_parse(&v, "1234 qwer"));
    EXPECT_EQ_INT(LEPT_PARSE_INVALID_VALUE, lept_parse(&v, "01234 qwer"));
    EXPECT_EQ_INT(LEPT_PARSE_INVALID_VALUE, lept_parse(&v, "-13.345eqwer"));    
    
    TEST_STRING("Hello", "\"Hello\"");
    #if 1
    TEST_STRING("", "\"\"");
    TEST_STRING("Hello", "\"Hello\"");
    TEST_STRING("Hello\nWorld", "\"Hello\nWorld\"");

    TEST_STRING("", "\"\"");
    TEST_STRING("Hello", "\"Hello\"");
    TEST_STRING("Hello\\nWorld", "\"Hello\\nWorld\"");
    TEST_STRING("\\ \b \f \t \r \n", "\"\\ \b \f \t \r \n\"");
    #endif

    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
}

int main(int argc, char **argv)
{
    printf("test\n");
    
    test_parse();
    
    return 0;
}


