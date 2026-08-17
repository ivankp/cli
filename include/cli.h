#ifndef CLI_H
#define CLI_H

typedef enum {
  CLI_STATUS_OK = 0,

  CLI_STATUS_ERROR = 1,

  CLI_STATUS_HELP = -0x100,
  CLI_STATUS_COMP = -0xFF,
  CLI_STATUS_COMP_BASH = -1,
} CliStatusCode;

typedef struct CliOption CliOption;
typedef struct CliCommand CliCommand;

typedef void (*CliAction)(const char* arg, void* data);

struct CliOption {
  const char* name;
  CliAction action;
  void* data;
  const char* help;
};

extern CliOption cliHelpOption;

struct CliCommand {
  const char* name;
  unsigned nOptions;
  unsigned nCommands;
  CliOption** options;
  CliCommand** commands;
  const char* help;
  const char* helpBottom;
  CliCommand* command;
};

CliStatusCode CliParse(
  CliCommand* command, const char* const* args, unsigned nArgs
);

const char* CliPathName(const char* arg);

#endif
