#include <stdio.h>
#include <string.h>

void print_mails(char* sort_by) {
    int column_to_sort_by;

    if (strcmp(sort_by, "nadawca") != 0) {
        column_to_sort_by = 3;
    } else if (strcmp(sort_by, "data") != 0) {
        column_to_sort_by = 4;
    } else {
        printf("Invalid argument!\n");
        return;
    }

    char command[1024];
    command[0] = '\0';
    sprintf(command, "mail -H | sort -k%d", column_to_sort_by);

    FILE* mail_output = popen(command, "r");
    char line[1025];

    while (fgets(line, 1024, mail_output) != NULL) {
        printf("%s", line);
    }
}


void send_mail(char* address, char* subject, char* body) {
    char command[1024];
    command[0] = '\0';
    sprintf(command, "mail -s %s %s", subject, address);

    FILE* mail_input = popen(command, "w");

    fputs("\n", mail_input);
    fputs(body, mail_input);
    fputs("\n", mail_input);

    pclose(mail_input);
}


int main(int argc, char* argv[]) {
    switch (argc) {
        case 2: {
            print_mails(argv[1]);
            break;
        }
        case 4: {
            send_mail(argv[1], argv[2], argv[3]);
            break;
        }
        default: {
            printf("Wrong number of arguments!\n");
            return 1;
        }
    }

    return 0;
}
