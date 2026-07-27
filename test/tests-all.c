#define ALL_TESTS \
  TEST(1)

#include "testing.h"

#include "cli.h"

void CountOpt(const char* value, void* cnt) {
    (void) value;
    ++*(int*)cnt;
}

TEST(1) {
    int cnt = 0;
    CliOption opt_a = { "a flag   b", &CountOpt, &cnt };
    CliOption* options[] = { &opt_a };
    CliParser parser = { 1, 0, options, NULL };

    // short option ------------------------------------------------------------
    {
        cnt = 0;
        const char* args[] = { NULL };
        TEST_EQ(CliParse(&parser, args, args), 0);
        TEST_EQ(cnt, 0);
    }
    {
        cnt = 0;
        const char* args[] = { "a" };
        TEST_EQ(CliParse(&parser, args, args + 1), 0);
        TEST_EQ(cnt, 0);
    }
    {
        cnt = 0;
        const char* args[] = { "-a" };
        TEST_EQ(CliParse(&parser, args, args + 1), 0);
        TEST_EQ(cnt, 1);
    }
    {
        cnt = 0;
        const char* args[] = { "-b" };
        TEST_EQ(CliParse(&parser, args, args + 1), 0);
        TEST_EQ(cnt, 1);
    }
    {
        cnt = 0;
        const char* args[] = { "-a", "-a" };
        TEST_EQ(CliParse(&parser, args, args + 2), 0);
        TEST_EQ(cnt, 2);
    }
    {
        cnt = 0;
        const char* args[] = { "-a", "-a", "-a" };
        TEST_EQ(CliParse(&parser, args, args + 3), 0);
        TEST_EQ(cnt, 3);
    }
    {
        cnt = 0;
        const char* args[] = { "-a", "-b" };
        TEST_EQ(CliParse(&parser, args, args + 2), 0);
        TEST_EQ(cnt, 2);
    }
    {
        cnt = 0;
        const char* args[] = { "-b", "-a" };
        TEST_EQ(CliParse(&parser, args, args + 2), 0);
        TEST_EQ(cnt, 2);
    }
    {
        cnt = 0;
        const char* args[] = { "-A" };
        TEST_NE(CliParse(&parser, args, args + 1), 0);
    }
    {
        cnt = 0;
        const char* args[] = { "-a", "-A" };
        TEST_NE(CliParse(&parser, args, args + 2), 0);
    }

    // double dash -------------------------------------------------------------
    {
        cnt = 0;
        const char* args[] = { "-a", "--" };
        TEST_EQ(CliParse(&parser, args, args + 2), 0);
        TEST_EQ(cnt, 1);
    }
    {
        cnt = 0;
        const char* args[] = { "--", "-a" };
        TEST_EQ(CliParse(&parser, args, args + 2), 0);
        TEST_EQ(cnt, 0);
    }
    {
        cnt = 0;
        const char* args[] = { "--", "-a", "--", "-a" };
        TEST_EQ(CliParse(&parser, args, args + 4), 0);
        TEST_EQ(cnt, 1);
    }
}
