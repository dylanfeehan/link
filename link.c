#include <string.h>
#include <stdio.h>
#include <stdlib.h>

struct string_queue {
	struct string_queue * next;
	char * string;
};

struct message_list {
	struct message_list * next;

	struct string_queue * string_queue_head;
	struct string_queue * string_queue_tail;

	int strings_count; // probably will end up being useless

	const char * to;
	const char * from;
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
struct message_list * message_list_head = NULL;
// holds the recently.. gotten.. node
struct message_list * cached_node = NULL;

// checked
void insert_head(struct message_list * node) {
	if(node == NULL) {
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
	node->next = message_list_head;
	message_list_head = node;
}

// NEEDS CHECKING
// a non null message node, a non null string node
void insert_string_at_tail(struct message_list * message_node, struct string_queue  * new_tail) {
	// tail of message node could be null???? yes... no... actuall no...  lets make it so that's not possible
	// set string node next to null, since it's the tail
	new_tail->next = NULL; // not null.
	// get the current tail of the... .. .. string queue for this message
	struct string_queue * current_tail = message_node->string_queue_tail;
	// make the new tail
	if(current_tail == NULL) { // if theres no string list make both poiters the new node
		message_node->string_queue_head = new_tail;
		message_node->string_queue_tail = new_tail;
	}
	else {
		current_tail->next = new_tail; // not null. // else update it
		// now the message_node tail field points to our new tail!
		message_node->string_queue_tail = new_tail;
	}
}

int compareHelper(struct message_list * node, const char * to, const char * from) {
	int sameDest = my_strcmp(to, node->to);
	int sameSource = my_strcmp(from, node->from);
	return sameDest && sameSource;
}

struct message_list * findMessageNode(const char * to, const char * from) {
	if(cached_node != NULL && compareHelper(cached_node, to, from)) {
		return cached_node;
	}
	struct message_list * node = message_list_head; 

	while(node != NULL && !compareHelper(node, to, from)) {
		node = node->next;
	}
	cached_node = node;
	return node;
}

void addMessageNode(const char * to, const char * message, const char * from) {
	struct message_list * nodeExists = findMessageNode(to, from); // pop 0
	struct string_queue * strings;

 	// if node exists, string tail shouldn't be nul
	if(nodeExists) {
		// no. we allocate the space here. the only thing 
		int length = strlen(message);
		// this allocates space for the struct
		struct string_queue * new_string_tail = sizeof(struct string_queue);
		// this allocates space for the pointer within the strut
		new_string_tail->next = malloc(sizeof(struct string_queue));
		// this allocates space for the pointer to string wtihin the sturct
		new_string_tail->string = malloc(sizeof(char) * length);
		// copies message into string node
		strcpy(new_string_tail, message);
		insert_string_at_tail(nodeExists, new_string_tail);
		nodeExists->strings_count = nodeExists->strings_count++;
	}
	else { // yo this code sucks ass yo!
		// YOU ACTUALLY have to take care of the to && from shit here...
		// before it exists!!!!
		struct message_list * new_message_node = malloc(sizeof(struct message_list));
		new_message_node->next = malloc(sizeof(struct message_list));
		// this is for the string list inside of the queue, we'll worry about this in insert... thats resonsible for maintainig this
		new_message_node->string_queue_head = malloc(sizeof(struct string_queue));
		new_message_node->string_queue_tail = malloc(sizeof(struct string_queue));


		// take care of the to & from shit
		int to_len = strlen(to);
		int from_len = strlen(from);
		new_message_node->to = malloc(sizeof(char) * to_len);// this is wrong.
		new_message_node->from = malloc(sizeof(char) * from_len);// this is wrong.
		strcpy(new_message_node->to, to);
		strcpy(new_message_node->from, from);

		// string_queue tail and head are both... um.... allocated... but their couterparts are not... time to allocate
		struct string_queue * new_ele = malloc(sizeof(struct string_queue));
		// allocate size for string
		int message_len = strlen(message);
		new_ele->string = malloc(sizeof(char) * message_len);
		strcpy(new_ele->string, message);
		// allocate size for pointer
		new_ele->next = malloc(sizeof(struct string_queue));
		new_ele->next = NULL; // will be tail so why not?
		// gives the new message node with to and from, and string queue fields allocated, and gives the new ele with string allocated and next allocated
		// this shoudl actually work!
		insert_string_at_tail(new_message_node, new_ele);
		insert_head(new_message_node);
		new_message_node->strings_count = 1;
	}
    return;
}

void freeMem(struct message_list * node) {

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

char * getStringListHead(char * msg, struct message_list * message_node) {
	// idk
	printf("1\n");
	if(message_node->strings_count == 0) {
		return NULL;
	}
	printf("2\n");
    struct string_queue * string_node = message_node->string_queue_head;
	printf("2.5\n");
	// here it is!
	if(string_node == NULL) {
		printf("you done fucked up gamer\n");
	}
	if(string_node->string == NULL) {
		printf("you done fucked up gamer pt 2\n");
	}
	int len = strlen(string_node->string);
	printf("3\n");
	char * val = malloc(sizeof(char) * len);
	msg = malloc(sizeof(char) * len);
	printf("4\n");
	strcpy(val, string_node->string);
	strcpy(msg, val);
	printf("5\n");

	// update this message node string list
	message_node->string_queue_head = string_node->next;
	message_node->strings_count = message_node->strings_count - 1;
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

