#define ALL_TESTS \
  TEST(short_opt) \
  TEST(long_opt) \
  TEST(explicit_value)

#include "testing.h"
#include "cli.h"
#include "string.h"

void DoNothing(const char* value, void* data) {
    (void) value;
    (void) data;
}

void CountOpt(const char* value, void* cnt) {
    (void) value;
    ++*(int*)cnt;
}

TEST(short_opt) {
    int cnt = 0;
    CliOption opt_a = { "a flag   b", &CountOpt, &cnt, NULL };
    CliOption opt_2 = { "aa bb ab ba", &DoNothing, NULL, NULL };
    CliOption* options[] = { &opt_a, &opt_2 };
    CliParser parser = { NULL, LEN(options), 0, options, NULL, NULL, NULL, NULL };

    // short option ------------------------------------------------------------
    {
        cnt = 0;
        const char* args[] = { NULL };
        TEST_OP(CliParse(&parser, args, 0), ==, 0);
        TEST_OP(cnt, ==, 0);
    }
    {
        cnt = 0;
        const char* args[] = { "a" };
        TEST_OP(CliParse(&parser, args, LEN(args)), ==, 0);
        TEST_OP(cnt, ==, 0);
    }
    {
        cnt = 0;
        const char* args[] = { "-a" };
        TEST_OP(CliParse(&parser, args, LEN(args)), ==, 0);
        TEST_OP(cnt, ==, 1);
    }
    {
        cnt = 0;
        const char* args[] = { "-b" };
        TEST_OP(CliParse(&parser, args, LEN(args)), ==, 0);
        TEST_OP(cnt, ==, 1);
    }
    {
        cnt = 0;
        const char* args[] = { "-a", "-a" };
        TEST_OP(CliParse(&parser, args, LEN(args)), ==, 0);
        TEST_OP(cnt, ==, 2);
    }
    {
        cnt = 0;
        const char* args[] = { "-a", "-a", "-a" };
        TEST_OP(CliParse(&parser, args, LEN(args)), ==, 0);
        TEST_OP(cnt, ==, 3);
    }
    {
        cnt = 0;
        const char* args[] = { "-a", "-b" };
        TEST_OP(CliParse(&parser, args, LEN(args)), ==, 0);
        TEST_OP(cnt, ==, 2);
    }
    {
        cnt = 0;
        const char* args[] = { "-b", "-a" };
        TEST_OP(CliParse(&parser, args, LEN(args)), ==, 0);
        TEST_OP(cnt, ==, 2);
    }
    {
        cnt = 0;
        const char* args[] = { "-A" };
        TEST_OP(CliParse(&parser, args, LEN(args)), >, 0);
    }
    {
        cnt = 0;
        const char* args[] = { "-a", "-A" };
        TEST_OP(CliParse(&parser, args, LEN(args)), >, 0);
    }

    // double dash -------------------------------------------------------------
    {
        cnt = 0;
        const char* args[] = { "-a", "--" };
        TEST_OP(CliParse(&parser, args, LEN(args)), ==, 0);
        TEST_OP(cnt, ==, 1);
    }
    {
        cnt = 0;
        const char* args[] = { "--", "-a" };
        TEST_OP(CliParse(&parser, args, LEN(args)), ==, 0);
        TEST_OP(cnt, ==, 0);
    }
    {
        cnt = 0;
        const char* args[] = { "--", "-a", "--", "-a" };
        TEST_OP(CliParse(&parser, args, LEN(args)), ==, 0);
        TEST_OP(cnt, ==, 1);
    }
}

TEST(long_opt) {
    int cntA = 0;
    int cntB = 0;
    CliOption opt_a = { "a opt-a", &CountOpt, &cntA, NULL };
    CliOption opt_b = { "opt-b b", &CountOpt, &cntB, NULL };
    CliOption* options[] = { &opt_a, &opt_b };
    CliParser parser = { NULL, LEN(options), 0, options, NULL, NULL, NULL, NULL };

    // short option ------------------------------------------------------------
    {
        cntA = 0;
        cntB = 0;
        const char* args[] = { "-a" };
        TEST_OP(CliParse(&parser, args, LEN(args)), ==, 0);
        TEST_OP(cntA, ==, 1);
        TEST_OP(cntB, ==, 0);
    }
    {
        cntA = 0;
        cntB = 0;
        const char* args[] = { "-b" };
        TEST_OP(CliParse(&parser, args, LEN(args)), ==, 0);
        TEST_OP(cntA, ==, 0);
        TEST_OP(cntB, ==, 1);
    }

    // long option -------------------------------------------------------------
    {
        cntA = 0;
        cntB = 0;
        const char* args[] = { "--opt-a" };
        TEST_OP(CliParse(&parser, args, LEN(args)), ==, 0);
        TEST_OP(cntA, ==, 1);
        TEST_OP(cntB, ==, 0);
    }
    {
        cntA = 0;
        cntB = 0;
        const char* args[] = { "--opt-b" };
        TEST_OP(CliParse(&parser, args, LEN(args)), ==, 0);
        TEST_OP(cntA, ==, 0);
        TEST_OP(cntB, ==, 1);
    }
    {
        cntA = 0;
        cntB = 0;
        const char* args[] = { "-a", "--opt-b", "-b" };
        TEST_OP(CliParse(&parser, args, LEN(args)), ==, 0);
        TEST_OP(cntA, ==, 1);
        TEST_OP(cntB, ==, 2);
    }
    {
        cntA = 0;
        cntB = 0;
        const char* args[] = { "--opt-A" };
        TEST_OP(CliParse(&parser, args, LEN(args)), >, 0);
    }
    {
        cntA = 0;
        cntB = 0;
        const char* args[] = { "--opt-a " };
        TEST_OP(CliParse(&parser, args, LEN(args)), >, 0);
    }
}

void SetString(const char* value, void* strPtr) {
    *(const char**)strPtr = value;
}

TEST(explicit_value) {
    const char* value = "default";
    CliOption opt = { "a opt-a", &SetString, &value, NULL };
    CliOption* options[] = { &opt };
    CliParser parser = { NULL, LEN(options), 0, options, NULL, NULL, NULL, NULL };

    {
        const char* args[] = { NULL };
        TEST_OP(CliParse(&parser, args, 0), ==, 0);
        TEST_OP(strcmp(value, "default"), ==, 0);
    }
    {
        const char* args[] = { "-a" };
        TEST_OP(CliParse(&parser, args, LEN(args)), ==, 0);
        TEST_OP(value, ==, NULL);
    }
    {
        const char* args[] = { "-anew" };
        TEST_OP(CliParse(&parser, args, LEN(args)), ==, 0);
        TEST_OP(strcmp(value, "new"), ==, 0);
    }
    {
        const char* args[] = { "--opt-a" };
        TEST_OP(CliParse(&parser, args, LEN(args)), ==, 0);
        TEST_OP(value, ==, NULL);
    }
    {
        const char* args[] = { "--opt-a=" };
        TEST_OP(CliParse(&parser, args, LEN(args)), ==, 0);
        TEST_OP(strcmp(value, ""), ==, 0);
    }
    {
        const char* args[] = { "--opt-a=new" };
        TEST_OP(CliParse(&parser, args, LEN(args)), ==, 0);
        TEST_OP(strcmp(value, "new"), ==, 0);
    }
}
