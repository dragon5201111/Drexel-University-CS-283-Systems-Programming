#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "dshlib.h"

const char * DRAGON_ASCII_CMP[] = {
    "72 1@4%23 ",
    "69 6%25 ",
    "68 6%26 ",
    "65 1%1 7%11 1@14 ",
    "64 10%8 7%11 ",
    "39 7%2 4%1@9 12%1@4 6%2 1@4%8 ",
    "34 22%6 28%10 ",
    "32 26%3 12%1 15%11 ",
    "31 29%1 19%5 3%12 ",
    "29 28%1@1 1@18%8 2%12 ",
    "28 33%1 22%16 ",
    "28 58%14 ",
    "28 50%1@6%1@14 ",
    "6 8%1@11 16%8 26%6 2%16 ",
    "4 13%9 2%1@12%11 11%1 12%6 1@1%16 ",
    "2 10%3 3%8 14%12 24%24 ",
    "1 9%7 1%9 13%13 12%1@11%23 ",
    "9%1@16 1%1 13%12 1@25%21 ",
    "8%1@17 2%1@12%12 1@28%18 ",
    "7%1@19 15%11 33%14 ",
    "10%18 15%10 35%6 4%2 ",
    "9%1@19 1@14%9 12%1@1 4%1 17%3 8%",
    "10%18 17%8 13%6 18%1 9%",
    "9%1@2%1@16 16%1@7 14%5 24%2 2%",
    "1 10%18 1%1 14%1@8 14%3 26%1 2%",
    "2 12%2 1@11 18%8 40%2 3%1 ",
    "3 13%1 2%2 1%2 1%1@1 18%10 37%4 3%1 ",
    "4 18%1 22%11 1@31%4 7%1 ",
    "5 39%14 28%8 3%3 ",
    "6 1@35%18 25%15 ",
    "8 32%22 19%2 7%10 ",
    "11 26%27 15%2 1@9%9 ",
    "14 20%11 1@1%1@1%18 1@18%3 3%8 ",
    "18 15%8 10%20 15%4 1%9 ",
    "16 36%22 14%12 ",
    "16 26%2 4%1 3%22 10%2 3%1@10 ",
    "21 19%1 6%1 2%26 13%1@10 ",
    "81 7%1@7 "
};

/*
 * Implement your main function by building a loop that prompts the
 * user for input.  Use the SH_PROMPT constant from dshlib.h and then
 * use fgets to accept user input.  Since we want fgets to also handle
 * end of file so we can run this headless for testing we need to check
 * the return code of fgets.  I have provided an example below of how
 * to do this assuming you are storing user input inside of the cmd_buff
 * variable.
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
 *
 *   Expected output:
 *
 *      CMD_OK_HEADER      if the command parses properly. You will
 *                         follow this by the command details
 *
 *      CMD_WARN_NO_CMD    if the user entered a blank command
 *      CMD_ERR_PIPE_LIMIT if the user entered too many commands using
 *                         the pipe feature, e.g., cmd1 | cmd2 | ... |
 *
 *  See the provided test cases for output expectations.
 */
int main()
{
    char *cmd_buff = (char *) malloc(SH_CMD_MAX * sizeof(char));

    if(cmd_buff == NULL){
        printf("Error allocating memory for cmd buffer.\n");
        exit(ERR_CMD_OR_ARGS_TOO_BIG);
    }

    command_list_t clist;

    while (1)
    {
        // Print prompt
        printf("%s", SH_PROMPT);

        if (fgets(cmd_buff, SH_CMD_MAX, stdin) == NULL)
        {
           printf("\n");
           break;
        }

        // remove the trailing \n from cmd_buff
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

        // Empty command
        if (!cmd_buff[0]) {
            printf(CMD_WARN_NO_CMD); 
            continue;
        }

        int pipe_cnt = count_pipes(cmd_buff);

        // Max pipe limit reached
        if(pipe_cnt >= CMD_MAX){
            printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
            continue;
        }

        // Dragon cmd
        if(strcmp(cmd_buff, DRAGON_CMD) == 0){
            int n_lines = sizeof(DRAGON_ASCII_CMP) / sizeof(DRAGON_ASCII_CMP[0]);
            print_dragon_cmp(DRAGON_ASCII_CMP, n_lines);
            continue;
        }


        // Exit cmd
        if(strcmp(cmd_buff, EXIT_CMD) == 0){
            free(cmd_buff);
            exit(OK);
        }

        // Print cmd list
        if(build_cmd_list(cmd_buff, &clist) == OK){
            print_cmd_list(clist);
        }

    }

    free(cmd_buff);
}

int count_pipes(char * cmd_buff){
    int pipe_cnt = 0;

    // Count pipes
    while(*cmd_buff){
        if(*cmd_buff == PIPE_CHAR) pipe_cnt++;
        cmd_buff++;
    }
    
    return pipe_cnt;
}

void print_cmd_list(struct command_list clist){
    printf(CMD_OK_HEADER, clist.num);

    for (int i = 0; i < clist.num; i++)
    {
        command_t current_cmd = clist.commands[i];
        
        printf(CMD_EXE_PRINT, (i+1), current_cmd.exe);

        if(current_cmd.args[0] != '\0'){
            printf(CMD_ARGS_PRINT, current_cmd.args);
        }
        
        printf("\n");
    }
}

void print_dragon_cmp(const char ** dragon_ascii_cmp, int n_lines) {
    const char * current_line;
    
    for (int i = 0; i < n_lines; i++) {
        current_line = dragon_ascii_cmp[i];

        while (*current_line) {
            char int_s[INT_DIGITS_BOUND];
            long unsigned k = 0;

            while (isdigit(*current_line) && k < INT_DIGITS_BOUND) {
                int_s[k++] = *current_line;
                current_line++;
            }

            int_s[k] = '\0';
            int p_times = atoi(int_s);

            if (*current_line) {
                char c = *(current_line++);

                for (int j = 0; j < p_times; j++) {
                    printf("%c", c);
                }
            }
        }
        
        printf("\n");
    }
}
