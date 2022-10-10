#include <stdio.h>
#include <syscall.h>
#include <stdlib.h>
#include <string.h>

void usage() {
    printf("usage:\n    <user>: receive messages\n    <to> <message> <from>: send message\n");
}
void send_message_wrapper(char ** args) {
    /* 
        syscall
    */
    int result = syscall(441, args[0], args[1], args[2]); 
}

void get_message_wrapper(char ** args) {
    char * str = args[0];
    int length = strlen(str);
    *(str + length - 1) = 0;
    char * message; // space for message
    char * from; // space for from
    int more_strings; // int for to know if more strings
    do {
        message = malloc(1024 * sizeof(char));
        from = malloc(1024 * sizeof(char));
        /*
            syscall
        */
        more_strings = syscall(442, str, message, from);
        //more_strings = get_message_syscall(str, message, from);
        if(more_strings == -1) {
            printf("error occurred.\n");
            break;
        }
        printf("message: %s\n", message);
        printf("from:    %s\n", from);
        free(message);
        free(from);
    }
    while(more_strings == 1);
}

int main(int argc, char ** argv) {
    while(1) {
        usage();
        printf("> ");
        char * input;
        size_t bufsize = 29;
        input = malloc(sizeof(char) * bufsize);
        fflush(stdin);
        getline(&input, &bufsize, stdin);
        char * pch;
        pch = strtok (input, " ");
        char * args[3];
        int count = 0;
        while(pch != NULL) {
            args[count] = pch;
            pch = strtok(NULL, " ");
            count = count + 1;
        }
        if(strncmp(args[0], "exit", 4) == 0) {
            break;
        }
        if(count == 1) {
            get_message_wrapper(args);
        }
        else if(count == 3) {
            send_message_wrapper(args);
        }
        else {
            usage();
            continue;
        }
    }
    return 0;
}
