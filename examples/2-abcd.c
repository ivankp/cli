#include "cli.h"
#include <stdio.h>
#include <string.h>

#define LEN(ARR) (sizeof((ARR)) / sizeof(*(ARR)))

void Echo(const char* value, void* data) {
  printf("%s: %s\n", (const char*)data, value);
}

int main(int argc, const char* const* argv) {
    CliOption aOpt = { "a", &Echo, "opt a", "" };
    CliOption bOpt = { "b", &Echo, "opt b", "" };
    CliOption* abOpts[] = { &aOpt, &bOpt };

    CliCommand abCmd = {
        "ab", LEN(abOpts), 0, abOpts, NULL,
        "First subcommand", "", NULL
    };

    CliOption cOpt = { "c", &Echo, "opt c", "" };
    CliOption dOpt = { "d", &Echo, "opt d", "" };
    CliOption* cdOpts[] = { &cOpt, &dOpt };

    CliCommand cdCmd = {
        "cd CD", LEN(cdOpts), 0, cdOpts, NULL,
        "Second subcommand", "", NULL
    };

    CliCommand* cmds[] = { &abCmd, &cdCmd };

    CliOption xOpt = { "x", &Echo, "opt x", "" };
    CliOption yOpt = { "y", &Echo, "opt y", "" };
    CliOption* opts[] = { &xOpt, &yOpt };

    CliCommand cmd = {
        "abcd", LEN(opts), LEN(cmds), opts, cmds,
        "Program with two subcommands", "", NULL
    };

    int ec = CliParse(&cmd, argv + 1, argc - 1);
    if (ec != 0)
        return ec < 0; // error codes are negative

    puts("Success");

    return 0;
}
