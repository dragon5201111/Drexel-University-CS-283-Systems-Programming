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
    int rc = OK;
    // TODO: Add set rc
    
    if(cmd_buff == NULL){
        perror(CMD_ERR_MEMORY_INIT);
        return ERR_MEMORY;
    }

    // Structure to hold individual commands
    command_list_t command_list;

    while(1){
        printf("%s", SH_PROMPT);
        
        if(read_stream_into_buff(cmd_buff, SH_CMD_MAX, stdin) == ERR_MEMORY){
            break;
        }

        flush_or_remove_new_line_buff(cmd_buff);


        if((rc = build_cmd_list(cmd_buff, &command_list)) != OK){
            // TODO: Add setting rc
            print_err_build_cmd_list(rc);
            free_cmd_list(&command_list);
            continue;
        }
        
        // Debug to print command_list
        _print_cmd_list(&command_list);
        // TODO:
        //- Execute pipeline
        //- Implement RC
        free_cmd_list(&command_list);
    }

    free(cmd_buff);
    return OK;
}

/*
Returns:
    OK - if stream was read successfully into buff
    ERR_MEMORY - if stream was not successfully read into buff
*/
int read_stream_into_buff(char * buff, int max, FILE * stream){
    if (fgets(buff, max, stream) == NULL){
        printf("\n");
        return ERR_MEMORY;
    }

    return OK;
}

void print_err_build_cmd_list(int rc){
    switch(rc) {
        case WARN_NO_CMDS:
            fprintf(stderr, CMD_WARN_NO_CMD);
            break;
        case ERR_TOO_MANY_COMMANDS:
            fprintf(stderr, CMD_ERR_PIPE_LIMIT, CMD_MAX);
            break;
        case ERR_MEMORY:
            fprintf(stderr, CMD_ERR_BUILD_CLIST);
            break;
        case ERR_CMD_ARGS_BAD:
            fprintf(stderr, CMD_OR_ARGS_BAD);
            break;
        case ERR_CMD_OR_ARGS_TOO_BIG:
            fprintf(stderr, CMD_OR_ARGS_TOO_BIG);
            break;
    }
}

// Flushes stdin on overflow so commands don't flow over to next command
void flush_or_remove_new_line_buff(char * buff){
    if (strchr(buff, '\n') == NULL) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
    } else {
        buff[strcspn(buff, "\n")] = '\0';
    }
}

// Debug function to print cmd list
void _print_cmd_list(command_list_t * clist){
    if(clist == NULL || clist->num <= 0) return;

    printf("=============================\n");
    printf("Clist Debug print:\n");
    printf("Total commands in clist: %d\n\n", clist->num);
    
    cmd_buff_t current_cmd;
    int arg_c;

    for(int i = 0; i < clist->num; i++){
        current_cmd = clist->commands[i];
        arg_c = current_cmd.argc;

        printf("Command <%d>: %s\n", i+1, current_cmd.argv[0]);
        printf("Args: [");

        
        for(int j = 1; j < arg_c; j++){
            printf((j + 1 == arg_c)? ("%s") : ("%s,"), current_cmd.argv[j]);
        }

        printf((i+1 == clist->num) ? "]\n" : "]\n\n");
    }
    printf("=============================\n");

}

/*
Returns:
    OK - on deallocation success
    ERR_MEMORY - on deallocation failure
*/
int clear_cmd_buff(cmd_buff_t *cmd_buff) {
    if (cmd_buff == NULL) return ERR_MEMORY; 

    if (cmd_buff->_cmd_buffer != NULL) {
        free(cmd_buff->_cmd_buffer);
    }

    return OK;
}

/*
Returns:
    OK - on deallocation success
    ERR_MEMORY - on deallocation failure
*/
int free_cmd_list(command_list_t *cmd_lst){
    if(cmd_lst == NULL || cmd_lst->num <= 0) return ERR_MEMORY;

    for(int i = 0; i < cmd_lst->num; i++){
        clear_cmd_buff(&cmd_lst->commands[i]);
    }

    cmd_lst->num = 0;
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
    int rc_build_cmd_buff;

    int num_commands = 0;
    char * command_token = strtok(cmd_line, PIPE_STRING);
    while(command_token != NULL){        
       
        if((rc_build_cmd_buff = build_cmd_buff(command_token,  &clist->commands[num_commands])) == WARN_NO_CMDS){
            return WARN_NO_CMDS;
        }else if(rc_build_cmd_buff == ERR_MEMORY){
            return ERR_MEMORY;
        }else if(rc_build_cmd_buff == ERR_CMD_OR_ARGS_TOO_BIG){
            return ERR_CMD_OR_ARGS_TOO_BIG;
        }else if(rc_build_cmd_buff == ERR_CMD_ARGS_BAD){
            return ERR_CMD_ARGS_BAD;
        }

        num_commands++;
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

            strcpy(cmd_buff->argv[cmd_buff->argc++], arg_start);
            arg_start = &cmd_buff->_cmd_buffer[i+1];
        }
    }

    return OK;
}

/*
Returns:
    OK
    ERR_CMD_OR_ARGS_TOO_BIG
    ERR_CMD_ARGS_BAD
*/
int can_insert_cmd_buff_argv(cmd_buff_t *cmd_buff, int arg_len){
    if(cmd_buff->argc == 0){
        return (arg_len < EXE_MAX) ? OK: ERR_CMD_OR_ARGS_TOO_BIG;
    }
    return ((cmd_buff->argc + 1 <= CMD_ARGV_MAX) && (arg_len < ARG_MAX)) ? OK : ERR_CMD_ARGS_BAD;
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

