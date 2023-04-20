#include "header.h"
#include <stdio.h>
#include <mqueue.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>


int client_id = -1;
mqd_t client_queue_desc = -1;
char client_queue_name[10];
mqd_t server_queue_desc = -1;
int command_listener_pid = -1;
int client_running = 1;


void init_client() {
    char request[MAX_MESSAGE_LENGTH], response[MAX_MESSAGE_LENGTH];
    sprintf(request, "%d%s", INIT, "/queue_1234");

    mq_send(server_queue_desc, request, MAX_MESSAGE_LENGTH, INIT);
    mq_receive(client_queue_desc, response, MAX_MESSAGE_LENGTH, NULL);
    client_id = atoi(&response[1]);

    printf("Client started\n");
}


void shutdown() {
    mq_close(server_queue_desc);
    mq_close(client_queue_desc);
    mq_unlink(client_queue_name);
    exit(0);
}

// ----- SEND MESSAGES -----

void send_list() {
    char new_message[MAX_MESSAGE_LENGTH];
    sprintf(new_message, "%d", LIST);
    mq_send(server_queue_desc, new_message, MAX_MESSAGE_LENGTH, LIST);
}


void send_2_all(char* message) {
    char new_message[MAX_MESSAGE_LENGTH];

    char client_id_string[3];
    if (client_id > 9) sprintf(client_id_string, "%d", client_id);
    else sprintf(client_id_string, "0%d", client_id);
    client_id_string[2] = '\0';

    sprintf(new_message, "%d%s%s", ALL, client_id_string, message);

    mq_send(server_queue_desc, new_message, MAX_MESSAGE_LENGTH, ALL);
}


void send_2_one(char* message, int recipient_id) {
    char new_message[MAX_MESSAGE_LENGTH];

    char client_id_string[3];
    if (client_id > 9) sprintf(client_id_string, "%d", client_id);
    else sprintf(client_id_string, "0%d", client_id);
    client_id_string[2] = '\0';

    if (recipient_id > 9) {
        sprintf(new_message, "%d0%d%s%s", ONE, recipient_id, client_id_string, message);
    } else {
        sprintf(new_message, "%d%d%s%s", ONE, recipient_id, client_id_string, message);
    }

    mq_send(server_queue_desc, new_message, MAX_MESSAGE_LENGTH, ONE);
}


void send_stop() {
    char new_message[MAX_MESSAGE_LENGTH];
    sprintf(new_message, "%d", STOP);
    mq_send(server_queue_desc, new_message, MAX_MESSAGE_LENGTH, STOP);
    // STOP to self, to shut down both processes
    mq_send(client_queue_desc, new_message, MAX_MESSAGE_LENGTH, STOP);
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


void parse_message(char* message) {
    int type = message[0] - 48;

    switch (type) {
        case MESSAGE: {
            printf("%s", &message[1]);
            break;
        }
        case STOP: {
            shutdown();
            kill(command_listener_pid, SIGINT);
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
    char server_message[MAX_MESSAGE_LENGTH];
    while (1) {
        mq_receive(server_queue_desc, server_message, MAX_MESSAGE_LENGTH, NULL);
        parse_message(server_message);
    }
}


int main() {
    sprintf(client_queue_name, "/queue%d", getpid());

    client_queue_desc = mq_open("/queue_1234", O_RDWR | O_CREAT, 0660);
    server_queue_desc = mq_open(SERVER_QUEUE_NAME, O_WRONLY);

    printf("%d\n", client_queue_desc);
    printf("%d\n", server_queue_desc);

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