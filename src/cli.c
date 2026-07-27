#include "cli.h"

#include <stdio.h>

#define CLI_OPT 1
#define CLI_OPT_SHORT 1
#define CLI_OPT_LONG 3
#define CLI_DOUBLE_DASH 4

CliOption* CliMatchOption(CliParser* parser, const char* arg, unsigned len) {
  CliOption **opts = parser->options, **optsEnd = opts + parser->nOptions;
  for (; opts != optsEnd; ++opts) {
    CliOption* opt = *opts;
    const char* name = opt->name;
next_name:
    for (unsigned i = 0; ; ++i) {
      if (i == len && (*name == '\0' || *name == ' '))
        return opt;
      if (*name != arg[i])
        break;
      ++name;
    }
skip_name:
    switch (*name) {
      case '\0': continue; // opt loop
      case ' ': ++name; break;
      default: ++name; goto skip_name;
    }
skip_space:
    switch (*name) {
      case ' ': ++name; goto skip_space;
      case '\0': continue; // opt loop
      default: ;
    }
    goto next_name;
  }
  return NULL;
}

int CliParse(CliParser* parser, const char* const* args, const char* const* argsEnd) {
  unsigned flags = 0;
  for (; args != argsEnd; ++args) {
    const char* arg = *args;

    if (flags & CLI_DOUBLE_DASH) {
      if (arg[0] == '-' && arg[1] == '-' && arg[2] == '\0') {
        flags ^= CLI_DOUBLE_DASH;
      } else {
        goto value;
      }

    } else if (arg[0] != '-' || arg[1] == '\0') { // value or command
      if (0) { // TODO: may be a command

      } else {
value:
        ;
      }

    } else if (arg[1] != '-') { // short option
      flags |= CLI_OPT_SHORT;
      CliOption* opt = CliMatchOption(parser, arg + 1, 1);
      if (!opt) {
#ifndef CLI_UNIT_TEST
        printf("Unexpected option -%c\n", arg[1]);
#endif
        return 1;
      }
      (*opt->action)(arg + 2, opt->data);

    } else if (!arg[2]) { // just --
      flags ^= CLI_DOUBLE_DASH;

    } else { // long option

    }
  }
  return 0;
}
