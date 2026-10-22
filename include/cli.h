#ifndef CLI_H
#define CLI_H

typedef enum {
  CLI_STATUS_OK = 0,

  CLI_DOUBLE_DASH = 0x200,
  CLI_STATUS_HELP = 0x100,
  CLI_STATUS_COMP = 0xFF,
  CLI_STATUS_COMP_BASH = 1,

  CLI_ERROR_OPTION = -1,
  CLI_ERROR_POSITIONAL = -2,
  CLI_ERROR_REPEATED = -3,
  CLI_ERROR_UNEXPECTED_ARG = -4,

} CliStatusCode;

typedef struct CliOption CliOption;
typedef struct CliCommand CliCommand;

typedef void (*CliAction)(const char* arg, void* data);

struct CliOption {
  const char* name;
  CliAction action;
  void* data;
  const char* help;

  unsigned char min; // minimum required number of values
  unsigned char max; // maximum allowed number of values
  unsigned char canRepeat : 1; // --opt a --opt b
  unsigned char allowNoArg : 1; // --opt --opt b
  unsigned char noTail : 1; // in `--opt a b`, b is not a value of opt
  unsigned char positional : 1; // can be used without option
  unsigned char fallThrough : 1; // can be used at deeper subcommand levels

  // TODO: mutable state variables create problems for reusing option definitions

  // Not set by user
  unsigned char passed : 1; // set on first appearance
  unsigned char cnt; // count number of values
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

  // Not set by user
  CliCommand* command;
};

CliStatusCode CliParse(
  CliCommand* command, const char* const* args, unsigned nArgs
);

const char* CliPathName(const char* arg);

#endif
