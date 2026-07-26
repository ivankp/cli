#include <stddef.h>
#include <stdio.h>

#include "cli.h"

void FlagPassed(const char*, void*) {
  puts("Flag passed");
}

int main(int argc, const char* const* argv) {
  CliOption opt_a = { "a flag", &FlagPassed, NULL };

  CliOption* options[] = {
    &opt_a
  };

  CliParser parser = { 1, 0, options, NULL };

  CliParse(argv + 1, argv + argc, &parser);

  return 0;
}
