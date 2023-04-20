#include "header.h"
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <string.h>


int clients_queues[MAX_CLIENTS];
int next_free_index = 0;
int user_count = 0;
int queue_id;


// ----- HANDLE CTRL+C -----
void shutdown_clients() {
    custom_message message;
    message.sender_id = SERVER_ID;
    message.type = STOP;

    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients_queues[i] != -1) {
            msgsnd(clients_queues[i], &message, sizeof(custom_message) - sizeof(long), 0);
        }
    }
}


void handler(int signum) {
    printf("\nServer shutting down\n");

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
        if (clients_queues[i] == -1) {
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
        if (clients_queues[i] != -1) {
            printf("Client queue id: %d\n", clients_queues[i]);
        }
    }
}

void all(custom_message *request) {
    custom_message new_message = prepare_message_to_pass_on(request);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients_queues[i] != -1 && i != request->sender_id) {
            msgsnd(clients_queues[i], &new_message, sizeof(custom_message) - sizeof(long), 0);
        }
    }
}


void one(custom_message *request) {
    custom_message new_message = prepare_message_to_pass_on(request);
    msgsnd(clients_queues[request->additional_data], &new_message, sizeof(custom_message) - sizeof(long), 0);
}


void stop(custom_message *request) {
    clients_queues[request->sender_id] = -1;
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
        clients_queues[index] = req_queue_id;

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


int main () {
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        clients_queues[i] = -1;
    }

    key_t queue_key = ftok(getenv("HOME"), PROJECT);
    queue_id = msgget(queue_key, IPC_CREAT | 0660);

    struct sigaction action;
    action.sa_handler = handler;
    sigemptyset(&(action.sa_mask));
    action.sa_flags = 0;
    sigaction(SIGINT, &action, NULL);

    printf("Server started\n");

    custom_message request;
    while (1) {
//        msgrcv(queue_id, &request, sizeof(custom_message) - sizeof(long), STOP, 0);
        if (msgrcv(queue_id, &request, sizeof(custom_message) - sizeof(long), -9, 0) == -1) continue;
        parse_message(&request);
    }
}
