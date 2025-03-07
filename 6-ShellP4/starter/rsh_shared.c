#include <sys/socket.h>
#include <arpa/inet.h>
#include "rshlib.h"
#include <stdlib.h>


/*
*   Attempts to create a tcp socket with specificed IP address and port. On success,
*   It returns a file descriptor of the tcp socket. On failure, it will return the error code specified.
*   Set the sockaddr_in structure with IPV4 address, port, and IP address specified.
*/
int create_af_inet_tcp_socket(char * ip_address, int port, struct sockaddr_in *socket_addr, int error){
    
    int socket_fd;

    // Create client socket
    if((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        return error;
    }

    socket_addr->sin_family = AF_INET;
    socket_addr->sin_port = htons(port);

    // Set server IP in sockaddr_in struct
    if(inet_pton(AF_INET, ip_address, &socket_addr->sin_addr) <= 0){
        return error;
    }

    return socket_fd;
}

void set_last_character_of_buffer(char buffer[], int size, char character){
    buffer[size - 1] = character;
}
