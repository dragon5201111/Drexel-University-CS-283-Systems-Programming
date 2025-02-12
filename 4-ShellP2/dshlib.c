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

    if(cmd_buff == NULL){
        perror("Unable to allocate memory for cmd_buff.\n");
        return ERR_MEMORY;
    }

    //int rc = 0;
    cmd_buff_t cmd;

    // TODO IMPLEMENT MAIN LOOP
    while(1){
        printf("%s", SH_PROMPT);
        
        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
            printf("\n");
        break;
        }

        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';

        // TODO IMPLEMENT parsing input to cmd_buff_t *cmd_buff
        if(parse_cmd_buff(&cmd_buff, strlen(cmd_buff)) == WARN_NO_CMDS){
            printf(CMD_WARN_NO_CMD);
            continue;
        }

        printf("After parsing-%s\n", cmd_buff);
        // TODO IMPLEMENT if built-in command, execute builtin logic for exit, cd (extra credit: dragon)
        // the cd command should chdir to the provided directory; if no directory is provided, do nothing

        // TODO IMPLEMENT if not built-in command, fork/exec as an external command
        // for example, if the user input is "ls -l", you would fork/exec the command "ls" with the arg "-l"
        
    }

    free(cmd_buff);
    return OK;
}

// CHANGE SO IT DOESN'T OVERWRITE CMD_BUFF
int parse_cmd_buff(char **cmd_buff, int cmd_buff_len) {
    if (cmd_buff_len == 0 || *cmd_buff == NULL) {
        return WARN_NO_CMDS;
    }

    int head_offset = 0;
    int tail_offset = 0;
    char *cmd_buff_p = *cmd_buff;
    
    while (head_offset < cmd_buff_len && isspace(*cmd_buff_p)) {
        cmd_buff_p++;
        head_offset++;
    }

    cmd_buff_p = *cmd_buff + cmd_buff_len - 1;
    
    while (cmd_buff_p >= *cmd_buff && isspace(*cmd_buff_p)) {
        tail_offset++;
        cmd_buff_p--;
    }

    int new_cmd_buff_len = cmd_buff_len - head_offset - tail_offset;
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

    for (cmd_buff_p = *cmd_buff + head_offset; cmd_buff_p < *cmd_buff + cmd_buff_len - tail_offset; cmd_buff_p++) {
        if (*cmd_buff_p == QUOTE_CHAR) {
            inside_quotes = !inside_quotes;
        }


        if (isspace(*cmd_buff_p)) {
            if (inside_quotes) {
                new_cmd_buff[i++] = *cmd_buff_p;
                prev_was_space = 0;
            } else {
                if (prev_was_space) {
                    continue;
                }
                prev_was_space = 1;
                new_cmd_buff[i++] = *cmd_buff_p;
            }
        } else {
            new_cmd_buff[i++] = *cmd_buff_p;
            prev_was_space = 0;
        }
    }

    new_cmd_buff[i] = '\0';

    free(*cmd_buff);
    *cmd_buff = new_cmd_buff;

    return new_cmd_buff_len;
}
