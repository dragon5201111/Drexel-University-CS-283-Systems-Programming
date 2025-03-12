#ifndef __RSH_LIB_H__
    #define __RSH_LIB_H__

#include "dshlib.h"

#include <sys/socket.h>
#include <arpa/inet.h>
//common remote shell client and server constants and definitions


//Constants for communication
//Note that these should work fine in a local VM but you will likely have
//to change the port number if you are working on tux.
#define RDSH_DEF_PORT           4545        //Default port #
#define RDSH_DEF_SVR_INTFACE    "0.0.0.0"   //Default start all interfaces
#define RDSH_DEF_CLI_CONNECT    "127.0.0.1" //Default server is running on
                                            //localhost 127.0.0.1

//constants for buffer sizes
#define RDSH_COMM_BUFF_SZ       (1024*64)   //64K
#define RDSH_SVR_BACKLOG_MAX 20
#define STOP_SERVER_SC          200         //returned from pipeline excution
                                            //if the command is to stop the
                                            //server.  See documentation for 
                                            //exec_client_requests() for more info

//end of message delimiter.  This is super important.  TCP is a stream, therefore
//the protocol designer is responsible for managing where messages begin and end
//there are many common techniques for this, but one of the simplest ways is to
//use an end of stream marker.  Since rsh is a "shell" program we will be using
//ascii code 0x04, which is commonly used as the end-of-file (EOF) character in
//linux based systems. 
static const char RDSH_EOF_CHAR = 0x04;

//rdsh specific error codes for functions
#define ERR_RDSH_COMMUNICATION  -50     //Used for communication errors
#define ERR_RDSH_SERVER         -51     //General server errors
#define ERR_RDSH_CLIENT         -52     //General client errors
#define ERR_RDSH_CMD_EXEC       -53     //RSH command execution errors
#define WARN_RDSH_NOT_IMPL      -99     //Not Implemented yet warning

//Output message constants for server
#define CMD_ERR_RDSH_COMM   "rdsh-error: communications error\n"
#define CMD_ERR_RDSH_EXEC   "rdsh-error: command execution error\n"
#define CMD_ERR_RDSH_ITRNL  "rdsh-error: internal server error - %d\n"
#define CMD_ERR_RDSH_SEND   "rdsh-error: partial send.  Sent %d, expected to send %d\n"
#define RCMD_SERVER_EXITED  "server appeared to terminate - exiting\n"

//Output message constants for client
#define RCMD_MSG_CLIENT_EXITED  "client exited: getting next connection...\n"
#define RCMD_MSG_SVR_STOP_REQ   "client requested server to stop, stopping...\n"
#define RCMD_MSG_SVR_EXEC_REQ   "rdsh-exec:  %s\n"
#define RCMD_MSG_SVR_RC_CMD     "rdsh-exec:  rc = %d\n"


//client prototypes for rsh_cli.c - - see documentation for each function to
//see what they do
int start_client(char *address, int port);
int client_cleanup(int cli_socket, char *cmd_buff, char *rsp_buff, int rc);
int exec_remote_cmd_loop(char *address, int port);
void print_response_from_server(ssize_t bytes_read, char receive_buffer[]);

//server prototypes for rsh_server.c - see documentation for each function to
//see what they do
int start_server(char *ifaces, int port, int is_threaded);
int boot_server(char *ifaces, int port);
int stop_server(int svr_socket);
int send_message_eof(int cli_socket);
int send_message_string(int cli_socket, char *buff);
int process_cli_requests(int server_socket);
int process_cli_requests_threaded(int server_socket);
int exec_client_requests(int cli_socket);
int rsh_execute_pipeline(int socket_fd, command_list_t *clist);
int rsh_start_supervisor_and_execute_pipeline(int client_socket_fd, command_list_t *clist);
void *handle_client(void *client_socket_ptr);

// Shared protoypes & Defines
#define BUFFER_END_IS_CHAR(buffer, bytes_received, c) (buffer[bytes_received - 1] == c)
int create_af_inet_tcp_socket(char * ip_address, int port, struct sockaddr_in *socket_addr, int error);
void set_last_character_of_buffer(char buffer[], int size, char character);

// SEE COMMENTS IN THE CODE, THESE ARE OPTIONAL IN CASE YOU WANT TO PROVIDE
// SUPPORT FOR BUILT-IN FUNCTIONS DIFFERENTLY 
Built_In_Cmds rsh_match_command(const char *input);
Built_In_Cmds rsh_exec_built_in_cmd(Built_In_Cmds bi_type);

#endif