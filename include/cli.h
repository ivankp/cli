#ifndef CLI_H
#define CLI_H

typedef struct CliOption CliOption;
typedef struct CliParser CliParser;

struct CliOption {
  const char* name;
  void (*action)(const char* arg, void* data);
  void* data;
  const char* help;
};

struct CliParser {
  const char* name;
  unsigned nOptions;
  unsigned nCommands;
  CliOption** options;
  CliParser** commands;
  const char* helpTop;
  const char* helpBottom;
  CliParser* command;
};

int CliParse(CliParser* parser, const char* const* args, unsigned nArgs);

const char* CliPathName(const char* arg);

#endif
