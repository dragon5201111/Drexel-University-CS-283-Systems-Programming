#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dshlib.h"

/*
 *  build_cmd_list
 *    cmd_line:     the command line from the user
 *    clist *:      pointer to clist structure to be populated
 *
 *  This function builds the command_list_t structure passed by the caller
 *  It does this by first splitting the cmd_line into commands by spltting
 *  the string based on any pipe characters '|'.  It then traverses each
 *  command.  For each command (a substring of cmd_line), it then parses
 *  that command by taking the first token as the executable name, and
 *  then the remaining tokens as the arguments.
 *
 *  NOTE your implementation should be able to handle properly removing
 *  leading and trailing spaces!
 *
 *  errors returned:
 *
 *    OK:                      No Error
 *    ERR_TOO_MANY_COMMANDS:   There is a limit of CMD_MAX (see dshlib.h)
 *                             commands.
 *    ERR_CMD_OR_ARGS_TOO_BIG: One of the commands provided by the user
 *                             was larger than allowed, either the
 *                             executable name, or the arg string.
 *
 *  Standard Library Functions You Might Want To Consider Using
 *      memset(), strcmp(), strcpy(), strtok(), strlen(), strchr()
 */
int build_cmd_list(char *cmd_line, command_list_t *clist)
{
    if(!cmd_line || !clist){
        return ERR_CMD_OR_ARGS_TOO_BIG;
    }

    char * token;
    int cmd_cnt = 0;

    token = strtok(cmd_line, PIPE_STRING);

    while(token != NULL){

        int token_len = strlen(token); 
        char token_cpy[token_len + 1];

        int token_cpy_len = strip_token(token_cpy, token, token_len);

        printf("Full CMD: %s, CMD #: %d, Size: %d\n", token_cpy, cmd_cnt, token_cpy_len);


        command_t * current_cmd = &clist->commands[cmd_cnt];
        
        char * first_space = strchr(token_cpy, SPACE_CHAR);
        int has_args = 0;

        if(first_space != NULL) has_args = 1;
        
        if(has_args){
            // Check exe size
            // Check arg size
        }else{
            // Check exe size
            if(token_cpy_len + 1 > EXE_MAX){
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }

            memcpy(current_cmd->exe, token_cpy, token_cpy_len+1);
            // FINISH HERE
        }

        
        cmd_cnt++;
        token = strtok(NULL, PIPE_STRING);
    }



    return OK;
}

// Remove trailing, and leading whitespace. Also ensures tokens have one space between them.
int strip_token(char * dest, char * token, int token_len){
    int head_offset = 0;
    int tail_offset = 0;
    char * token_p = token;
        
    while(isspace(*token_p) && head_offset < token_len){
        token_p++;
        head_offset++;
    }

    token_p = token + token_len - 1;
    
    while(token_p >= token && isspace(*token_p)){
        tail_offset++;
        token_p--;
    }

    token_p = (token + head_offset);

    int i = 0;
    int prev_was_space = 0;

    for(; token_p <= (token + token_len - tail_offset - 1); token_p++){
        if(isspace(*token_p)){
            if (prev_was_space) {
                continue;
            }
            prev_was_space = 1;
        } else {
            prev_was_space = 0;
        }
        dest[i++] = *token_p;
    }

    dest[i] = '\0';
    return i;
}