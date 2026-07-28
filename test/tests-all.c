#define ALL_TESTS \
  TEST(1) \
  TEST(2) \
  TEST(3)

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

TEST(1) {
    int cnt = 0;
    CliOption opt_a = { "a flag   b", &CountOpt, &cnt };
    CliOption opt_2 = { "aa bb ab ba", &DoNothing, NULL };
    CliOption* options[] = { &opt_a, &opt_2 };
    CliParser parser = { LEN(options), 0, options, NULL };

    // short option ------------------------------------------------------------
    {
        cnt = 0;
        const char* args[] = { NULL };
        TEST_EQ(CliParse(&parser, args, 0), 0);
        TEST_EQ(cnt, 0);
    }
    {
        cnt = 0;
        const char* args[] = { "a" };
        TEST_EQ(CliParse(&parser, args, LEN(args)), 0);
        TEST_EQ(cnt, 0);
    }
    {
        cnt = 0;
        const char* args[] = { "-a" };
        TEST_EQ(CliParse(&parser, args, LEN(args)), 0);
        TEST_EQ(cnt, 1);
    }
    {
        cnt = 0;
        const char* args[] = { "-b" };
        TEST_EQ(CliParse(&parser, args, LEN(args)), 0);
        TEST_EQ(cnt, 1);
    }
    {
        cnt = 0;
        const char* args[] = { "-a", "-a" };
        TEST_EQ(CliParse(&parser, args, LEN(args)), 0);
        TEST_EQ(cnt, 2);
    }
    {
        cnt = 0;
        const char* args[] = { "-a", "-a", "-a" };
        TEST_EQ(CliParse(&parser, args, LEN(args)), 0);
        TEST_EQ(cnt, 3);
    }
    {
        cnt = 0;
        const char* args[] = { "-a", "-b" };
        TEST_EQ(CliParse(&parser, args, LEN(args)), 0);
        TEST_EQ(cnt, 2);
    }
    {
        cnt = 0;
        const char* args[] = { "-b", "-a" };
        TEST_EQ(CliParse(&parser, args, LEN(args)), 0);
        TEST_EQ(cnt, 2);
    }
    {
        cnt = 0;
        const char* args[] = { "-A" };
        TEST_NE(CliParse(&parser, args, LEN(args)), 0);
    }
    {
        cnt = 0;
        const char* args[] = { "-a", "-A" };
        TEST_NE(CliParse(&parser, args, LEN(args)), 0);
    }

    // double dash -------------------------------------------------------------
    {
        cnt = 0;
        const char* args[] = { "-a", "--" };
        TEST_EQ(CliParse(&parser, args, LEN(args)), 0);
        TEST_EQ(cnt, 1);
    }
    {
        cnt = 0;
        const char* args[] = { "--", "-a" };
        TEST_EQ(CliParse(&parser, args, LEN(args)), 0);
        TEST_EQ(cnt, 0);
    }
    {
        cnt = 0;
        const char* args[] = { "--", "-a", "--", "-a" };
        TEST_EQ(CliParse(&parser, args, LEN(args)), 0);
        TEST_EQ(cnt, 1);
    }
}

TEST(2) {
    int cntA = 0;
    int cntB = 0;
    CliOption opt_a = { "a opt-a", &CountOpt, &cntA };
    CliOption opt_b = { "opt-b b", &CountOpt, &cntB };
    CliOption* options[] = { &opt_a, &opt_b };
    CliParser parser = { LEN(options), 0, options, NULL };

    // short option ------------------------------------------------------------
    {
        cntA = 0;
        cntB = 0;
        const char* args[] = { "-a" };
        TEST_EQ(CliParse(&parser, args, LEN(args)), 0);
        TEST_EQ(cntA, 1);
        TEST_EQ(cntB, 0);
    }
    {
        cntA = 0;
        cntB = 0;
        const char* args[] = { "-b" };
        TEST_EQ(CliParse(&parser, args, LEN(args)), 0);
        TEST_EQ(cntA, 0);
        TEST_EQ(cntB, 1);
    }

    // long option -------------------------------------------------------------
    {
        cntA = 0;
        cntB = 0;
        const char* args[] = { "--opt-a" };
        TEST_EQ(CliParse(&parser, args, LEN(args)), 0);
        TEST_EQ(cntA, 1);
        TEST_EQ(cntB, 0);
    }
    {
        cntA = 0;
        cntB = 0;
        const char* args[] = { "--opt-b" };
        TEST_EQ(CliParse(&parser, args, LEN(args)), 0);
        TEST_EQ(cntA, 0);
        TEST_EQ(cntB, 1);
    }
    {
        cntA = 0;
        cntB = 0;
        const char* args[] = { "-a", "--opt-b", "-b" };
        TEST_EQ(CliParse(&parser, args, LEN(args)), 0);
        TEST_EQ(cntA, 1);
        TEST_EQ(cntB, 2);
    }
    {
        cntA = 0;
        cntB = 0;
        const char* args[] = { "--opt-A" };
        TEST_NE(CliParse(&parser, args, LEN(args)), 0);
    }
    {
        cntA = 0;
        cntB = 0;
        const char* args[] = { "--opt-a " };
        TEST_NE(CliParse(&parser, args, LEN(args)), 0);
    }
}

void SetString(const char* value, void* strPtr) {
    *(const char**)strPtr = value;
}

TEST(3) {
    const char* value = "default";
    CliOption opt = { "a opt-a", &SetString, &value };
    CliOption* options[] = { &opt };
    CliParser parser = { LEN(options), 0, options, NULL };

    {
        const char* args[] = { NULL };
        TEST_EQ(CliParse(&parser, args, 0), 0);
        TEST_EQ(strcmp(value, "default"), 0);
    }
    {
        const char* args[] = { "-a" };
        TEST_EQ(CliParse(&parser, args, LEN(args)), 0);
        TEST_EQ(value, NULL);
    }
    {
        const char* args[] = { "-anew" };
        TEST_EQ(CliParse(&parser, args, LEN(args)), 0);
        TEST_EQ(strcmp(value, "new"), 0);
    }
    {
        const char* args[] = { "--opt-a" };
        TEST_EQ(CliParse(&parser, args, LEN(args)), 0);
        TEST_EQ(value, NULL);
    }
    {
        const char* args[] = { "--opt-a=" };
        TEST_EQ(CliParse(&parser, args, LEN(args)), 0);
        TEST_EQ(strcmp(value, ""), 0);
    }
    {
        const char* args[] = { "--opt-a=new" };
        TEST_EQ(CliParse(&parser, args, LEN(args)), 0);
        TEST_EQ(strcmp(value, "new"), 0);
    }
}
