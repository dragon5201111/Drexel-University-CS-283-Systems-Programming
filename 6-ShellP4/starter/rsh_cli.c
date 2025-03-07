
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>

#include "dshlib.h"
#include "rshlib.h"



/*
 * exec_remote_cmd_loop(server_ip, port)
 *      server_ip:  a string in ip address format, indicating the servers IP
 *                  address.  Note 127.0.0.1 is the default meaning the server
 *                  is running on the same machine as the client
 *              
 *      port:   The port the server will use.  Note the constant 
 *              RDSH_DEF_PORT which is 1234 in rshlib.h.  If you are using
 *              tux you may need to change this to your own default, or even
 *              better use the command line override -c implemented in dsh_cli.c
 *              For example ./dsh -c 10.50.241.18:5678 where 5678 is the new port
 *              number and the server address is 10.50.241.18    
 * 
 *      This function basically implements the network version of 
 *      exec_local_cmd_loop() from the last assignemnt.  It will:
 *  
 *          1. Allocate buffers for sending and receiving data over the 
 *             network
 *          2. Create a network connection to the server, getting an active
 *             socket by calling the start_client(server_ip, port) function.
 *          2. Go into an infinite while(1) loop prompting the user for
 *             input commands. 
 * 
 *             a. Accept a command from the user via fgets()
 *             b. Send that command to the server using send() - it should
 *                be a null terminated string
 *             c. Go into a loop and receive client requests.  Note each
 *                receive might not be a C string so you need to print it
 *                out using:
 *                     printf("%.*s", (int)bytes_received, rsp_buff);
 *                this version of printf() uses the "%.*s" flag that indicates
 *                that the rsp_buff might be a null terminated string, or
 *                it might not be, if its not, print exactly bytes_received
 *                bytes. 
 *             d. In the recv() loop described above. Each time you receive
 *                data from the server, see if the last byte received is the
 *                EOF character. This indicates the server is done and you can
 *                send another command by going to the top of the loop.  The
 *                best way to do this is as follows assuming you are receiving
 *                data into a buffer called recv_buff, and you received
 *                recv_bytes in the call to recv:
 * 
 *                  recv_bytes = recv(sock, recv_buff, recv_buff_sz, 0)
 *                  
 *                if recv_bytes:
 *                  <negative_number>: communication error
 *                    0:    Didn't receive anything, likely server down
 *                  > 0:    Got some data. Check if the last byte is EOF
 *                          is_eof = (recv_buff[recv_bytes-1] == RDSH_EOF_CHAR) ? 1 : 0;
 *                    if is_eof is true, this is the last part of the transmission
 *                    from the server and you can break out of the recv() loop. 
 * 
 *   returns:
 *          OK:      The client executed all of its commands and is exiting
 *                   either by the `exit` command that terminates the client
 *                   or the `stop-server` command that terminates both the
 *                   client and the server. 
 *          ERR_MEMORY:             If this function cannot allocate memory via
 *                                  malloc for the send and receive buffers
 *          ERR_RDSH_CLIENT:        If the client cannot connect to the server. 
 *                                  AKA the call to start_client() fails.
 *          ERR_RDSH_COMMUNICATION: If there is a communication error, AKA
 *                                  any failures from send() or recv().
 * 
 *   NOTE:  Since there are several exit points and each exit point must
 *          call free() on the buffers allocated, close the socket, and
 *          return an appropriate error code.  Its suggested you use the
 *          helper function client_cleanup() for these purposes.  For example:
 * 
 *   return client_cleanup(cli_socket, request_buff, resp_buff, ERR_RDSH_COMMUNICATION);
 *   return client_cleanup(cli_socket, request_buff, resp_buff, OK);
 *
 *   The above will return ERR_RDSH_COMMUNICATION and OK respectively to the main()
 *   function after cleaning things up.  See the documentation for client_cleanup()
 *      
 */
int exec_remote_cmd_loop(char *address, int port)
{   
    // Initialize buffers for sending and receiving data
    char *send_buffer = (char *) malloc(sizeof(char) * RDSH_COMM_BUFF_SZ);
    char *receive_buffer = (char *) malloc(sizeof(char) * RDSH_COMM_BUFF_SZ);
    ssize_t bytes_read;

    if (send_buffer == NULL || receive_buffer == NULL) {
        free(send_buffer);
        free(receive_buffer);
        return ERR_MEMORY;
    }
    
    int client_socket_fd, is_end_of_stream;

    // Establish connection with server
    client_socket_fd = start_client(address, port);
    if (client_socket_fd == ERR_RDSH_CLIENT) {
        free(send_buffer);
        free(receive_buffer);
        return ERR_RDSH_CLIENT;
    }

    while (1) {
        printf(SH_PROMPT);

        // Get command from stdin
        if (fgets(send_buffer, RDSH_COMM_BUFF_SZ, stdin) != NULL) {
            size_t send_buffer_len = strlen(send_buffer);
            // Get rid of newline
            send_buffer[send_buffer_len - 1] = NULL_BYTE;

            if (send(client_socket_fd, send_buffer, send_buffer_len + 1, 0) == -1) {
                printf(CMD_ERR_RDSH_COMM);
                return client_cleanup(client_socket_fd, send_buffer, receive_buffer, ERR_RDSH_COMMUNICATION);
            }

            // Client wanted to exited
            if(strings_are_equal(EXIT_CMD, send_buffer) || strings_are_equal(EXIT_CMD_SERVER, send_buffer)){
                return client_cleanup(client_socket_fd, send_buffer, receive_buffer, OK);
            }
            
            // Read response from server
            while ((bytes_read = recv(client_socket_fd, receive_buffer, RDSH_COMM_BUFF_SZ, 0)) > 0) {
                // Communication error
                if (bytes_read < 0) {
                    printf(CMD_ERR_RDSH_COMM);
                    return client_cleanup(client_socket_fd, send_buffer, receive_buffer, ERR_RDSH_COMMUNICATION);
                }

                // Server might be down, so break out of loop
                if (bytes_read == 0) {
                    printf(RCMD_SERVER_EXITED);
                    return client_cleanup(client_socket_fd, send_buffer, receive_buffer, ERR_RDSH_CLIENT);
                }

                // We received some bytes, check if end of stream
                is_end_of_stream = BUFFER_END_IS_CHAR(receive_buffer, bytes_read, RDSH_EOF_CHAR);
                if (is_end_of_stream) {
                    receive_buffer[bytes_read - 1] = '\0';
                }

                printf("%.*s\n", (int)bytes_read, receive_buffer);

                if (is_end_of_stream)
                    break;
            }
        }
    }

    return client_cleanup(client_socket_fd, send_buffer, receive_buffer, OK);
}


/*
 * start_client(server_ip, port)
 *      server_ip:  a string in ip address format, indicating the servers IP
 *                  address.  Note 127.0.0.1 is the default meaning the server
 *                  is running on the same machine as the client
 *              
 *      port:   The port the server will use.  Note the constant 
 *              RDSH_DEF_PORT which is 1234 in rshlib.h.  If you are using
 *              tux you may need to change this to your own default, or even
 *              better use the command line override -c implemented in dsh_cli.c
 *              For example ./dsh -c 10.50.241.18:5678 where 5678 is the new port
 *              number and the server address is 10.50.241.18    
 * 
 *      This function basically runs the client by: 
 *          1. Creating the client socket via socket()
 *          2. Calling connect()
 *          3. Returning the client socket after connecting to the server
 * 
 *   returns:
 *          client_socket:      The file descriptor fd of the client socket
 *          ERR_RDSH_CLIENT:    If socket() or connect() fail
 * 
 */
int start_client(char *server_ip, int port){
    struct sockaddr_in server_addr;
    int client_socket_fd;

    // Create client socket
    if((client_socket_fd = create_af_inet_tcp_socket(server_ip, port, &server_addr, ERR_RDSH_CLIENT)) == ERR_RDSH_CLIENT)
        return ERR_RDSH_CLIENT;

    // Establish connection with server
    if(connect(client_socket_fd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        return ERR_RDSH_CLIENT;
    }

    return client_socket_fd;
}

/*
 * client_cleanup(int cli_socket, char *cmd_buff, char *rsp_buff, int rc)
 *      cli_socket:   The client socket
 *      cmd_buff:     The buffer that will hold commands to send to server
 *      rsp_buff:     The buffer that will hld server responses
 * 
 *   This function does the following: 
 *      1. If cli_socket > 0 it calls close(cli_socket) to close the socket
 *      2. It calls free() on cmd_buff and rsp_buff
 *      3. It returns the value passed as rc
 *  
 *   Note this function is intended to be helper to manage exit conditions
 *   from the exec_remote_cmd_loop() function given there are several
 *   cleanup steps.  We provide it to you fully implemented as a helper.
 *   You do not have to use it if you want to develop an alternative
 *   strategy for cleaning things up in your exec_remote_cmd_loop()
 *   implementation. 
 * 
 *   returns:
 *          rc:   This function just returns the value passed as the 
 *                rc parameter back to the caller.  This way the caller
 *                can just write return client_cleanup(...)
 *      
 */
int client_cleanup(int cli_socket, char *cmd_buff, char *rsp_buff, int rc){
    //If a valid socket number close it.
    if(cli_socket > 0){
        close(cli_socket);
    }

    //Free up the buffers 
    free(cmd_buff);
    free(rsp_buff);

    //Echo the return value that was passed as a parameter
    return rc;
}