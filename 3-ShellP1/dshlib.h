#ifndef __DSHLIB_H__
#define __DSHLIB_H__

// Constants for command structure sizes
#define EXE_MAX 64
#define ARG_MAX 256
#define CMD_MAX 8
// Longest command that can be read from the shell
#define SH_CMD_MAX EXE_MAX + ARG_MAX
// Constants for max values
#define DECIMAL_DIGITS_BOUND(t) (241 * sizeof(t) / 100 + 1)
#define INT_DIGITS_BOUND (DECIMAL_DIGITS_BOUND(int))

typedef struct command
{
    char exe[EXE_MAX];
    char args[ARG_MAX];
} command_t;

typedef struct command_list
{
    int num;
    command_t commands[CMD_MAX];
} command_list_t;

// Special character #defines
#define SPACE_CHAR ' '
#define PIPE_CHAR '|'
#define PIPE_STRING "|"
#define SPACE_STRING " "

#define SH_PROMPT "dsh> "
#define EXIT_CMD "exit"
#define DRAGON_CMD "dragon"

// Standard Return Codes
#define OK 0
#define WARN_NO_CMDS -1
#define ERR_TOO_MANY_COMMANDS -2
#define ERR_CMD_OR_ARGS_TOO_BIG -3

// starter code
#define M_NOT_IMPL "The requested operation is not implemented yet!\n"
#define EXIT_NOT_IMPL 3
#define NOT_IMPLEMENTED_YET 0

// prototypes
void print_dragon_cmp(const char**, int);
int build_cmd_list(char *cmd_line, command_list_t *clist);
int strip_token(char * dest, char * token, int token_len);
void print_cmd_list(struct command_list);
int count_pipes(char *);

// output constants
#define CMD_OK_HEADER "PARSED COMMAND LINE - TOTAL COMMANDS %d\n"
#define CMD_WARN_NO_CMD "warning: no commands provided\n"
#define CMD_ERR_PIPE_LIMIT "error: piping limited to %d commands\n"
#define CMD_EXE_PRINT "<%d> %s "
#define CMD_ARGS_PRINT "[%s]"


#endif