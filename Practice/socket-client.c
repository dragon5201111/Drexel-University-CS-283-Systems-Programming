#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 80
#define MAX_BUFFER_SIZE 4096
#define GET_REQUEST "GET / HTTP/1.1\r\n\r\n"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Must specify server IP address.\n");
        exit(EXIT_FAILURE);
    }

    int clientSocket;
    struct sockaddr_in serverSock;
    char recvBuffer[MAX_BUFFER_SIZE];
    char sendBuffer[MAX_BUFFER_SIZE];
    bzero(sendBuffer, sizeof(sendBuffer));
    bzero(recvBuffer, sizeof(recvBuffer));

    // Create TCP socket
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Unable to create client socket");
        exit(EXIT_FAILURE);
    }

    // Zero out server socket
    bzero(&serverSock, sizeof(serverSock));
    serverSock.sin_family = AF_INET;
    serverSock.sin_port = htons(PORT); // Set port with network byte ordering

    // Set server socket IP address (IPv4)
    if (inet_pton(AF_INET, argv[1], &serverSock.sin_addr) <= 0) {
        perror("Invalid IP address format");
        close(clientSocket);
        exit(EXIT_FAILURE);
    }

    // Connect client to server
    if (connect(clientSocket, (const struct sockaddr *)&serverSock, sizeof(serverSock)) < 0) {
        perror("Unable to connect client to server!\n");
        close(clientSocket);
        exit(EXIT_FAILURE);
    }

    // Send the GET request using send()
    snprintf(sendBuffer, sizeof(sendBuffer), "%s", GET_REQUEST);
    ssize_t bytesSent = send(clientSocket, sendBuffer, strlen(sendBuffer), 0);
    if (bytesSent < 0) {
        perror("Unable to send GET request to server.\n");
        close(clientSocket);
        exit(EXIT_FAILURE);
    }

    ssize_t bytesReceived = 0;
    // Receive the server's response using recv()
    while ((bytesReceived = recv(clientSocket, recvBuffer, sizeof(recvBuffer) - 1, 0)) > 0) {
        recvBuffer[bytesReceived] = '\0';  // Null-terminate the response
        printf("%s\n", recvBuffer); // Print received data
    }

    if (bytesReceived == 0) {
        printf("Connection closed by server.\n");
    } else if (bytesReceived < 0) {
        perror("Error receiving data");
    }

    close(clientSocket);
    exit(EXIT_SUCCESS);
}
