#include "cli.h"

#include <stdio.h>

#define CLI_DOUBLE_DASH 2

const char* CliPathName(const char* path) {
  const char* name = path;
next:
  switch (*path++) {
    case '\0': return name;
    case '/': name = path;
    default: goto next;
  }
}

CliOption* CliMatchOption(CliCommand* command, const char* arg, unsigned len) {
  CliOption **opts = command->options, **optsEnd = opts + command->nOptions;
  for (; opts != optsEnd; ++opts) {
    CliOption* opt = *opts;
    const char* name = opt->name;
    unsigned i;
skip_space:
    switch (*name) {
      case ' ': ++name; goto skip_space;
      case '\0': continue; // next option
    }
    i = 0;
compare:
    if (i == len) {
      switch (*name) {
        case '\0':
        case ' ': return opt;
        default: goto skip_letter;
      }
    }
    if (*name == arg[i]) {
      ++i;
      ++name;
      goto compare;
    }
skip_letter:
    switch (*name) {
      case '\0': continue; // next option
      case ' ': ++name; goto skip_space;
      default: ++name; goto skip_letter;
    }
  }
  return NULL;
}

CliCommand* CliMatchCommand(CliCommand* command, const char* arg) {
  CliCommand **cmds = command->commands, **cmdsEnd = cmds + command->nCommands;
  for (; cmds != cmdsEnd; ++cmds) {
    CliCommand* cmd = *cmds;
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

void CliPrintHelp(CliCommand* cmd) {
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

  { // Top help text
    const char* h = cmd->help;
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

  { // Bottom help text
    const char* h = cmd->helpBottom;
    if (h && *h) {
      printf("\n%s\n", h);
    }
  }
}

int CliParse(CliCommand* command, const char* const* args, unsigned nArgs) {
#ifndef CLI_UNIT_TEST
  CliCommand* rootCommand = command;
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
      CliCommand* cmd = CliMatchCommand(command, arg);
      if (cmd) {
        command->command = cmd;
        command = cmd;
        // TODO: finalize options for the previous command level
      } else {
value:
        ;
      }

    } else if (arg[1] != '-') { // short option ................................
      if (arg[1] == 'h')
        goto help;

      // TODO: -abcd

      CliOption* opt = CliMatchOption(command, arg + 1, 1);
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
      CliOption* opt = CliMatchOption(command, a, n);
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
  CliPrintHelp(rootCommand);
#endif
  return -1;
}
