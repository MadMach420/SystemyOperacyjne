#include "header.h"
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/epoll.h>


int clients_fd[MAX_CLIENTS];
int next_free_index = 0;
int user_count = 0;
int queue_id;

int socket_un;
int socket_in;
int eboll;


// ----- HANDLE CTRL+C -----
void shutdown_clients() {
    custom_message message;
    message.sender_id = SERVER_ID;
    message.type = STOP;

    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients_fd[i] != -1) {
            close(clients_fd[i])
        }
    }
}


void handler(int signum) {
    printf("\nServer shutting down\n");

    //
//    shutdown(socket_fd, SHUT_RDWR);
//    close();

    shutdown_clients();
    msgctl(queue_id, IPC_RMID, NULL);

    exit(0);
}


// ----- UTILS -----


int get_index() {
    if (user_count == MAX_CLIENTS) return -1;
    if (next_free_index != MAX_CLIENTS) {
        ++user_count;
        return next_free_index++;
    }

    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients_fd[i] == -1) {
            ++user_count;
            return i;
        }
    }

    return -1;
}


custom_message prepare_message_to_pass_on(custom_message *request) {
    custom_message new_message;
    new_message.type = MESSAGE;
    new_message.sender_id = SERVER_ID;

    time_t current_time;
    time(&current_time);
    char time[32];
    strftime(time, 32, "%d-%m-%Y %H:%M:%S", localtime(&current_time));

    char new_string[MAX_MESSAGE_LENGTH + 50];
    sprintf(new_string, "\nUser %d:%s%s\n\n", request->sender_id, request->message, time);
    if (strlen(new_string) >= MAX_MESSAGE_LENGTH - 50) {
        sprintf(new_message.message, "\nUser %d:\n<Message too long>\n%s\n\n", request->sender_id, time);
    } else {
        char short_message[MAX_MESSAGE_LENGTH - 50];
        strcpy(short_message, new_string);
        sprintf(new_message.message, "%s", short_message);
    }

    return new_message;
}


// ----- COMMANDS -----

void list() {
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients_fd[i] != -1) {
            printf("Client queue id: %d\n", clients_fd[i]);
        }
    }
}

void all(custom_message *request) {
    custom_message new_message = prepare_message_to_pass_on(request);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients_fd[i] != -1 && i != request->sender_id) {
            msgsnd(clients_fd[i], &new_message, sizeof(custom_message) - sizeof(long), 0);
        }
    }
}


void one(custom_message *request) {
    custom_message new_message = prepare_message_to_pass_on(request);
    msgsnd(clients_fd[request->additional_data], &new_message, sizeof(custom_message) - sizeof(long), 0);
}


void stop(custom_message *request) {
    clients_fd[request->sender_id] = -1;
    --user_count;
}


void init(custom_message *request) {
    custom_message response;
    response.type = INIT;
    response.sender_id = SERVER_ID;

    int req_queue_id = request->additional_data;

    int index = get_index();
    if (index == -1) {
        sprintf(response.message, "Server at max capacity");
        response.additional_data = -1;
    } else {
        clients_fd[index] = req_queue_id;

        sprintf(response.message, "Client initialized");
        response.additional_data = index;
    }

    msgsnd(req_queue_id, &response, sizeof(custom_message) - sizeof(long), 0);
}


void parse_message(custom_message *request) {
    printf("Message received\n");

    long type = request->type;

    switch (type) {
        case LIST: {
            list();
            break;
        }
        case ALL: {
            all(request);
            break;
        }
        case ONE: {
            one(request);
            break;
        }
        case STOP: {
            stop(request);
            break;
        }
        case INIT: {
            init(request);
            break;
        }
        default: {
            printf("Received invalid message type\n");
            break;
        }
    }
}


// ----- INITS -----
void start_unix(char* unix_port) {
    socket_un = socket(AF_UNIX, SOCK_STREAM, 0);

    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    sprintf(addr.sun_path, "%s", unix_port);

    if (bind(socket_un, (const struct sockaddr *) &addr, sizeof(addr)) == -1) {
        exit(1);
    }

    listen(socket_un, MAX_CLIENTS);
}


void start_inet(int port) {
    socket_in = socket(AF_UNIX, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    struct in_addr localhost;
    localhost.s_addr = INADDR_LOOPBACK;
    addr.sin_addr = localhost;

    if (bind(socket_in, (const struct sockaddr *) &addr, sizeof(addr)) == -1) {
        exit(1);
    }

    listen(socket_in, MAX_CLIENTS);
}


int main (int argc, char** argv) {
    if (argc != 2) {
        exit(1);
    }

    struct sigaction action;
    action.sa_handler = handler;
    sigemptyset(&(action.sa_mask));
    action.sa_flags = 0;
    sigaction(SIGINT, &action, NULL);

    in_port_t port = atoi(argv[1]);
    char* unix_port = argv[2];
    start_inet(port);
    start_unix(unix_port);

    struct epoll_event event;
    event.events = EPOLLIN | EPOLLOUT;
    eboll = epoll_create1(0);
    epoll_ctl(eboll, EPOLL_CTL_ADD, socket_in, NULL);
    epoll_ctl(eboll, EPOLL_CTL_ADD, socket_un, NULL);
}