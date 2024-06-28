# Sisop-FP-2024-MH-IT22
## Anggota Kelompok
- 5027231003  Chelsea Vania Hariyono
- 5027231024  Furqon Aryadana
- 5027231057  Elgracito Iryanda Endia


### Discorit
```c
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
    // else if(strcmp(argv[1], "LOGIN")==0){
    //     if(argc<5||strcmp(argv[3],"-p")!=0){
    //         printf("Invalid command\n");
    //         return 0;
    //     }

    //     char *username = argv[2];
    //     char *password = argv[4];
    //     snprintf(command, sizeof(command), "LOGIN %s %s", username, password);

    //     if(send(clientSocket, command, strlen(command), 0) < 0){
    //         perror("send failed");
    //         exit(EXIT_FAILURE);
    //     }

    //     char response[1024];
    //     memset(response, 0, sizeof(response));
    // }

    // Send and receive data
    // TODO: Add your code here

    close(clientSocket);

    return 0;
}
```

### Server
```c
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
#include <stdbool.h>
#include <pthread.h>

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

void create_directory(const char *path) {
    struct stat st = {0};

    if (stat(path, &st) == -1) {
        if (mkdir(path, 0700) < 0) {
            perror("Failed to create directory");
            return;
        }
    }
    return;
}

void reg_user(int sock, char *username, char *password, client_t *clinfo) {
    if(username == NULL || password == NULL) {
        char response[] = "Invalid command";
        if(write(clinfo->socket, response, strlen(response)) < 0){
            perror("write failed");
        }
        return;
    }
    char *path="/home/tka/sisop/fp/DiscorIT";
    create_directory(path);


    FILE *fp = fopen("/home/tka/sisop/fp/DiscorIT/users.csv", "r+");
    if(!fp){
        fp = fopen("/home/tka/sisop/fp/DiscorIT/users.csv", "w+");
        if(!fp){
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

    while(fgets(line,sizeof(line),fp)){
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
        fclose(fp);
        return;
    }

    fseek(fp, 0, SEEK_END);

    char salt[64];
    snprintf(salt, sizeof(salt), "$2y$12$%.22s", "bcrypt_gensalt_random");
    char hashed_password[BCRYPT_HASHSIZE];
    bcrypt_hashpw(password, salt, hashed_password);

    if(hashed_password==NULL){
        char response[] = "Failed to hash password";
        if(write(clinfo->socket, response, strlen(response)) < 0){
            perror("write failed");
        }
        fclose(fp);
        return;
    }

    fprintf(fp, "%d, %s, %s, %s\n", user_count+1, username, hashed_password, user_count==0?"admin":"user");
    fclose(fp);

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

void *handle_client(void *arg) {
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
        // else if(strcmp(command, "LOGIN") == 0) {
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
        // }
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
    printf("Server running\n");
    
    while(1){
        if((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("failed to accept");
            exit(EXIT_FAILURE);
        }
        pthread_t thread;
        client_t *clinfo = (client_t *)malloc(sizeof(client_t));
        clinfo->socket = new_socket;
        clinfo->address = address;

        pthread_create(&thread, NULL, handle_client, (void *)clinfo);
    }
}
```

### Monitor
1. User dapat menampilkan isi chat secara real-time menggunakan monitor. Jika ada perubahan pada isi chat, perubahan tersebut akan langsung ditampilkan di terminal.
2. Sebelum dapat menggunakan monitor, user harus login terlebih dahulu seperti login di DiscorIT.
3. Untuk keluar dari room dan menghentikan program monitor dengan perintah "EXIT".
4. Monitor dapat digunakan untuk menampilkan semua chat pada room, mulai dari chat pertama hingga chat yang akan datang.
```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/inotify.h>
#include <limits.h>

#define LOG_FILE "/tmp/monitor.log"
#define WATCH_DIR "/path/to/watch"
#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))

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

    int log_fd = open(LOG_FILE, O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (log_fd < 0) {
        perror("failed to open log file");
        exit(EXIT_FAILURE);
    }
    dup2(log_fd, STDOUT_FILENO);
    dup2(log_fd, STDERR_FILENO);
}

void monitor_directory(const char *path) {
    int length, i = 0;
    int fd;
    int wd;
    char buffer[EVENT_BUF_LEN];

    fd = inotify_init();
    if (fd < 0) {
        perror("inotify_init failed");
        exit(EXIT_FAILURE);
    }

    wd = inotify_add_watch(fd, path, IN_CREATE | IN_MODIFY | IN_DELETE);
    if (wd == -1) {
        fprintf(stderr, "Could not watch %s\n", path);
        exit(EXIT_FAILURE);
    }

    while (1) {
        length = read(fd, buffer, EVENT_BUF_LEN);
        if (length < 0) {
            perror("read failed");
            exit(EXIT_FAILURE);
        }

        while (i < length) {
            struct inotify_event *event = (struct inotify_event *)&buffer[i];
            if (event->len) {
                if (event->mask & IN_CREATE) {
                    printf("The file %s was created.\n", event->name);
                } else if (event->mask & IN_DELETE) {
                    printf("The file %s was deleted.\n", event->name);
                } else if (event->mask & IN_MODIFY) {
                    printf("The file %s was modified.\n", event->name);
                }
            }
            i += EVENT_SIZE + event->len;
        }
        i = 0;
    }

    inotify_rm_watch(fd, wd);
    close(fd);
}

int main() {
    daemonize();
    monitor_directory(WATCH_DIR);
    return 0;
}
```
