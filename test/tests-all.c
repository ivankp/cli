#define ALL_TESTS \
  TEST(match_name) \
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

TEST(match_name) {
    {
        const char* param_opt[] = {
            "cat", " cat", "   cat", "cat ", "cat  ",
            "dog cat", "dog  cat", "dog    cat", "dog cat ", "dog cat  ",
            "cat dog", " cat dog", "   cat dog", "cat  dog", "cat   dog"
        };
        const char* param_bad_arg[] = {
            "--bat", "--ca", "--catt", "--ccat"
        };
        for (unsigned i = 0; i < LEN(param_opt); ++i) {
            int cnt = 0;
            CliOption opt = { param_opt[i], &CountOpt, &cnt, NULL };
            CliOption* options[] = { &opt };
            CliCommand command = { NULL, LEN(options), 0, options, NULL, NULL, NULL, NULL };

            {
                const char* args[] = { "--cat" };
                TEST_TRUE(CliParse(&command, args, LEN(args)) == 0);
                TEST_TRUE(cnt == 1);
            }
            for (unsigned j = 0; j < LEN(param_bad_arg); ++j) {
                cnt = 0;
                const char* args[] = { param_bad_arg[j] };
                TEST_TRUE(CliParse(&command, args, LEN(args)) == 1);
                TEST_TRUE(cnt == 0);
            }
        }
    }
    {
        const char* param_opt[] = {
            "c", " c", "c ", " c ",
            "c cat", "  c cat", "  c  cat",
            "cat c", "cat c ", "cat f c", "cat c frog"
        };
        const char* param_bad_arg[] = {
            "-b", "-C", "-d"
        };
        for (unsigned i = 0; i < LEN(param_opt); ++i) {
            int cnt = 0;
            CliOption opt = { param_opt[i], &CountOpt, &cnt, NULL };
            CliOption* options[] = { &opt };
            CliCommand command = { NULL, LEN(options), 0, options, NULL, NULL, NULL, NULL };

            {
                const char* args[] = { "-c" };
                TEST_TRUE(CliParse(&command, args, LEN(args)) == 0);
                TEST_TRUE(cnt == 1);
            }
            for (unsigned j = 0; j < LEN(param_bad_arg); ++j) {
                cnt = 0;
                const char* args[] = { param_bad_arg[j] };
                TEST_TRUE(CliParse(&command, args, LEN(args)) == 1);
                TEST_TRUE(cnt == 0);
            }
        }
    }
}

TEST(short_opt) {
    int cnt = 0;
    CliOption opt_a = { "a flag   b", &CountOpt, &cnt, NULL };
    CliOption opt_2 = { "aa bb ab ba", &DoNothing, NULL, NULL };
    CliOption* options[] = { &opt_a, &opt_2 };
    CliCommand command = { NULL, LEN(options), 0, options, NULL, NULL, NULL, NULL };

    // short option ------------------------------------------------------------
    {
        cnt = 0;
        const char* args[] = { NULL };
        TEST_TRUE(CliParse(&command, args, 0) == 0);
        TEST_TRUE(cnt == 0);
    }
    {
        cnt = 0;
        const char* args[] = { "a" };
        TEST_TRUE(CliParse(&command, args, LEN(args)) == 0);
        TEST_TRUE(cnt == 0);
    }
    {
        cnt = 0;
        const char* args[] = { "-a" };
        TEST_TRUE(CliParse(&command, args, LEN(args)) == 0);
        TEST_TRUE(cnt == 1);
    }
    {
        cnt = 0;
        const char* args[] = { "-b" };
        TEST_TRUE(CliParse(&command, args, LEN(args)) == 0);
        TEST_TRUE(cnt == 1);
    }
    {
        cnt = 0;
        const char* args[] = { "-a", "-a" };
        TEST_TRUE(CliParse(&command, args, LEN(args)) == 0);
        TEST_TRUE(cnt == 2);
    }
    {
        cnt = 0;
        const char* args[] = { "-a", "-a", "-a" };
        TEST_TRUE(CliParse(&command, args, LEN(args)) == 0);
        TEST_TRUE(cnt == 3);
    }
    {
        cnt = 0;
        const char* args[] = { "-a", "-b" };
        TEST_TRUE(CliParse(&command, args, LEN(args)) == 0);
        TEST_TRUE(cnt == 2);
    }
    {
        cnt = 0;
        const char* args[] = { "-b", "-a" };
        TEST_TRUE(CliParse(&command, args, LEN(args)) == 0);
        TEST_TRUE(cnt == 2);
    }
    {
        cnt = 0;
        const char* args[] = { "-A" };
        TEST_TRUE(CliParse(&command, args, LEN(args)) > 0);
    }
    {
        cnt = 0;
        const char* args[] = { "-a", "-A" };
        TEST_TRUE(CliParse(&command, args, LEN(args)) > 0);
    }

    // double dash -------------------------------------------------------------
    {
        cnt = 0;
        const char* args[] = { "-a", "--" };
        TEST_TRUE(CliParse(&command, args, LEN(args)) == 0);
        TEST_TRUE(cnt == 1);
    }
    {
        cnt = 0;
        const char* args[] = { "--", "-a" };
        TEST_TRUE(CliParse(&command, args, LEN(args)) == 0);
        TEST_TRUE(cnt == 0);
    }
    {
        cnt = 0;
        const char* args[] = { "--", "-a", "--", "-a" };
        TEST_TRUE(CliParse(&command, args, LEN(args)) == 0);
        TEST_TRUE(cnt == 1);
    }
}

TEST(long_opt) {
    int cntA = 0;
    int cntB = 0;
    CliOption opt_a = { "a opt-a", &CountOpt, &cntA, NULL };
    CliOption opt_b = { "opt-b b", &CountOpt, &cntB, NULL };
    CliOption* options[] = { &opt_a, &opt_b };
    CliCommand command = { NULL, LEN(options), 0, options, NULL, NULL, NULL, NULL };

    // short option ------------------------------------------------------------
    {
        cntA = 0;
        cntB = 0;
        const char* args[] = { "-a" };
        TEST_TRUE(CliParse(&command, args, LEN(args)) == 0);
        TEST_TRUE(cntA == 1);
        TEST_TRUE(cntB == 0);
    }
    {
        cntA = 0;
        cntB = 0;
        const char* args[] = { "-b" };
        TEST_TRUE(CliParse(&command, args, LEN(args)) == 0);
        TEST_TRUE(cntA == 0);
        TEST_TRUE(cntB == 1);
    }

    // long option -------------------------------------------------------------
    {
        cntA = 0;
        cntB = 0;
        const char* args[] = { "--opt-a" };
        TEST_TRUE(CliParse(&command, args, LEN(args)) == 0);
        TEST_TRUE(cntA == 1);
        TEST_TRUE(cntB == 0);
    }
    {
        cntA = 0;
        cntB = 0;
        const char* args[] = { "--opt-b" };
        TEST_TRUE(CliParse(&command, args, LEN(args)) == 0);
        TEST_TRUE(cntA == 0);
        TEST_TRUE(cntB == 1);
    }
    {
        cntA = 0;
        cntB = 0;
        const char* args[] = { "-a", "--opt-b", "-b" };
        TEST_TRUE(CliParse(&command, args, LEN(args)) == 0);
        TEST_TRUE(cntA == 1);
        TEST_TRUE(cntB == 2);
    }
    {
        cntA = 0;
        cntB = 0;
        const char* args[] = { "--opt-A" };
        TEST_TRUE(CliParse(&command, args, LEN(args)) > 0);
    }
    {
        cntA = 0;
        cntB = 0;
        const char* args[] = { "--opt-a " };
        TEST_TRUE(CliParse(&command, args, LEN(args)) > 0);
    }
}

void SetString(const char* value, void* strPtr) {
    *(const char**)strPtr = value;
}

TEST(explicit_value) {
    const char* value = "default";
    CliOption opt = { "a opt-a", &SetString, &value, NULL };
    CliOption* options[] = { &opt };
    CliCommand command = { NULL, LEN(options), 0, options, NULL, NULL, NULL, NULL };

    {
        const char* args[] = { NULL };
        TEST_TRUE(CliParse(&command, args, 0) == 0);
        TEST_TRUE(strcmp(value, "default") == 0);
    }
    {
        const char* args[] = { "-a" };
        TEST_TRUE(CliParse(&command, args, LEN(args)) == 0);
        TEST_TRUE(value == NULL);
    }
    {
        const char* args[] = { "-anew" };
        TEST_TRUE(CliParse(&command, args, LEN(args)) == 0);
        TEST_TRUE(strcmp(value, "new") == 0);
    }
    {
        const char* args[] = { "--opt-a" };
        TEST_TRUE(CliParse(&command, args, LEN(args)) == 0);
        TEST_TRUE(value == NULL);
    }
    {
        const char* args[] = { "--opt-a=" };
        TEST_TRUE(CliParse(&command, args, LEN(args)) == 0);
        TEST_TRUE(strcmp(value, "") == 0);
    }
    {
        const char* args[] = { "--opt-a=new" };
        TEST_TRUE(CliParse(&command, args, LEN(args)) == 0);
        TEST_TRUE(strcmp(value, "new") == 0);
    }
}
