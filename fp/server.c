#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <bcrypt.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 10240

typedef struct {
    int socket;
    struct sockaddr_in address;
    int addr_len;
} client_t;

void daemonize() {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    if (setsid() < 0) {
        perror("setsid failed");
        exit(EXIT_FAILURE);
    }

    if (chdir("/") < 0) {
        perror("chdir failed");
        exit(EXIT_FAILURE);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    int log_fd = open("/tmp/log.txt", O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (log_fd < 0) {
        perror("failed to open log file");
        exit(EXIT_FAILURE);
    }
    dup2(log_fd, STDOUT_FILENO);
    dup2(log_fd, STDERR_FILENO);
}

void reg_user(int sock, char *username, char *password, client_t *clinfo) {
    if(username == NULL || password == NULL) {
        char response[] = "Invalid command";
        if(write(clinfo->socket, response, strlen(response)) < 0){
            perror("write failed");
        }
        return;
    }
    struct stat st = {0};
    char *path="/home/tka/sisop/fp/DiscorIT";

    if(stat(path, &st) == -1){
        if(mkdir(path, 0700)<0){
            char response[] = "Failed to create directory";
            if(write(clinfo->socket, response, strlen(response)) < 0){
                perror("write failed");
            }
            return;
        }
    }

    FILE *fp = fopen("/home/tka/sisop/fp/DiscorIT/users.csv", "r+");
    if(!file){
        fp = fopen("/home/tka/sisop/fp/DiscorIT/users.csv", "w+");
        if(!file){
            char response[] = "Failed to open file";
            if(write(clinfo->socket, response, strlen(response)) < 0){
                perror("write failed");
            }
            return;
        }
    }

    char line[1024];
    bool found = false;
    int user_count = 0;

    while(fgets(line,sizeof(line),file)){
        char *username_file = strtok(line, " ");
        if(username_file == NULL) continue;
        username_file=strtok(NULL, ",");
        if(username_file && strcmp(username_file, username)==0){
            found = true;
            break;
        }
        user_count++;
    }

    if(found){
        char response[] = "Username already exists";
        if(write(clinfo->socket, response, strlen(response)) < 0){
            perror("write failed");
        }
        fclose(file);
        return;
    }

    fseek(file, 0, SEEK_END);

    snprintf(line, sizeof(line), "$2y$12$%.22s", "bcrypt_gensalt_random");
    char hashed_password[BCRYPT_HASHSIZE];
    bcrypt_hashpw(password, 10, hashed_password);

    if(hash==NULL){
        char response[] = "Failed to hash password";
        if(write(clinfo->socket, response, strlen(response)) < 0){
            perror("write failed");
        }
        fclose(file);
        return;
    }

    fprintf(file, "%d, %s, %s, %s\n", user_count+1, username, hashed_password, user_count==0?"admin":"user");
    fclose(file);

    char response[100];
    snprintf(response, sizeof(response), "User %s registered", username);
    if(write(clinfo->socket, response, strlen(response)) < 0){
        perror("write failed");
    }
}

// void login_user(int sock, char *username, char *password) {
//     char buffer[1024];
//     sprintf(buffer, "LOGIN %s %s", username, password);
//     send(sock, buffer, strlen(buffer), 0);
//     read(sock, buffer, 1024);
//     printf("%s\n", buffer);
// }

// void create_room(int sock, char *room_name) {
//     char buffer[1024];
//     sprintf(buffer, "CREATE_ROOM %s", room_name);
//     send(sock, buffer, strlen(buffer), 0);
//     read(sock, buffer, 1024);
//     printf("%s\n", buffer);
// }

// void add_allowed_user(int sock, char *room_name, char *username) {
//     char buffer[1024];
//     sprintf(buffer, "ADD_USER %s %s", room_name, username);
//     send(sock, buffer, strlen(buffer), 0);
//     read(sock, buffer, 1024);
//     printf("%s\n", buffer);
// }

// void chat(int sock, char *room_name, char *message) {
//     char buffer[1024];
//     sprintf(buffer, "CHAT %s %s", room_name, message);
//     send(sock, buffer, strlen(buffer), 0);
//     read(sock, buffer, 1024);
//     printf("%s\n", buffer);
// }

// void edit_chat(int sock, char *room_name, int message_id, char *new_message) {
//     char buffer[1024];
//     sprintf(buffer, "EDIT_CHAT %s %d %s", room_name, message_id, new_message);
//     send(sock, buffer, strlen(buffer), 0);
//     read(sock, buffer, 1024);
//     printf("%s\n", buffer);
// }

void handle_client(void *arg) {
    client_t *clinfo = (client_t *)arg;
    char buffer[BUFFER_SIZE];
    int readClient;

    while((readClient=read(clinfo->socket, buffer, sizeof(buffer))) > 0) {
        buffer[readClient] = '\0';
        printf("Client: %s\n", buffer);
        
        char *command = strtok(buffer, " ");
        if(command==NULL){
            char response[] = "Invalid command";
            if(write(clinfo->socket, response, strlen(response)) < 0){
                perror("write failed");
            }
            continue;
        }

        if(strcmp(command, "REGISTER") == 0) {
            char *username = strtok(NULL, " ");
            char *password = strtok(NULL, " ");
            int is_admin = atoi(strtok(NULL, " "));
            reg_user(clinfo->socket, username, password, clinfo);
        } 
        else if(strcmp(command, "LOGIN") == 0) {
        //     char *username = strtok(NULL, " ");
        //     char *password = strtok(NULL, " ");
        //     if(username == NULL || password == NULL){
        //         char response[] = "Invalid command";
        //         if(write(clinfo->socket, response, strlen(response)) < 0){
        //             perror("write failed");
        //         }
        //         continue;
        //     }
        //     login_user(clinfo->socket, username, password);
        // } else if(strcmp(command, "CREATE_ROOM") == 0) {
        //     char *room_name = strtok(NULL, " ");
        //     if(room_name != NULL){
        //         char response[] = "Invalid command";
        //         if(write(clinfo->socket, response, strlen(response)) < 0){
        //             perror("write failed");
        //         }
        //         continue;
        //     }
        //     create_room(clinfo->socket, room_name);
        // } else if(strcmp(command, "ADD_USER") == 0) {
        //     char *room_name = strtok(NULL, " ");
        //     char *username = strtok(NULL, " ");
        //     add_allowed_user(clinfo->socket, room_name, username);
        // } else if(strcmp(command, "CHAT") == 0) {
        //     char *room_name = strtok(NULL, " ");
        //     char *message = strtok(NULL, " ");
        //     chat(clinfo->socket, room_name, message);
        // } else if(strcmp(command, "EDIT_CHAT") == 0) {
        //     char *room_name = strtok(NULL, " ");
        //     int message_id = atoi(strtok(NULL, " "));
        //     char *new_message = strtok(NULL, " ");
        //     edit_chat(clinfo->socket, room_name, message_id, new_message);
        // } else {
        //     char response[] = "Invalid command";
        //     if(write(clinfo->socket, response, strlen(response)) < 0){
        //         perror("write failed");
        //     }
        }
    }
}

int main() {
    daemonize();
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *hello = "Hello from server";

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("failed to listen");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
        perror("failed to accept");
        exit(EXIT_FAILURE);
    }
    valread = read(new_socket, buffer, 1024);
    printf("%s\n", buffer);
    send(new_socket, hello, strlen(hello), 0);
    printf("Hello message sent\n");
    return 0;
}
