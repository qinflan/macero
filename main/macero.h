// header file for structs, globals, etc in my main file:

// add screen drivers, includes, structs, api, helper functions

#include "captive_portal.h"
#include

typedef struct MenuNode {
	char* options[10];
	int num_options;
	struct MenuNode** children;
	int num_children;
	struct MenuNode* parent;
	void (*actions[10])(SSD1306_t *dev);
} MenuNode;

typedef struct ListNode {
	login_data_t data;
	struct ListNode *next;
} login_node_t;

