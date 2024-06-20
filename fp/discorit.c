#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define MAX_RETRY_ATTEMPTS 5
#define RETRY_INTERVAL 2

void connect_to_server(int *clientSocket, struct sockaddr_in *serverAddr) {
    *clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (*clientSocket < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    serverAddr->sin_family = AF_INET;
    serverAddr->sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &(serverAddr->sin_addr)) <= 0) {
        perror("Invalid address/Address not supported");
        exit(EXIT_FAILURE);
    }

    int attempts = 0;
    while (connect(*clientSocket, (struct sockaddr *)serverAddr, sizeof(*serverAddr)) < 0) {
        if (errno == ECONNREFUSED) {
            if (++attempts >= MAX_RETRY_ATTEMPTS) {
                perror("Connection failed after retries");
                exit(EXIT_FAILURE);
            }
            printf("Connection refused, retrying in %d seconds...\n", RETRY_INTERVAL);
            sleep(RETRY_INTERVAL);
        } else {
            perror("Error connecting to server");
            exit(EXIT_FAILURE);
        }
    }
    printf("Connected to server successfully.\n");
}

int main(int argc, char *argv[]) {
    int clientSocket;
    struct sockaddr_in serverAddr;
    char buffer[1024];

    connect_to_server(&clientSocket, &serverAddr);

    char command[1024];
    memset(command, 0, sizeof(command));

    if(strcmp(argv[1], "REGISTER")==0){
        if(argc<5||strcmp(argv[3],"-p")!=0){
            printf("Invalid command\n");
            return 0;
        }

        char *username = argv[2];
        char *password = argv[4];
        snprintf(command, sizeof(command), "REGISTER %s %s", username, password);
    }
    else if(strcmp(argv[1], "LOGIN")==0){
        if(argc<5||strcmp(argv[3],"-p")!=0){
            printf("Invalid command\n");
            return 0;
        }

        char *username = argv[2];
        char *password = argv[4];
        snprintf(command, sizeof(command), "LOGIN %s %s", username, password);

        if(send(serverAddr, command, strlen(command), 0) < 0){
            perror("send failed");
            exit(EXIT_FAILURE);
        }

        char response[1024];
        memset(response, 0, sizeof(response));
    }

    // Send and receive data
    // TODO: Add your code here

    close(clientSocket);

    return 0;
}
