#include "header.h"
#include <pthread.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


typedef struct {
    int fd;
    enum client_state { empty = 0, init, ready } state;
    char nickname[NICKNAME_LEN];
    struct game_state* game_state;
    int responding;
} client;


typedef struct {
    enum event_type { socket_event, client_event } type;
    union data { client* client; int socket; } data;
} epoll_data_struct;


pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
int epoll;
int socket_un;
int socket_in;
client clients[MAX_CLIENTS];


// ----- CLIENT UTILS -----
void disconnect_client(client* client) {
    client->state = empty;
    client->nickname[0] = 0;
    epoll_ctl(epoll, EPOLL_CTL_DEL, client->fd, NULL);
    close(client->fd);
}


int check_if_client_exists(client* client1) {
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (strcmp(clients[i].nickname, client1->nickname) == 0) {
            return 1;
        }
    }
    return 0;
}


client* new_client(int client_fd) {
    pthread_mutex_lock(&clients_mutex);
    int index = -1;
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].state == empty) {
            index = i;
            break;
        }
    }
    if (index == -1) return NULL;
    client* client = &clients[index];

    client->fd = client_fd;
    client->state = init;
    client->responding = 1;
    pthread_mutex_unlock(&clients_mutex);
    return client;
}


// ----- MESSAGES -----
void send_msg(client* client, message_type type, char message_text[MSG_LEN]) {
    message msg;
    msg.type = type;
    sprintf(msg.message, "%s", message_text);
    write(client->fd, &msg, sizeof(msg));
}


void get_message(client* client) {
    printf("Message received\n");

    if (client->state == init) {
        ssize_t nickname_length = read(client->fd, client->nickname, NICKNAME_LEN);
        pthread_mutex_lock(&clients_mutex);
        if (check_if_client_exists(client)) {
            client->nickname[nickname_length] = 0;
            client->state = ready;
        } else {
            message msg;
            msg.type = msg_username_taken;
            write(client->fd, &msg, sizeof(message));
            disconnect_client(client);
        }
        pthread_mutex_unlock(&clients_mutex);
    } else {
        message msg;
        read(client->fd, &msg, sizeof msg);

        if (msg.type == msg_ping) {
            pthread_mutex_lock(&clients_mutex);
            client->responding = 1;
            pthread_mutex_unlock(&clients_mutex);
        } else if (msg.type == msg_disconnect || msg.type == msg_stop) {
            pthread_mutex_lock(&clients_mutex);
            disconnect_client(client);
            pthread_mutex_unlock(&clients_mutex);
        } else if (msg.type == msg_all) {
            char message_text[MSG_LEN] = "";
            strcat(message_text, client->nickname);
            strcat(message_text, ": ");
            strcat(message_text, msg.message);

            for (int i = 0; i < MAX_CLIENTS; ++i) {
                if (clients[i].state != empty && strcmp(clients[i].nickname, client->nickname) != 0) {
                    send_msg(&clients[i], msg_get, message_text);
                }
            }
        } else if (msg.type == msg_one) {
            char message_text[256] = "";
            strcat(message_text, client->nickname);
            strcat(message_text, ": ");
            strcat(message_text, msg.message);

            for (int i = 0; i < MAX_CLIENTS; ++i) {
                if (clients[i].state != empty) {
                    if (strcmp(clients[i].nickname, msg.additional_data) == 0) {
                        send_msg(clients+i, msg_get, message_text);
                    }
                }
            }
        } else if (msg.type == msg_list) {
            for (int i = 0; i < MAX_CLIENTS; ++i) {
                if (clients[i].state != empty) {
                    printf("Client %d: %s\n", i, clients[i].nickname);
                }
            }
        }
    }
}


// ----- INIT -----
void init_socket(int socket, void* addr, int addr_size) {
    if ((bind(socket, (struct sockaddr*) addr, addr_size)) == -1) {
        exit(1);
    }
    listen(socket, MAX_CLIENTS);
    struct epoll_event event = { .events = EPOLLIN | EPOLLPRI };
    epoll_data_struct* event_data = event.data.ptr = malloc(sizeof *event_data);
    event_data->type = socket_event;
    event_data->data.socket = socket;
    epoll_ctl(epoll, EPOLL_CTL_ADD, socket, &event);
}


// ----- SECOND THREAD -----
void* ping(void* data) {
    message msg;
    msg.type = msg_ping;
    while (1) {
        sleep(PING_INTERVAL);
        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].state != empty) {
                if (clients[i].responding) {
                    clients[i].responding = 0;
                    write(clients[i].fd, &msg, sizeof msg);
                }
                else { // jesli nie odpowiedzial na ostatni ping
                    disconnect_client(&clients[i]);
                }
            }
        }
        pthread_mutex_unlock(&clients_mutex);
    }
}


void loop() {
    struct epoll_event events[1];
    while (1) {
        epoll_wait(epoll, events, 1, -1);

        epoll_data_struct* data = events[0].data.ptr;
        if (data->type == socket_event) {
            int client_fd = accept(data->data.socket, NULL, NULL);
            client* client = new_client(client_fd);
            if (client == NULL) {
                printf("Server is full\n");
                message msg = { .type = msg_server_full };
                write(client_fd, &msg, sizeof msg);
                close(client_fd);
                continue;
            }

            epoll_data_struct* eventData = malloc(sizeof(epoll_data_struct));
            eventData->type = client_event;
            eventData->data.client = client;
            struct epoll_event event;
            event.events = EPOLLIN;
            event.data.ptr = eventData;
            epoll_ctl(epoll, EPOLL_CTL_ADD, client_fd, &event);
        } else {
            get_message(data->data.client);
        }
    }
}


int main(int argc, char** argv) {
    if (argc != 3) {
        printf("Invalid number of arguments\n");
        exit(1);
    }
    int port = atoi(argv[1]);
    char* unix_path = argv[2];

    epoll = epoll_create1(0);

    struct sockaddr_un un_addr = { .sun_family = AF_UNIX };
    sprintf(un_addr.sun_path, "%s", unix_path);
    unlink(unix_path); // jakby podano tą samą nazwę
    socket_un = socket(AF_UNIX, SOCK_STREAM, 0);
    init_socket(socket_un, &un_addr, sizeof un_addr);

    struct sockaddr_in inet_addr;
    inet_addr.sin_family = AF_INET;
    inet_addr.sin_port = htons(port);
    struct in_addr localhost;
//    localhost.s_addr = INADDR_LOOPBACK;
    localhost.s_addr = INADDR_ANY;
    inet_addr.sin_addr = localhost;
    socket_in = socket(AF_INET, SOCK_STREAM, 0);
    init_socket(socket_in, &inet_addr, sizeof inet_addr);

    pthread_t thread;
    pthread_create(&thread, NULL, ping, NULL);

    printf("Server started\n");

    loop();
}