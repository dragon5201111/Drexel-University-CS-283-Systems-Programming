
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/prctl.h>
#include <signal.h>
#include <pthread.h>


//INCLUDES for extra credit
//#include <signal.h>
//#include <pthread.h>
//-------------------------

#include "dshlib.h"
#include "rshlib.h"


/*
 * start_server(ifaces, port, is_threaded)
 *      ifaces:  a string in ip address format, indicating the interface
 *              where the server will bind.  In almost all cases it will
 *              be the default "0.0.0.0" which binds to all interfaces.
 *              note the constant RDSH_DEF_SVR_INTFACE in rshlib.h
 * 
 *      port:   The port the server will use.  Note the constant 
 *              RDSH_DEF_PORT which is 1234 in rshlib.h.  If you are using
 *              tux you may need to change this to your own default, or even
 *              better use the command line override -s implemented in dsh_cli.c
 *              For example ./dsh -s 0.0.0.0:5678 where 5678 is the new port  
 * 
 *      is_threded:  Used for extra credit to indicate the server should implement
 *                   per thread connections for clients  
 * 
 *      This function basically runs the server by: 
 *          1. Booting up the server
 *          2. Processing client requests until the client requests the
 *             server to stop by running the `stop-server` command
 *          3. Stopping the server. 
 * 
 *      This function is fully implemented for you and should not require
 *      any changes for basic functionality.  
 * 
 *      IF YOU IMPLEMENT THE MULTI-THREADED SERVER FOR EXTRA CREDIT YOU NEED
 *      TO DO SOMETHING WITH THE is_threaded ARGUMENT HOWEVER.  
 */
int start_server(char *ifaces, int port, int is_threaded){
    int svr_socket;
    int rc;

    //
    //TODO:  If you are implementing the extra credit, please add logic
    //       to keep track of is_threaded to handle this feature
    //

    svr_socket = boot_server(ifaces, port);
    if (svr_socket < 0){
        int err_code = svr_socket;  //server socket will carry error code
        perror("Error booting server");
        return err_code;
    }

    if(is_threaded){
        rc = process_cli_requests_threaded(svr_socket);
    }else{
        rc = process_cli_requests(svr_socket);
    }

    stop_server(svr_socket);


    return rc;
}

/*
 * stop_server(svr_socket)
 *      svr_socket: The socket that was created in the boot_server()
 *                  function. 
 * 
 *      This function simply returns the value of close() when closing
 *      the socket.  
 */
int stop_server(int svr_socket){
    return close(svr_socket);
}

/*
 * boot_server(ifaces, port)
 *      ifaces & port:  see start_server for description.  They are passed
 *                      as is to this function.   
 * 
 *      This function "boots" the rsh server.  It is responsible for all
 *      socket operations prior to accepting client connections.  Specifically: 
 * 
 *      1. Create the server socket using the socket() function. 
 *      2. Calling bind to "bind" the server to the interface and port
 *      3. Calling listen to get the server ready to listen for connections.
 * 
 *      after creating the socket and prior to calling bind you might want to 
 *      include the following code:
 * 
 *      int enable=1;
 *      setsockopt(svr_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
 * 
 *      when doing development you often run into issues where you hold onto
 *      the port and then need to wait for linux to detect this issue and free
 *      the port up.  The code above tells linux to force allowing this process
 *      to use the specified port making your life a lot easier.
 * 
 *  Returns:
 * 
 *      server_socket:  Sockets are just file descriptors, if this function is
 *                      successful, it returns the server socket descriptor, 
 *                      which is just an integer.
 * 
 *      ERR_RDSH_COMMUNICATION:  This error code is returned if the socket(),
 *                               bind(), or listen() call fails. 
 * 
 */
int boot_server(char *ifaces, int port){
    struct sockaddr_in server_addr;
    int server_socket_fd;

    // Create server socket
    if((server_socket_fd = create_af_inet_tcp_socket(ifaces, port, &server_addr, ERR_RDSH_COMMUNICATION)) == ERR_RDSH_COMMUNICATION) 
        return ERR_RDSH_COMMUNICATION;

    int enable_reuse_addr = 1;
    setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable_reuse_addr, sizeof(int));

    // Bind socket to IP address & port; associate specific IP address and port number with server socket
    if(bind(server_socket_fd, (const struct sockaddr *) &server_addr, sizeof(server_addr)) == -1){
        return ERR_RDSH_COMMUNICATION;
    }

    // Listen for incoming connections
    if(listen(server_socket_fd, RDSH_SVR_BACKLOG_MAX) == -1){
        return ERR_RDSH_COMMUNICATION;
    }

    return server_socket_fd;
}

void *handle_client(void *client_socket_ptr) {
    int client_socket = *(int *)client_socket_ptr; 

    int exec_client_requests_rc;
    exec_client_requests_rc = exec_client_requests(client_socket);

    if (exec_client_requests_rc == ERR_RDSH_COMMUNICATION) {
        close(client_socket);
    } else if (exec_client_requests_rc == OK) {
        printf(RCMD_MSG_CLIENT_EXITED);
        close(client_socket);
    } else if (exec_client_requests_rc == OK_EXIT) {
        printf(RCMD_MSG_SVR_STOP_REQ);
        close(client_socket);
    }

    return NULL;
}

int process_cli_requests_threaded(int server_socket) {
    int client_socket_fd;
    pthread_t client_thread_id;

    while (1) {
        if ((client_socket_fd = accept(server_socket, NULL, NULL)) == -1) {
            return ERR_RDSH_COMMUNICATION;
        }

        
        if(pthread_create(&client_thread_id, NULL, handle_client, (void *)&client_socket_fd) < 0) {
            close(client_socket_fd);
            return ERR_RDSH_COMMUNICATION;
        }
    }

    return OK_EXIT;
}

/*
 * process_cli_requests(svr_socket)
 *      svr_socket:  The server socket that was obtained from boot_server()
 *   
 *  This function handles managing client connections.  It does this using
 *  the following logic
 * 
 *      1.  Starts a while(1) loop:
 *  
 *          a. Calls accept() to wait for a client connection. Recall that 
 *             the accept() function returns another socket specifically
 *             bound to a client connection. 
 *          b. Calls exec_client_requests() to handle executing commands
 *             sent by the client. It will use the socket returned from
 *             accept().
 *          c. Loops back to the top (step 2) to accept connecting another
 *             client.  
 * 
 *          note that the exec_client_requests() return code should be
 *          negative if the client requested the server to stop by sending
 *          the `stop-server` command.  If this is the case step 2b breaks
 *          out of the while(1) loop. 
 * 
 *      2.  After we exit the loop, we need to cleanup.  Dont forget to 
 *          free the buffer you allocated in step #1.  Then call stop_server()
 *          to close the server socket. 
 * 
 *  Returns:
 * 
 *      OK_EXIT:  When the client sends the `stop-server` command this function
 *                should return OK_EXIT. 
 * 
 *      ERR_RDSH_COMMUNICATION:  This error code terminates the loop and is
 *                returned from this function in the case of the accept() 
 *                function failing. 
 * 
 *      OTHERS:   See exec_client_requests() for return codes.  Note that positive
 *                values will keep the loop running to accept additional client
 *                connections, and negative values terminate the server. 
 * 
 */
int process_cli_requests(int server_socket){
    int client_socket_fd;
    int exec_client_requests_rc;

    while(1){

        if((client_socket_fd = accept(server_socket, NULL, NULL)) == -1)
            return ERR_RDSH_COMMUNICATION;
        
        if((exec_client_requests_rc = exec_client_requests(client_socket_fd)) == ERR_RDSH_COMMUNICATION){
            close(client_socket_fd);
            return ERR_RDSH_COMMUNICATION;
        }else if(exec_client_requests_rc == OK){
            // Client is done, close client
            printf(RCMD_MSG_CLIENT_EXITED);
            close(client_socket_fd);
            continue;
        }else if(exec_client_requests_rc == OK_EXIT){
            // Close client and server
            printf(RCMD_MSG_SVR_STOP_REQ);
            close(client_socket_fd);
            break;
        }
    }

    return OK_EXIT;
}

/*
 * exec_client_requests(cli_socket)
 *      cli_socket:  The server-side socket that is connected to the client
 *   
 *  This function handles accepting remote client commands. The function will
 *  loop and continue to accept and execute client commands.  There are 2 ways
 *  that this ongoing loop accepting client commands ends:
 * 
 *      1.  When the client executes the `exit` command, this function returns
 *          to process_cli_requests() so that we can accept another client
 *          connection. 
 *      2.  When the client executes the `stop-server` command this function
 *          returns to process_cli_requests() with a return code of OK_EXIT
 *          indicating that the server should stop. 
 * 
 *  Note that this function largely follows the implementation of the
 *  exec_local_cmd_loop() function that you implemented in the last 
 *  shell program deliverable. The main difference is that the command will
 *  arrive over the recv() socket call rather than reading a string from the
 *  keyboard. 
 * 
 *  This function also must send the EOF character after a command is
 *  successfully executed to let the client know that the output from the
 *  command it sent is finished.  Use the send_message_eof() to accomplish 
 *  this. 
 * 
 *  Of final note, this function must allocate a buffer for storage to 
 *  store the data received by the client. For example:
 *     io_buff = malloc(RDSH_COMM_BUFF_SZ);
 *  And since it is allocating storage, it must also properly clean it up
 *  prior to exiting.
 * 
 *  Returns:
 * 
 *      OK:       The client sent the `exit` command.  Get ready to connect
 *                another client. 
 *      OK_EXIT:  The client sent `stop-server` command to terminate the server
 * 
 *      ERR_RDSH_COMMUNICATION:  A catch all for any socket() related send
 *                or receive errors. 
 */
int exec_client_requests(int client_socket_fd) {
    char *receive_buffer = (char *) malloc(sizeof(char) * RDSH_COMM_BUFF_SZ);

    if(receive_buffer == NULL){
        return ERR_MEMORY;
    }

    ssize_t bytes_read;
    command_list_t command_list;

    int is_end_of_stream;
    int return_code;

    while ((bytes_read = recv(client_socket_fd, receive_buffer, RDSH_COMM_BUFF_SZ, 0)) > 0){
        if(bytes_read < 0)
            return ERR_RDSH_COMMUNICATION;

        if(bytes_read == 0)
            return ERR_RDSH_COMMUNICATION;

        is_end_of_stream = BUFFER_END_IS_CHAR(receive_buffer, bytes_read, NULL_BYTE);

        if(is_end_of_stream){
            set_last_character_of_buffer(receive_buffer, bytes_read, NULL_BYTE);
        }

        if(is_end_of_stream){

            return_code = build_cmd_list(receive_buffer, &command_list, NULL_BYTE);
            if(return_code != OK){
                // Indicate error
                send_message_eof(client_socket_fd);
                continue;
            }

            cmd_buff_t first_cmd = command_list.commands[0];
            Built_In_Cmds bi_type = rsh_match_command(first_cmd.argv[0]);
            
            if(clist_has_no_pipe_redirection_and_built_in(bi_type, &command_list)){
                switch (bi_type)
                {
                case BI_CMD_EXIT:
                    send_message_string(client_socket_fd, EXIT_CMD);
                    free_cmd_list(&command_list);
                    break;
                case BI_CMD_STOP_SVR:
                    send_message_string(client_socket_fd, EXIT_CMD);
                    free_cmd_list(&command_list);
                    free(receive_buffer);
                    return OK_EXIT;
                case BI_CMD_CD:
                    return_code = chdir(first_cmd.argv[1]);
                    send_message_eof(client_socket_fd);
                    break;
                default:
                    send_message_eof(client_socket_fd);
                    break;
                }
            }else{
                // Pipeline
                return_code = rsh_start_supervisor_and_execute_pipeline(client_socket_fd, &command_list);
                send_message_eof(client_socket_fd);
            }

            free_cmd_list(&command_list);
        }
    } 

    free(receive_buffer);
    return OK;
}

/*
 * send_message_eof(cli_socket)
 *      cli_socket:  The server-side socket that is connected to the client

 *  Sends the EOF character to the client to indicate that the server is
 *  finished executing the command that it sent. 
 * 
 *  Returns:
 * 
 *      OK:  The EOF character was sent successfully. 
 * 
 *      ERR_RDSH_COMMUNICATION:  The send() socket call returned an error or if
 *           we were unable to send the EOF character. 
 */
int send_message_eof(int cli_socket){
    if(send(cli_socket, &RDSH_EOF_CHAR, 1, 0) != 1){
        return ERR_RDSH_COMMUNICATION;
    }

    return OK;
}

/*
 * send_message_string(cli_socket, char *buff)
 *      cli_socket:  The server-side socket that is connected to the client
 *      buff:        A C string (aka null terminated) of a message we want
 *                   to send to the client. 
 *   
 *  Sends a message to the client.  Note this command executes both a send()
 *  to send the message and a send_message_eof() to send the EOF character to
 *  the client to indicate command execution terminated. 
 * 
 *  Returns:
 * 
 *      OK:  The message in buff followed by the EOF character was 
 *           sent successfully. 
 * 
 *      ERR_RDSH_COMMUNICATION:  The send() socket call returned an error or if
 *           we were unable to send the message followed by the EOF character. 
 */
int send_message_string(int cli_socket, char *buff){
    // Send message
    if(send(cli_socket, buff, strlen(buff), 0) == -1){
        return ERR_RDSH_COMMUNICATION;
    }

    // Send EOF character
    if(send_message_eof(cli_socket) != OK) 
        return ERR_RDSH_COMMUNICATION;

    return OK;
}


/*
 * rsh_execute_pipeline(int cli_sock, command_list_t *clist)
 *      cli_sock:    The server-side socket that is connected to the client
 *      clist:       The command_list_t structure that we implemented in
 *                   the last shell. 
 *   
 *  This function executes the command pipeline.  It should basically be a
 *  replica of the execute_pipeline() function from the last deliverable. 
 *  The only thing different is that you will be using the cli_sock as the
 *  main file descriptor on the first executable in the pipeline for STDIN,
 *  and the cli_sock for the file descriptor for STDOUT, and STDERR for the
 *  last executable in the pipeline.  See picture below:  
 * 
 *      
 *┌───────────┐                                                    ┌───────────┐
 *│ cli_sock  │                                                    │ cli_sock  │
 *└─────┬─────┘                                                    └────▲──▲───┘
 *      │   ┌──────────────┐     ┌──────────────┐     ┌──────────────┐  │  │    
 *      │   │   Process 1  │     │   Process 2  │     │   Process N  │  │  │    
 *      │   │              │     │              │     │              │  │  │    
 *      └───▶stdin   stdout├─┬──▶│stdin   stdout├─┬──▶│stdin   stdout├──┘  │    
 *          │              │ │   │              │ │   │              │     │    
 *          │        stderr├─┘   │        stderr├─┘   │        stderr├─────┘    
 *          └──────────────┘     └──────────────┘     └──────────────┘   
 *                                                      WEXITSTATUS()
 *                                                      of this last
 *                                                      process to get
 *                                                      the return code
 *                                                      for this function       
 * 
 *  Returns:
 * 
 *      EXIT_CODE:  This function returns the exit code of the last command
 *                  executed in the pipeline.  If only one command is executed
 *                  that value is returned.  Remember, use the WEXITSTATUS()
 *                  macro that we discussed during our fork/exec lecture to
 *                  get this value. 
 */
int rsh_execute_pipeline(int client_socket_fd, command_list_t *clist) {
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

            if(i == 0){
                dup2(client_socket_fd, STDIN_FILENO);
            }

            if(i == num_commands - 1){
                dup2(client_socket_fd, STDOUT_FILENO);
                dup2(client_socket_fd, STDERR_FILENO);
            }

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

           
            execvp(cmd_to_exec.argv[0], cmd_to_exec.argv);
            exit(errno);
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


int rsh_start_supervisor_and_execute_pipeline(int client_socket_fd, command_list_t *clist) {
    pid_t supervisor = fork();
    int supervisor_rc;

    if (supervisor == -1) {
        perror("Fork supervisor creation failed");
        exit(errno);
    }

    if (supervisor == 0) {
        // Kill children if parent dies
        prctl(PR_SET_PDEATHSIG, SIGKILL);

        int pipe_line_rc = rsh_execute_pipeline(client_socket_fd, clist);
        exit(pipe_line_rc);
    }

    waitpid(supervisor, &supervisor_rc, 0);
    if (WIFEXITED(supervisor_rc)) {
        return WEXITSTATUS(supervisor_rc);
    } else {
        return errno;
    }
}

/**************   OPTIONAL STUFF  ***************/
/****
 **** NOTE THAT THE FUNCTIONS BELOW ALIGN TO HOW WE CRAFTED THE SOLUTION
 **** TO SEE IF A COMMAND WAS BUILT IN OR NOT.  YOU CAN USE A DIFFERENT
 **** STRATEGY IF YOU WANT.  IF YOU CHOOSE TO DO SO PLEASE REMOVE THESE
 **** FUNCTIONS AND THE PROTOTYPES FROM rshlib.h
 **** 
 */

/*
 * rsh_match_command(const char *input)
 *      cli_socket:  The string command for a built-in command, e.g., dragon,
 *                   cd, exit-server
 *   
 *  This optional function accepts a command string as input and returns
 *  one of the enumerated values from the BuiltInCmds enum as output. For
 *  example:
 * 
 *      Input             Output
 *      exit              BI_CMD_EXIT
 *      dragon            BI_CMD_DRAGON
 * 
 *  This function is entirely optional to implement if you want to handle
 *  processing built-in commands differently in your implementation. 
 * 
 *  Returns:
 * 
 *      BI_CMD_*:   If the command is built-in returns one of the enumeration
 *                  options, for example "cd" returns BI_CMD_CD
 * 
 *      BI_NOT_BI:  If the command is not "built-in" the BI_NOT_BI value is
 *                  returned. 
 */
Built_In_Cmds rsh_match_command(const char *input)
{
    if(strings_are_equal(input, EXIT_CMD)){
        return BI_CMD_EXIT;
    }else if(strings_are_equal(input, DRAGON_CMD)){
        return BI_CMD_DRAGON;
    }else if(strings_are_equal(input, CD_CMD)){
        return BI_CMD_CD;
    }else if(strings_are_equal(input, RC_CMD)){
        return BI_CMD_RC;
    }else if(strings_are_equal(input, EXIT_CMD_SERVER)){
        return BI_CMD_STOP_SVR;
    }
    return BI_NOT_BI;
}

/*
 * rsh_built_in_cmd(cmd_buff_t *cmd)
 *      cmd:  The cmd_buff_t of the command, remember, this is the 
 *            parsed version fo the command
 *   
 *  This optional function accepts a parsed cmd and then checks to see if
 *  the cmd is built in or not.  It calls rsh_match_command to see if the 
 *  cmd is built in or not.  Note that rsh_match_command returns BI_NOT_BI
 *  if the command is not built in. If the command is built in this function
 *  uses a switch statement to handle execution if appropriate.   
 * 
 *  Again, using this function is entirely optional if you are using a different
 *  strategy to handle built-in commands.  
 * 
 *  Returns:
 * 
 *      BI_NOT_BI:   Indicates that the cmd provided as input is not built
 *                   in so it should be sent to your fork/exec logic
 *      BI_EXECUTED: Indicates that this function handled the direct execution
 *                   of the command and there is nothing else to do, consider
 *                   it executed.  For example the cmd of "cd" gets the value of
 *                   BI_CMD_CD from rsh_match_command().  It then makes the libc
 *                   call to chdir(cmd->argv[1]); and finally returns BI_EXECUTED
 *      BI_CMD_*     Indicates that a built-in command was matched and the caller
 *                   is responsible for executing it.  For example if this function
 *                   returns BI_CMD_STOP_SVR the caller of this function is
 *                   responsible for stopping the server.  If BI_CMD_EXIT is returned
 *                   the caller is responsible for closing the client connection.
 * 
 *   AGAIN - THIS IS TOTALLY OPTIONAL IF YOU HAVE OR WANT TO HANDLE BUILT-IN
 *   COMMANDS DIFFERENTLY. 
 */
Built_In_Cmds rsh_exec_built_in_cmd(Built_In_Cmds bi_type)
{
    return bi_type;
}
