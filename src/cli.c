#include "cli.h"

#include <stdio.h>

#define CLI_DOUBLE_DASH 2

const char* CliPathName(const char* arg) {
  const char* name = arg;
next:
  switch (*arg++) {
    case '\0': return name;
    case '/': name = arg;
    default: goto next;
  }
}

CliOption* CliMatchOption(CliParser* parser, const char* arg, unsigned argLen) {
  CliOption **opts = parser->options, **optsEnd = opts + parser->nOptions;
  for (; opts != optsEnd; ++opts) {
    CliOption* opt = *opts;
    const char* name = opt->name;
    // TODO: skip spaces first
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

CliParser* CliMatchCommand(CliParser* parser, const char* arg) {
  CliParser **cmds = parser->commands, **cmdsEnd = cmds + parser->nCommands;
  for (; cmds != cmdsEnd; ++cmds) {
    CliParser* cmd = *cmds;
    const char* name = cmd->name;
    // TODO: skip spaces first
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

void CliPrintHelp(CliParser* cmd) {
  printf("usage:");
  for (;;) {
    const char* name = cmd->name;

    int n = 0;
next_char_1:
    switch (name[n]) {
      case '\0':
      case ' ': break;
      default: ++n; goto next_char_1;
    }
    printf(" %.*s", n, name);

    CliOption **opts = cmd->options, **optsEnd = opts + cmd->nOptions;
    for (; opts != optsEnd; ++opts) {
      CliOption* opt = *opts;
      const char* name = opt->name;

      int n = 0;
next_char_2:
      switch (name[n]) {
        case '\0':
        case ' ': break;
        default: ++n; goto next_char_2;
      }
      printf(" %s%.*s", "--" + (n == 1), n, name);
    }

    if (!cmd->command)
      break;
    cmd = cmd->command;
  }
  printf("\n");

  { // Top text
    const char* h = cmd->helpTop;
    if (h && *h) {
      printf("\n%s\n", h);
    }
  }

  // TODO: commands

  // TODO: positional arguments

  if (cmd->nOptions) {
    printf("\noptions:\n");

    CliOption **opts = cmd->options, **optsEnd = opts + cmd->nOptions;
    for (; opts != optsEnd; ++opts) {
      CliOption* opt = *opts;
      const char *a = opt->name, *b;
      char sep = ' ';

skip_space:
      switch (*a) {
        case ' ': ++a; goto skip_space;
        case '\0': continue; // next option
      }

      for (b = a; ; ++b) {
        const char c = *b;
        if (c == '\0' || c == ' ') {
          const int n = b - a;
          if (n > 0) {
            printf("%c %s%.*s", sep, "--" + (n == 1), n, a);
            sep = ',';
          }
          if (c == '\0')
            break;
          a = b + 1;
          goto skip_space;
        }
      }
      printf("\n");
      const char* h = opt->help;
      if (h && *h) {
        printf("    %s\n", h);
      }
    }
  }

  { // Bottom text
    const char* h = cmd->helpBottom;
    if (h && *h) {
      printf("\n%s\n", h);
    }
  }
}

int CliParse(CliParser* parser, const char* const* args, unsigned nArgs) {
#ifndef CLI_UNIT_TEST
  CliParser* rootParser = parser;
#endif
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
      CliParser* cmd = CliMatchCommand(parser, arg);
      if (cmd) {
        parser->command = cmd;
        parser = cmd;
        // TODO: finalize options for the previous command level
      } else {
value:
        ;
      }

    } else if (arg[1] != '-') { // short option ................................
      if (arg[1] == 'h')
        goto help;

      // TODO: -abcd

      CliOption* opt = CliMatchOption(parser, arg + 1, 1);
      if (!opt) {
#ifndef CLI_UNIT_TEST
        fprintf(stderr, "Unknown option -%c\n", arg[1]);
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
      const char *a = arg + 2;
      if (a[0] == 'h' && a[1] == 'e' && a[2] == 'l' && a[3] == 'p') {
        if (a[4] == '\0' || a[4] == '=')
          goto help;
      }

      const char *b = a, *value;
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
        fprintf(stderr, "Unknown option --%.*s\n", (int)n, a);
#endif
        return 1;
      }

      (*opt->action)(value, opt->data);
    }
  } // end args loop

  // TODO: `--opt arg` same as `--opt=arg`

  return 0;

help:
#ifndef CLI_UNIT_TEST
  CliPrintHelp(rootParser);
#endif
  return -1;
}
