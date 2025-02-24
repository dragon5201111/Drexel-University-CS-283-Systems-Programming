#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "dshlib.h"

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
    int rc = 0;
    
    if(cmd_buff == NULL){
        perror(CMD_ERR_MEMORY_INIT);
        return ERR_MEMORY;
    }

    // Structure to hold individual commands
    command_list_t command_list;

    while(1){
        printf("%s", SH_PROMPT);
        
        if (fgets(cmd_buff, SH_CMD_MAX, stdin) == NULL){
            printf("\n");
            break;
        }
        
        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';

        if((rc = build_cmd_list(cmd_buff, &command_list)) == WARN_NO_CMDS){
            fprintf(stderr, CMD_WARN_NO_CMD);
            continue;
        }else if(rc == ERR_TOO_MANY_COMMANDS){
            fprintf(stderr, CMD_ERR_PIPE_LIMIT, CMD_MAX);
            continue;
        }else if(rc == ERR_MEMORY){
            fprintf(stderr, CMD_ERR_BUILD_CLIST);
            free_cmd_list(&command_list);
            continue;
        }else if(rc == ERR_CMD_ARGS_BAD){
            fprintf(stderr, CMD_OR_ARGS_TOO_BIG);
            free_cmd_list(&command_list);
            continue;
        }else if(rc == ERR_CMD_OR_ARGS_TOO_BIG){
            fprintf(stderr, CMD_OR_ARGS_TOO_BIG);
            free_cmd_list(&command_list);
            continue;
        }
        
    }
    
    free(cmd_buff);
    return OK;
}


/*
Returns:
    WARN_NO_CMDS - Empty command line
    ERR_TOO_MANY_COMMANDS - Pipe limit was reached 
    ERR_CMD_OR_ARGS_TOO_BIG 
    ERR_CMD_ARGS_BAD
    ERR_MEMORY - On allocation error
    OK - Command list was built successfully
*/
int build_cmd_list(char *cmd_line, command_list_t *clist){
    // Empty command line
    int cmd_line_len = strlen(cmd_line);
    if(cmd_line_len == 0) return WARN_NO_CMDS;

    // Count pipes
    int pipe_cnt = 0;
    for(int i = 0; i < cmd_line_len; i++)
        if(cmd_line[i] == PIPE_CHAR) pipe_cnt++;
    
    // Pipe limit exceeded
    if(pipe_cnt >= CMD_MAX) return ERR_TOO_MANY_COMMANDS;

    // Parse individual commands
    cmd_buff_t * cmd_buff;
    int rc_build_cmd_buff;

    int num_commands = 0;
    char * command_token = strtok(cmd_line, PIPE_STRING);
    while(command_token != NULL){        
        // Allocate for cmd_buff
        cmd_buff = (cmd_buff_t *) malloc(sizeof(cmd_buff_t));
        if(cmd_buff == NULL) return ERR_MEMORY;
       
        if((rc_build_cmd_buff = build_cmd_buff(command_token, cmd_buff)) == WARN_NO_CMDS){
            return WARN_NO_CMDS;
        }else if(rc_build_cmd_buff == ERR_MEMORY){
            return ERR_MEMORY;
        }else if(rc_build_cmd_buff == ERR_CMD_OR_ARGS_TOO_BIG){
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }else if(rc_build_cmd_buff == ERR_CMD_ARGS_BAD){
            return ERR_CMD_ARGS_BAD;
        }

        // Populate clist with parsed commands
        clist->commands[num_commands++] = *cmd_buff;
        command_token = strtok(NULL, PIPE_STRING);
    }

    clist->num = num_commands;
    return OK;
}

/*
Returns:
    OK - if cmd_buff was built successfully
    CMD_WARN_NO_CMD - if cmd_line was empty
    ERR_MEMORY - if any memory issues arise
    ERR_CMD_OR_ARGS_TOO_BIG - exe size exceeds maximum
    ERR_CMD_ARGS_BAD - arg count is too large or arg size exceeds maximum
*/
int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff){
    int cmd_line_len = strlen(cmd_line);
    if(cmd_line_len == 0) return WARN_NO_CMDS;

    // Initialize to zero
    cmd_buff->argc = 0;
    
    // Copy formatted cmd_line
    int formatted_cmd_line_len;
    if((formatted_cmd_line_len = format_cmd_line(&cmd_buff->_cmd_buffer, cmd_line, cmd_line_len)) ==  ERR_MEMORY) 
        return ERR_MEMORY;

    // Debug to print after formatting
    //printf("After formatting:%s, Length:%d\n", cmd_buff->_cmd_buffer, formatted_cmd_line_len);

    // Copy args into cmd_buff
    char * arg_start = cmd_buff->_cmd_buffer;
    int can_insert = 0;
    
    for(int i = 0; i <= formatted_cmd_line_len; i++){
        if(cmd_buff->_cmd_buffer[i] == NULL_BYTE){
            // Debug to print arg
            //printf("Arg:%s\n", arg_start);
            if((can_insert = can_insert_cmd_buff_argv(cmd_buff, strlen(arg_start))) != OK){
                return can_insert;
            }

            cmd_buff->argv[cmd_buff->argc++] = strdup(arg_start);

            if(cmd_buff->argv[cmd_buff->argc - 1] == NULL) return ERR_MEMORY;
            arg_start = &cmd_buff->_cmd_buffer[i+1];
        }
    }

    printf("Command: %s\n", cmd_buff->argv[0]);
    for(int i = 1; i < cmd_buff->argc; i++){
        printf("Arg: %s\n", cmd_buff->argv[i]);
    }

    return OK;
}


int can_insert_cmd_buff_argv(cmd_buff_t *cmd_buff, int arg_len){
    if(cmd_buff->argc == 0){
        return (arg_len <= EXE_MAX) ? OK: ERR_CMD_OR_ARGS_TOO_BIG;
    }
    // Does not exceed arg count and arg_len <= arg max size
    return ((cmd_buff->argc + 1 <= CMD_ARGV_MAX) && (arg_len <= ARG_MAX)) ? OK : ERR_CMD_ARGS_BAD;
}



/*
Returns:
    ERR_MEMORY on any memory issues
    Destination size on success
*/
int format_cmd_line(char **dest, char *src, int src_len) {
    int start = 0;
    while (start < src_len && isspace(src[start])) start++;

    int end = src_len - 1;
    while (end > start && isspace(src[end])) end--;

    int max_len = end - start + 2;
    char *formatted = (char *)malloc(max_len);
    if (!formatted) return ERR_MEMORY;

    int i = start, j = 0;
    int in_quotes = 0;
    int last_was_space = 0;

    while (i <= end) {
        char current = src[i];

        if (current == QUOTE_CHAR) {
            in_quotes = !in_quotes;
            formatted[j++] = current;
            last_was_space = 0;
        } else if (isspace((unsigned char)current)) {
            if (in_quotes) {
                formatted[j++] = current;
            } else if (!last_was_space) {
                // Delimitter for args
                formatted[j++] = NULL_BYTE;
                last_was_space = 1;
            }
        } else {
            formatted[j++] = current;
            last_was_space = 0;
        }
        i++;
    }

    formatted[j] = '\0';
    *dest = formatted;
    return j;
}

