#include "cli.h"

#include <stdio.h>

#define CLI_OPT 1
#define CLI_OPT_SHORT 1
#define CLI_OPT_LONG 3
#define CLI_DOUBLE_DASH 4

CliOption* CliMatchOption(CliParser* parser, const char* arg, unsigned argLen) {
  CliOption **opts = parser->options, **optsEnd = opts + parser->nOptions;
  for (; opts != optsEnd; ++opts) {
    CliOption* opt = *opts;
    const char* name = opt->name;
next_name:
    for (unsigned i = 0; ; ++i, ++name) {
      if (i == argLen) {
        switch (*name) {
          case '\0':
          case ' ': return opt;
        }
      }
      if (*name != arg[i])
        goto skip_name;
    }
skip_name:
    switch (*name) {
      case '\0': continue; // next option
      case ' ': ++name; goto skip_space;
      default: ++name; goto skip_name;
    }
skip_space:
    switch (*name) {
      case ' ': ++name; goto skip_space;
      case '\0': continue; // next option
      default: goto next_name;
    }
  }
  return NULL;
}

CliCommand* CliMatchCommand(CliParser* parser, const char* arg) {
  CliCommand **cmds = parser->commands, **cmdsEnd = cmds + parser->nCommands;
  for (; cmds != cmdsEnd; ++cmds) {
    CliCommand* cmd = *cmds;
    const char* name = cmd->name;
next_name:
    for (const char* a = arg; ; ++a, ++name) {
      if (*a == '\0') {
        switch (*name) {
          case '\0':
          case ' ': return cmd;
        }
      }
      if (*name != *a)
        goto skip_name;
    }
skip_name:
    switch (*name) {
      case '\0': continue; // next command
      case ' ': ++name; goto skip_space;
      default: ++name; goto skip_name;
    }
skip_space:
    switch (*name) {
      case ' ': ++name; goto skip_space;
      case '\0': continue; // next command
      default: goto next_name;
    }
  }
  return NULL;
}

void CliParseHelp(CliParser* parser, const char* const* args, unsigned nArgs) {
  unsigned flags = 0;
  const char* const* const argsEnd = args + nArgs;
  for (; args != argsEnd; ++args) {
    const char* arg = *args;

    if (flags & CLI_DOUBLE_DASH) { // ..........................................
      if (arg[0] == '-' && arg[1] == '-' && arg[2] == '\0') {
        flags ^= CLI_DOUBLE_DASH;
      }

    } else if (arg[0] != '-' || arg[1] == '\0') { // value or command ..........
      CliCommand* cmd = CliMatchCommand(parser, arg);
      if (cmd) {
        parser = cmd->parser;
      }

    } else if (arg[1] != '-') { // short option ................................
      for (++arg; *arg == 'h'; ++arg)
        ++help;
      break;

    } else if (arg[2] == '\0') { // just -- ....................................
      flags ^= CLI_DOUBLE_DASH;

    } else { // long option ....................................................
      ++arg;
      if (*++arg == 'h' && *++arg == 'e' && *++arg == 'l' && *++arg == 'p') {
        if (*++arg == '\0' || *arg == '=')
          goto help;
      }
    }
  } // end args loop
  return;
help:
  printf("HELP\n");
}

int CliParse(CliParser* parser, const char* const* args, unsigned nArgs) {
  unsigned flags = 0;
  const char* const* const argsEnd = args + nArgs;
  for (; args != argsEnd; ++args) {
    const char* arg = *args;

    if (flags & CLI_DOUBLE_DASH) { // ..........................................
      if (arg[0] == '-' && arg[1] == '-' && arg[2] == '\0') {
        flags ^= CLI_DOUBLE_DASH;
      } else {
        goto value;
      }

    } else if (arg[0] != '-' || arg[1] == '\0') { // value or command ..........
      CliCommand* cmd = CliMatchCommand(parser, arg);
      if (cmd) {
        // TODO: finalize options for the previous command level
        parser = cmd->parser;
      } else {
value:
        ;
      }

    } else if (arg[1] != '-') { // short option ................................
      flags |= CLI_OPT_SHORT;
      CliOption* opt = CliMatchOption(parser, arg + 1, 1);
      if (!opt) {
#ifndef CLI_UNIT_TEST
        fprintf(stderr, "Unexpected option -%c\n", arg[1]);
#endif
        return 1;
      }
      const char* value = arg + 2;
      if (*value == '\0')
        value = NULL;
      (*opt->action)(value, opt->data);

    } else if (arg[2] == '\0') { // just -- ....................................
      flags ^= CLI_DOUBLE_DASH;

    } else { // long option ....................................................
      flags |= CLI_OPT_LONG;
      const char *a = arg + 2, *b = a, *value;
      for (;; ++b) {
        switch (*b) {
          case '\0':
            value = NULL;
            goto match_option;
          case '=':
            value = b + 1;
            goto match_option;
        }
      }
match_option: ;
      const unsigned n = b - a;
      CliOption* opt = CliMatchOption(parser, a, n);
      if (!opt) {
#ifndef CLI_UNIT_TEST
        fprintf(stderr, "Unexpected option --%.*s\n", (int)n, a);
#endif
        return 1;
      }
      (*opt->action)(value, opt->data);
    }
  } // end args loop
  return 0;
}
