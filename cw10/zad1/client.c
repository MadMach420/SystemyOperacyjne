#include "header.h"
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>


int socket_fd;
int epoll;


// ----- CONNECT TO SERVER -----
void start_unix(char* path) {
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof addr.sun_path -1);

    socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    connect(socket_fd, (struct sockaddr*) &addr, sizeof addr);
}


void start_inet(int port) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    connect(socket_fd, (struct sockaddr*) &addr, sizeof addr);
}


// ----- HANDLE CTRL+C
void handler(int signum) {
    message msg = { .type = msg_disconnect };
    write(socket_fd, &msg, sizeof msg);
    exit(0);
}


// ----- READ STDIN AND MESSAGES
void loop() {
    struct epoll_event events[1];
    while(1) {
        epoll_wait(epoll, events, 1, -1);
        if (events[0].data.fd == STDIN_FILENO) {
            char buffer[512];
            int read_len = read(STDIN_FILENO, &buffer, 512);
            buffer[read_len] = 0;

            char *token = NULL;
            int index = 0;
            token = strtok( buffer, " \t\n" );

            message_type type = -1;
            char other_nickname[MSG_LEN] = {};
            char message_text[MSG_LEN] = {};

            int error = 0;

            if (token == NULL) {
                continue;
            }

            while( token != NULL ) {
                switch (index) {
                    case 0: {
                        if (strcmp(token, "LIST") == 0) type = msg_list;
                        else if (strcmp(token, "2ALL") == 0) type = msg_all;
                        else if (strcmp(token, "2ONE") == 0) type = msg_one;
                        else if (strcmp(token, "STOP") == 0) type = msg_stop;
                        else {
                            error = 1;
                            printf("Invalid command\n");
                        }
                        break;
                    }
                    case 1: {
                        memcpy(message_text, token, strlen(token));
                        message_text[strlen(token)] = 0;
                        break;
                    }
                    case 2: {
                        memcpy(other_nickname, token, strlen(token));
                        other_nickname[strlen(token)] = 0;
                        break;
                    }
                    case 3: {
                        error = 1;
                        break;
                    }
                }

                if (error) {
                    break;
                }

                ++index;
                token = strtok( NULL, " \t\n" );
            }
            if (error) {
                continue;
            }

            message msg;
            msg.type = type;
            sprintf(msg.message, "%s", message_text);
            sprintf(msg.additional_data, "%s", other_nickname);
            write(socket_fd, &msg, sizeof msg);
        } else {
            message msg;
            read(socket_fd, &msg, sizeof msg);

            if (msg.type == msg_username_taken || msg.type == msg_server_full) {
                printf("Could not connect to server");
                close(socket_fd);
                exit(0);
            } else if (events[0].events & EPOLLHUP) {
                printf("Disconnected\n");
                exit(0);
            } else if (msg.type == msg_ping) {
                write(socket_fd, &msg, sizeof msg);
            } else if (msg.type == msg_stop) {
                printf("Client stopped\n");
                close(socket_fd);
                exit(0);
            } else if (msg.type == msg_get) {
                printf("%s\n", msg.message);
            }
        }
    }
}


int main(int argc, char** argv) {
    if (strcmp(argv[2], "web") == 0 && argc == 5) {
        start_inet(atoi(argv[4]));
    }
    else if (strcmp(argv[2], "unix") == 0 && argc == 4) {
        start_unix(argv[3]);
    }
    else {
        printf("Invalid arguments\n");
        exit(1);
    }

    signal(SIGINT, handler);

    char* nickname = argv[1];
    write(socket_fd, nickname, strlen(nickname));

    epoll = epoll_create1(0);

    struct epoll_event socket_event;
    socket_event.events = EPOLLIN | EPOLLPRI | EPOLLHUP;
    socket_event.data.fd = socket_fd;
    epoll_ctl(epoll, EPOLL_CTL_ADD, socket_fd, &socket_event);

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = STDIN_FILENO;
    epoll_ctl(epoll, EPOLL_CTL_ADD, STDIN_FILENO, &event);

    printf("Client started\n");

    loop();
}