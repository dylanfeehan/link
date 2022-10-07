#include <string.h>
#include <stdio.h>
#include <stdlib.h>

struct message_node {
	struct message_node * next;
	char * from;
	char * message;
};

struct user_node {
	struct user_node * next;
	char * user;

	struct message_node * message_queue_head;
	struct message_node * message_queue_tail;

	int message_count; // probably will end up being useless
};

// test in kernel, see if this ends being obsolete
int my_strcmp(char * a, char * b) {
	int i = 0;
	while(*(a + i) != 0 && *(b + i) != 0) {
		if(*(a + i) != *(b + i)) {
			return 0;
		}
		i = i + 1;
	}
	if(*(a + i) != *(b + i)) {
		return 0;
	}
	return 1;
}

// holds the messages
struct user_node * user_list_head = NULL;
// holds the recently.. gotten.. node
struct user_node * cached_node = NULL;

// checked
void insert_head(struct user_node * user) {
	if(user == NULL) {
		printf("cannot give insert_head a null head to insert\n");
		exit(0);
	}
	/* cases:
		1. message_list_head is null
			- node->next is null
			- message_list_head variable now points to node, so it's the new head :)
		2. message_list_head isn't null
			- node->next = message_list_head, this is fine
			- message_list_head - node.. still fine... all good
	*/ 
	user->next = user_list_head;
	user_list_head = user;
}

// NEEDS CHECKING
// a non null message node, a non null string node
void insert_message_at_tail(struct user_node * user_node, struct message_node * new_tail) {
	// tail of message node could be null???? yes... no... actuall no...  lets make it so that's not possible
	// set string node next to null, since it's the tail
	new_tail->next = NULL; // not null.
	// get the current tail of the... .. .. string queue for this message
	struct message_node * current_tail = user_node->message_queue_tail;
	// make the new tail
	if(current_tail == NULL) { // if theres no string list make both poiters the new node
		user_node->message_queue_head = new_tail;
		user_node->message_queue_tail = new_tail;
	}
	else {
		current_tail->next = new_tail; // not null. // else update it
		// now the message_node tail field points to our new tail!
		user_node->message_queue_tail = new_tail;
	}
}

// returns 1 if the specified user matches the user of the user_node
int findUser(struct user_node * node, char * user) {
	return my_strcmp(node->user, user);
}

struct user_node * findUserNode(char * user) {
	if(cached_node != NULL && compareHelper(cached_node, user)) {
		return cached_node;
	}
	struct user_node * node = user_list_head; 

	while(node != NULL && !compareHelper(node, user)) {
		node = node->next;
	}
	cached_node = node;
	return node;
}

void addMessageNode(const char * to, const char * message, const char * from) {
	// from .. we place that in the message node... 
	// to: to find the node
	struct user_node * nodeExists = findMessageNode(to); // since it's add
	struct message_node * messages;

 	// if node exists, string tail shouldn't be nul
	if(nodeExists) {
		// no. we allocate the space here. the only thing 
		int length = strlen(message);
		// this allocates space for the struct
		struct message_node * new_message_tail = sizeof(struct message_node);
		// this allocates space for the pointer within the strut
		new_message_tail->next = malloc(sizeof(struct message_node));
		// this allocates space for the pointer to string wtihin the sturct
		new_message_tail->message = malloc(sizeof(char) * length);
		// copies message into string node
		strcpy(new_message_tail->message, message);

		insert_message_at_tail(nodeExists, new_message_tail);
		nodeExists->message_count = nodeExists->message_count++;
	}
	else { // yo this code sucks ass yo!
		// YOU ACTUALLY have to take care of the to && from shit here...
		// before it exists!!!!
		struct user_node * new_user_node = malloc(sizeof(struct user_node));
		new_user_node->next = malloc(sizeof(struct user_node));
		// this is for the string list inside of the queue, we'll worry about this in insert... thats resonsible for maintainig this
		new_user_node->message_queue_head = malloc(sizeof(struct message_node));
		new_user_node->message_queue_tail = malloc(sizeof(struct message_node));


		// take care of the to & from shit
		//
		int to_len = strlen(to); // for top level user node
		new_user_node->user = malloc(sizeof(char) * to_len);// this is wrong.
		strcpy(new_user_node->user, to);

		int from_len = strlen(from); // for inner message node
		int message_len = strlen(message);
		// string_queue tail and head are both... um.... allocated... but their couterparts are not... time to allocate
		struct message_node * new_message_node = malloc(sizeof(struct message_node));
		// allocate size for string
		new_message_node->message = malloc(sizeof(char) * message_len);
		new_message_node->from = malloc(sizeof(char) * from_len);
		strcpy(new_message_node->message, message);
		strcpy(new_message_node->from, from);

		// allocate size for pointer
		new_message_node->next = malloc(sizeof(struct message_node));
		new_message_node->next = NULL; // will be tail so why not?
		// gives the new message node with to and from, and string queue fields allocated, and gives the new ele with string allocated and next allocated
		// this shoudl actually work!
		insert_message_at_tail(new_user_node, new_message_node);
		insert_head(new_user_node);
		new_user_node->message_count = 1;
	}
    return;
}

void freeMem(struct message_list * node) {
	// nah
}

int cs1550_send_msg(const char * to, const char * msg, const char * from) {
	char * k_to = malloc(sizeof(char) * strlen(to));
	char * k_msg = malloc(sizeof(char) * strlen(msg));
	char * k_from = malloc(sizeof(char) * strlen(from));

	strcpy(k_to, to);
	strcpy(k_msg, msg);
	strcpy(k_from, from);
	
	addMessageNode(k_to, k_msg, k_from); 
	return 0;
}

char * getStringListHead(char * msg, struct user_node * curr_user_node) {
	// idk
	printf("1\n");
	if(curr_user_node->message_count == 0) {
		return NULL; // not an error.. you're just done bby
	}
	printf("2\n");
    struct message_node * curr_message_node = curr_user_node->message_queue_head;
	printf("2.5\n");
	// here it is!
	if(curr_message_node == NULL) {
		printf("you done fucked up gamer\n");
	}
	if(curr_message_node->message == NULL) {
		printf("you done fucked up gamer pt 2\n");
	}
	int len = strlen(curr_message_node->message);
	printf("3\n");
	char * val = malloc(sizeof(char) * len);
	msg = malloc(sizeof(char) * len);
	printf("4\n");
	strcpy(val, curr_message_node->message);
	strcpy(msg, val);
	printf("5\n");

	// update this message node string list
	curr_user_node->message_queue_head = curr_message_node->next;
	curr_user_node->message_count = curr_user_node->message_count - 1;
	return msg;
}

int cs1550_get_msg(const char * to, char * msg, const char * from)
{
	printf("initiationg to and from\n");
	char * k_to;
	char * k_from;
	k_to = malloc(sizeof(char*)); k_from = malloc(sizeof(char*));
	strcpy(k_to, to);
	strcpy(k_from, from);

	printf("checking if node exists\n");
	struct message_list * nodeExists = findMessageNode(to, from);
	if(!nodeExists) {
		return -1;
	}
	// TODO: this is a probem vv
	printf("getting string\n");
	   // wait how to do this?
    msg = getStringListHead(msg, nodeExists);
	if(!msg) {
		return 0;
	 	freeMem(nodeExists);
	}
	printf("done");
	return 1;
}

int main() {
	printf("sending message\n");
	cs1550_send_msg("dylan", "big", "feehan");
	char * idk = NULL;
	printf("receiving message\n");
	cs1550_get_msg("dylan", &idk, "feehan");
	if(idk == NULL) {
		printf("bad stuff happened.\n");
		return 0;
	}
	printf("Message: %s\n", idk);
	return 0;
}

