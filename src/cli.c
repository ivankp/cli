#include "cli.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define CLI_DOUBLE_DASH 2

const char* CliPathName(const char* path) {
  const char* name = path;
next:
  switch (*path++) {
    case '\0': return name;
#ifdef _WIN32
    case '\\':
#endif
    case '/': name = path;
    default: goto next;
  }
}

FILE *flog;

unsigned CliParseUnsigned(const char* str) {
  unsigned u = 0;
  char c;
next:
  c = *str;
  if ('0' <= c && c <= '9') {
    u *= 10;
    u += (c - '0');
    ++str;
    goto next;
  }
  if (c == '\0')
    return u;
  return -1u;
}

static const char* CliEnvVarNameEnd(const char* var) {
next:
  switch (*var) {
    case '=':
    case '\0':
      return var;
    default:
      ++var;
      goto next;
  }
}

static bool CliStrEqZE(const char* ref, const char* str, const char* end) {
  for (;; ++ref, ++str) {
    const char c = *ref;
    if (str == end)
      return c == '\0';
    if (c == '\0' || c != *str)
      return false;
  }
}

static bool CliMatchName(
  const char* name, const char* arg, const char* end
) {
  const char* a;
skip_space:
  switch (*name) {
    case ' ': ++name; goto skip_space;
    case '\0': return false; // didn't match
  }
  a = arg;
compare:
  if (end ? a == end : *a == '\0') {
    switch (*name) {
      case '\0':
      case ' ': return true; // matched
      default: goto skip_name;
    }
  }
  if (*name == *a) {
    ++name;
    ++a;
    goto compare;
  }
skip_name:
  switch (*name) {
    case '\0': return false; // didn't match
    case ' ': ++name; goto skip_space;
    default: ++name; goto skip_name;
  }
}

CliOption* CliMatchOption(
  CliCommand* command, const char* arg, const char* end
) {
  CliOption **opts = command->options, **optsEnd = opts + command->nOptions;
  for (; opts != optsEnd; ++opts) {
    CliOption* opt = *opts;
    if (CliMatchName(opt->name, arg, end))
      return opt;
  }
  return NULL;
}

CliCommand* CliMatchCommand(
  CliCommand* command, const char* arg
) {
  CliCommand **cmds = command->commands, **cmdsEnd = cmds + command->nCommands;
  for (; cmds != cmdsEnd; ++cmds) {
    CliCommand* cmd = *cmds;
    if (CliMatchName(cmd->name, arg, NULL))
      return cmd;
  }
  return NULL;
}

#ifndef CLI_UNIT_TEST
static void CliPrintHelpOptionUsage(CliOption* opt) {
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
#endif

#ifndef CLI_UNIT_TEST
static void CliPrintHelpOption(CliOption* opt) {
  const char *a = opt->name, *b;
  char sep = ' ';

skip_space:
  switch (*a) {
    case ' ': ++a; goto skip_space;
    case '\0': return;
  }

  for (b = a; ; ++b) {
    const char c = *b;
    if (c == '\0' || c == ' ') {
      const int n = b - a;
      if (n > 0) { // TODO: is this always true?
        printf("%c %s%.*s", sep, "--" + (n == 1), n, a);
        sep = ',';
      }
      if (c == '\0') // TODO: simplify
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
#endif

static void CliPrintHelp(const char*, CliCommand*);

CliOption cliHelpOption = {
  "h help", (CliAction)&CliPrintHelp, NULL,
  "Print this help message"
};

static void CliPrintHelp(const char* value, CliCommand* cmd) {
  (void) value;
#ifdef CLI_UNIT_TEST
  (void) cmd;
#else
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
      CliPrintHelpOptionUsage(*opts);
    }
    CliPrintHelpOptionUsage(&cliHelpOption);

    if (!cmd->command)
      break;
    cmd = cmd->command;
  }
  printf("\n");

  { // Top help text
    const char* text = cmd->help;
    if (text && *text) {
      printf("\n%s\n", text);
    }
  }

  // TODO: commands

  // TODO: positional arguments

  if (cmd->nOptions) {
    printf("\noptions:\n");

    CliOption **opts = cmd->options, **optsEnd = opts + cmd->nOptions;
    for (; opts != optsEnd; ++opts) {
      CliPrintHelpOption(*opts);
    }
    CliPrintHelpOption(&cliHelpOption);
  }

  { // Bottom help text
    const char* h = cmd->helpBottom;
    if (h && *h) {
      printf("\n%s\n", h);
    }
  }
#endif
}

static CliStatusCode CliParseImpl(
  CliCommand* command, const char* const* args, unsigned nArgs
) {
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
      ++arg;
      const char* value = arg + 1;
      CliOption* opt = CliMatchOption(command, arg, value);

      if (CliMatchName(cliHelpOption.name, arg, value))
        goto help;

      // TODO: -abcd

      if (!opt) {
#ifndef CLI_UNIT_TEST
        fprintf(stderr, "Unknown option -%c\n", *arg);
#endif
        return CLI_STATUS_ERROR;
      }

      if (*value == '\0')
        value = NULL;

      (*opt->action)(value, opt->data);

    } else if (arg[2] == '\0') { // just -- ....................................
      flags ^= CLI_DOUBLE_DASH;

    } else { // long option ....................................................
      arg += 2;
      const char *b = arg, *value;
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
      CliOption* opt = CliMatchOption(command, arg, b);

      if (CliMatchName(cliHelpOption.name, arg, b))
        goto help;

      if (!opt) {
#ifndef CLI_UNIT_TEST
        fprintf(stderr, "Unknown option --%.*s\n", (int)(b - arg), arg);
#endif
        return CLI_STATUS_ERROR;
      }

      (*opt->action)(value, opt->data);
    }
  } // end args loop

  // TODO: `--opt arg` same as `--opt=arg`

  return CLI_STATUS_OK;

help:
#ifndef CLI_UNIT_TEST
  cliHelpOption.action(NULL, rootCommand);
#endif
  return CLI_STATUS_HELP;
}

extern char **environ;

// 1. Resolve quotes
// 2. Remove substitutions, subshell invocations, etc.
// 3. Ask for variables
// 4. Parse permissively, skipping bad arguments

static bool CliCompletionBash(CliCommand* command) {
  typedef struct {
    const char *value, *name;
  } EnvVar;

  struct {
    EnvVar key, line, point, type;
  } bashComp = {
    // https://man7.org/linux/man-pages/man1/bash.1.html
    { NULL, "COMP_KEY" },
    { NULL, "COMP_LINE" },
    { NULL, "COMP_POINT" },
    { NULL, "COMP_TYPE" }
  };
  const unsigned nComps = sizeof(bashComp) / sizeof(EnvVar);

  for (int i = 0;; ++i) {
    const char* var = environ[i];
    if (var == NULL)
      break;
    const char* d = CliEnvVarNameEnd(var);

    for (unsigned i = 0; i < nComps; ++i) {
      EnvVar* comp = (EnvVar*)&bashComp + i;
      if (CliStrEqZE(comp->name, var, d))
        comp->value = d + 1;
    }
  }

  for (unsigned i = 0; i < nComps; ++i) {
    EnvVar* comp = (EnvVar*)&bashComp + i;
    if (comp->value == NULL)
      return false;
  }

  flog = fopen("/home/ivanp/projects/cli/examples/comp.log", "a");

  fprintf(flog, "\n");
  for (unsigned i = 0; i < nComps; ++i) {
    EnvVar* comp = (EnvVar*)&bashComp + i;
    fprintf(flog, "%s = %s\n", comp->name, comp->value);
  }

  const unsigned point = CliParseUnsigned(bashComp.point.value);
  if (point == -1u)
    return true;

  const char* p = bashComp.line.value + point - 1;
  if (p[0] == '-' && (p[-1] == ' ' || (p[-1] == '-' && p[-2] == ' '))) {
    CliOption **opts = command->options, **optsEnd = opts + command->nOptions;
    for (; opts != optsEnd; ++opts) {
      CliOption* opt = *opts;
      const char *a = opt->name, *b;
      fprintf(flog, "%s\n", a);
skip_space:
      switch (*a) {
        case ' ': ++a; goto skip_space;
        case '\0': continue;
      }

      for (b = a; ; ++b) {
        const char c = *b;
        if (c == '\0' || c == ' ') {
          const int n = b - a;
          printf("%s%.*s\n", "--" + (n == 1), n, a);
          a = b + (c != '\0');
          goto skip_space;
        }
      }
    }
  }

  // fprintf(flog, "%u %c\n", point, bashComp.line.value[point-1]);
  // fprintf(flog, "%u %s %ld\n", point, bashComp.point.value, strlen(bashComp.point.value));

  fclose(flog);
  return true;
}

CliStatusCode CliParse(
  CliCommand* command, const char* const* args, unsigned nArgs
) {
  if (CliCompletionBash(command)) {
    return CLI_STATUS_COMP_BASH;
  }

  return CliParseImpl(command, args, nArgs);
}
