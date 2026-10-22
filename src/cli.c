#include "cli.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h> // malloc

// Ideas
// - allow options to fall through to the subsequent command levels
// - min & max number of values
// - allow option repetition (--opt --opt)
// - options with no name (positional arguments)
// - greedy (default) vs lazy options?
// - regex-like rules for multiple nameless options
// - value sets (choices)

FILE *flog; // debug

typedef struct {
  CliCommand* command;
  CliOption* option;
  int flags;
} CliParserState;

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

static bool CliStrEqZE(const char* ref, const char* str, const char* strEnd) {
  for (;; ++ref, ++str) {
    const char c = *ref;
    if (str == strEnd)
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

#ifndef CLI_UNIT_TEST
static void CliPrintHelpCommand(CliCommand* cmd) {
  const char *a = cmd->name, *b;
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
        printf("%c %.*s", sep, n, a);
        sep = ',';
      }
      if (c == '\0') // TODO: simplify
        break;
      a = b + 1;
      goto skip_space;
    }
  }
  printf("\n");
  const char* h = cmd->help;
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

    // commands can have space-separated aliases
    // print only the first word in name
    int n = 0;
next_char_1:
    switch (name[n]) {
      case '\0':
      case ' ': break;
      default: ++n; goto next_char_1;
    }
    printf(" %.*s", n, name);

    if (!cmd->command)
      break;
    cmd = cmd->command;
  }

  CliOption **opts = cmd->options, **optsEnd = opts + cmd->nOptions;
  for (; opts != optsEnd; ++opts) {
    CliPrintHelpOptionUsage(*opts);
  }

  printf("\n");

  { // Top help text
    const char* h = cmd->help;
    if (h && *h) {
      printf("\n%s\n", h);
    }
  }

  if (cmd->nCommands) {
    printf("\ncommands:\n");
    CliCommand **cmds = cmd->commands, **cmdsEnd = cmds + cmd->nCommands;
    for (; cmds != cmdsEnd; ++cmds) {
      CliPrintHelpCommand(*cmds);
    }
  }

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

// Parse a single command line argument
static CliStatusCode CliParseArg(CliParserState* state, const char* arg) {
  // opt can be set at the top, because previous option may need to be
  // finalized when the next option is encountered
  CliOption* opt = state->option;
  const char *value, *optEnd;

  if (state->flags & CLI_DOUBLE_DASH) { // closing double dash .................
    if (arg[0] == '-' && arg[1] == '-' && arg[2] == '\0')
      goto double_dash;
    else
      goto value;

  } else if (arg[0] != '-' || arg[1] == '\0') { // command or value ............
    CliCommand* cmd = CliMatchCommand(state->command, arg);
    if (cmd) {
      state->command->command = cmd;
      state->command = cmd;
      // TODO: finalize options for the previous command level
      goto ok;
    }

value:
    value = arg;
    if (!opt) {
      // TODO: check if a positional argument is ok
      // TODO: regex logic
      return CLI_ERROR_POSITIONAL;
    }

  } else if (arg[1] != '-') { // short option ..................................
next_short:
    ++arg;
    optEnd = arg + 1;
    value = *optEnd ? optEnd : NULL;

    goto match_option;

  } else if (arg[2] == '\0') { // opening double dash ..........................
double_dash:
    state->flags ^= CLI_DOUBLE_DASH;
    goto ok;

  } else { // long option ......................................................
    arg += 2;
    optEnd = arg;
    for (;; ++optEnd) {
      switch (*optEnd) {
        case '\0':
          value = NULL;
          goto match_option;
        case '=':
          value = optEnd + 1;
          goto match_option;
      }
    }

match_option: ;
    const int optLen = optEnd - arg;
    opt = CliMatchOption(state->command, arg, optEnd);

    if (!opt) {
      // help has lowest precedence
      if (CliMatchName(cliHelpOption.name, arg, optEnd))
        return CLI_STATUS_HELP;

#ifndef CLI_UNIT_TEST
      fprintf(stderr, "Unknown option %s%.*s\n",
        "--" + (optLen == 1), optLen, arg);
#endif
      return CLI_ERROR_OPTION;
    }

    if (opt->passed && !opt->canRepeat) {
#ifndef CLI_UNIT_TEST
      fprintf(stderr, "Repeated option %s%.*s\n",
        "--" + (optLen == 1), optLen, arg);
#endif
      return CLI_ERROR_REPEATED;
    }
    opt->passed = true;

    if (opt->max == 0) { // option takes no arguments
      if (value && optLen > 1) {
#ifndef CLI_UNIT_TEST
        fprintf(stderr, "Option --%.*s takes no arguments\n", optLen, arg);
#endif
        return CLI_ERROR_UNEXPECTED_ARG;
      }
      opt->action(NULL, opt->data);
      state->option = NULL;
      if (value)
        goto next_short;
      goto ok;
    }

    state->option = opt;
  }

  // TODO: `--opt arg` same as `--opt=arg`

  // If value is NULL, delay action execution until value is provided,
  // or the next option is specified, or end of arguments
  if (value) {
    // Invoke the callback function for the current option
    CliAction action = opt->action;
    if (action)
      action(value, opt->data);
  }

ok:
  return CLI_STATUS_OK;
}

static const char* CliBashNextArg(const char** line, const char* point) {
  const char *a = *line, *b;

skip_space:
  if (!(a < point))
    goto noarg;

  switch (*a) {
    case ' ':
    case '\t': ++a; goto skip_space;
    case '\0': goto noarg; // end of line
  }

  b = a;

  // TODO: quotes, escapes, expansions

arg:
  if (!(b < point)) {
    *line = NULL;
    goto done;
  }

  switch (*b) {
    case ' ':
    case '\t': // end of argument
      *line = b;
      goto done;
    case '\0': // end of line
      *line = NULL;
      goto done;
    default: ++b; goto arg;
  }

done: ;
  // TODO: Prevent these allocations from leaking.
  // TODO: In the final implementation,
  // TODO: these strings may need to live until the program stops.
  const size_t len = b - a;
  char* dup = malloc(len + 1);
  memcpy(dup, a, len);
  dup[len] = '\0';
  return dup;

noarg:
  *line = NULL;
  return NULL;
}

extern char **environ;

static bool CliCompletionBash(CliParserState* state) {
  typedef struct {
    const char *value, *name;
  } EnvVar;

  // Bash completion variables
  struct {
    EnvVar key, line, point, type;
  } comp = {
    // https://man7.org/linux/man-pages/man1/bash.1.html
    { NULL, "COMP_KEY" },
    { NULL, "COMP_LINE" },
    { NULL, "COMP_POINT" },
    { NULL, "COMP_TYPE" }
  };
  const EnvVar* const endCompVars =
    (EnvVar*)&comp + (sizeof(comp) / sizeof(EnvVar));

  // Collect bash completion variables
  for (char** env = environ;; ++env) {
    const char* var = *env;
    if (var == NULL)
      break;
    const char* d = CliEnvVarNameEnd(var);

    for (EnvVar *compVar = (EnvVar*)&comp; compVar < endCompVars; ++compVar) {
      if (CliStrEqZE(compVar->name, var, d))
        compVar->value = d + 1;
    }
  }

  // Check that all COMP variables are present
  for (EnvVar *compVar = (EnvVar*)&comp; compVar < endCompVars; ++compVar) {
    if (compVar->value == NULL)
      return false;
  }

  const unsigned point = CliParseUnsigned(comp.point.value);
  if (point == -1u)
    return true;

  // Debug
  flog = fopen("/home/ivanp/projects/cli/examples/comp.log", "a");
  fprintf(flog, "\n");
  for (EnvVar *compVar = (EnvVar*)&comp; compVar < endCompVars; ++compVar) {
    fprintf(flog, "%s = %s\n", compVar->name, compVar->value);
  }

  state->flags = CLI_STATUS_COMP_BASH;

  const char* arg;
  const char* line = comp.line.value;
  const char* const pointPtr = line + point;
  while (line && (arg = CliBashNextArg(&line, pointPtr))) {
    fprintf(flog, "%s\n", arg);
    // if (CliParseArg(state, arg))
    //   break;
  }

  fclose(flog);
  return true;
}

CliStatusCode CliParse(
  CliCommand* command, const char* const* args, unsigned nArgs
) {
  CliParserState state = { command, NULL, 0 };

  if (CliCompletionBash(&state)) {
    return CLI_STATUS_COMP_BASH;
  }

  // TODO: look for help flags before full parsing?

  const char* const* const argsEnd = args + nArgs;
  for (; args != argsEnd; ++args) {
    const CliStatusCode status = CliParseArg(&state, *args);
    if (status < 0)
      return status;
    if (status == CLI_STATUS_HELP)
      goto help;
  }

  return CLI_STATUS_OK;

help:
#ifndef CLI_UNIT_TEST
  cliHelpOption.action(NULL, command);
#endif
  return CLI_STATUS_HELP;
}
