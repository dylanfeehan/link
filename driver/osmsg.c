#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int send_message_syscall(char * to, char * message, char * from);
int get_message_syscall(char * to, char * message, char * from);

void usage() {
        printf("usage:\n    <user>: receive messages\n    <to> <message> <from>: send message\n");
}
void send_message_wrapper(char ** args) {
    send_message_syscall(args[0], args[1], args[2]);
}
void get_message_wrapper(char ** args) {
    char * str = args[0];
    int length = strlen(str);
    *(str + length - 1) = 0;
//void get_message_wrapper(char * receiver) {
    // args[0] is to
    char * message; // space for message
    char * from; // space for from
    int more_strings; // int for to know if more strings
    do {
        message = malloc(1024 * sizeof(char));
        from = malloc(1024 * sizeof(char));
        more_strings = get_message_syscall(str, message, from);
        //more_strings = get_message_syscall(receiver, message, from);
            printf("message: %s\n", message);
            printf("from:    %s\n", from);
            free(message);
            free(from);
    }
    while(more_strings);
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
    /*
    send_message_syscall("jon", "hey jon", "dylan");
    get_message_wrapper("jon");
    send_message_syscall("dylan", "hey dylan", "jon");
    get_message_wrapper("dylan");

    send_message_syscall("jon", "hey again jon", "dylan");
    send_message_syscall("jon", "hey again again jon", "dylan");
    get_message_wrapper("jon");
    send_message_syscall("dylan", "hey again dylan", "jon");
    send_message_syscall("dylan", "hey again again dylan", "jon");
    get_message_wrapper("dylan");
    */
}

//////////////////
// KERNEL SPACE //
//////////////////

struct message_node {
    struct message_node * next;
    char * from;
    char * message;
};

struct user_node {
    struct user_node * next;
    // identifier
    char * user;

    // messages
    struct message_node * message_queue_head;
    struct message_node * message_queue_tail;

    int message_count;
};

struct user_node * user_list_head = NULL;
struct user_node * cached_node = NULL;

// this makes no assumptions about the state of the user list (null or nonnull),
// it only assumes that it is given a nonnull user node
// also, user->next should not be mallocd.. because it's either going to point to an alrady allocated struct, 
// or null. so we take care of that here
void insert_user_node(struct user_node * user) {
    if(user_list_head == NULL) {
        user_list_head = user;
        user->next = NULL;
    }
    else {
        user->next = user_list_head;
        user_list_head = user;
    }
}

// assumes a non null message node, and assuems that the message_node was allocated and populated
/* struct message_node {
        next: unpopulated, either points to allocated or NULL
        from: populated
        message: populated
}
*/
void append_message_node(struct user_node * curr_user, struct message_node * new_tail) {
    new_tail->next = NULL; // appending to end
    struct message_node * curr_tail = curr_user->message_queue_tail;

    /* 
    either the messages were retrieved and the message queues were nullified, 
    or there's messages in the queue
    */
    if(curr_tail == NULL) {
        curr_user->message_queue_head = new_tail;
        curr_user->message_queue_tail = new_tail;
    }
    else {
        curr_tail->next = new_tail;
        curr_user->message_queue_tail = new_tail;
    }
}

struct user_node * findUserNode(char * user) {
    if(cached_node != NULL && strcmp(cached_node->user, user) == 0) {
        return cached_node;
    }
    struct user_node * node = user_list_head;
    while(node != NULL && strcmp(node->user, user) != 0) {
        node = node->next;
    }
    cached_node = node;
    return node;
}

struct message_node * create_message_node(char * from, char * message) {
    struct message_node * new_message_node = malloc(sizeof(struct message_node));
    if(new_message_node == NULL) {
        printf("new message node is null\n");
    }
    if(from == NULL) {
        printf("from is null\n");
    }
    new_message_node->from = NULL;
    char * idk = malloc(sizeof(char) * 3);
    new_message_node->from = malloc(sizeof(char) * strlen(from));
    new_message_node->message = malloc(sizeof(char) * strlen(message));
    strcpy(new_message_node->from, from);
    strcpy(new_message_node->message, message);
    return new_message_node;
}

// this needs to return both from and to... how about a char**?
struct message_node * get_message_queue_head(struct user_node * curr_user_node) {
    if(curr_user_node->message_count == 0) {
        return NULL; // this means .. we're out of strings.. return 0;
    }
    struct message_node * ret_message_node = curr_user_node->message_queue_head;
    if(ret_message_node == NULL) {
        printf("probably bad\n");
    }

    // update the user's head and tail references
    curr_user_node->message_queue_head = ret_message_node->next; // THIS IS A SIDE EFFECT OF THE ERROR!
    if(curr_user_node->message_queue_head == NULL) {
        curr_user_node->message_queue_tail == NULL;
    }

    curr_user_node->message_count = curr_user_node->message_count - 1;
    return ret_message_node;
}


int send_message_syscall(char * to, char * message, char * from) {
    struct user_node * node_exists = findUserNode(to); // since it's add
    struct message_node * new_message_node = create_message_node(from, message);

    if(node_exists) {
        append_message_node(node_exists, new_message_node);
        node_exists->message_count = node_exists->message_count + 1;
    }
    else {
        // node does not exist yet. will have to insert it
        struct user_node * new_user_node = malloc(sizeof(struct user_node));
        new_user_node->user = malloc(sizeof(char) * strlen(to));
        strcpy(new_user_node->user, to);

        append_message_node(new_user_node, new_message_node);
        insert_user_node(new_user_node);
        new_user_node->message_count = 1;
    }
    return 0;
}

int get_message_syscall(char * to, char * message, char * from) {
    struct user_node * node_exists = findUserNode(to);   
    if(!node_exists) {
        printf("user does not exist\n");
        exit(0);
    }
    if(node_exists->message_count == 0) {
        strcpy(from, "n/a");
        strcpy(message, "No current messages");
        return 0;
    }
    struct message_node * message_queue_head = get_message_queue_head(node_exists);

    strcpy(from, message_queue_head->from);
    strcpy(message, message_queue_head->message);
    ///////////////////
    // free the node //
    ///////////////////
    // free ret_message_node    
    free(message_queue_head->from);
    free(message_queue_head->message);
    free(message_queue_head);
    if(node_exists->message_queue_head == NULL) {
        node_exists->message_queue_tail = NULL;
    }

    return node_exists->message_count > 0;
}