#ifndef __DSHLIB_H__
    #define __DSHLIB_H__


//Constants for command structure sizes
#define EXE_MAX 64
#define ARG_MAX 256
#define CMD_MAX 8
#define CMD_ARGV_MAX (CMD_MAX + 1)
// Longest command that can be read from the shell
#define SH_CMD_MAX EXE_MAX + ARG_MAX

typedef struct command
{
    char exe[EXE_MAX];
    char args[ARG_MAX];
} command_t;

#include <stdio.h>
#include <stdbool.h>

typedef struct cmd_buff
{
    int  argc;
    char *argv[CMD_ARGV_MAX];
    char *_cmd_buffer;
    char *input_file;  // extra credit, stores input redirection file (for `<`)
    char *output_file; // extra credit, stores output redirection file (for `>`)
    bool append_mode; // extra credit, sets append mode fomr output_file
} cmd_buff_t;

typedef struct command_list{
    int num;
    cmd_buff_t commands[CMD_MAX];
}command_list_t;

//Special character #defines
#define SPACE_CHAR  ' '
#define PIPE_CHAR   '|'
#define PIPE_STRING "|"
#define QUOTE_CHAR '"'
#define REDIR_INPUT_STRING "<"
#define REDIR_OUTPUT_STRING ">"
#define REDIR_APPEND_STRING ">>"
#define NULL_BYTE '\0'

#define SH_PROMPT       "dsh4> "
#define EXIT_CMD        "exit"
#define RC_SC           99
#define EXIT_SC         100
#define DRAGON_CMD "dragon"
#define CD_CMD "cd"
#define RC_CMD "rc"
#define RC_FORMAT "%d\n"

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
//int alloc_cmd_buff(cmd_buff_t *cmd_buff);
//int free_cmd_buff(cmd_buff_t *cmd_buff);
int clear_cmd_buff(cmd_buff_t *cmd_buff);
int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff);
//int close_cmd_buff(cmd_buff_t *cmd_buff);
int build_cmd_list(char *cmd_line, command_list_t *clist);
int free_cmd_list(command_list_t *cmd_lst);
int format_cmd_line(char **dest, char *src, int src_len);
int can_insert_cmd_buff_argv(cmd_buff_t *cmd_buff, int arg_len);
void _print_cmd_list(command_list_t * clist);
void print_err_build_cmd_list(int rc);
void flush_or_remove_new_line_buff(char * cmd_buff);
int read_stream_into_buff(char * cmd_buff, int max, FILE * stream);
int start_supervisor_and_execute_pipeline(command_list_t *clist);
void print_exec_rc(int);
int strings_are_equal(const char *, const char*);
int cmd_buff_has_no_args(cmd_buff_t * cmd_buff);


//built in command stuff
typedef enum {
    BI_CMD_EXIT,
    BI_CMD_DRAGON,
    BI_CMD_CD,
    BI_CMD_RC,              //extra credit command
    BI_CMD_STOP_SVR,        //new command "stop-server"
    BI_NOT_BI,
    BI_EXECUTED,
    BI_NOT_IMPLEMENTED,
} Built_In_Cmds;
Built_In_Cmds match_command(const char *input); 
Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd, Built_In_Cmds bi_type, int rc);
Built_In_Cmds exec_cd(cmd_buff_t * cmd);
Built_In_Cmds clist_has_no_io_redirection_and_built_in(Built_In_Cmds bi_rc, command_list_t * command_list);

typedef enum {
    R_NONE,
    R_APPEND,
    R_OUTPUT,
    R_INPUT
}Redirect_Flag;
Redirect_Flag check_if_arg_is_redirect_and_set_flag(char * arg_start, Redirect_Flag * redirect_flag);
int redirect_flag_was_set(Redirect_Flag redirect_flag);
int set_input_output_file_cmd_buff(cmd_buff_t * cmd_buff, char * arg_start, Redirect_Flag redirect_flag);
void set_arg_to_next_string_in_cmd_buff(char ** arg_start, cmd_buff_t * cmd_buff, int i);


//main execution context
int exec_local_cmd_loop();
int exec_cmd(cmd_buff_t *cmd);
int execute_pipeline(command_list_t *clist);


//output constants
#define CMD_OK_HEADER       "PARSED COMMAND LINE - TOTAL COMMANDS %d\n"
#define CMD_WARN_NO_CMD     "warning: no commands provided\n"
#define CMD_ERR_PIPE_LIMIT  "error: piping limited to %d commands\n"
#define CMD_ERR_MEMORY_INIT "error: unable to initialize memory for buffers.\n"
#define CMD_OR_ARGS_TOO_BIG "error: exe size is too large.\n"
#define CMD_OR_ARGS_BAD "error: arg(s) are too large or maximum number of args exceeded.\n"
#define CMD_ERR_BUILD_CLIST "error: cannot build command list. Unable to allocate memory for command list.\n"
#define CMD_ACCESS_DENIED "error: access denied.\n"
#define CMD_NOT_DIR "error: not a directory.\n"
#define CMD_NO_FILE_DIR "error: no such file or directory.\n"
#define CMD_ERR_MEM "error: not enough space/cannot allocate memory.\n"

#endif