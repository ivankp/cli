#include "cli.h"
#include <stdio.h>
#include <stdlib.h>

#define CLI_OPT 1
#define CLI_OPT_SHORT 1
#define CLI_OPT_LONG 3

void CliParse(const char* const* args, const char* const* argsEnd, CliParser* parser) {
  (void) parser;

  // unsigned flags = 0;

  for (; args != argsEnd; ++args) {
    const char* arg = *args;
    if (arg[0] != '-' || arg[1] == '\0') { // value or command
      // TODO: may be a command
    } else if (arg[1] != '-') { // short option
      const char c = arg[1];
      // flags |= CLI_OPT_SHORT;
      CliOption **opts = parser->options, **optsEnd = opts + parser->nOptions;
      for (; opts != optsEnd; ++opts) { // find option with matching name
        CliOption* opt = *opts;
        const char* name = opt->name;
next_name:
        if (name[0] == c && (name[1] == '\0' || name[1] == ' ')) {
          (*opt->action)(arg + 2, opt->data);
          opts = NULL;
          break; // opt loop
        } else {
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
      }
      if (opts) {
        printf("Unexpected option -%c\n", c);
        exit(1);
      }
    } else if (!arg[2]) { // just --
    } else { // long option
    }
  }
}
