#include <stdio.h>
#include <stdlib.h>
struct string_list {
	struct string_list * next;
	char * string;
};
struct message_list {
	// points to next message_list node
	struct message_list * next;
	struct string_list * str_list_head;
	int strings_count;

	const char * to;
	const char * from;
};
struct message_list * message_list_head = NULL;

// helpers
// returns whether or not the source and destination of a given node match the source and destination of the requester
int compareHelper(struct message_list * node, const char * to, const char * from) {
	int sameDest = strcmp(to, node->to);
	int sameSource = strcmp(from, node->from);
	return sameDest && sameSource;
}

// takes no args bc head of message list is kept. returns null if there are no matches.
struct message_list * findMessageNode(const char * to, const char * from) {
	struct message_list * node = message_list_head;
	while(node != NULL && !compareHelper(node, to, from)) {
		node = node->next;
	}
	return node;
}

// methods for inserting (called by send message)
void insert_head(const char * to, const char * message, const char * from) {
	// typical linked list insert at head
	struct message_list * node = malloc(sizeof(struct message_list));
	node->next = message_list_head;
	message_list_head = node;
}

// we kind of have to check for this special case, so quirky lol
void insert_string_at_tail(struct string_list * strings, const char * message) {
	if(strings == NULL) {
		strings = malloc(sizeof(struct string_list));
		strings->next = NULL;
		strcpy(strings->string, message);
		return;
	}
	struct string_list * curr = strings;
	while(curr->next != NULL) {
		curr = curr->next;
	}
	struct string_list * newNode = malloc(sizeof(struct string_list));
	newNode->next = NULL;
	strcpy(newNode->string, message);
	curr->next = newNode;
}

void addMessageNode(const char * to, const char * message, const char * from) {
	// how about we look through, and if we don't find it, we just fucking idk fucking reutrn the fucking head
	// traverse the list until you find either a NULL node (end or empty) or we find a to & from match
	struct message_list * nodeExists = findMessageNode(to, from);
	struct string_list * strings;
	if(nodeExists) {
		struct string_list * strings  = nodeExists->str_list_head;
		insert_string_at_tail(strings, message);
		nodeExists->strings_count = nodeExists->strings_count++;
	}
	else {
		struct message_list * node = malloc(sizeof(struct message_list));
		node->next = NULL;
		strcpy(node->to, to);
		strcpy(node->from, from);
		strings = node->str_list_head;
		insert_string_at_tail(strings, message);
		insert_head(to, message, from);
		node->strings_count = 1;
	}

}

void freeMem(struct message_list * node) {
	// i'll take care of this later :)
}



int cs1550_send_msg(const char * to, const char * msg, const char * from)
{
	printf("entered send\n");
	

	char * k_to = malloc(sizeof(char));
	printf("progress;\n");
	char * k_msg = malloc(sizeof(char));
	printf("progress2;\n");
	char * k_from = malloc(sizeof(char));
	printf("progress3;\n");
	strcpy(k_to, to);
	strcpy(k_msg, msg);
	strcpy(k_from, from);
	printf("FINISHEd\n");
	
	addMessageNode(k_to, k_msg, k_from); // this currently segfaults.

	printf("finished send\n");

	// copy everything to kernel space pointers so i dont have to deal with stupid user space poitners.. which i cant deref
	return 0;
}
int cs1550_get_msg(const char * to, char * msg, const char * from)
{
	printf("ENTERED GET\n");
	char * k_to;
	char * k_msg;
	char * k_from;
	strcpy(k_to, to);
	strcpy(k_msg, msg);
	strcpy(k_from, from);

	struct message_list * nodeExists = findMessageNode(to, from);
	if(!nodeExists) {
		return -1;
	}
	struct string_list * strings = nodeExists->str_list_head;
	int count = nodeExists->strings_count;
	// make an array of 8 poitners..
	char ** str_array = calloc(sizeof(char *), count);
	struct string_list * curr = strings;
	for(int i = 0; i < count; i++) {
		char * tempStr = *(str_array + count);
		strcpy(tempStr, strings->string);
		curr = curr->next;
	}

	freeMem(nodeExists);
	*msg = str_array;

	// i'm going to return an array of pointers!

	// wait how the fuck do i return the strings
	// copy everything to kernel space pointers so i dont have to deal with stupid user space poitners.. which i cant deref
	return 0;
}




int main() {
	cs1550_send_msg("dylan", "hello world", "feehan");
	char * idk = NULL;
	cs1550_get_msg("dylan\0", &idk, "feehan\0");
	return 0;
}

