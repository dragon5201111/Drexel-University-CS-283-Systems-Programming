#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/prctl.h>
#include <signal.h>
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
    int rc = OK;
    int cmd_rc = OK;
        
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

        // assignment unit test "Pipes" only works with this here for some reason
        //printf("%s", SH_PROMPT);

        rc = build_cmd_list(cmd_buff, &command_list, NULL_BYTE);

        if(rc != OK){
            print_err_build_cmd_list(rc);
            continue;
        }

        // Debug to print command_list
        #ifdef DSH_DEBUG
            _print_cmd_list(&command_list);
        #endif
        // TODO:
        // Print RC after execution (errors only)
       
        cmd_buff_t first_cmd = command_list.commands[0];
        Built_In_Cmds bi_type = match_command(first_cmd.argv[0]);

        if(clist_has_no_pipe_redirection_and_built_in(bi_type, &command_list)){
            cmd_rc = exec_built_in_cmd(&first_cmd, bi_type, cmd_rc);
            
            // Clean-up and exit loop
            if(cmd_rc == BI_CMD_EXIT){
                free_cmd_list(&command_list);
                break;
            }

        }else{
            cmd_rc = start_supervisor_and_execute_pipeline(&command_list);
        }
        
        // Implement print_exec_rc
        print_exec_rc(cmd_rc);
        free_cmd_list(&command_list);
    }

    free(cmd_buff);
    return OK;
}

int strings_are_equal(const char * str_one, const char* str_two){
    return strcmp(str_one, str_two) == 0;
}

void print_exec_rc(int cmd_rc){
    switch (cmd_rc)
    {
    case EACCES:
        printf(CMD_ACCESS_DENIED);
        break;
    case ENOTDIR:
        printf(CMD_NOT_DIR);
        break;
    case ENOENT:
        printf(CMD_NO_FILE_DIR);
        break;
    case ENOMEM:
        printf(CMD_ERR_MEM);
    default:
        break;
    }
}

/*
Returns:
    Exit code of last child, or errno on fail
*/
int start_supervisor_and_execute_pipeline(command_list_t *clist) {
    pid_t supervisor = fork();
    int supervisor_rc;

    if (supervisor == -1) {
        perror("Fork supervisor creation failed");
        exit(errno);
    }

    if (supervisor == 0) {
        // Kill children if parent dies
        prctl(PR_SET_PDEATHSIG, SIGKILL);

        int pipe_line_rc = execute_pipeline(clist);
        exit(pipe_line_rc);
    }

    waitpid(supervisor, &supervisor_rc, 0);
    if (WIFEXITED(supervisor_rc)) {
        return WEXITSTATUS(supervisor_rc);
    } else {
        return errno;
    }
}

/*
Returns:
    Exit code of last child, or errno on fail
*/
int execute_pipeline(command_list_t *clist) {
    int num_commands = clist->num;
    int pipes[num_commands - 1][2];  // Array of pipes
    pid_t pids[num_commands];        // Array to store process IDs

    // Create all necessary pipes
    for (int i = 0; i < num_commands - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("Pipe creation failed");
            exit(errno);
        }
    }

    // Create processes for each command
    for (int i = 0; i < num_commands; i++) {
        pids[i] = fork();
        if (pids[i] == -1) {
            perror("Forking children of supervisor failed");
            exit(errno);
        }

        if (pids[i] == 0) {  // Child process
            // Kill child if parent dies
            prctl(PR_SET_PDEATHSIG, SIGKILL);
            cmd_buff_t cmd_to_exec = clist->commands[i];

            if (cmd_to_exec.input_file != NULL) {
                int in_fd = open(cmd_to_exec.input_file, O_RDONLY);
                if (in_fd == -1) {
                    exit(errno);
                }
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
            }

            if (cmd_to_exec.output_file != NULL) {
                int out_fd;
                if(cmd_to_exec.append_mode){
                    out_fd = open(cmd_to_exec.output_file, O_WRONLY | O_APPEND | O_CREAT, 0644);
                }else{
                    out_fd = open(cmd_to_exec.output_file, O_WRONLY | O_TRUNC | O_CREAT, 0644);
                }
                if (out_fd == -1) {
                    exit(errno);
                }
                dup2(out_fd, STDOUT_FILENO);
                close(out_fd);
            }

            // Set up input pipe for all except first process
            if (i > 0) {

                dup2(pipes[i-1][0], STDIN_FILENO);
            }

            if (i < num_commands - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            for (int j = 0; j < num_commands - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            Built_In_Cmds bi_type;

            if((bi_type = match_command(cmd_to_exec.argv[0])) != BI_NOT_BI){
                exit(exec_built_in_cmd(&cmd_to_exec, bi_type, errno));
            }else{
                execvp(cmd_to_exec.argv[0], cmd_to_exec.argv);
                exit(errno);
            }
        }
    }

    // Parent process: close all pipe ends
    for (int i = 0; i < num_commands - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    int pipeline_rc = OK;

    for (int i = 0; i < num_commands; i++) {
        waitpid(pids[i], &pipeline_rc, 0);
        if (WIFEXITED(pipeline_rc)) {
            pipeline_rc = WEXITSTATUS(pipeline_rc);  // Save last child's exit status
        }
    }

    exit(pipeline_rc);
}

/*
Returns:
    0 if clist is not of size 1, and bi_rc does not match to a Built_In_Cmds enum, 1 otherwise
*/
Built_In_Cmds clist_has_no_pipe_redirection_and_built_in(Built_In_Cmds bi_rc, command_list_t * clist){
    return bi_rc != BI_NOT_BI && clist->num == 1;
}

/*
Returns:
    errno upon failure, BI_EXECUTED if success
*/
Built_In_Cmds exec_cd(cmd_buff_t * cmd){
    // Ignore no arguments
    if(cmd->argc != 2) return BI_EXECUTED;
    return (chdir(cmd->argv[1]) == -1) ? errno : BI_EXECUTED;
}

/*
Returns:
    BI_NOT_BI if no built-in is matched, otherwise appropriate builtin enum
*/ 
Built_In_Cmds match_command(const char *input){
    if(strings_are_equal(input, EXIT_CMD)){
        return BI_CMD_EXIT;
    }else if(strings_are_equal(input, DRAGON_CMD)){
        return BI_CMD_DRAGON;
    }else if(strings_are_equal(input, CD_CMD)){
        return BI_CMD_CD;
    }else if(strings_are_equal(input, RC_CMD)){
        return BI_CMD_RC;
    }
    return BI_NOT_BI;
}

/*
Returns:
    BI_EXECUTED on success
    BI_CMD_EXIT when bi_type is BI_CMD_EXIT
    Otherwise, errno (from cd) if failure, else, BI_EXECUTED
*/
Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd, Built_In_Cmds bi_type, int rc){
    switch (bi_type)
    {
    case BI_CMD_EXIT:
        return BI_CMD_EXIT;
        break;
    case BI_CMD_DRAGON:
        print_dragon();
        break;
    case BI_CMD_CD:
        return exec_cd(cmd);
    case BI_CMD_RC:
        printf(RC_FORMAT, rc);
        break;
    // Ignore anything else
    default:
        break;
    }

    return BI_EXECUTED;
}

/*
Returns:
    OK - if stream was read successfully into cmd_buff
    ERR_MEMORY - if stream was not successfully read into cmd_buff
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

        printf("Command <%d>: \"%s\"\n", i+1, current_cmd.argv[0]);
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

    // Free the memory for each argument in argv
    for (int i = 0; i < cmd_buff->argc; i++) {
        free(cmd_buff->argv[i]);
    }

    // Free the command buffer
    if (cmd_buff->_cmd_buffer != NULL) {
        free(cmd_buff->_cmd_buffer);
    }

    if (cmd_buff->output_file != NULL) {
        free(cmd_buff->output_file);
    }

    if (cmd_buff->input_file != NULL) {
        free(cmd_buff->input_file);
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
int build_cmd_list(char *cmd_line, command_list_t *clist, char arg_delimitter){
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
       
        if((rc_build_cmd_buff = build_cmd_buff(command_token,  &clist->commands[num_commands], arg_delimitter)) == WARN_NO_CMDS){
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

Redirect_Flag check_if_arg_is_redirect_and_set_flag(char * arg_start, Redirect_Flag * redirect_flag){
    if(strings_are_equal(arg_start, REDIR_APPEND_STRING)){
        *redirect_flag = R_APPEND;
    }else if(strings_are_equal(arg_start, REDIR_OUTPUT_STRING)){
        *redirect_flag = R_OUTPUT;
    }else if (strings_are_equal(arg_start, REDIR_INPUT_STRING)){
        *redirect_flag = R_INPUT;
    }
    
    return *redirect_flag;
}


int redirect_flag_was_set(Redirect_Flag redirect_flag){
    return (redirect_flag == R_APPEND || redirect_flag == R_INPUT || redirect_flag == R_OUTPUT);
}

int set_input_output_file_cmd_buff(cmd_buff_t * cmd_buff, char * arg_start, Redirect_Flag redirect_flag){
    switch (redirect_flag)
    {
    case R_APPEND:
        cmd_buff->output_file = strdup(arg_start);
        cmd_buff->append_mode = 1;
        if(cmd_buff->output_file == NULL) return ERR_MEMORY;
        break;
    case R_INPUT:
        cmd_buff->input_file = strdup(arg_start);
        if(cmd_buff->input_file == NULL) return ERR_MEMORY;
        break;
    case R_OUTPUT:
        cmd_buff->output_file = strdup(arg_start);
        if(cmd_buff->output_file == NULL) return ERR_MEMORY;
        break;
    default:
        return ERR_MEMORY;
    }
    return OK;
}


void set_arg_to_next_string_in_cmd_buff(char ** arg_start, cmd_buff_t * cmd_buff, int i){
    *arg_start = &cmd_buff->_cmd_buffer[i+1];
}


/*
Returns:
    OK - if cmd_buff was built successfully
    CMD_WARN_NO_CMD - if cmd_line was empty
    ERR_MEMORY - if any memory issues arise
    ERR_CMD_OR_ARGS_TOO_BIG - exe size exceeds maximum
    ERR_CMD_ARGS_BAD - arg count is too large or arg size exceeds maximum
*/
int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff, char arg_delimitter){
    int cmd_line_len = strlen(cmd_line);
    if(cmd_line_len == 0) return WARN_NO_CMDS;
    // Initialize to zero
    cmd_buff->_cmd_buffer = (char *) malloc(sizeof(char) * strlen(cmd_line) + 1);
    if(cmd_buff->_cmd_buffer == NULL) return ERR_MEMORY;
    
    cmd_buff->argc = 0;
    cmd_buff->input_file = NULL;
    cmd_buff->output_file = NULL;
    cmd_buff->append_mode = 0;

    // Copy formatted cmd_line
    int formatted_cmd_line_len = format_cmd_line(cmd_buff->_cmd_buffer, cmd_line, cmd_line_len, arg_delimitter);

    // Debug to print after formatting
    //printf("After formatting:%s, Length:%d\n", cmd_buff->_cmd_buffer, formatted_cmd_line_len);

    // Copy args into cmd_buff
    char * arg_start = cmd_buff->_cmd_buffer;
    int can_insert = 0;
    Redirect_Flag redirect_flag = R_NONE;
    
    for(int i = 0; i <= formatted_cmd_line_len; i++){
        if(cmd_buff->_cmd_buffer[i] == arg_delimitter){
            if(redirect_flag_was_set(redirect_flag)){
                if(set_input_output_file_cmd_buff(cmd_buff, arg_start, redirect_flag) != OK)
                    return ERR_MEMORY;
                
                redirect_flag = R_NONE;
                set_arg_to_next_string_in_cmd_buff(&arg_start, cmd_buff, i);
                continue;
            }
            
            // Flag was set
            if(check_if_arg_is_redirect_and_set_flag(arg_start, &redirect_flag) != R_NONE){
                if(cmd_buff_has_no_args(cmd_buff)) return WARN_NO_CMDS;
                set_arg_to_next_string_in_cmd_buff(&arg_start, cmd_buff, i);
                continue;
            }
            
            
            if((can_insert = can_insert_cmd_buff_argv(cmd_buff, strlen(arg_start))) != OK){
                return can_insert;
            }

            cmd_buff->argv[cmd_buff->argc] = strdup(arg_start);

            if (cmd_buff->argv[cmd_buff->argc] == NULL) return ERR_MEMORY;

            cmd_buff->argc++;
            set_arg_to_next_string_in_cmd_buff(&arg_start, cmd_buff, i);
        }
    }

    // Set last string to null for execvp
    cmd_buff->argv[cmd_buff->argc] = NULL;
    return OK;
}

int cmd_buff_has_no_args(cmd_buff_t * cmd_buff){
    return cmd_buff->argc == 0;
}
/*
Returns:
    OK
    ERR_CMD_OR_ARGS_TOO_BIG
    ERR_CMD_ARGS_BAD
*/
int can_insert_cmd_buff_argv(cmd_buff_t *cmd_buff, int arg_len){
    if(cmd_buff_has_no_args(cmd_buff)){
        return (arg_len < EXE_MAX) ? OK: ERR_CMD_OR_ARGS_TOO_BIG;
    }
    return ((cmd_buff->argc + 1 <= CMD_ARGV_MAX) && (arg_len < ARG_MAX)) ? OK : ERR_CMD_ARGS_BAD;
}

/*
Returns:
    Destination size
*/
int format_cmd_line(char *dest, char *src, int src_len, char arg_delimitter) {
    int start = 0;
    while (start < src_len && isspace(src[start])) start++;

    int end = src_len - 1;
    while (end > start && isspace(src[end])) end--;

    int i = start, j = 0;
    int in_quotes = 0;
    int last_was_space = 0;

    while (i <= end) {
        char current = src[i];

        if (current == QUOTE_CHAR) {
            in_quotes = !in_quotes;
            //formatted[j++] = current;
            last_was_space = 0;
        } else if (isspace((unsigned char)current)) {
            if (in_quotes) {
                dest[j++] = current;
            } else if (!last_was_space) {
                // Delimitter for args
                dest[j++] = arg_delimitter;
                last_was_space = 1;
            }
        } else {
            dest[j++] = current;
            last_was_space = 0;
        }
        i++;
    }

    dest[j] = '\0';
    return j;
}

