#include "header.h"
#include <stdio.h>
#include <mqueue.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>


mqd_t clients_queues[MAX_CLIENTS];
int next_free_index = 0;
int user_count = 0;
mqd_t queue_desc;



// ----- HANDLE CTRL+C -----
void shutdown_clients() {
    char message[MAX_MESSAGE_LENGTH];
    sprintf(message, "%d", STOP);

    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients_queues[i] != -1) {
            mq_close(clients_queues[i]);
            mq_send(clients_queues[i], message, MAX_MESSAGE_LENGTH, STOP);
        }
    }
}


void handler(int signum) {
    printf("\nServer shutting down\n");

    shutdown_clients();
    mq_close(queue_desc);
    mq_unlink(SERVER_QUEUE_NAME);

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
        if (clients_queues[i] == -1) {
            ++user_count;
            return i;
        }
    }

    return -1;
}


int prepare_message_to_pass_on(char* request, char* response) {
    char sender_id_string[3];
    strncpy(sender_id_string, request, 2);
    sender_id_string[2] = '\0';

    time_t current_time;
    time(&current_time);
    char time[32];
    strftime(time, 32, "%d-%m-%Y %H:%M:%S", localtime(&current_time));

    char new_string[MAX_MESSAGE_LENGTH + 50];
    sprintf(new_string, "%d\nUser %s:%s\n%s\n\n", MESSAGE, sender_id_string, &request[3], time);
    if (strlen(new_string) >= MAX_MESSAGE_LENGTH - 50) {
        sprintf(response, "%d\nUser %s:\n<Message too long>\n%s\n\n", MESSAGE, sender_id_string, time);
    } else {
        char short_message[MAX_MESSAGE_LENGTH - 50];
        strcpy(short_message, new_string);
        sprintf(response, "%s", short_message);
    }

    return atoi(sender_id_string);
}

// ----- COMMANDS -----

void list() {
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients_queues[i] != -1) {
            printf("Client %d queue descriptor: %d\n", i, clients_queues[i]);
        }
    }
}

void all(char* request) {
    char response[MAX_MESSAGE_LENGTH];
    int sender = prepare_message_to_pass_on(request, response);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients_queues[i] != -1 && i != sender) {
            mq_send(clients_queues[i], response, MAX_MESSAGE_LENGTH, MESSAGE);
        }
    }
}


void one(char* request) {
    char response[MAX_MESSAGE_LENGTH];
    char recipient_id_string[3];
    strncpy(recipient_id_string, request, 2);
    recipient_id_string[2] = '\0';

    prepare_message_to_pass_on(&request[3], response);

    mq_send(clients_queues[atoi(recipient_id_string)], response, MAX_MESSAGE_LENGTH, MESSAGE);
}


void stop(char* request) {
    clients_queues[atoi(request)] = -1;
    --user_count;
}


void init(char* request) {
    char response[MAX_MESSAGE_LENGTH];

    int index = get_index();
    mqd_t client_desc = mq_open(request, O_WRONLY);
    clients_queues[index] = client_desc;

    sprintf(response, "%d%d", INIT, index);
    mq_send(client_desc, response, MAX_MESSAGE_LENGTH, INIT);
}

// ----- HANDLE INCOMING MESSAGES -----

void parse_message(char* request) {
    printf("Message received\n");

    int type = request[0] - 48;

    switch (type) {
        case LIST: {
            list();
            break;
        }
        case ALL: {
            all(&request[1]);
            break;
        }
        case ONE: {
            one(&request[1]);
            break;
        }
        case STOP: {
            stop(&request[1]);
            break;
        }
        case INIT: {
            init(&request[1]);
            break;
        }
        default: {
            printf("Received invalid message type\n");
            break;
        }
    }
}


int main() {
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        clients_queues[i] = -1;
    }

    queue_desc = mq_open(SERVER_QUEUE_NAME, O_RDWR | O_CREAT, 0666);

    struct sigaction action;
    action.sa_handler = handler;
    sigemptyset(&(action.sa_mask));
    action.sa_flags = 0;
    sigaction(SIGINT, &action, NULL);

    printf("Server started\n");

    char request[MAX_MESSAGE_LENGTH];
    while (1) {
//        if (mq_receive(queue_desc, request, MAX_MESSAGE_LENGTH, NULL) == -1) continue;
        mq_receive(queue_desc, request, MAX_MESSAGE_LENGTH, NULL);
        parse_message(request);
    }
}