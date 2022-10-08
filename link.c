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

// checked for refactor
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

// checked for refactor
/*
	inserts the given new message node at the tail of the queue
	@param user_node curr_user_node the node containing the message queue

*/
void insert_message_at_tail(struct user_node * curr_user_node, struct message_node * new_tail) {
	// why not.. it already has the message and from btw
	new_tail->next = NULL; // not null.
	// curr tail of the message queue
	struct message_node * curr_tail = curr_user_node->message_queue_tail;
	// make the new tail
	if(curr_tail == NULL || curr_tail->message == NULL) { // if theres no string list make both pointers point to the new node
		printf("i happen.. its null");
		curr_user_node->message_queue_head = new_tail;
		curr_user_node->message_queue_tail = new_tail;
	}
	else {
		curr_tail->next = new_tail; // not null. // else update it
		// now the message_node tail field points to our new tail!
		curr_user_node->message_queue_tail = new_tail;
	}
}

// checked for refactoring
// gets the node of a user based on specified ... user param
// sneding them a message? find them to populate their message list
// recieving a message? find you to retrieve your messages... there ya go :)
struct user_node * findUserNode(char * user) {
	if(cached_node != NULL && my_strcmp(cached_node->user, user)) {
		return cached_node;
	}
	struct user_node * node = user_list_head; 

	while(node != NULL && !my_strcmp(node->user, user)) {
		node = node->next;
	}
	cached_node = node;
	return node;
}

// woah this is fucking really shitty!!!!
// checking for refactor
void addMessageNode(const char * to, const char * message, const char * from) {
	// from .. we place that in the message node... 
	// to: to find the node
	struct user_node * nodeExists = findUserNode(to); // since it's add
	struct message_node * messages;

 	// if node exists, string tail shouldn't be nul
	if(nodeExists) {
		// no. we allocate the space here. the only thing 
		int length = strlen(message);
		// this allocates space for the struct
		struct message_node * new_message_tail = malloc(sizeof(struct message_node));
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
		printf("new_message_node->message: %s\n", new_message_node->message);

		// allocate size for pointer
		new_message_node->next = malloc(sizeof(struct message_node));
		new_message_node->next = NULL; // will be tail so why not?
		// gives the new message node with to and from, and string queue fields allocated, and gives the new ele with string allocated and next allocated
		// this shoudl actually work!
		insert_message_at_tail(new_user_node, new_message_node);
		insert_head(new_user_node);
		new_user_node->message_count = 1;
		printf("message added: %s\n", new_user_node->message_queue_tail->message); // this verifies that the issue is in get!
		printf("message from send: %s\n", user_list_head->message_queue_tail->message);
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

char * getStringListHead(struct user_node * curr_user_node) {
	int len;
	char * val;
	if(curr_user_node->message_count == 0) {
		return NULL; // not an error.. you're just done bby
	}
    struct message_node * curr_message_node = curr_user_node->message_queue_head;
	// here it is!
	if(curr_message_node == NULL) {
		printf("you done fucked up gamer\n");
	}
	if(curr_message_node->message == NULL) {
		printf("inside of %s\n", curr_user_node->user);
		printf("you done fucked up gamer pt 2\n");
	}
	len = strlen(curr_message_node->message);
	val = malloc(sizeof(char) * len);

	strcpy(val, curr_message_node->message);

	// update this message node string list
	curr_user_node->message_queue_head = curr_message_node->next;
	curr_user_node->message_count = curr_user_node->message_count - 1;
	printf("message: %s\n", val);
	return val;
}

int cs1550_get_msg(const char * to, char * msg, const char * from)
{
	printf("message from get: %s\n", user_list_head->message_queue_head->message);
	printf("initiationg to and from\n");
	char * k_to;
	char * k_from;
	k_to = malloc(sizeof(char*)); k_from = malloc(sizeof(char*));
	strcpy(k_to, to);
	strcpy(k_from, from);

	printf("checking if node exists\n");
	struct user_node * nodeExists = findUserNode(to);
	if(!nodeExists) {
		return -1;
	}
	// TODO: this is a probem vv
	printf("getting string\n");
	   // wait how to do this?
	char * temp = malloc(1024);
	char * tempFrom = malloc(1024);
    temp = getStringListHead(nodeExists);
	printf("yo: %s\n", temp);
	printf("temp before cpy: %s\n", temp);
	strcpy(msg, temp);

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
	cs1550_send_msg("richie", "small", "dingle");
	char * idk = malloc(1024 * sizeof(char));
	char * from = malloc(1024 * sizeof(char));
	printf("receiving message\n");
	cs1550_get_msg("dylan", idk, from);
	if(idk == NULL) {
		printf("bad stuff happened.\n");
		return 0;
	}
	char * m2 = malloc(1024 * sizeof(char));
	cs1550_get_msg("richie", m2, "dingle");
	printf("Message: %s\n", idk);
	printf("Message2: %s\n", m2);
	printf("it was sending\n");
	cs1550_send_msg("dylan", "bigger", "feehan");
	cs1550_send_msg("richie", "boy", "feehan");
	char * m3 = malloc(1024 * sizeof(char));
	cs1550_get_msg("dylan", m3, "feehan");
	printf("Message3: %s\n", m2);
	char * m4 = malloc(1024 * sizeof(char));
	cs1550_get_msg("richie", m3, "feehan");
	printf("Message4: %s\n", m2);
	return 0;
}