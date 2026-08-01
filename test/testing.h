#include <stdlib.h>
#include <stdio.h>

#define STR(X) STR1(X)
#define STR1(X) #X

#define LEN(ARR) (sizeof((ARR)) / sizeof(*(ARR)))

//------------------------------------------------------------------------------

#define TEST_PREFIX \
    "\033[31m" __FILE__ ":" STR(__LINE__) ": "

#define TEST_OP(LHS, OP, RHS) \
    if (!( (LHS) OP (RHS) )) { \
        puts(TEST_PREFIX "FAILED\033[0m\n" #LHS " " STR(OP) " " #RHS); \
        exit(1); \
    }

#define TEST_TRUE(X) \
    if (!(X)) { \
        puts(TEST_PREFIX #X " is false" "\033[0m"); \
        exit(1); \
    }

#define TEST_FALSE(X) \
    if ((X)) { \
        puts(TEST_PREFIX #X " is true" "\033[0m"); \
        exit(1); \
    }

#define TEST_FAIL \
    { \
        puts(TEST_PREFIX "test failed" "\033[0m"); \
        exit(1); \
    }

//------------------------------------------------------------------------------

#define TEST(NAME) void TEST_##NAME##_(void);
ALL_TESTS
#undef TEST

int main(void) {
#define TEST(NAME) TEST_##NAME##_();
    ALL_TESTS
#undef TEST
    puts("\033[32m" "PASSED" "\033[0m");
}

#define TEST(NAME) void TEST_##NAME##_(void)
