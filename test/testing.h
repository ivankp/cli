#include <stdlib.h>
#include <stdio.h>

#define STR(X) STR1(X)
#define STR1(X) #X

#define LEN(ARR) (sizeof((ARR)) / sizeof(*(ARR)))

//------------------------------------------------------------------------------

#define TEST_EQ(LHS, RHS) \
    if (!( (LHS) == (RHS) )) { \
        puts("\033[31m" __FILE__ ":" STR(__LINE__) ": " #LHS " != " #RHS "\033[0m"); \
        exit(1); \
    }

#define TEST_NE(LHS, RHS) \
    if (!( (LHS) != (RHS) )) { \
        puts("\033[31m" __FILE__ ":" STR(__LINE__) ": " #LHS " == " #RHS "\033[0m"); \
        exit(1); \
    }

#define TEST_TRUE(X) \
    if (!(X)) { \
        puts("\033[31m" __FILE__ ":" STR(__LINE__) ": " #X " is false" "\033[0m"); \
        exit(1); \
    }

#define TEST_FALSE(X) \
    if ((X)) { \
        puts("\033[31m" __FILE__ ":" STR(__LINE__) ": " #X " is true" "\033[0m"); \
        exit(1); \
    }

#define TEST_FAIL \
    { \
        puts("\033[31m" __FILE__ ":" STR(__LINE__) ": test failed" "\033[0m"); \
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
