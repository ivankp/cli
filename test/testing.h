#include <stdlib.h>
#include <stdio.h>

#define STR(X) STR1(X)
#define STR1(X) #X

#define LEN(ARR) (sizeof((ARR)) / sizeof(*(ARR)))

//------------------------------------------------------------------------------

#define TEST_PREFIX \
    "\033[31mFAILED " __FILE__ ":" STR(__LINE__) "\033[0m\n"

#define TEST_TRUE(X) \
    if (!(X)) { \
        puts(TEST_PREFIX "\033[31mFALSE:\033[0m " #X); \
        exit(1); \
    }

#define TEST_FALSE(X) \
    if ((X)) { \
        puts(TEST_PREFIX "\033[31mTRUE:\033[0m " #X); \
        exit(1); \
    }

#define TEST_FAIL \
    { \
        puts(TEST_PREFIX); \
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
