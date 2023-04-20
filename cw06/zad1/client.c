#include "header.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>


int client_id = -1;
int client_queue_id = -1;
int server_queue_id = -1;
int command_listener_pid = -1;
int client_running = 1;


void init_client() {
    custom_message init_request, init_response;

    init_request.type = INIT;
    init_request.additional_data = client_queue_id;
    msgsnd(server_queue_id, &init_request, sizeof(custom_message) - sizeof(long), 0);

    init_response.additional_data = -1;
    msgrcv(client_queue_id, &init_response, sizeof(custom_message) - sizeof(long), 0, 0);

    if (init_response.additional_data == -1) {
        printf("%s\n", init_response.message);
        exit(1);
    } else {
        client_id = init_response.additional_data;
        printf("Client started\n");
    }
}


void shutdown() {
    msgctl(client_queue_id, IPC_RMID, NULL);
    exit(0);
}

// ----- SEND MESSAGES -----

void send_list() {
    custom_message new_message;
    new_message.type = LIST;
    new_message.sender_id = client_id;
    msgsnd(server_queue_id, &new_message, sizeof(custom_message) - sizeof(long), 0);
}


void send_2_all(char* message) {
    custom_message new_message;
    new_message.type = ALL;
    new_message.sender_id = client_id;
    sprintf(new_message.message, "%s", message);
    msgsnd(server_queue_id, &new_message, sizeof(custom_message) - sizeof(long), 0);
}


void send_2_one(char* message, int recipient_id) {
    custom_message new_message;
    new_message.type = ONE;
    new_message.sender_id = client_id;
    new_message.additional_data = recipient_id;
    sprintf(new_message.message, "%s", message);
    msgsnd(server_queue_id, &new_message, sizeof(custom_message) - sizeof(long), 0);
}


void send_stop() {
    custom_message new_message;
    new_message.type = STOP;
    new_message.sender_id = client_id;
    msgsnd(server_queue_id, &new_message, sizeof(custom_message) - sizeof(long), 0);
    // STOP to self, to shut down both processes
    msgsnd(client_queue_id, &new_message, sizeof(custom_message) - sizeof(long), 0);
}

// ----- PARSERS -----

void parse_command(char* input) {
    char* command = strtok(input, " \n");

    if (strcmp(command, "list") == 0) {
        send_list();
    } else if (strcmp(command, "2all") == 0) {
        char* message = strtok(NULL, "");
        send_2_all(message);
    } else if (strcmp(command, "2one") == 0) {
        int recipient_id = atoi(strtok(NULL, " "));
        char* message = strtok(NULL, "");
        send_2_one(message, recipient_id);
    } else if (strcmp(command, "stop") == 0) {
        send_stop();
        shutdown();
    }
}


void parse_message(custom_message *message) {
    switch (message->type) {
        case MESSAGE: {
            printf("%s", message->message);
            break;
        }
        case STOP: {
            kill(command_listener_pid, SIGINT);
            shutdown();
            break;
        }
        default: {
            break;
        }
    }
}


void handler(int signum) {
    if (getpid() == command_listener_pid) {
        send_stop();
        client_running = 0;
        exit(0);
    }
}


// ----- LISTENERS -----

void command_listener() {
    char line[MAX_MESSAGE_LENGTH + 50];
    while (client_running) {
        printf("> ");
        fgets(line, MAX_MESSAGE_LENGTH + 50, stdin);

        parse_command(line);
    }
}


void message_listener() {
    custom_message server_message;
    while (1) {
        if (msgrcv(client_queue_id, &server_message, sizeof(custom_message) - sizeof(long), -9, 0) == -1) continue;
        parse_message(&server_message);
    }
}


int main() {
    client_queue_id = msgget(IPC_PRIVATE, 0660);
    server_queue_id = msgget(ftok(getenv("HOME"), PROJECT), 0);

    init_client();

    struct sigaction action;
    action.sa_handler = handler;
    sigemptyset(&(action.sa_mask));
    action.sa_flags = 0;
    sigaction(SIGINT, &action, NULL);
    command_listener_pid = getpid();

    if (fork() == 0) {
        message_listener();
    } else {
        command_listener();
    }
}
