#include <stdlib.h>
#include <sys/prctl.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include "dshlib.h"
#include "dragon.h"

/*
 * Implement your exec_local_cmd_loop function by building a loop that prompts the 
 * user for input.  Use the SH_PROMPT constant from dshlib.h and then
 * use fgets to accept user input.
 * 
 *      while(1){
 *        printf("%s", SH_PROMPT);
 *        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
 *           printf("\n");
 *           break;
 *        }
 *        //remove the trailing \n from cmd_buff
 *        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';
 * 
 *        //IMPLEMENT THE REST OF THE REQUIREMENTS
 *      }
 * 
 *   Also, use the constants in the dshlib.h in this code.  
 *      SH_CMD_MAX              maximum buffer size for user input
 *      EXIT_CMD                constant that terminates the dsh program
 *      SH_PROMPT               the shell prompt
 *      OK                      the command was parsed properly
 *      WARN_NO_CMDS            the user command was empty
 *      ERR_TOO_MANY_COMMANDS   too many pipes used
 *      ERR_MEMORY              dynamic memory management failure
 * 
 *   errors returned
 *      OK                     No error
 *      ERR_MEMORY             Dynamic memory management failure
 *      WARN_NO_CMDS           No commands parsed
 *      ERR_TOO_MANY_COMMANDS  too many pipes used
 *   
 *   console messages
 *      CMD_WARN_NO_CMD        print on WARN_NO_CMDS
 *      CMD_ERR_PIPE_LIMIT     print on ERR_TOO_MANY_COMMANDS
 *      CMD_ERR_EXECUTE        print on execution failure of external command
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 1+)
 *      malloc(), free(), strlen(), fgets(), strcspn(), printf()
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 2+)
 *      fork(), execvp(), exit(), chdir()
 */
int exec_local_cmd_loop()
{
    char *cmd_buff = (char *) malloc(SH_CMD_MAX * sizeof(char));
    cmd_buff_t cmd;
    int rc = 0;


    if(cmd_buff == NULL){
        perror("Unable to allocate memory for cmd_buff.\n");
        return ERR_MEMORY;
    }

    if((alloc_cmd_buff(&cmd)) == ERR_MEMORY){
        perror("Unable to allocate memory for cmd.\n");
        return ERR_MEMORY;
    }
    
    while(1){
        printf("%s", SH_PROMPT);
        
        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
            printf("\n");
            break;
        }

        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';

        if((rc = build_cmd_buff(cmd_buff, &cmd)) == WARN_NO_CMDS){
            printf(CMD_WARN_NO_CMD);
            continue;
        }else if(rc == ERR_MEMORY){
            return ERR_MEMORY;
        }else if(rc == ERR_CMD_OR_ARGS_TOO_BIG){
            printf("Too many arguments supplied.\n");
            continue;
        }

        // rc should be OK at this point.
        
        // TODO IMPLEMENT if built-in command, execute builtin logic for exit, cd (extra credit: dragon)
        // the cd command should chdir to the provided directory; if no directory is provided, do nothing
        Built_In_Cmds rc_built_in;
        if((rc_built_in = match_command(cmd.argv[0])) != BI_NOT_BI){
            // Command failed to execute
            if((rc_built_in = exec_built_in_cmd(&cmd, rc_built_in)) == BI_N_EXECUTED){
                printf(CMD_ERR_EXECUTE, cmd.argv[0]);
            }else if(rc_built_in == BI_CMD_EXIT){
                break;
            }
        }else{

            //Maintain pointer to last arg, and set to null
            char * last_arg = cmd.argv[cmd.argc];
            cmd.argv[cmd.argc] = NULL;

            if((rc = exec_cmd(&cmd)) == ERR_EXEC_CMD){
                printf(CMD_ERR_EXECUTE, cmd.argv[0]);
            }

            cmd.argv[cmd.argc] = last_arg;
        }

        //Clear cmd
        if(clear_cmd_buff(&cmd) == ERR_MEMORY){
            printf(CMD_ERR_CLEAR);
        }
    }

    free_cmd_buff(&cmd);
    free(cmd_buff);
    return OK;
}

/*
Returns:
    ERR_EXEC_CMD - on fork or exec failure
    child return code - on success or failure
*/
int exec_cmd(cmd_buff_t *cmd){
    int f_result = fork();

    // Fork failed
    if(f_result < 0){
        return ERR_EXEC_CMD;
    }

    // Child process
    if(f_result == 0){
        prctl(PR_SET_PDEATHSIG, SIGKILL);

        int rc = execvp(cmd->argv[0], cmd->argv);
        
        if(rc < 0){
            exit(errno);
        }
    }else{
        int status;
        wait(&status);
        return WEXITSTATUS(status);
    }
}

/*
Returns:
    ERR_MEMORY - nothing to clear or an error occured while trying to clear
    OK - successfully cleared cmd_buff
*/
int clear_cmd_buff(cmd_buff_t *cmd_buff){
    if(cmd_buff == NULL) return ERR_MEMORY;


    if(cmd_buff->_cmd_buffer != NULL){
        memset(cmd_buff->_cmd_buffer, 0, strlen(cmd_buff->_cmd_buffer) + 1);
    }
    
    
    for (int i = 0; i < cmd_buff->argc; i++) {
        if (cmd_buff->argv[i] != NULL) {
            memset(cmd_buff->argv[i], 0, strlen(cmd_buff->argv[i]) + 1);
        }
    }
    
    cmd_buff->argc = 0;
    return OK;
}

/*
Returns:
    BI_EXECUTED - on success
    BI_N_EXECUTED - on fail
*/
Built_In_Cmds exec_cd_cmd(char * path){
    // Ignore no arguments
    if(strlen(path) == 0) return BI_EXECUTED;

    if(chdir(path) == -1){
        return BI_N_EXECUTED;
    }
    return BI_EXECUTED;
}

/*
Returns:
    BI_EXECUTED - if cmd was executed successfully
    BI_N_EXECUTED - if cmd was not executed successfully
    BI_CMD_EXIT - if cmd was exit, to signal main loop to break
*/
Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd,  Built_In_Cmds built_in){
    Built_In_Cmds rc;
    switch (built_in)
    {
        case BI_CMD_DRAGON:
            rc = print_dragon();
            break;
        case BI_CMD_EXIT:
            rc = BI_CMD_EXIT;
            break;
        case BI_CMD_CD:
            rc = exec_cd_cmd(cmd->argv[1]);
            break;
        default:
            rc = BI_N_EXECUTED;
    }

    return rc;
}

/*
Returns:
    If a input char * is a match to one of these, it returns the match

    BI_CMD_EXIT,
    BI_CMD_DRAGON,
    BI_CMD_CD,
    BI_RC

    otherwise, it returns:
    BI_NOT_BI

*/
Built_In_Cmds match_command(const char *input){
    if(strcmp(input, EXIT_CMD) == 0){
        return BI_CMD_EXIT;
    }else if(strcmp(input, DRAGON_CMD) == 0){
        return BI_CMD_DRAGON;
    }else if(strcmp(input, CD_CMD) == 0){
        return BI_CMD_CD;
    }else if(strcmp(input, RC_CMD) == 0){
        return BI_RC;
    }
    return BI_NOT_BI;
}

/*
Returns:
    OK - on allocated successfully
    ERR_MEMRORY - on memory allocation failure
*/
int alloc_cmd_buff(cmd_buff_t *cmd_buff){
    // Empty cmd_buff
    if(cmd_buff == NULL) return ERR_MEMORY;

    cmd_buff->argc = 0;
    
    if((cmd_buff->_cmd_buffer = (char *) malloc(sizeof(char) * (SH_CMD_MAX + 1))) == NULL){
        return ERR_MEMORY;
    }

    // Allocate for exe
    if((cmd_buff->argv[0] = (char *) malloc(sizeof(char) * (EXE_MAX + 1))) == NULL){
        free_cmd_buff(cmd_buff);
        return ERR_MEMORY;
    }

    memset(cmd_buff->argv[0], NULL_BYTE, EXE_MAX + 1);

    // Allocate for args
    for(int i = 1; i < CMD_ARGV_MAX; i++){
        if((cmd_buff->argv[i] = (char *) malloc(sizeof(char) * (ARG_MAX + 1))) == NULL){
            free_cmd_buff(cmd_buff);
            return ERR_MEMORY;
        }
        memset(cmd_buff->argv[i], NULL_BYTE, ARG_MAX + 1);
    }
    return OK;
}


/*
Returns:
    OK - on deallocation success
    ERR_MEMORY - on deallocation failure
*/
int free_cmd_buff(cmd_buff_t *cmd_buff){
    if(cmd_buff == NULL) return ERR_MEMORY;

    if(cmd_buff->_cmd_buffer != NULL){
        free(cmd_buff->_cmd_buffer);
    }

    for(int i = 0; i < CMD_ARGV_MAX; i++){
        if(cmd_buff->argv[i] != NULL){
            free(cmd_buff->argv[i]);
        }
    }

    cmd_buff->argc = 0;
    return OK;
}

/*
    Returns:
        OK - on building cmd_buff successfully
        ERR_MEMORY - memory allocation failed
        WARN_NO_CMDS - empty command or command of only whitespace
        ERR_CMD_OR_ARGS_TOO_BIG - exe or args were too large/exceeded maximum size
*/
int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff){
    char * formatted_cmd_line = NULL;
    int formatted_cmd_line_len = format_cmd_line(&formatted_cmd_line, cmd_line, strlen(cmd_line));

    if(formatted_cmd_line_len == WARN_NO_CMDS){
        return WARN_NO_CMDS;
    }else if(formatted_cmd_line_len == ERR_MEMORY){
        return ERR_MEMORY;
    }

    cmd_buff->argc = 0;
    strcpy(cmd_buff->_cmd_buffer, formatted_cmd_line);

    char *arg_start = formatted_cmd_line;

    for (int i = 0; i <= formatted_cmd_line_len; i++) {
        if (formatted_cmd_line[i] == NULL_BYTE) {
            int arg_start_len = strlen(arg_start);

            if(!can_insert_cmd_buff_argv(cmd_buff, arg_start_len)){
                free(formatted_cmd_line);
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }
            
            strcpy(cmd_buff->argv[cmd_buff->argc++], arg_start);
            arg_start = &formatted_cmd_line[i + 1];
        }
    }


    free(formatted_cmd_line);
    return OK;
}

int can_insert_cmd_buff_argv(cmd_buff_t *cmd_buff, int arg_len){
    if(cmd_buff->argc == 0){
        return arg_len <= EXE_MAX;
    }

    // Does not exceed arg count and arg_len <= arg max size
    return (cmd_buff->argc + 1 <= CMD_ARGV_MAX) && (arg_len <= ARG_MAX);
}

/*
Returns:
    WARN_NO_CMDS - there is nothing to format
    ERR_MEMORY - Error allocating memory
    Size of formatted line - if successful
*/
int format_cmd_line(char ** dest, char *cmd_line, int cmd_line_len) {
    if (cmd_line_len == 0 || cmd_line == NULL) {
        return WARN_NO_CMDS;
    }

    int head_offset = 0;
    int tail_offset = 0;
    char *cmd_line_p = cmd_line;
    
    // Skip leading spaces
    while (head_offset < cmd_line_len && isspace(*cmd_line_p)) {
        cmd_line_p++;
        head_offset++;
    }

    cmd_line_p = &cmd_line[cmd_line_len - 1];
    
    // Skip trailing spaces
    while (cmd_line_p >= cmd_line && isspace(*cmd_line_p)) {
        tail_offset++;
        cmd_line_p--;
    }

    int new_cmd_buff_len = cmd_line_len - head_offset - tail_offset;
    if (new_cmd_buff_len <= 0) {
        return WARN_NO_CMDS;
    }

    char *new_cmd_buff = (char *) malloc(new_cmd_buff_len + 1);
    if (new_cmd_buff == NULL) {
        return ERR_MEMORY;
    }

    int i = 0;
    int inside_quotes = 0;
    int prev_was_space = 0;

    for (cmd_line_p = cmd_line + head_offset; cmd_line_p < cmd_line + cmd_line_len - tail_offset; cmd_line_p++) {
        if (*cmd_line_p == QUOTE_CHAR) {
            inside_quotes = !inside_quotes;
            continue;
        }

        if (isspace(*cmd_line_p)) {
            if (inside_quotes) {
                new_cmd_buff[i++] = *cmd_line_p;
                prev_was_space = 0;
            } else {
                if (prev_was_space) {
                    continue;
                }
                prev_was_space = 1;
                new_cmd_buff[i++] = NULL_BYTE;
            }
        } else {
            new_cmd_buff[i++] = *cmd_line_p;
            prev_was_space = 0;
        }
    }

    new_cmd_buff[i] = NULL_BYTE;

    *dest = new_cmd_buff;

    return i;
}
