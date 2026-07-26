#include <stddef.h>
#include <stdio.h>

#include "cli.h"

void FlagPassed(const char*, void*) {
  puts("Flag passed");
}

int main(int argc, const char* const* argv) {
  CliOption opt_a = { "a flag   b", &FlagPassed, NULL };

  CliOption* options[] = { &opt_a };

  CliParser parser = { sizeof(options)/sizeof(*options), 0, options, NULL };

  if (CliParse(argv + 1, argv + argc, &parser))
    return 1;

  return 0;
}
