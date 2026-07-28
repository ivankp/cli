#ifndef IVAN_CLI_HH
#define IVAN_CLI_HH

typedef struct CliOption CliOption;
typedef struct CliCommand CliCommand;
typedef struct CliParser CliParser;

struct CliOption {
  const char* name;
  void (*action)(const char* arg, void* data);
  void* data;
};

struct CliCommand {
  const char* name;
  CliParser* parser;
};

struct CliParser {
  unsigned nOptions;
  unsigned nCommands;
  CliOption** options;
  CliCommand** commands;
};

int CliParse(CliParser* parser, const char* const* args, unsigned nArgs);

#endif
