#ifndef __DSHLIB_H__
    #define __DSHLIB_H__



//Constants for command structure sizes
#define EXE_MAX 64
#define ARG_MAX 256
#define CMD_MAX 8
#define CMD_ARGV_MAX (CMD_MAX + 1)
// Longest command that can be read from the shell
#define SH_CMD_MAX EXE_MAX + ARG_MAX

typedef struct cmd_buff
{
    int  argc;
    char *argv[CMD_ARGV_MAX];
    char *_cmd_buffer;
} cmd_buff_t;

/* WIP - Move to next assignment 
#define N_ARG_MAX    15     //MAX number of args for a command
typedef struct command{
    char exe [EXE_MAX];
    char args[ARG_MAX];
    int  argc;
    char *argv[N_ARG_MAX + 1];  //last argv[LAST] must be \0
}command_t;
*/


//Special character #defines
#define SPACE_CHAR  ' '
#define PIPE_CHAR   '|'
#define PIPE_STRING "|"
#define QUOTE_CHAR '"'
#define NULL_BYTE '\0'

#define SH_PROMPT "dsh2> "
#define EXIT_CMD "exit"
#define DRAGON_CMD "dragon"
#define CD_CMD "cd"
#define RC_CMD "rc"

//Standard Return Codes
#define OK                       0
#define WARN_NO_CMDS            -1
#define ERR_TOO_MANY_COMMANDS   -2
#define ERR_CMD_OR_ARGS_TOO_BIG -3
#define ERR_CMD_ARGS_BAD        -4      //for extra credit
#define ERR_MEMORY              -5
#define ERR_EXEC_CMD            -6
#define OK_EXIT                 -7

//prototypes
int alloc_cmd_buff(cmd_buff_t *cmd_buff);
int free_cmd_buff(cmd_buff_t *cmd_buff);
int clear_cmd_buff(cmd_buff_t *cmd_buff);
int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff);
int can_insert_cmd_buff_argv(cmd_buff_t *cmd_buff, int arg_len);
int format_cmd_line(char ** dest, char *cmd_line, int cmd_line_len);
void print_error_n_builtin(char * exe_name, int err);
void set_rc(int new_rc, int * rc);

//built in command stuff
typedef enum {
    BI_CMD_EXIT,
    BI_CMD_DRAGON,
    BI_CMD_CD,
    BI_NOT_BI,
    BI_EXECUTED,
    BI_N_EXECUTED,
    BI_RC,
} Built_In_Cmds;

Built_In_Cmds match_command(const char *input); 
Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd, Built_In_Cmds built_in);
Built_In_Cmds exec_cd_cmd(char * path);
void print_error_builtin(Built_In_Cmds built_in);
void print_rc(int rc);

//main execution context
int exec_local_cmd_loop();
int exec_cmd(cmd_buff_t *cmd);


//output constants
#define CMD_OK_HEADER       "PARSED COMMAND LINE - TOTAL COMMANDS %d\n"
#define CMD_WARN_NO_CMD     "warning: no commands provided\n"
#define CMD_ERR_PIPE_LIMIT  "error: piping limited to %d commands\n"
#define CMD_ERR_TOO_MANY_ARGS "Too many arguments supplied.\n"
#define CMD_ERR_EXECUTE "error: cannot execute %s\n"
#define CMD_ERR_CLEAR "error: cannot clear cmd buffer.\n"
#define CMD_ERR_BUILTIN "Unable to execute builtin.\n"
#define CMD_ERR_PERM_OP "error: Operation not permitted while executing '%s'.\n"
#define CMD_ERR_N_FOUND "error: Command '%s' not found.\n"
#define CMD_ERR_PERM_EXEC "error: Permission denied to execute '%s'.\n"
#define CMD_ERR_EXEC "error: '%s' is not executable.\n"
#define CMD_ERR_EXEC_MEM "error: Not enough memory to execute '%s'.\n"
#define CMD_EXEC_DEFAULT "error: Failed to execute '%s'. error code: %d.\n"

#define ERR_MEMORY_INIT "Unable to initialize memory for buffers.\n"

extern int return_code_cmd;

#endif