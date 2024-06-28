### Sisop-FP-2024-MH-IT22
---
# Laporan Resmi Fun Project Sistem Operasi Kelompok IT22
## Anggota Kelompok
- 5027231003  Chelsea Vania Hariyono
- 5027231024  Furqon Aryadana
- 5027231057  Elgracito Iryanda Endia
---

__Disclaimer__
_Program server, discorit, dan monitor TIDAK DIPERBOLEHKAN menggunakan perintah system();_


__A. Autentikasi__
---
Setiap user harus memiliki username dan password untuk mengakses DiscorIT. Username, password, dan global role disimpan dalam file user.csv. Jika tidak ada user lain dalam sistem, user pertama yang mendaftar otomatis mendapatkan role "ROOT". Username harus bersifat unique dan password wajib di encrypt menggunakan menggunakan bcrypt. Ketika user pertama kali masuk ke channel, mereka memiliki akses terbatas. Jika mereka telah masuk sebelumnya, akses mereka meningkat sesuai dengan admin dan root.

__B. Perbedaan Akses antara Root, Admin, dan User__
---
Root: memiliki akses penuh untuk mengelola semua channel, room, dan user. Root adalah akun yang pertama kali mendaftar.  
Admin: memiliki akses untuk mengelola channel dan room yang mereka buat, serta mengelola user dalam channel mereka.  
User: dapat mengirim pesan chat, melihat channel, dan room. user menjadi admin di channel yang mereka buat.
1. List Channel dan Room
   Setelah Login user dapat melihat daftar channel yang tersedia.
2. Akses Channel dan Room
   Akses channel admin dan root. Ketika user pertama kali masuk ke channel, mereka memiliki akses terbatas. Jika mereka telah masuk sebelumnya, akses mereka meningkat sesuai dengan admin dan root.  
3. Fitur Chat
   Setiap user dapat mengirim pesan dalam chat. ID pesan chat dimulai dari 1 dan semua pesan disimpan dalam file chat.csv. User dapat melihat pesan-pesan chat yang ada dalam room. Serta user dapat edit dan delete pesan yang sudah dikirim dengan menggunakan ID pesan.

__C. Root__
---
Akun yang pertama kali mendaftar otomatis mendapatkan peran "root". Root dapat masuk ke channel manapun tanpa key dan create, update, dan delete pada channel dan room, mirip dengan admin [D]. Root memiliki kemampuan khusus untuk mengelola user, seperti: list, edit, dan Remove.

__D. Admin Channel__
---
Setiap user yang membuat channel otomatis menjadi admin di channel tersebut. Informasi tentang user disimpan dalam file auth.csv. Admin dapat create, update, dan delete pada channel dan room, serta dapat remove, ban, dan unban user di channel mereka.
1. Channel: informasi tentang semua channel disimpan dalam file channel.csv. Semua perubahan dan aktivitas user pada channel dicatat dalam file users.log.
2. Room: semua perubahan dan aktivitas user pada room dicatat dalam file users.log.
3. Ban: admin dapat melakukan ban pada user yang nakal. Aktivitas ban tercatat pada users.log. Ketika di ban, role "user" berubah menjadi "banned". Data tetap tersimpan dan user tidak dapat masuk ke dalam channel.
4. Unban: admin dapat melakukan unban pada user yang sudah berperilaku baik. Aktivitas unban tercatat pada users.log. Ketika di unban, role "banned" berubah kembali menjadi "user" dan dapat masuk ke dalam channel.
5. Remove user: admin dapat remove user dan tercatat pada users.log.

__E. User__
---
User dapat mengubah informasi profil mereka, user yang di ban tidak dapat masuk kedalam channel dan dapat keluar dari room, channel, atau keluar sepenuhnya dari DiscorIT.
1. Edit User Username
2. Edit User Password
3. Banned User
4. Exit

__F. Error Handling__
---
Jika ada command yang tidak sesuai penggunaannya. Maka akan mengeluarkan pesan error dan tanpa keluar dari program client.

__G. Monitor__
---
User dapat menampilkan isi chat secara real-time menggunakan monitor. Jika ada perubahan pada isi chat, perubahan tersebut akan langsung ditampilkan di terminal. Sebelum dapat menggunakan monitor, user harus login terlebih dahulu seperti login di DiscorIT. Untuk keluar dari room dan menghentikan program monitor dengan perintah "EXIT". Monitor dapat digunakan untuk menampilkan semua chat pada room, mulai dari chat pertama hingga chat yang akan datang.

---
## 1. Discorit
Untuk mengakses DiscorIT, user perlu membuka program client (discorit). Discorit hanya bekerja sebagai client yang mengirimkan request user kepada server. Program server berjalan sebagai server yang menerima semua request dari client dan mengembalikan response kepada client sesuai ketentuan pada soal. __Program server berjalan sebagai daemon__. Untuk hanya menampilkan chat, user perlu membuka program client (monitor). Lebih lengkapnya pada poin monitor. Program client dan server berinteraksi melalui socket. Server dapat terhubung dengan lebih dari satu client.

Deklarasi library yang akan digunakan
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <crypt.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/stat.h>
```
Definisi buffer yang digunakan
```c
#define PORT 8080
#define BUFFER_SIZE 1024
```
Fungsi untuk menyambungkan _socket_

```c
int server_fd;

void connect_to_server() {
    struct sockaddr_in serv_addr;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    if (connect(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        exit(EXIT_FAILURE);
    }
}
```
Fungsi untuk _handle command_

```c
void handle_command(const char *command, char *username, char *channel, char *room) {
    if (command == NULL) {
        printf("Perintah tidak boleh kosong\n");
        return;
    }

    if (send(server_fd, command, strlen(command), 0) < 0) {
        perror("Gagal mengirim perintah ke server");
    }

    char response[BUFFER_SIZE];
    memset(response, 0, sizeof(response));
    int n = 0;
    if (recv(server_fd, response, BUFFER_SIZE - 1, 0) < 0) {
        perror("Gagal menerima respons dari server");
    } else {
        if (strstr(response, "Key:") != NULL) {
            char key[50];
            printf("Key: ");
            fgets(key, sizeof(key), stdin);
            key[strcspn(key, "\n")] = '\0'; 

            if (send(server_fd, key, strlen(key), 0) < 0) {
                perror("Gagal mengirim key ke server");
            }

            memset(response, 0, sizeof(response));
            if (recv(server_fd, response, BUFFER_SIZE - 1, 0) < 0) {
                perror("Gagal menerima respons dari server");
            } else {
                if (strstr(response, "Key salah") != NULL) {
                    if (strlen(room) > 0) {
                        room[0] = '\0';
                    } else if (strlen(channel) > 0) {
                        channel[0] = '\0';
                    }
                }
            }
        } else if (strstr(response, "tidak ada") != NULL || strstr(response, "Key salah") != NULL || strstr(response, "Anda telah diban") != NULL) {
            if (strlen(room) > 0) {
                room[0] = '\0';
            } else if (strlen(channel) > 0) {
                channel[0] = '\0';
            }
            printf("%s\n", response);
        } else if (strstr(response, "Anda telah keluar dari aplikasi") != NULL) {
            close(server_fd);
            exit(0);  
        } else if(strncmp(command, "JOIN ", 5) == 0 || strcmp(command, "EXIT") == 0 || strstr(response, "berhasil dikirim") != NULL
                    || strstr(response, "diedit") != NULL || strstr(response, "selamanya") != NULL){
            n++;
        } else {
            printf("%s\n", response);
        }
    }
}
```
Fungsi ```main``` untuk memangggil fungsi dan mengeluarkan output

```c
int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Penggunaan: %s REGISTER/LOGIN username -p password\n", argv[0]);
        return 1;
    }

    connect_to_server();

    char command[BUFFER_SIZE];
    memset(command, 0, sizeof(command));

    char username[50];
    char channel[50] = "";
    char room[50] = "";

    if (strcmp(argv[1], "REGISTER") == 0) {
        if (argc < 5 || strcmp(argv[3], "-p") != 0) {
            printf("Penggunaan: %s REGISTER username -p password\n", argv[0]);
            return 1;
        }

        snprintf(username, sizeof(username), "%s", argv[2]);
        char *password = argv[4];

        snprintf(command, sizeof(command), "REGISTER %s %s", username, password);
    } else if (strcmp(argv[1], "LOGIN") == 0) {
        if (argc < 5 || strcmp(argv[3], "-p") != 0) {
            printf("Penggunaan: %s LOGIN username -p password\n", argv[0]);
            return 1;
        }

        snprintf(username, sizeof(username), "%s", argv[2]);
        char *password = argv[4];

        snprintf(command, sizeof(command), "LOGIN %s %s", username, password);

        if (send(server_fd, command, strlen(command), 0) < 0) {
            perror("Gagal mengirim perintah ke server");
        }

        char response[BUFFER_SIZE];
        memset(response, 0, sizeof(response));

        if (recv(server_fd, response, BUFFER_SIZE - 1, 0) < 0) {
            perror("Gagal menerima respons dari server");
        } else {
            printf("%s\n", response);

            if (strstr(response, "berhasil login") != NULL) {
                while (1) {
                    if (strlen(room) > 0) {
                        printf("[%s/%s/%s] ", username, channel, room);
                    } else if (strlen(channel) > 0) {
                        printf("[%s/%s] ", username, channel);
                    } else {
                        printf("[%s] ", username);
                    }

                    if (fgets(command, BUFFER_SIZE, stdin) == NULL) {
                        printf("Gagal membaca perintah\n");
                        continue;
                    }
                    command[strcspn(command, "\n")] = '\0';

                    if (strncmp(command, "JOIN ", 5) == 0) {
                        if (strlen(channel) == 0) {
                            snprintf(channel, sizeof(channel), "%s", command + 5);
                        } else {
                            snprintf(room, sizeof(room), "%s", command + 5);
                        }
                    } else if (strcmp(command, "EXIT") == 0) {
                        if (strlen(room) > 0) {
                            room[0] = '\0';
                        } else if (strlen(channel) > 0) {
                            channel[0] = '\0';
                        }
                    } else if(strncmp(command, "EDIT PROFILE SELF -u ", 21) == 0){
                        snprintf(username, sizeof(username), "%s", command + 21);
                    }

                    handle_command(command, username, channel, room);
                }
            }
        }

        close(server_fd);
        return 0;
    } else {
        printf("Perintah tidak valid\n");
        return 1;
    }

    handle_command(command, username, channel, room);

    close(server_fd);
    return 0;
}
```

___

## 2. Server
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <bcrypt.h>
#include <stdbool.h>
#include <errno.h>
#include <dirent.h>
#include <ctype.h>
#include <time.h>
```
Deklarasi library yang akan digunakan

```c
#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 10240
#define SALT_SIZE 64
#define USERS_FILE "/home/tka/sisop/fp/DiscorIT/users.c"
#define CHANNELS_FILE "/home/tka/sisop/fp/DiscorIT/channels.csv"


typedef struct {
    int socket;
    struct sockaddr_in address;
    char logged_in_user[50];
    char logged_in_role[10];
    char logged_in_channel[50];
    char logged_in_room[50];
} client_info;
```
Definisikan _port, cbuffer, path,_ dan _struct_

```c
client_info *clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void *handle_client(void *arg);
void daemonize();

void register_user(const char *username, const char *password, client_info *client);
void login_user(const char *username, const char *password, client_info *client);

void create_directory(const char *path, client_info *client);
void create_channel(const char *username, const char *channel, const char *key, client_info *client);
void create_room(const char *username, const char *channel, const char *room, client_info *client);
void list_channels(client_info *client);
void list_rooms(const char *channel, client_info *client);
void list_users(const char *channel, client_info *client);
void join_channel(const char *username, const char *channel, client_info *client);
void verify_key(const char *username, const char *channel, const char *key, client_info *client);
void join_room(const char *channel, const char *room, client_info *client);
void send_chat(const char *username, const char *channel, const char *room, const char *message, client_info *client);
void see_chat(const char *channel, const char *room, client_info *client);
void edit_chat(const char *channel, const char *room, int id_chat, const char *new_text, client_info *client);
void edit_channel(const char *old_channel, const char *new_channel, client_info *client);
void edit_room(const char *channel, const char *old_room, const char *new_room, client_info *client);
void edit_profile_self(const char *username, const char *new_value, bool is_password, client_info *client);
void delete_chat(const char *channel, const char *room, int chat_id, client_info *client);
void delete_directory(const char *path);
void delete_channel(const char *channel, client_info *client);
void delete_room(const char *channel, const char *room, client_info *client);
void delete_all_rooms(const char *channel, client_info *client);
void ban_user(const char *channel, const char *user_to_ban, client_info *client);
void unban_user(const char *channel, const char *user_to_unban, client_info *client);
void remove_user(const char *channel, const char *username, client_info *client);
void log_activity(const char *channel, const char *message);


void list_users_root(client_info *client);
void edit_user(const char *target_user, const char *new_value, bool is_password, client_info *client);
void remove_user_root(const char *target_user, client_info *client);

void handle_exit(client_info *client);
```
Deklarasi _array_ clients dan clients mutex serta fungsi untuk:
1. handle client
2. berjalan secara daemon
3. register user
4. login user
5. create directory
6. create channel
7. create room
8. list channels
9. list rooms
10. list users
11. join channel
12. verify key
13. join room
14. send chat
15. see chat
16. edit chat
17. edit channel
18. edit room
19. edit profile self
20. delete chat
21. delete directory
22. delete channel
23. delete room
24. delete all rooms
25. ban user
26. unban user
27. remove user
28. log activity
29. list users root
30. edit user
31. remove user root
32. handle exit

```c
int main() {
    daemonize();

    int server_fd, new_socket;
    struct sockaddr_in address;
    socklen_t addr_len = sizeof(address);
    pthread_t tid;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Gagal membuat socket");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Gagal bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Gagal listen");
        exit(EXIT_FAILURE);
    }

    printf("Server berjalan sebagai daemon pada port %d\n", PORT);

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addr_len)) < 0) {
            perror("Gagal melakukan accept");
            exit(EXIT_FAILURE);
        }

        pthread_t tid;
        client_info *client = (client_info *)malloc(sizeof(client_info));
        client->socket = new_socket;
        client->address = address;
        memset(client->logged_in_user, 0, sizeof(client->logged_in_user));
        memset(client->logged_in_role, 0, sizeof(client->logged_in_role));
        memset(client->logged_in_channel, 0, sizeof(client->logged_in_channel));
        memset(client->logged_in_room, 0, sizeof(client->logged_in_room));

        pthread_create(&tid, NULL, handle_client, (void *)client);
    }

    return 0;
}

void daemonize() {
    pid_t pid, sid;

    pid = fork();

    if (pid < 0) {
        exit(EXIT_FAILURE);
    }

    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    umask(0);

    sid = setsid();
    if (sid < 0) {
        exit(EXIT_FAILURE);
    }

    if ((chdir("/")) < 0) {
        exit(EXIT_FAILURE);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    int log_fd = open("/tmp/server.log", O_WRONLY | O_CREAT | O_APPEND, 0600);
    if (log_fd < 0) {
        exit(EXIT_FAILURE);
    }
    dup2(log_fd, STDOUT_FILENO);
    dup2(log_fd, STDERR_FILENO);
}

void *handle_client(void *arg) {
    client_info *cli = (client_info *)arg;
    char buffer[BUFFER_SIZE];
    int n;

    while ((n = read(cli->socket, buffer, sizeof(buffer))) > 0) {
        buffer[n] = '\0';
        printf("Pesan dari client: %s\n", buffer);

        char *token = strtok(buffer, " ");
        if (token == NULL) {
            char response[] = "Perintah tidak dikenali";
            if (write(cli->socket, response, strlen(response)) < 0) {
                perror("Gagal mengirim respons ke client");
            }
            continue;
        }

        if (strcmp(token, "REGISTER") == 0) {
            char *username = strtok(NULL, " ");
            char *password = strtok(NULL, " ");
            register_user(username, password, cli);
        } else if (strcmp(token, "LOGIN") == 0) {
            char *username = strtok(NULL, " ");
            char *password = strtok(NULL, " ");
            if (username == NULL || password == NULL) {
                char response[] = "Format perintah LOGIN tidak valid";
                if (write(cli->socket, response, strlen(response)) < 0) {
                    perror("Gagal mengirim respons ke client");
                }
                continue;
            }
            login_user(username, password, cli);
        } else if (strcmp(token, "CREATE") == 0) {
            token = strtok(NULL, " ");
            if (token == NULL) {
                char response[] = "Format perintah CREATE tidak valid";
                if (write(cli->socket, response, strlen(response)) < 0) {
                    perror("Gagal mengirim respons ke client");
                }
                continue;
            }
            if (strcmp(token, "CHANNEL") == 0) {
                char *channel = strtok(NULL, " ");
                token = strtok(NULL, " ");
                char *key = strtok(NULL, " ");
                if (channel == NULL || key == NULL) {
                    char response[] = "Penggunaan perintah: CREATE CHANNEL <channel> -k <key>";
                    if (write(cli->socket, response, strlen(response)) < 0) {
                        perror("Gagal mengirim respons ke client");
                    }
                    continue;
                }
                create_channel(cli->logged_in_user, channel, key, cli);
            } else if (strcmp(token, "ROOM") == 0) {
                char *room = strtok(NULL, " ");
                if (room == NULL) {
                    char response[] = "Penggunaan perintah: CREATE ROOM <room>";
                    if (write(cli->socket, response, strlen(response)) < 0) {
                        perror("Gagal mengirim respons ke client");
                    }
                    continue;
                }
                create_room(cli->logged_in_user, cli->logged_in_channel, room, cli);
            } else {
                char response[] = "Format perintah CREATE tidak valid";
                if (write(cli->socket, response, strlen(response)) < 0) {
                    perror("Gagal mengirim respons ke client");
                }
            }
        } else if(strcmp(token, "LIST") == 0){
            token = strtok(NULL, " ");
            if (token == NULL) {
                char response[] = "Format perintah LIST tidak valid";
                if (write(cli->socket, response, strlen(response)) < 0) {
                    perror("Gagal mengirim respons ke client");
                }
                continue;
            }
            if (strcmp(token, "CHANNEL") == 0) {
                list_channels(cli);
            } else if (strcmp(token, "ROOM") == 0) {
                list_rooms(cli->logged_in_channel, cli);
            } else if (strcmp(token, "USER") == 0) {
                strstr(cli->logged_in_role, "ROOT") != NULL ? list_users_root(cli) : list_users(cli->logged_in_channel, cli);
            } else {
                char response[] = "Format perintah LIST tidak valid";
                if (write(cli->socket, response, strlen(response)) < 0) {
                    perror("Gagal mengirim respons ke client");
                }
            }
        } else if (strcmp(token, "JOIN") == 0) {
            token = strtok(NULL, " ");
            if (token == NULL) {
                char response[] = "Format perintah JOIN tidak valid";
                if (write(cli->socket, response, strlen(response)) < 0) {
                    perror("Gagal mengirim respons ke client");
                }
                continue;
            }
            if (strlen(cli->logged_in_channel) == 0) {
                char *channel = token;
                join_channel(cli->logged_in_user, channel, cli);
            } else {
                char *room = token;
                join_room(cli->logged_in_channel, room, cli);
            }
        } else if(strcmp(token, "CHAT") == 0) {
            char *message = buffer + 5;

            // Periksa apakah pengguna sudah tergabung dalam channel dan room
            if (strlen(cli->logged_in_channel) == 0 || strlen(cli->logged_in_room) == 0) {
                char response[] = "Anda belum tergabung dalam room";
                if (write(cli->socket, response, strlen(response)) < 0) {
                    perror("Gagal mengirim respons ke client");
                }
                continue;
            }

            // Kirim pesan chat
            send_chat(cli->logged_in_user, cli->logged_in_channel, cli->logged_in_room, message, cli);
        } else if (strcmp(token, "SEE") == 0) {
            token = strtok(NULL, " ");
            if (token == NULL || strcmp(token, "CHAT") != 0 || strlen(cli->logged_in_channel) == 0 || strlen(cli->logged_in_room) == 0) {
                char response[] = "Format perintah SEE CHAT tidak valid atau anda belum tergabung dalam room";
                if (write(cli->socket, response, strlen(response)) < 0) {
                    perror("Gagal mengirim respons ke client");
                }
                continue;
            }
            see_chat(cli->logged_in_channel, cli->logged_in_room, cli);
        } else if (strcmp(token, "EDIT") == 0) {
            token = strtok(NULL, " ");
            if (token == NULL) {
                char response[] = "Format perintah EDIT tidak valid";
                if (write(cli->socket, response, strlen(response)) < 0) {
                    perror("Gagal mengirim respons ke client");
                }
                continue;
            }
            if (strcmp(token, "CHAT") == 0) {
                char *id_str = strtok(NULL, " ");
                char *new_text = strtok(NULL, "\"");
                if (id_str == NULL || new_text == NULL) {
                    char response[] = "Penggunaan perintah: EDIT CHAT <id> \"<text>\"";
                    if (write(cli->socket, response, strlen(response)) < 0) {
                        perror("Gagal mengirim respons ke client");
                    }
                    continue;
                }
                int id_chat = atoi(id_str);
                edit_chat(cli->logged_in_channel, cli->logged_in_room, id_chat, new_text, cli);
            } else if (strcmp(token, "CHANNEL") == 0) {
                char *old_channel = strtok(NULL, " ");
                strtok(NULL, " ");  // skip "TO"
                char *new_channel = strtok(NULL, " ");
                if (old_channel == NULL || new_channel == NULL) {
                    char response[] = "Penggunaan perintah: EDIT CHANNEL <old_channel> TO <new_channel>";
                    if (write(cli->socket, response, strlen(response)) < 0) {
                        perror("Gagal mengirim respons ke client");
                    }
                    continue;
                }
                if(strlen(cli->logged_in_channel) > 0 || strlen(cli->logged_in_room) > 0){
                    char response[] = "Anda harus keluar dari channel";
                    if (write(cli->socket, response, strlen(response)) < 0) {
                        perror("Gagal mengirim respons ke client");
                    }
                    continue;
                }else{
                    edit_channel(old_channel, new_channel, cli);
                }
            } else if (strcmp(token, "ROOM") == 0) {
                char *old_room = strtok(NULL, " ");
                strtok(NULL, " ");  // skip "TO"
                char *new_room = strtok(NULL, " ");
                if (old_room == NULL || new_room == NULL) {
                    char response[] = "Penggunaan perintah: EDIT ROOM <old_room> TO <new_room>";
                    if (write(cli->socket, response, strlen(response)) < 0) {
                        perror("Gagal mengirim respons ke client");
                    }
                    continue;
                }
                if(strlen(cli->logged_in_room) > 0){
                    char response[] = "Anda harus keluar dari room";
                    if (write(cli->socket, response, strlen(response)) < 0) {
                        perror("Gagal mengirim respons ke client");
                    }
                    continue;
                }else{
                    edit_room(cli->logged_in_channel, old_room, new_room, cli);
                }
            } else if (strcmp(token, "PROFILE") == 0) {
                strtok(NULL, " ");  // skip "SELF"
                char *option = strtok(NULL, " ");
                char *new_value = strtok(NULL, " ");
                if (option == NULL || new_value == NULL || strcmp(option, "-u") != 0 && strcmp(option, "-p") != 0) {
                    char response[] = "Penggunaan perintah: EDIT PROFILE SELF -u <new_username> atau -p <new_password>";
                    if (write(cli->socket, response, strlen(response)) < 0) {
                        perror("Gagal mengirim respons ke client");
                    }
                    continue;
                }
                bool is_password = (strcmp(option, "-p") == 0);
                edit_profile_self(cli->logged_in_user, new_value, is_password, cli);
            } else if (strcmp(token, "WHERE") == 0) {
                char *target_user = strtok(NULL, " ");
                char *option = strtok(NULL, " ");
                char *new_value = strtok(NULL, " ");
                if (target_user == NULL || option == NULL || new_value == NULL) {
                    char response[] = "Penggunaan perintah: EDIT WHERE <username> -u <new_username> atau -p <new_password>";
                    if (write(cli->socket, response, strlen(response)) < 0) {
                        perror("Gagal mengirim respons ke client");
                    }
                    continue;
                }
                bool is_password = (strcmp(option, "-p") == 0);
                edit_user(target_user, new_value, is_password, cli);
            } else {
                char response[] = "Format perintah EDIT tidak valid";
                if (write(cli->socket, response, strlen(response)) < 0) {
                    perror("Gagal mengirim respons ke client");
                }
            }
        } else if (strcmp(token, "DEL") == 0){
            token = strtok(NULL, " ");
            if (token == NULL) {
                char response[] = "Format perintah DEL tidak valid";
                if (write(cli->socket, response, strlen(response)) < 0) {
                    perror("Gagal mengirim respons ke client");
                }
                continue;
            }
            if (strcmp(token, "CHAT") == 0) {
                char *chat_id_str = strtok(NULL, " ");
                if(cli->logged_in_channel == NULL || cli->logged_in_room == NULL){
                    char response[] = "Anda belum tergabung dalam room";
                    if (write(cli->socket, response, strlen(response)) < 0) {
                        perror("Gagal mengirim respons ke client");
                    }
                    continue;
                }
                if (chat_id_str == NULL) {
                    char response[] = "Penggunaan perintah: DEL CHAT <id>";
                    if (write(cli->socket, response, strlen(response)) < 0) {
                        perror("Gagal mengirim respons ke client");
                    }
                    continue;
                }
                int chat_id = atoi(chat_id_str);
                delete_chat(cli->logged_in_channel, cli->logged_in_room, chat_id, cli);
            } else if (strcmp(token, "CHANNEL") == 0) {
                char *channel = strtok(NULL, " ");
                if(strlen(cli->logged_in_channel) > 0 || strlen(cli->logged_in_room) > 0){
                    char response[] = "Anda harus keluar dari channel";
                    if (write(cli->socket, response, strlen(response)) < 0) {
                        perror("Gagal mengirim respons ke client");
                    }
                    continue;
                }else if(channel == NULL){
                    char response[] = "Penggunaan perintah: DEL CHANNEL <channel>";
                    if (write(cli->socket, response, strlen(response)) < 0) {
                        perror("Gagal mengirim respons ke client");
                    }
                    continue;
                    
                }else{
                    delete_channel(channel, cli);
                }
            } else if (strcmp(token, "ROOM") == 0) {
                token = strtok(NULL, " ");
                if (strcmp(token, "ALL") == 0){
                    if(strlen(cli->logged_in_room) > 0 || strlen(cli->logged_in_channel) == 0){
                        char response[] = "Anda harus keluar dari room atau bergabung ke dalam channel";
                        if (write(cli->socket, response, strlen(response)) < 0) {
                            perror("Gagal mengirim respons ke client");
                        }
                        continue;
                    }else{
                        delete_all_rooms(cli->logged_in_channel, cli);
                    }
                }else{
                    char *room = token;
                    if(strlen(cli->logged_in_room) > 0 || strlen(cli->logged_in_channel) == 0){
                        char response[] = "Anda harus keluar dari room atau bergabung ke dalam channel";
                        if (write(cli->socket, response, strlen(response)) < 0) {
                            perror("Gagal mengirim respons ke client");
                        }
                        continue;
                    }else if(room == NULL){
                        char response[] = "Penggunaan perintah: DEL ROOM <room>";
                        if (write(cli->socket, response, strlen(response)) < 0) {
                            perror("Gagal mengirim respons ke client");
                        }
                        continue;
                    }else{
                        delete_room(cli->logged_in_channel, room, cli);
                    }
                }
            } else {
                char response[] = "Format perintah DEL tidak valid";
                if (write(cli->socket, response, strlen(response)) < 0) {
                    perror("Gagal mengirim respons ke client");
                }
            }
        } else if (strcmp(token, "BAN") == 0) {
            if(strlen(cli->logged_in_channel) == 0){
                    char response[] = "Anda belum bergabung dalam channel";
                    if (write(cli->socket, response, strlen(response)) < 0) {
                        perror("Gagal mengirim respons ke client");
                    }
                    continue;
            }
            char *user_to_ban = strtok(NULL, " ");
            if (user_to_ban == NULL) {
                char response[] = "Format perintah BAN tidak valid";
                if (write(cli->socket, response, strlen(response)) < 0) {
                    perror("Gagal mengirim respons ke client");
                }
                continue;
            }
            ban_user(cli->logged_in_channel, user_to_ban, cli);
        } else if (strcmp(token, "UNBAN") == 0) {
            if(strlen(cli->logged_in_channel) == 0){
                    char response[] = "Anda belum bergabung dalam channel";
                    if (write(cli->socket, response, strlen(response)) < 0) {
                        perror("Gagal mengirim respons ke client");
                    }
                    continue;
            }
            char *user_to_unban = strtok(NULL, " ");
            if (user_to_unban == NULL) {
                char response[] = "Format perintah UNBAN tidak valid";
                if (write(cli->socket, response, strlen(response)) < 0) {
                    perror("Gagal mengirim respons ke client");
                }
                continue;
            }
            unban_user(cli->logged_in_channel, user_to_unban, cli);
        } else if (strcmp(token, "REMOVE") == 0) {
            token = strtok(NULL, " ");
            if (token == NULL) {
                char response[] = "Format perintah REMOVE tidak valid";
                if (write(cli->socket, response, strlen(response)) < 0) {
                    perror("Gagal mengirim respons ke client");
                }
                continue;
            }
            if (strcmp(token, "USER") == 0) {
                if(strlen(cli->logged_in_channel) == 0){
                    char response[] = "Anda belum bergabung dalam channel";
                    if (write(cli->socket, response, strlen(response)) < 0) {
                        perror("Gagal mengirim respons ke client");
                    }
                    continue;
                }
                char *target_user = strtok(NULL, " ");
                if (target_user == NULL) {
                    char response[] = "Penggunaan perintah: REMOVE USER <username>";
                    if (write(cli->socket, response, strlen(response)) < 0) {
                        perror("Gagal mengirim respons ke client");
                    }
                    continue;
                }
                remove_user(cli->logged_in_channel, target_user, cli);
            } else {
                char *target_user = token;
                remove_user_root(target_user, cli);
            }
        } else if (strcmp(token, "EXIT") == 0) {
            handle_exit(cli);
        } else {
            char response[] = "Perintah tidak dikenali";
            if (write(cli->socket, response, strlen(response)) < 0) {
                perror("Gagal mengirim respons ke client");
            }
        }
    }

    close(cli->socket);
    free(cli);
    pthread_exit(NULL);
}

void create_directory(const char *path, client_info *client) {
    struct stat st = {0};

    if (stat(path, &st) == -1) {
        if (mkdir(path, 0700) < 0) {
            char response[] = "Gagal membuat direktori";
            if (write(client->socket, response, strlen(response)) < 0) {
                perror("Gagal mengirim respons ke client");
            }
        }
    }
}

void register_user(const char *username, const char *password, client_info *client) {
    if (username == NULL || password == NULL) {
        char response[] = "Username atau password tidak boleh kosong";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }
    create_directory("/home/tka/sisop/fp/DiscorIT", client);

    FILE *file = fopen(USERS_FILE, "r+");
    if (!file) {
        file = fopen(USERS_FILE, "w+");
        if (!file) {
            perror("Tidak dapat membuka atau membuat file");
            char response[] = "Tidak dapat membuka atau membuat file users.csv";
            if (write(client->socket, response, strlen(response)) < 0) {
                perror("Gagal mengirim respons ke client");
            }
            return;
        }
    }

    char line[256];
    bool user_exists = false;
    int user_count = 0;

    while (fgets(line, sizeof(line), file)) {
        char *token = strtok(line, ",");
        if (token == NULL) continue;
        token = strtok(NULL, ",");
        if (token && strcmp(token, username) == 0) {
            user_exists = true;
            break;
        }
        user_count++;
    }

    if (user_exists) {
        char response[100];
        snprintf(response, sizeof(response), "%s sudah terdaftar", username);
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        fclose(file);
        return;
    }

    fseek(file, 0, SEEK_END);

    char salt[SALT_SIZE];
    snprintf(salt, sizeof(salt), "$2y$12$%.22s", "inistringsaltuntukbcrypt");
    char hash[BCRYPT_HASHSIZE];
    bcrypt_hashpw(password, salt, hash);

    if (hash == NULL) {
        char response[] = "Gagal membuat hash password";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        fclose(file);
        return;
    }

    fprintf(file, "%d,%s,%s,%s\n", user_count + 1, username, hash, user_count == 0 ? "ROOT" : "USER");
    fclose(file);

    char response[100];
    snprintf(response, sizeof(response), "%s berhasil register", username);
    if (write(client->socket, response, strlen(response)) < 0) {
        perror("Gagal mengirim respons ke client");
    }
}

void login_user(const char *username, const char *password, client_info *client) {
    FILE *file = fopen(USERS_FILE, "r");
    if (!file) {
        char response[] = "Tidak dapat membuka file users.csv atau user belum terdaftar";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    char line[256];
    bool user_found = false;

    while (fgets(line, sizeof(line), file)) {
        char *token = strtok(line, ",");
        token = strtok(NULL, ",");
        if (token && strcmp(token, username) == 0) {
            user_found = true;
            token = strtok(NULL, ","); // Hash password
            char *stored_hash = token;

            if (bcrypt_checkpw(password, stored_hash) == 0){
                snprintf(client->logged_in_user, sizeof(client->logged_in_user), "%s", username);
                token = strtok(NULL, ","); // Role
                snprintf(client->logged_in_role, sizeof(client->logged_in_role), "%s", token);

                char response[BUFFER_SIZE];
                snprintf(response, sizeof(response), "%s berhasil login", username);
                if (write(client->socket, response, strlen(response)) < 0) {
                    perror("Gagal mengirim respons ke client");
                }
            } else {
                char response[] = "Password salah";
                if (write(client->socket, response, strlen(response)) < 0) {
                    perror("Gagal mengirim respons ke client");
                }
            }
            break;
        }
    }

    if (!user_found) {
        char response[] = "Username tidak ditemukan";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
    }

    fclose(file);
}

void create_channel(const char *username, const char *channel, const char *key, client_info *client) {
    FILE *channels_file = fopen(CHANNELS_FILE, "r+");
    if (!channels_file) {
        channels_file = fopen(CHANNELS_FILE, "w+");
        if (!channels_file) {
            perror("Tidak dapat membuka atau membuat file channels");
            return;
        }
    }

    char line[256];
    bool channel_exists = false;
    int channel_count = 0;

    while (fgets(line, sizeof(line), channels_file)) {
        char *token = strtok(line, ",");
        token = strtok(NULL, ",");
        if (token && strcmp(token, channel) == 0) {
            channel_exists = true;
            break;
        }
        channel_count++;
    }

    if (channel_exists) {
        char response[100];
        snprintf(response, sizeof(response), "Channel %s sudah ada silakan cari nama lain", channel);
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        fclose(channels_file);
        return;
    }

    fseek(channels_file, 0, SEEK_END);

    char salt[SALT_SIZE];
    snprintf(salt, sizeof(salt), "$2y$12$%.22s", "inistringsaltuntukbcrypt");
    char hash[BCRYPT_HASHSIZE];
    bcrypt_hashpw(key, salt, hash);

    fprintf(channels_file, "%d,%s,%s\n", channel_count + 1, channel, hash);
    fclose(channels_file);

    char path[256];
    snprintf(path, sizeof(path), "/home/tka/sisop/fp/DiscorIT/%s", channel);
    create_directory(path, client);

    snprintf(path, sizeof(path), "/home/tka/sisop/fp/DiscorIT/%s/admin", channel);
    create_directory(path, client);

    snprintf(path, sizeof(path), "/home/tka/sisop/fp/DiscorIT/%s/admin/auth.csv", channel);
    FILE *auth_file = fopen(path, "w+");
    if (auth_file) {
        // Get user id from users.csv
        char user_id[10];
        FILE *users_file = fopen(USERS_FILE, "r");
        if (!users_file) {
            char response[] = "Gagal membuka file users.csv";
            if (write(client->socket, response, strlen(response)) < 0) {
                perror("Gagal mengirim respons ke client");
            }
            fclose(auth_file);
            return;
        }

        char user_line[256];
        bool user_found = false;

        while (fgets(user_line, sizeof(user_line), users_file)) {
            char *token = strtok(user_line, ",");
            strcpy(user_id, token);
            token = strtok(NULL, ",");
            if (token && strcmp(token, username) == 0) {
                user_found = true;
                break;
            }
        }

        fclose(users_file);

        if (user_found) {
            fprintf(auth_file, "%s,%s,ADMIN\n", user_id, username);
        } else {
            char response[] = "User tidak ditemukan";
            if (write(client->socket, response, strlen(response)) < 0) {
                perror("Gagal mengirim respons ke client");
            }
        }
        fclose(auth_file);
    } else {
        char response[] = "Gagal membuat file auth.csv";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
    }

    char log_path[256];
    snprintf(log_path, sizeof(log_path), "/home/tka/sisop/fp/DiscorIT/%s/admin/user.log", channel);
    FILE *log_file = fopen(log_path, "w+");
    if (log_file) {
        fclose(log_file);
    } else {
        char response[] = "Gagal membuat file user.log";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
    }

    char response[100];
    snprintf(response, sizeof(response), "Channel %s dibuat", channel);
    if (write(client->socket, response, strlen(response)) < 0) {
        perror("Gagal mengirim respons ke client");
    }

    char log_message[100];
    snprintf(log_message, sizeof(log_message), "ADMIN membuat channel %s", channel);
    log_activity(channel, log_message);
}

void create_room(const char *username, const char *channel, const char *room, client_info *client) {
    char auth_path[256];
    snprintf(auth_path, sizeof(auth_path), "/home/tka/sisop/fp/DiscorIT/%s/admin/auth.csv", channel);

    FILE *auth_file = fopen(auth_path, "r");
    if (!auth_file) {
        char response[] = "Gagal membuka file auth.csv atau anda tidak tergabung dalam channel";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    char line[256];
    bool is_admin = false;
    bool is_root = false;

    while (fgets(line, sizeof(line), auth_file)) {
        char *token = strtok(line, ",");
        if (token == NULL) continue;
        token = strtok(NULL, ",");
        if (token == NULL) continue;
        if (strcmp(token, username) == 0) {
            token = strtok(NULL, ",");
            if (strstr(token, "ADMIN") != NULL) {
                is_admin = true;
            } else if (strstr(token, "ROOT") != NULL) {
                is_root = true;
            }
        }
    }

    fclose(auth_file);

    if (!is_admin && !is_root) {
        char response[] = "Anda tidak memiliki izin untuk membuat room di channel ini";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    char check_path[256];
    snprintf(check_path, sizeof(check_path), "/home/tka/sisop/fp/DiscorIT/%s/%s", channel, room);
    struct stat st;
    if (stat(check_path, &st) == 0 && S_ISDIR(st.st_mode)) {
        char response[] = "Nama room sudah digunakan";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    char path[256];
    snprintf(path, sizeof(path), "/home/tka/sisop/fp/DiscorIT/%s/%s", channel, room);
    create_directory(path, client);

    snprintf(path, sizeof(path), "/home/tka/sisop/fp/DiscorIT/%s/%s/chat.csv", channel, room);
    FILE *chat_file = fopen(path, "w+");
    if(chat_file){
        fclose(chat_file);
    }else{
        char response[] = "Gagal membuat file chat.csv";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }
    
    char response[BUFFER_SIZE];
    snprintf(response, sizeof(response), "Room %s dibuat", room);
    if (write(client->socket, response, strlen(response)) < 0) {
        perror("Gagal mengirim respons ke client");
    }

    char log_message[100];
    if(is_root){
        snprintf(log_message, sizeof(log_message), "ROOT membuat room %s", room);
    }else{
        snprintf(log_message, sizeof(log_message), "ADMIN membuat room %s", room);
    }
    log_activity(channel, log_message);
}

void list_channels(client_info *client) {
    char path[256];
    strcpy(path, CHANNELS_FILE);
    FILE *channels_file = fopen(path, "r+");
    if (channels_file == NULL) {
        char response[] = "Gagal membuka file channels.csv atau belum ada channel yang dibuat";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    char line[256];
    char response[BUFFER_SIZE] = "";

    while (fgets(line, sizeof(line), channels_file)) {
        char *token = strtok(line, ",");
        if (token == NULL) continue;
        token = strtok(NULL, ",");
        if (token == NULL) continue;
        snprintf(response + strlen(response), sizeof(response) - strlen(response), "%s ", token);
    }

    if(strlen(response) == 0){
        snprintf(response, sizeof(response), "Tidak ada channel yang ditemukan");
    }

    if (write(client->socket, response, strlen(response)) < 0) {
        perror("Gagal mengirim respons ke client");
    }

    fclose(channels_file);
}

void list_rooms(const char *channel, client_info *client) {
    char path[256];
    snprintf(path, sizeof(path), "/home/tka/sisop/fp/DiscorIT/%s", channel);
    DIR *dir = opendir(path);
    if (dir == NULL) {
        char response[] = "Gagal membuka direktori channel atau belum ada room yang dibuat";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    struct dirent *entry;
    char response[BUFFER_SIZE] = "";

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0 && strcmp(entry->d_name, "admin") != 0) {
            char entry_path[512];
            snprintf(entry_path, sizeof(entry_path), "%s/%s", path, entry->d_name);
            struct stat entry_stat;
            if (stat(entry_path, &entry_stat) == 0 && S_ISDIR(entry_stat.st_mode)) {
                snprintf(response + strlen(response), sizeof(response) - strlen(response), "%s ", entry->d_name);
            }
        }
    }

    if (strlen(response) == 0) {
        snprintf(response, sizeof(response), "Tidak ada room yang ditemukan");
    }

    if (write(client->socket, response, strlen(response)) < 0) {
        perror("Gagal mengirim respons ke client");
    }

    closedir(dir);
}

void list_users(const char *channel, client_info *client) {
    char path[256];
    snprintf(path, sizeof(path), "/home/tka/sisop/fp/DiscorIT/%s/admin/auth.csv", channel);
    FILE *auth_file = fopen(path, "r+");
    if (auth_file == NULL) {
        char response[] = "Gagal membuka file auth.csv atau anda sedang tidak tergabung dalam channel";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    char line[256];
    char response[BUFFER_SIZE] = "";

    while (fgets(line, sizeof(line), auth_file)) {
        char *token = strtok(line, ",");
        if (token == NULL) continue;
        token = strtok(NULL, ",");
        if (token == NULL) continue;
        snprintf(response + strlen(response), sizeof(response) - strlen(response), "%s ", token);
    }

    if (write(client->socket, response, strlen(response)) < 0) {
        perror("Gagal mengirim respons ke client");
    }

    fclose(auth_file);
}

void list_users_root(client_info *client) {
    FILE *users_file = fopen(USERS_FILE, "r+");
    if (users_file == NULL) {
        char response[] = "Gagal membuka file users.csv";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    char line[256];
    char response[BUFFER_SIZE] = "";

    while (fgets(line, sizeof(line), users_file)) {
        char *token = strtok(line, ",");
        if (token == NULL) continue;
        token = strtok(NULL, ",");
        if (token == NULL) continue;
        snprintf(response + strlen(response), sizeof(response) - strlen(response), "%s ", token);
    }

    if (write(client->socket, response, strlen(response)) < 0) {
        perror("Gagal mengirim respons ke client");
    }

    fclose(users_file);
}

void join_channel(const char *username, const char *channel, client_info *client) {
    // Check if the channel directory exists
    char channel_path[256];
    snprintf(channel_path, sizeof(channel_path), "/home/tka/sisop/fp/DiscorIT/%s", channel);
    struct stat st;
    if (stat(channel_path, &st) == -1 || !S_ISDIR(st.st_mode)) {
        char response[BUFFER_SIZE];
        snprintf(response, sizeof(response), "Channel %s tidak ada", channel);
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    // Check if user is ROOT in users.csv
    FILE *users_file = fopen(USERS_FILE, "r");
    if (!users_file) {
        char response[] = "Gagal membuka file users.csv";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    char line[256];
    bool is_root = false;
    char user_id[10];

    while (fgets(line, sizeof(line), users_file)) {
        char *token = strtok(line, ",");
        strcpy(user_id, token);
        token = strtok(NULL, ",");
        char *name = token;
        if (token && strstr(name, username) != NULL){
            token = strtok(NULL, ",");
            token = strtok(NULL, ",");
            char *role = token;
            if (strstr(role, "ROOT") != NULL){
                is_root = true;
            }
            break;
        }
    }

    fclose(users_file);

    if (is_root) {
        // If ROOT, join without further checks
        snprintf(client->logged_in_channel, sizeof(client->logged_in_channel), "%s", channel);

        // Ensure ROOT role is recorded in auth.csv
        char auth_path[256];
        snprintf(auth_path, sizeof(auth_path), "/home/tka/sisop/fp/DiscorIT/%s/admin/auth.csv", channel);
        FILE *auth_file = fopen(auth_path, "r+");
        if (auth_file) {
            bool root_exists = false;
            while (fgets(line, sizeof(line), auth_file)) {
                char *token = strtok(line, ",");
                if (token == NULL) continue;
                token = strtok(NULL, ",");
                if (token == NULL) continue;
                if (strcmp(token, username) == 0) {
                    root_exists = true;
                    break;
                }
            }

            if (!root_exists) {
                auth_file = fopen(auth_path, "a");
                if (auth_file) {
                    fprintf(auth_file, "%s,%s,ROOT\n", user_id, username);
                    fclose(auth_file);
                }
            } else {
                fclose(auth_file);
            }
        } else {
            char response[] = "Gagal membuka file auth.csv";
            if (write(client->socket, response, strlen(response)) < 0) {
                perror("Gagal mengirim respons ke client");
            }
            return;
        }

        char response[BUFFER_SIZE];
        snprintf(response, sizeof(response), "[%s/%s]", username, channel);
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    // Check if user is ADMIN/USER/BANNED in auth.csv
    char auth_path[256];
    snprintf(auth_path, sizeof(auth_path), "/home/tka/sisop/fp/DiscorIT/%s/admin/auth.csv", channel);
    FILE *auth_file = fopen(auth_path, "r");
    if (!auth_file) {
        char response[] = "Gagal membuka file auth.csv";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    bool is_admin = false;
    bool is_user = false;
    bool is_banned = false;

    while (fgets(line, sizeof(line), auth_file)) {
        char *token = strtok(line, ",");
        if (token == NULL) continue;
        token = strtok(NULL, ",");
        if (token == NULL) continue;
        if (strcmp(token, username) == 0) {
            token = strtok(NULL, ",");
            if (strstr(token, "ADMIN") != NULL) {
                is_admin = true;
                break;
            } else if (strstr(token, "USER") != NULL){
                is_user = true;
                break;
            } else if (strstr(token, "BANNED") != NULL){
                is_banned = true;
                break;
            }
        }
    }

    fclose(auth_file);

    if (is_banned) {
        char response[] = "Anda telah diban, silahkan menghubungi admin";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    if (is_admin || is_user) {
        snprintf(client->logged_in_channel, sizeof(client->logged_in_channel), "%s", channel);
        char response[BUFFER_SIZE];
        snprintf(response, sizeof(response), "[%s/%s]", username, channel);
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return; // ADMIN or already registered USER joined without further checks
    } else {
        // If not ROOT, ADMIN, or already registered USER, prompt for key
        char response[] = "Key: ";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }

        char key[BUFFER_SIZE];
        memset(key, 0, sizeof(key));

        if (recv(client->socket, key, sizeof(key), 0) < 0) {
            perror("Gagal menerima key dari client");
            return;
        }
        verify_key(username, channel, key, client);
    }
}

void verify_key(const char *username, const char *channel, const char *key, client_info *client) {
    FILE *channels_file = fopen(CHANNELS_FILE, "r");
    if (!channels_file) {
        char response[] = "Gagal membuka file channels.csv";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    char line[256];
    bool key_valid = false;

    while (fgets(line, sizeof(line), channels_file)) {
        char *token = strtok(line, ",");
        token = strtok(NULL, ",");
        if (token && strcmp(token, channel) == 0) {
            token = strtok(NULL, ","); // Get the stored hash
            char *stored_hash = token;
            stored_hash[strcspn(stored_hash, "\n")] = 0;
            if(token && bcrypt_checkpw(key, stored_hash) == 0){
                key_valid = true;
            }
            break;
        }
    }

    fclose(channels_file);

    if (key_valid) {
        FILE *users_file = fopen(USERS_FILE, "r");
        if (!users_file) {
            char response[] = "Gagal membuka file users.csv";
            if (write(client->socket, response, strlen(response)) < 0) {
                perror("Gagal mengirim respons ke client");
            }
            return;
        }

        char user_line[256];
        char user_id[10];
        bool user_found = false;

        while (fgets(user_line, sizeof(user_line), users_file)) {
            char *token = strtok(user_line, ",");
            strcpy(user_id, token);
            token = strtok(NULL, ",");
            if (token && strcmp(token, username) == 0) {
                user_found = true;
                break;
            }
        }

        fclose(users_file);

        if (user_found) {
            char auth_path[256];
            snprintf(auth_path, sizeof(auth_path), "/home/tka/sisop/fp/DiscorIT/%s/admin/auth.csv", channel);
            FILE *auth_file = fopen(auth_path, "a");
            if (auth_file) {
                fprintf(auth_file, "%s,%s,USER\n", user_id, username);
                fclose(auth_file);

                snprintf(client->logged_in_channel, sizeof(client->logged_in_channel), "%s", channel);
                char response[BUFFER_SIZE];
                snprintf(response, sizeof(response), "[%s/%s]", username, channel);
                if (write(client->socket, response, strlen(response)) < 0) {
                    perror("Gagal mengirim respons ke client");
                }
            } else {
                char response[] = "Gagal membuka file auth.csv";
                if (write(client->socket, response, strlen(response)) < 0) {
                    perror("Gagal mengirim respons ke client");
                }
            }
        } else {
            char response[] = "User tidak ditemukan";
            if (write(client->socket, response, strlen(response)) < 0) {
                perror("Gagal mengirim respons ke client");
            }
        }
    } else {
        char response[] = "Key salah";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
    }

    
}

void join_room(const char *channel, const char *room, client_info *client) {
    // Check if the room directory exists
    char room_path[256];
    snprintf(room_path, sizeof(room_path), "/home/tka/sisop/fp/DiscorIT/%s/%s", channel, room);
    struct stat st;
    if (stat(room_path, &st) == -1 || !S_ISDIR(st.st_mode)) {
        char response[BUFFER_SIZE];
        snprintf(response, sizeof(response), "Room %s tidak ada di channel %s", room, channel);
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    snprintf(client->logged_in_room, sizeof(client->logged_in_room), "%s", room);
    char response[BUFFER_SIZE];
    snprintf(response, sizeof(response), "[%s/%s/%s]", client->logged_in_user, channel, room);
    if (write(client->socket, response, strlen(response)) < 0) {
        perror("Gagal mengirim respons ke client");
    }
}

void send_chat(const char *username, const char *channel, const char *room, const char *message, client_info *client) {
    char *startquote = strchr(message, '\"');
    char *endquote = strrchr(message, '\"');

    if (startquote == NULL || endquote == NULL || startquote == endquote) {
        char response[] = "Penggunaan: CHAT \"<pesan>\"";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    char message_trimmed[BUFFER_SIZE];
    memset(message_trimmed, 0, sizeof(message_trimmed));
    strncpy(message_trimmed, message + 1, endquote - startquote - 1);

    if(strlen(message_trimmed) == 0){
        char response[] = "Pesan tidak boleh kosong";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    char path[256];
    snprintf(path, sizeof(path), "/home/tka/sisop/fp/DiscorIT/%s/%s/chat.csv", channel, room);
    FILE *chat_file = fopen(path, "a+");
    if (!chat_file) {
        char response[] = "Gagal membuka file chat.csv";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    // Get the last chat ID
    int last_id = 0;
    char line[512];
    while (fgets(line, sizeof(line), chat_file)) {
        char *token = strtok(line, "|"); // date
        token = strtok(NULL, "|"); // id_chat
        if (token) {
            last_id = atoi(token);
        }
    }

    int id_chat = last_id + 1;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char date[30];
    strftime(date, sizeof(date), "%d/%m/%Y %H:%M:%S", t);

    fprintf(chat_file, "%s|%d|%s|%s\n", date, id_chat, username, message_trimmed);
    fclose(chat_file);

    char response[100];
    snprintf(response, sizeof(response), "Pesan berhasil dikirim");
    if (write(client->socket, response, strlen(response)) < 0) {
        perror("Gagal mengirim respons ke client");
    }
}

void see_chat(const char *channel, const char *room, client_info *client) {
    char path[256];
    snprintf(path, sizeof(path), "/home/tka/sisop/fp/DiscorIT/%s/%s/chat.csv", channel, room);
    FILE *chat_file = fopen(path, "r");
    if (!chat_file) {
        char response[] = "Gagal membuka file chat.csv atau belum ada chat di room ini";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    char line[512];
    char response[BUFFER_SIZE] = "";
    bool has_chat = false;

    while (fgets(line, sizeof(line), chat_file)) {
        has_chat = true;
        char *date = strtok(line, "|");
        char *id_chat = strtok(NULL, "|");
        char *sender = strtok(NULL, "|");
        char *chat = strtok(NULL, "|");

        chat[strcspn(chat, "\n")] = '\0';
            
        if (date && id_chat && sender && chat) {
            snprintf(response + strlen(response), sizeof(response) - strlen(response), "[%s][%s][%s] “%s” \n", date, id_chat, sender, chat);
        }
    }

    if (!has_chat) {
        snprintf(response, sizeof(response), "Belum ada chat");
    }

    if (write(client->socket, response, strlen(response)) < 0) {
        perror("Gagal mengirim respons ke client");
    }

    fclose(chat_file);
}

void edit_chat(const char *channel, const char *room, int id_chat, const char *new_text, client_info *client) {
    char path[256];
    snprintf(path, sizeof(path), "/home/tka/sisop/fp/DiscorIT/%s/%s/chat.csv", channel, room);
    FILE *chat_file = fopen(path, "r+");
    if (!chat_file) {
        char response[] = "Gagal membuka file chat.csv";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    char temp_path[256];
    snprintf(temp_path, sizeof(temp_path), "/home/tka/sisop/fp/DiscorIT/%s/%s/chat_temp.csv", channel, room);
    FILE *temp_file = fopen(temp_path, "w");
    if (!temp_file) {
        char response[] = "Gagal membuat file sementara untuk edit chat";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        fclose(chat_file);
        return;
    }

    char line[512];
    bool found = false;

    while (fgets(line, sizeof(line), chat_file)) {
        char *date = strtok(line, "|");
        char *id_str = strtok(NULL, "|");
        int id = atoi(id_str);
        char *sender = strtok(NULL, "|");
        char *chat = strtok(NULL, "\n");

        if (id == id_chat) {
            found = true;
            fprintf(temp_file, "%s|%d|%s|%s\n", date, id, sender, new_text);
        } else {
            fprintf(temp_file, "%s|%d|%s|%s\n", date, id, sender, chat);
        }
    }

    fclose(chat_file);
    fclose(temp_file);

    if (found) {
        remove(path);
        rename(temp_path, path);
        char response[100];
        snprintf(response, sizeof(response), "Chat dengan ID %d diedit", id_chat);
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
    } else {
        remove(temp_path);
        char response[100];
        snprintf(response, sizeof(response), "Chat dengan ID %d tidak ditemukan", id_chat);
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
    }
}

void edit_channel(const char *old_channel, const char *new_channel, client_info *client) {
    char auth_path[256];
    snprintf(auth_path, sizeof(auth_path), "/home/tka/sisop/fp/DiscorIT/%s/admin/auth.csv", old_channel);
    FILE *auth_file = fopen(auth_path, "r");
    if (!auth_file) {
        char response[] = "Gagal membuka file auth.csv";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    FILE *users_file = fopen(USERS_FILE, "r");
    if (!users_file) {
        char response[] = "Gagal membuka file users.csv";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    char rootline[256];
    bool is_root = false;

    while (fgets(rootline, sizeof(rootline), users_file)) {
        char *token = strtok(rootline, ",");
        token = strtok(NULL, ",");
        if (token && strcmp(token, client->logged_in_user) == 0) {
            token = strtok(NULL, ",");
            token = strtok(NULL, ",");
            if (strstr(token, "ROOT") != NULL){
                is_root = true;
            }
            break;
        }
    }

    fclose(users_file);

    char line[256];
    bool is_admin = false;
    while (fgets(line, sizeof(line), auth_file)) {
        char *token = strtok(line, ",");
        if (token == NULL) continue;
        token = strtok(NULL, ",");
        if (token == NULL) continue;
        if (strcmp(token, client->logged_in_user) == 0) {
            token = strtok(NULL, ",");
            if (strstr(token, "ADMIN") != NULL) {
                is_admin = true;
                break;
            }
        }
    }

    fclose(auth_file);

    if (!is_admin && !is_root) {
        char response[] = "Anda tidak memiliki izin untuk mengedit channel";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    FILE *channels_file = fopen(CHANNELS_FILE, "r+");
    if (!channels_file) {
        char response[] = "Gagal membuka file channels.csv";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    char temp_path[256];
    snprintf(temp_path, sizeof(temp_path), "/home/tka/sisop/fp/DiscorIT/channels_temp.csv");
    FILE *temp_file = fopen(temp_path, "w+");
    if (!temp_file) {
        char response[] = "Gagal membuat file sementara untuk edit channel";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        fclose(channels_file);
        return;
    }

    char line_channel[256];
    bool found = false;
    bool channel_exists = false;

    while (fgets(line_channel, sizeof(line_channel), channels_file)) {
        char *id = strtok(line_channel, ",");
        char *channel_name = strtok(NULL, ",");
        char *key = strtok(NULL, ",");
        if (channel_name && strcmp(channel_name, new_channel) == 0) {
            channel_exists = true;
            break;
        }
        if (channel_name && strcmp(channel_name, old_channel) == 0) {
            found = true;
            fprintf(temp_file, "%s,%s,%s", id, new_channel, key);
        } else {
            fprintf(temp_file, "%s,%s,%s", id, channel_name, key);
        }
    }

    fclose(channels_file);
    fclose(temp_file);

    if (channel_exists) {
        remove(temp_path);
        char response[] = "Nama channel sudah digunakan";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    if (found) {
        remove(CHANNELS_FILE);
        rename(temp_path, CHANNELS_FILE);

        char old_path[256];
        snprintf(old_path, sizeof(old_path), "/home/tka/sisop/fp/DiscorIT/%s", old_channel);
        char new_path[256];
        snprintf(new_path, sizeof(new_path), "/home/tka/sisop/fp/DiscorIT/%s", new_channel);
        rename(old_path, new_path);

        char log_message[100];
        if(is_root){
            snprintf(log_message, sizeof(log_message), "ROOT mengubah channel %s menjadi %s", old_channel, new_channel);
        } else {
            snprintf(log_message, sizeof(log_message), "ADMIN mengubah channel %s menjadi %s", old_channel, new_channel);
        }
        log_activity(new_channel, log_message);

        char response[100];
        snprintf(response, sizeof(response), "%s berhasil diubah menjadi %s", old_channel, new_channel);
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
    } else {
        remove(temp_path);
        char response[100];
        snprintf(response, sizeof(response), "Channel %s tidak ditemukan", old_channel);
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
    }
}

void edit_room(const char *channel, const char *old_room, const char *new_room, client_info *client) {
    FILE *users_file = fopen(USERS_FILE, "r");
    if (!users_file) {
        char response[] = "Gagal membuka file users.csv";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    char rootline[256];
    bool is_admin = false;
    bool is_root = false;

    while (fgets(rootline, sizeof(rootline), users_file)) {
        char *token = strtok(rootline, ",");
        token = strtok(NULL, ",");
        if (token && strcmp(token, client->logged_in_user) == 0) {
            token = strtok(NULL, ",");
            token = strtok(NULL, ",");
            if (strstr(token, "ROOT") != NULL){
                is_root = true;
            }
            break;
        }
    }

    fclose(users_file);

    char auth_path[256];
    snprintf(auth_path, sizeof(auth_path), "/home/tka/sisop/fp/DiscorIT/%s/admin/auth.csv", channel);
    FILE *auth_file = fopen(auth_path, "r+");
    if (!auth_file) {
        char response[] = "Gagal membuka file auth.csv";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    char line[256];

    while (fgets(line, sizeof(line), auth_file)) {
        char *token = strtok(line, ",");
        if (token == NULL) continue;
        token = strtok(NULL, ",");
        if (token == NULL) continue;
        if (strcmp(token, client->logged_in_user) == 0) {
            token = strtok(NULL, ",");
            if (strstr(token, "ADMIN") != NULL){
                is_admin = true;
                break;
            }
        }
    }

    fclose(auth_file);

    if (!is_admin && !is_root) {
        char response[] = "Anda tidak memiliki izin untuk mengedit room";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    char check_path[256];
    snprintf(check_path, sizeof(check_path), "/home/tka/sisop/fp/DiscorIT/%s/%s", channel, new_room);
    struct stat st;
    if (stat(check_path, &st) == 0) {
        char response[] = "Nama room sudah digunakan";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    char path[256];
    snprintf(path, sizeof(path), "/home/tka/sisop/fp/DiscorIT/%s/%s", channel, old_room);
    if (stat(path, &st) == -1 || !S_ISDIR(st.st_mode)) {
        char response[100];
        snprintf(response, sizeof(response), "Room %s tidak ada di channel %s", old_room, channel);
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    char old_path[256];
    snprintf(old_path, sizeof(old_path), "/home/tka/sisop/fp/DiscorIT/%s/%s", channel, old_room);
    char new_path[256];
    snprintf(new_path, sizeof(new_path), "/home/tka/sisop/fp/DiscorIT/%s/%s", channel, new_room);

    if (rename(old_path, new_path) == 0) {
        char log_message[100];
        if(is_root){
            snprintf(log_message, sizeof(log_message), "ROOT mengubah room %s menjadi %s", old_room, new_room);
        } else {
            snprintf(log_message, sizeof(log_message), "ADMIN mengubah room %s menjadi %s", old_room, new_room);
        }
        log_activity(channel, log_message);

        char response[100];
        snprintf(response, sizeof(response), "%s berhasil diubah menjadi %s", old_room, new_room);
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
    } else {
        char response[] = "Gagal mengubah nama room";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
    }
}

void edit_profile_self(const char *username, const char *new_value, bool is_password, client_info *client) {
    FILE *file = fopen(USERS_FILE, "r+");
    if (!file) {
        char response[] = "Gagal membuka file users.csv";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    char line[256];
    char temp_path[256];
    snprintf(temp_path, sizeof(temp_path), "/home/tka/sisop/fp/DiscorIT/users_temp.csv");
    FILE *temp_file = fopen(temp_path, "w");
    if (!temp_file) {
        char response[] = "Gagal membuat file sementara";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        fclose(file);
        return;
    }

    bool found = false;
    bool name_exists = false;
    while (fgets(line, sizeof(line), file)) {
        char *user_id = strtok(line, ",");
        char *user_name = strtok(NULL, ",");
        char *hash = strtok(NULL, ",");
        char *role = strtok(NULL, ",");

        if (user_name && strcmp(user_name, new_value) == 0 && !is_password) {
            name_exists = true;
            break;
        }

        if (user_name && strcmp(user_name, username) == 0) {
            found = true;
            if (is_password) {
                char salt[SALT_SIZE];
                snprintf(salt, sizeof(salt), "$2y$12$%.22s", "inistringsaltuntukbcrypt");
                char new_hash[BCRYPT_HASHSIZE];
                bcrypt_hashpw(new_value, salt,new_hash);
                
                fprintf(temp_file, "%s,%s,%s,%s", user_id, user_name, new_hash, role);
            } else {
                fprintf(temp_file, "%s,%s,%s,%s", user_id, new_value, hash, role);
                snprintf(client->logged_in_user, sizeof(client->logged_in_user), "%s", new_value);
            }
        } else {
            fprintf(temp_file, "%s,%s,%s,%s", user_id, user_name, hash, role);
        }
    }

    fclose(file);
    fclose(temp_file);

    if (name_exists) {
        remove(temp_path);
        char response[] = "Username sudah digunakan";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    if (found) {
        remove(USERS_FILE);
        rename(temp_path, USERS_FILE);
        char response[100];
        snprintf(response, sizeof(response), is_password ? "Password diupdate" : "Profil diupdate");
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
    } else {
        remove(temp_path);
        char response[] = "User tidak ditemukan";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
    }
}

void edit_user(const char *target_user, const char *new_value, bool is_password, client_info *client) {
    FILE *file = fopen(USERS_FILE, "r+");
    if (!file) {
        char response[] = "Gagal membuka file users.csv";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    char line[256];
    char temp_path[256];
    snprintf(temp_path, sizeof(temp_path), "/home/tka/sisop/fp/DiscorIT/users_temp.csv");
    FILE *temp_file = fopen(temp_path, "w+");
    if (!temp_file) {
        char response[] = "Gagal membuat file sementara";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        fclose(file);
        return;
    }

    bool is_root = false;
    bool found = false;
    bool name_exists = false;

    while (fgets(line, sizeof(line), file)) {
        char *user_id = strtok(line, ",");
        char *user_name = strtok(NULL, ",");
        char *hash = strtok(NULL, ",");
        char *role = strtok(NULL, ",");

        if (user_name && strcmp(user_name, client->logged_in_user) == 0) {
            if (strstr(role, "ROOT") != NULL) {
                is_root = true;
            }
        }

        if (user_name && strcmp(user_name, new_value) == 0 && !is_password) {
            name_exists = true;
            break;
        }

        if (user_name && strcmp(user_name, target_user) == 0) {
            found = true;
            if (is_password) {
                char salt[SALT_SIZE];
                snprintf(salt, sizeof(salt), "$2y$12$%.22s", "inistringsaltuntukbcrypt");
                char new_hash[BCRYPT_HASHSIZE];
                bcrypt_hashpw(new_value, salt,new_hash);

                fprintf(temp_file, "%s,%s,%s,%s", user_id, user_name, new_hash, role);
            } else {
                fprintf(temp_file, "%s,%s,%s,%s", user_id, new_value, hash, role);
            }
        } else {
            fprintf(temp_file, "%s,%s,%s,%s", user_id, user_name, hash, role);
        }
    }

    fclose(file);
    fclose(temp_file);

    if (!is_root) {
        remove(temp_path);
        char response[] = "Anda tidak memiliki izin untuk mengedit user";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    if (name_exists) {
        remove(temp_path);
        char response[] = "Username sudah digunakan";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    if (found) {
        remove(USERS_FILE);
        rename(temp_path, USERS_FILE);

        char response[100];
        if (is_password) {
            snprintf(response, sizeof(response), "Password %s berhasil diubah", target_user);
        } else {
            snprintf(response, sizeof(response), "%s berhasil diubah menjadi %s", target_user, new_value);
        }
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
    } else {
        remove(temp_path);
        char response[] = "User tidak ditemukan";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
    }
}

void delete_chat(const char *channel, const char *room, int chat_id, client_info *client) {
    char path[256];
    snprintf(path, sizeof(path), "/home/tka/sisop/fp/DiscorIT/%s/%s/chat.csv", channel, room);

    FILE *file = fopen(path, "r");
    if (!file) {
        char response[] = "Gagal membuka file chat.csv";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    char temp_path[256];
    snprintf(temp_path, sizeof(temp_path), "/home/tka/sisop/fp/DiscorIT/%s/%s/chat_temp.csv", channel, room);
    FILE *temp_file = fopen(temp_path, "w");
    if (!temp_file) {
        char response[] = "Gagal membuat file sementara";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        fclose(file);
        return;
    }

    char line[256];
    bool found = false;

    while (fgets(line, sizeof(line), file)) {
        char *date = strtok(line, "|");
        char *id_str = strtok(NULL, "|");
        int id = atoi(id_str);
        char *sender = strtok(NULL, "|");
        char *chat = strtok(NULL, "\n");

        if (id == chat_id) {
            found = true;
        } else {
            fprintf(temp_file, "%s|%d|%s|%s\n", date, id, sender, chat);
        }
    }

    fclose(file);
    fclose(temp_file);

    if (found) {
        remove(path);
        rename(temp_path, path);
        char response[100];
        snprintf(response, sizeof(response), "Chat dengan id %d berhasil dihapus selamanya", chat_id);
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
    } else {
        remove(temp_path);
        char response[100];
        snprintf(response, sizeof(response), "Chat dengan id %d tidak ditemukan", chat_id);
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
    }
}

void delete_directory(const char *path) {
    struct dirent *entry;
    DIR *dir = opendir(path);

    if (dir == NULL) {
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        struct stat st;
        if (stat(full_path, &st) == -1) {
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            delete_directory(full_path);
        } else {
            unlink(full_path);
        }
    }

    closedir(dir);
    rmdir(path);
}

void delete_channel(const char *channel, client_info *client) {
    FILE *users_file = fopen(USERS_FILE, "r");
    if (!users_file) {
        char response[] = "Gagal membuka file users.csv";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    char line[256];
    bool is_admin = false;

    while (fgets(line, sizeof(line), users_file)) {
        char *token = strtok(line, ",");
        token = strtok(NULL, ",");
        if (token && strcmp(token, client->logged_in_user) == 0) {
            token = strtok(NULL, ",");
            token = strtok(NULL, ",");
            if (strstr(token, "ROOT") != NULL) {
                is_admin = true;
            }
            break;
        }
    }

    fclose(users_file);

    if (!is_admin) {
        char auth_path[256];
        snprintf(auth_path, sizeof(auth_path), "/home/tka/sisop/fp/DiscorIT/%s/admin/auth.csv", channel);
        FILE *auth_file = fopen(auth_path, "r");
        if (!auth_file) {
            char response[] = "Gagal membuka file auth.csv";
            if (write(client->socket, response, strlen(response)) < 0) {
                perror("Gagal mengirim respons ke client");
            }
            return;
        }

        while (fgets(line, sizeof(line), auth_file)) {
            char *token = strtok(line, ",");
            if (token == NULL) continue;
            token = strtok(NULL, ",");
            if (token == NULL) continue;
            if (strcmp(token, client->logged_in_user) == 0) {
                token = strtok(NULL, ",");
                if (strstr(token, "ADMIN") != NULL) {
                    is_admin = true;
                }
                break;
            }
        }

        fclose(auth_file);
    }

    if (!is_admin) {
        char response[] = "Anda tidak memiliki izin untuk menghapus channel ini";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    char path[256];
    snprintf(path, sizeof(path), "/home/tka/sisop/fp/DiscorIT/%s", channel);
    struct stat st;
    if (stat(path, &st) == -1 || !S_ISDIR(st.st_mode)) {
        char response[] = "Channel tidak ditemukan";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    // Delete directory recursively
    delete_directory(path);

    // Update channels.csv
    FILE *channels_file = fopen(CHANNELS_FILE, "r");
    if (!channels_file) {
        char response[] = "Gagal membuka file channels.csv";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    char temp_path[256];
    snprintf(temp_path, sizeof(temp_path), "/home/tka/sisop/fp/DiscorIT/channels_temp.csv");
    FILE *temp_file = fopen(temp_path, "w");
    if (!temp_file) {
        char response[] = "Gagal membuat file sementara";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        fclose(channels_file);
        return;
    }

    while (fgets(line, sizeof(line), channels_file)) {
        char *token = strtok(line, ",");
        token = strtok(NULL, ",");
        if (token && strcmp(token, channel) != 0) {
            fprintf(temp_file, "%s", line);
        }
    }

    fclose(channels_file);
    fclose(temp_file);

    remove(CHANNELS_FILE);
    rename(temp_path, CHANNELS_FILE);

    char response[100];
    snprintf(response, sizeof(response), "%s berhasil dihapus", channel);
    if (write(client->socket, response, strlen(response)) < 0) {
        perror("Gagal mengirim respons ke client");
    }

    char log_message[100];
    snprintf(log_message, sizeof(log_message), "%s menghapus channel %s", client->logged_in_role, channel);
    log_activity(channel, log_message);
}

void delete_room(const char *channel, const char *room, client_info *client) {
        bool is_admin = false;
        bool is_root = false;
        char auth_path[256];
        char line[256];
        snprintf(auth_path, sizeof(auth_path), "/home/tka/sisop/fp/DiscorIT/%s/admin/auth.csv", channel);
        FILE *auth_file = fopen(auth_path, "r");
        if (!auth_file) {
            char response[] = "Gagal membuka file auth.csv";
            if (write(client->socket, response, strlen(response)) < 0) {
                perror("Gagal mengirim respons ke client");
            }
            return;
        }

        while (fgets(line, sizeof(line), auth_file)) {
            char *token = strtok(line, ",");
            if (token == NULL) continue;
            token = strtok(NULL, ",");
            if (token == NULL) continue;
            if (strcmp(token, client->logged_in_user) == 0) {
                token = strtok(NULL, ",");
                if (strstr(token, "ADMIN") != NULL) {
                    is_admin = true;
                }else if (strstr(token, "ROOT") != NULL){
                    is_root = true;
                }
                break;
            }
        }

        fclose(auth_file);

    if (!is_admin && !is_root) {
        char response[] = "Anda tidak memiliki izin untuk menghapus room";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    char path[256];
    snprintf(path, sizeof(path), "/home/tka/sisop/fp/DiscorIT/%s/%s", channel, room);
    struct stat st;
    if (stat(path, &st) == -1 || !S_ISDIR(st.st_mode)) {
        char response[] = "Room tidak ditemukan";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    // Delete directory
    delete_directory(path);

    char response[100];
    snprintf(response, sizeof(response), "%s berhasil dihapus", room);
    if (write(client->socket, response, strlen(response)) < 0) {
        perror("Gagal mengirim respons ke client");
    }

    char log_message[100];
    if(is_root){
        snprintf(log_message, sizeof(log_message), "ROOT menghapus room %s", room);
    } else {
        snprintf(log_message, sizeof(log_message), "ADMIN menghapus room %s", room);
    }
    log_activity(channel, log_message);
}

void delete_all_rooms(const char *channel, client_info *client) {
    char auth_path[256];
    snprintf(auth_path, sizeof(auth_path), "/home/tka/sisop/fp/DiscorIT/%s/admin/auth.csv", channel);
    FILE *auth_file = fopen(auth_path, "r");
    if (!auth_file) {
        char response[] = "Gagal membuka file auth.csv";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    char line[256];
    bool is_admin = false;
    bool is_root = false;

    while (fgets(line, sizeof(line), auth_file)) {
        char *token = strtok(line, ",");
        if (token == NULL) continue;
        token = strtok(NULL, ",");
        if (token == NULL) continue;
        if (strcmp(token, client->logged_in_user) == 0) {
            token = strtok(NULL, ",");
            if (strstr(token, "ADMIN") != NULL) {
                is_admin = true;
            } else if (strstr(token, "ROOT") != NULL){
                is_root = true;
            }
            break;
        }
    }

    fclose(auth_file);

    if (!is_admin && !is_root) {
        char response[] = "Anda tidak memiliki izin untuk menghapus semua room";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    char path[256];
    snprintf(path, sizeof(path), "/home/tka/sisop/fp/DiscorIT/%s", channel);
    DIR *dir = opendir(path);
    if (dir == NULL) {
        char response[] = "Gagal membuka direktori channel";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, "admin") != 0 && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            char room_path[1024];
            snprintf(room_path, sizeof(room_path), "%s/%s", path, entry->d_name);

            struct stat st;
            if (stat(room_path, &st) == 0 && S_ISDIR(st.st_mode)) {
                delete_directory(room_path);
            }
        }
    }
    closedir(dir);

    char response[] = "Semua room dihapus";
    if (write(client->socket, response, strlen(response)) < 0) {
        perror("Gagal mengirim respons ke client");
    }

    char log_message[100];
    if(is_root){
        snprintf(log_message, sizeof(log_message), "ROOT menghapus semua room");
    } else {
        snprintf(log_message, sizeof(log_message), "ADMIN menghapus semua room");
    }
    log_activity(channel, log_message);
}

void ban_user(const char *channel, const char *target_user, client_info *client) {
    char auth_path[256];
    snprintf(auth_path, sizeof(auth_path), "/home/tka/sisop/fp/DiscorIT/%s/admin/auth.csv", channel);
    FILE *auth_file = fopen(auth_path, "r");
    if (!auth_file) {
        char response[] = "Gagal membuka file auth.csv";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    bool is_admin = false;
    bool is_root = false;
    char auth_line[256];

    while (fgets(auth_line, sizeof(auth_line), auth_file)) {
        char *token = strtok(auth_line, ",");
        if (token == NULL) continue;
        token = strtok(NULL, ",");
        if (token == NULL) continue;
        if (strcmp(token, client->logged_in_user) == 0) {
            token = strtok(NULL, ",");
            if (strstr(token, "ROOT") != NULL) {
                is_root = true;
            } else if (strstr(token, "ADMIN") != NULL) {
                is_admin = true;
            }
            break;
        }
    }

    fclose(auth_file);

    if (!is_admin && !is_root) {
        char response[] = "Anda tidak memiliki izin untuk melakukan ban user";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    // Open auth.csv to ban the target user
    auth_file = fopen(auth_path, "r+");
    if (!auth_file) {
        char response[] = "Gagal membuka file auth.csv";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    char temp_path[256];
    snprintf(temp_path, sizeof(temp_path), "/home/tka/sisop/fp/DiscorIT/%s/admin/auth_temp.csv", channel);
    FILE *temp_file = fopen(temp_path, "w+");
    if (!temp_file) {
        char response[] = "Gagal membuat file sementara";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        fclose(auth_file);
        return;
    }

    bool found = false;
    bool banable = true;
    char line[256];

    while (fgets(line, sizeof(line), auth_file)) {
        char *user_id = strtok(line, ",");
        char *user_name = strtok(NULL, ",");
        char *role = strtok(NULL, ",");

        if (user_name && strcmp(user_name, target_user) == 0) {
            found = true;
            if (strstr(role, "ROOT") != NULL || strstr(role, "ADMIN") != NULL) {
                banable = false;
            } else {
                fprintf(temp_file, "%s,%s,BANNED", user_id, user_name);
                continue;
            }
        }
        fprintf(temp_file, "%s,%s,%s", user_id, user_name, role);
    }

    fclose(auth_file);
    fclose(temp_file);

    if (!found) {
        remove(temp_path);
        char response[] = "User tidak ditemukan di channel ini";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    if (!banable) {
        remove(temp_path);
        char response[] = "Anda tidak bisa melakukan ban pada ROOT atau ADMIN";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    remove(auth_path);
    rename(temp_path, auth_path);

    char response[100];
    snprintf(response, sizeof(response), "%s diban", target_user);
    if (write(client->socket, response, strlen(response)) < 0) {
        perror("Gagal mengirim respons ke client");
    }

    char log_message[100];
    if(is_root){
        snprintf(log_message, sizeof(log_message), "ROOT ban %s", target_user);
    } else {
        snprintf(log_message, sizeof(log_message), "ADMIN ban %s", target_user);
    }
    log_activity(channel, log_message);
}

void unban_user(const char *channel, const char *target_user, client_info *client) {
    char auth_path[256];
    snprintf(auth_path, sizeof(auth_path), "/home/tka/sisop/fp/DiscorIT/%s/admin/auth.csv", channel);
    FILE *auth_file = fopen(auth_path, "r");
    if (!auth_file) {
        char response[] = "Gagal membuka file auth.csv";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    bool is_admin = false;
    bool is_root = false;
    char auth_line[256];

    while (fgets(auth_line, sizeof(auth_line), auth_file)) {
        char *token = strtok(auth_line, ",");
        if (token == NULL) continue;
        token = strtok(NULL, ",");
        if (token == NULL) continue;
        if (strcmp(token, client->logged_in_user) == 0) {
            token = strtok(NULL, ",");
            if (strstr(token, "ROOT") != NULL) {
                is_root = true;
            } else if (strstr(token, "ADMIN") != NULL) {
                is_admin = true;
            }
            break;
        }
    }

    fclose(auth_file);

    if (!is_admin && !is_root) {
        char response[] = "Anda tidak memiliki izin untuk melakukan ban user";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    // Open auth.csv to ban the target user
    auth_file = fopen(auth_path, "r+");
    if (!auth_file) {
        char response[] = "Gagal membuka file auth.csv";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    char temp_path[256];
    snprintf(temp_path, sizeof(temp_path), "/home/tka/sisop/fp/DiscorIT/%s/admin/auth_temp.csv", channel);
    FILE *temp_file = fopen(temp_path, "w+");
    if (!temp_file) {
        char response[] = "Gagal membuat file sementara";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        fclose(auth_file);
        return;
    }

    bool found = false;
    char line[256];

    while (fgets(line, sizeof(line), auth_file)) {
        char *user_id = strtok(line, ",");
        char *user_name = strtok(NULL, ",");
        char *role = strtok(NULL, ",");

        if (user_name && strcmp(user_name, target_user) == 0) {
            found = true;
            if(strstr(role, "USER") != NULL || strstr(role, "ADMIN") != NULL || strstr(role, "ROOT") != NULL){
                char response[] = "User tidak dalam status banned";
                if (write(client->socket, response, strlen(response)) < 0) {
                    perror("Gagal mengirim respons ke client");
                }
                return;
            } else {
                fprintf(temp_file, "%s,%s,USER", user_id, user_name);
                continue;
            }
        }
        fprintf(temp_file, "%s,%s,%s", user_id, user_name, role);
    }

    fclose(auth_file);
    fclose(temp_file);

    if (!found) {
        remove(temp_path);
        char response[] = "User tidak ditemukan di channel ini";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    remove(auth_path);
    rename(temp_path, auth_path);

    char response[100];
    snprintf(response, sizeof(response), "%s kembali", target_user);
    if (write(client->socket, response, strlen(response)) < 0) {
        perror("Gagal mengirim respons ke client");
    }

    char log_message[100];
    if(is_root){
        snprintf(log_message, sizeof(log_message), "ROOT unban %s", target_user);
    } else {
        snprintf(log_message, sizeof(log_message), "ADMIN unban %s", target_user);
    }
    log_activity(channel, log_message);
}

void remove_user(const char *channel, const char *target_user, client_info *client) {
    char auth_path[256];
    snprintf(auth_path, sizeof(auth_path), "/home/tka/sisop/fp/DiscorIT/%s/admin/auth.csv", channel);
    FILE *auth_file = fopen(auth_path, "r");
    if (!auth_file) {
        char response[] = "Gagal membuka file auth.csv";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    bool is_admin = false;
    bool is_root = false;
    char auth_line[256];

    while (fgets(auth_line, sizeof(auth_line), auth_file)) {
        char *token = strtok(auth_line, ",");
        if (token == NULL) continue;
        token = strtok(NULL, ",");
        if (token == NULL) continue;
        if (strcmp(token, client->logged_in_user) == 0) {
            token = strtok(NULL, ",");
            if (strstr(token, "ROOT") != NULL){
                is_root = true;
            } else if (strstr(token, "ADMIN") != NULL) {
                is_admin = true;
            }
            break;
        }
    }

    fclose(auth_file);

    if (!is_admin && !is_root) {
        char response[] = "Anda tidak memiliki izin untuk menghapus user dari channel";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    // Open auth.csv to remove the target user
    auth_file = fopen(auth_path, "r+");
    if (!auth_file) {
        char response[] = "Gagal membuka file auth.csv";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    char temp_path[256];
    snprintf(temp_path, sizeof(temp_path), "/home/tka/sisop/fp/DiscorIT/%s/admin/auth_temp.csv", channel);
    FILE *temp_file = fopen(temp_path, "w+");
    if (!temp_file) {
        char response[] = "Gagal membuat file sementara";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        fclose(auth_file);
        return;
    }

    bool found = false;
    bool removable = true;
    char line[256];

    while (fgets(line, sizeof(line), auth_file)) {
        char *user_id = strtok(line, ",");
        char *user_name = strtok(NULL, ",");
        char *role = strtok(NULL, ",");

        if (user_name && strcmp(user_name, target_user) == 0) {
            found = true;
            if (strstr(role, "ROOT") != NULL || strstr(role, "ADMIN") != NULL) {
                removable = false;
            } else {
                continue;  // Skip writing this user to temp file
            }
        }
        fprintf(temp_file, "%s,%s,%s", user_id, user_name, role);
    }

    fclose(auth_file);
    fclose(temp_file);

    if (!found) {
        remove(temp_path);
        char response[] = "User tidak ditemukan di channel ini";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    if (!removable) {
        remove(temp_path);
        char response[] = "Anda tidak bisa menghapus ROOT atau ADMIN dari channel";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    remove(auth_path);
    rename(temp_path, auth_path);

    char response[100];
    snprintf(response, sizeof(response), "%s dikick", target_user);
    if (write(client->socket, response, strlen(response)) < 0) {
        perror("Gagal mengirim respons ke client");
    }

    char log_message[100];
    if(is_root){
        snprintf(log_message, sizeof(log_message), "ROOT kick %s", target_user);
    } else {
        snprintf(log_message, sizeof(log_message), "ADMIN kick %s", target_user);
    }
    log_activity(channel, log_message);
}

void remove_user_root(const char *target_user, client_info *client) {
    FILE *rootfile = fopen(USERS_FILE, "r");
    if (!rootfile) {
        char response[] = "Gagal membuka file users.csv";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        return;
    }

    bool is_root = false;
    char line[256];

    while (fgets(line, sizeof(line), rootfile)) {
        char *token = strtok(line, ",");
        token = strtok(NULL, ",");
        if (token && strcmp(token, client->logged_in_user) == 0) {
            token = strtok(NULL, ",");
            token = strtok(NULL, ",");
            if (strstr(token, "ROOT") != NULL) {
                is_root = true;
                break;
            }
        }
    }

    if (!is_root) {
        char response[] = "Anda tidak memiliki izin untuk menghapus user secara permanen";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        fclose(rootfile);
        return;
    }

    fclose(rootfile);

    FILE *file = fopen(USERS_FILE, "r");
    if (!file) {
        char response[] = "Gagal membuka file users.csv";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        fclose(file);
        return;
    }

    char temp_path[256];
    snprintf(temp_path, sizeof(temp_path), "/home/tka/sisop/fp/DiscorIT/users_temp.csv");
    FILE *temp_file = fopen(temp_path, "w+");
    if (!temp_file) {
        char response[] = "Gagal membuat file sementara";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        fclose(temp_file);
        return;
    }

    bool found = false;

    while (fgets(line, sizeof(line), file)) {
        char *user_id = strtok(line, ",");
        char *user_name = strtok(NULL, ",");
        char *hash = strtok(NULL, ",");
        char *role = strtok(NULL, ",");

        if (user_name && strcmp(user_name, target_user) == 0) {
            found = true;
            continue;  // Skip writing this user to temp file
        }
        fprintf(temp_file, "%s,%s,%s,%s", user_id, user_name, hash, role);
    }

    fclose(file);
    fclose(temp_file);

    if (found) {
        remove(USERS_FILE);
        rename(temp_path, USERS_FILE);
        char response[100];
        snprintf(response, sizeof(response), "%s berhasil dihapus", target_user);
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
    } else {
        remove(temp_path);
        char response[] = "User tidak ditemukan";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
    }
}

void log_activity(const char *channel, const char *message) {
    char log_path[256];
    snprintf(log_path, sizeof(log_path), "/home/tka/sisop/fp/DiscorIT/%s/admin/user.log", channel);

    FILE *log_file = fopen(log_path, "a+");
    if (!log_file) {
        perror("Gagal membuka file user.log");
        return;
    }

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char date[30];
    strftime(date, sizeof(date), "%d/%m/%Y %H:%M:%S", t);

    fprintf(log_file, "[%s] %s\n", date, message);
    fclose(log_file);
}


void handle_exit(client_info *client) {
    if (strlen(client->logged_in_room) > 0) {
        memset(client->logged_in_room, 0, sizeof(client->logged_in_room));
        char response[BUFFER_SIZE];
        snprintf(response, sizeof(response), "[%s/%s]", client->logged_in_user, client->logged_in_channel);
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
    } else if (strlen(client->logged_in_channel) > 0) {
        memset(client->logged_in_channel, 0, sizeof(client->logged_in_channel));
        char response[BUFFER_SIZE];
        snprintf(response, sizeof(response), "[%s]", client->logged_in_user);
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
    } else {
        char response[] = "Anda telah keluar dari aplikasi";
        if (write(client->socket, response, strlen(response)) < 0) {
            perror("Gagal mengirim respons ke client");
        }
        close(client->socket);
        pthread_exit(NULL);
    }
}
```
Fungsi Main yang berisi fungsi yang dipanggil untuk:
1. **Fungsi Utama:**
    - handle_client
    - daemonize

2. **User Authentication:**
    - register_user
    - login_user

3. **Channel and Room Management:**
    - create_directory
    - create_channel
    - create_room
    - list_channels
    - list_rooms
    - join_channel
    - verify_key
    - join_room
    - edit_channel
    - edit_room
    - delete_channel
    - delete_room
    - delete_all_rooms
    - ban_user
    - unban_user
    - remove_user

4. **Message Handling:**
    - send_chat
    - see_chat
    - edit_chat
    - delete_chat

5. **Additional Functions:**
    - create_directory
    - delete_directory
    - list_users
    - log_activity
    - handle_exit

6. **Root User Functions:**
    - list_users_root
    - edit_user
    - remove_user_root

___

## 3. Monitor
1. User dapat menampilkan isi chat secara real-time menggunakan monitor. Jika ada perubahan pada isi chat, perubahan tersebut akan langsung ditampilkan di terminal.
2. Sebelum dapat menggunakan monitor, user harus login terlebih dahulu seperti login di DiscorIT.
3. Untuk keluar dari room dan menghentikan program monitor dengan perintah "EXIT".
4. Monitor dapat digunakan untuk menampilkan semua chat pada room, mulai dari chat pertama hingga chat yang akan datang.


Deklarasi library yang akan digunakan
```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <crypt.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <pthread.h>
#include <time.h>
```

Definisikan _buffer, port,_ dan _array_ yang digunakan
```c
#define PORT 8080
#define BUFFER_SIZE 1024

int server_fd;
bool running = true;
char username[50];
char channel[50] = "";
char room[50] = "";
```

Fungsi untuk menyambungkan _socket_
```c
void connect_to_server() {
    struct sockaddr_in serv_addr;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    if (connect(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        exit(EXIT_FAILURE);
    }
}
```

Fungsi untuk _handling command_
```c
void handle_command(const char *command) {
    if (command == NULL) {
        printf("Perintah tidak boleh kosong\n");
        return;
    }

    if (send(server_fd, command, strlen(command), 0) < 0) {
        perror("Gagal mengirim perintah ke server");
    }

    char response[BUFFER_SIZE];
    memset(response, 0, sizeof(response));
    if (recv(server_fd, response, BUFFER_SIZE - 1, 0) < 0) {
        perror("Gagal menerima respons dari server");
    } else {
        if (strstr(response, "Key:") != NULL) {
            char key[50];
            printf("Key: ");
            fgets(key, sizeof(key), stdin);
            key[strcspn(key, "\n")] = '\0';

            if (send(server_fd, key, strlen(key), 0) < 0) {
                perror("Gagal mengirim key ke server");
            }

            memset(response, 0, sizeof(response));
            if (recv(server_fd, response, BUFFER_SIZE - 1, 0) < 0) {
                perror("Gagal menerima respons dari server");
            } else {
                if (strstr(response, "Key salah") != NULL) {
                    // Reset state if key is invalid
                    if (strlen(room) > 0) {
                        room[0] = '\0';
                    } else if (strlen(channel) > 0) {
                        channel[0] = '\0';
                    }
                }
            }
        } else if (strstr(response, "tidak ada") != NULL || strstr(response, "Key salah") != NULL || strstr(response, "Anda telah diban") != NULL || strstr(response, "tidak dikenali") != NULL){
            if (strlen(room) > 0) {
                room[0] = '\0';
            } else if (strlen(channel) > 0) {
                channel[0] = '\0';
            }
            printf("%s\n", response);
        } else if (strstr(response, "Anda telah keluar dari aplikasi") != NULL) {
            close(server_fd);
            exit(0);
        } else {
            printf("%s\n", response);
        }
    }
}
```

Fungsi untuk menampilkan chat
```c
void display_chat_history(const char *filepath) {
    FILE *chat_file = fopen(filepath, "r");
    if (!chat_file) {
        perror("Gagal membuka file");
        return;
    }

    char response[BUFFER_SIZE];
    memset(response, 0, sizeof(response));
    char line[BUFFER_SIZE];
    bool has_chat = false;

    while (fgets(line, sizeof(line), chat_file)) {
        has_chat = true;
        char *date = strtok(line, "|");
        char *id_chat = strtok(NULL, "|");
        char *sender = strtok(NULL, "|");
        char *chat = strtok(NULL, "|");

        chat[strcspn(chat, "\n")] = '\0';

        if (date && id_chat && sender && chat) {
            snprintf(response + strlen(response), sizeof(response) - strlen(response), "[%s][%s][%s] \"%s\"\n", date, id_chat, sender, chat);
        }
    }

    fclose(chat_file);

    if (!has_chat) {
        snprintf(response, sizeof(response), "Belum ada chat\n");
    }

    printf("\033[H\033[J");

    printf("%s", response);

    printf("[%s/%s/%s] ", username, channel, room);

    fflush(stdout);
}
```

Fungsi untuk monitor ```csv```
```c
void *monitor_csv(void *arg) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "/home/tka/sisop/fp/DiscorIT/%s/%s/chat.csv", channel, room);
    struct stat file_stat;
    time_t last_mod_time = 0;

    while (running) {
        if (stat(filepath, &file_stat) == 0) {
            if (file_stat.st_mtime != last_mod_time) {
                last_mod_time = file_stat.st_mtime;
                printf("\033[H\033[J");

                display_chat_history(filepath);
            }
        } else {
            perror("Gagal mendapatkan status file");
        }
        sleep(1);
    }
    return NULL;
}

```

Fungsi ```main``` untuk memanggil fungsi lainnya untuk menampilkan output
```c
int main(int argc, char *argv[]) {
    if (argc <= 3) {
        printf("Penggunaan: %s LOGIN username -p password\n", argv[0]);
        return 1;
    }

    connect_to_server();

    char command[BUFFER_SIZE];
    memset(command, 0, sizeof(command));

    if (strcmp(argv[1], "LOGIN") == 0) {
        if (argc < 5 || strcmp(argv[3], "-p") != 0) {
            printf("Penggunaan: %s LOGIN username -p password\n", argv[0]);
            return 1;
        }

        snprintf(username, sizeof(username), "%s", argv[2]);
        char *password = argv[4];

        snprintf(command, sizeof(command), "LOGIN %s %s", username, password);

        if (send(server_fd, command, strlen(command), 0) < 0) {
            perror("Gagal mengirim perintah ke server");
        }

        char response[BUFFER_SIZE];
        memset(response, 0, sizeof(response));

        if (recv(server_fd, response, BUFFER_SIZE - 1, 0) < 0) {
            perror("Gagal menerima respons dari server");
        } else {
            printf("%s\n", response);

            if (strstr(response, "berhasil login") != NULL) {
                while (1) {
                    printf("[%s] ", username);

                    if (fgets(command, BUFFER_SIZE, stdin) == NULL) {
                        printf("Gagal membaca perintah\n");
                        continue;
                    }
                    command[strcspn(command, "\n")] = '\0';

                    char *token = strtok(command, " ");
                    if (token == NULL) {
                        printf("Penggunaan: -channel <nama_channel> -room <nama_room>\n");
                        continue;
                    }

                    if (strcmp(token, "-channel") == 0) {
                        token = strtok(NULL, " ");
                        if (token == NULL) {
                            printf("Penggunaan: -channel <nama_channel> -room <nama_room>\n");
                            continue;
                        }
                        snprintf(channel, sizeof(channel), "%s", token);
                        token = strtok(NULL, " ");
                        if(token == NULL){
                            printf("Penggunaan: -channel <nama_channel> -room <nama_room>\n");
                            continue;
                        }
                        if(strcmp(token, "-room") != 0){
                            printf("Penggunaan: -channel <nama_channel> -room <nama_room>\n");
                            continue;
                        }
                        token = strtok(NULL, " ");
                        if(token == NULL){
                            printf("Penggunaan: -channel <nama_channel> -room <nama_room>\n");
                            continue;
                        }
                        snprintf(room, sizeof(room), "%s", token);

                        pthread_t csv_thread;
                        running = true;
                        if (pthread_create(&csv_thread, NULL, monitor_csv, NULL) != 0) {
                            perror("Gagal membuat thread untuk memonitor CSV");
                            continue;
                        }

                        pthread_detach(csv_thread);

                        char filepath[256];
                        snprintf(filepath, sizeof(filepath), "/home/tka/sisop/fp/DiscorIT/%s/%s/chat.csv", channel, room);
                        display_chat_history(filepath);
                    } else if (strcmp(token, "EXIT") == 0) {
                        if (strlen(room) > 0 || strlen(channel) > 0) {
                            room[0] = '\0';
                            channel[0] = '\0';
                            running = false;
                            printf("\033[H\033[J");
                        } else {
                            handle_command(command);
                            break;
                        }
                    } else {
                        printf("Penggunaan: -channel <nama_channel> -room <nama_room>\nJika ingin keluar, gunakan perintah EXIT\n");
                    }
                }
            }
            close(server_fd);
            return 0;
        }
    } else {
        printf("Penggunaan: %s LOGIN username -p password\n", argv[0]);
        return 1;
    }
    close(server_fd);
    return 0;
}
```
---
