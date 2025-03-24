#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include "linked_list.h"

Node * enqueue(Node* head, int n_id, int n_class, double n_arrival, int n_service){

	Node * cur = head;

	//set up new node;
	Node * new_node =(Node *)malloc(sizeof(Node));
	//int size_new_path = sizeof(new_path);
	//char * new_path_str = (char*)malloc(size_new_path);
	//strncpy(new_path_str, new_path, size_new_path);

	new_node->user_id = n_id;
	new_node->a_class = n_class;
    new_node->arrival_t = n_arrival;
	new_node->service_t = n_service;
	new_node->next = NULL;

	// if the head DNE yet
	if(cur == NULL){
		head = new_node;
	}
	else{
		/*
        if(cur->arrival_t > n_arrival){
            new_node->next = cur;
            head = new_node;
            return head;
        }*/
		while(cur->next != NULL ){
			cur = cur->next;
		}
		/*
        if(cur->next != NULL){
            new_node->next = cur->next;
        }*/
		cur->next = new_node;
	}
	return head;
}


Node * dequeue(Node* head){
	// will check for node with speicic pid, if it is not in the list, returns a NULL, otherwise removes the node from the linked list
	Node * cur = head;
	//Node * de_node = NULL;

	// This shouldn't happen, but let's be safe
	if (cur == NULL)
	{	
		return NULL;
	}

	// If there is more list...
	if(cur->next != NULL){
		head = cur->next;
		
	}else{
		// Now we have an empty list
		head = NULL;
	}

	cur->next = NULL;

	free(cur);
	return head;
	

/*
	if(PIfExist(head, pid) == 1){

			while(cur != NULL ){
				
				// if the node with pid is found
				if (checkNodeP(cur, pid) == 1){
					if(list_id == 0){ //if pid is the head node
					
						head = cur->next;
						cur->next = NULL;
						
					}else{ //any other node
						prev->next = cur->next;
						cur->next = NULL;
					}
					free(cur);
					return head;
				}
				prev = cur;
				cur = cur->next;
				list_id++;
			}
		}else{
			perror("list is empty \n");
		}
	}*/
	//return de_node, head;
}

void printList(Node *node){

	Node * cur = node;
	int num_customers = 0;
	printf("ID Class Arrival Service\n");
	if (node != NULL){
		while (cur != NULL){
		///	int p_id = (int) cur->pid;

			printf("%d: %d, %.2f, %d\n",cur->user_id, cur->a_class, cur->arrival_t, cur->service_t);
			cur = cur->next;
			num_customers++;
		}
	}
	
	printf("Number of Customer in Queue : %d \n", num_customers);// your code here
}

int queue_len(Node *node){

	Node * cur = node;
	int q_len = 0;

	while(cur != NULL){
		q_len++;
		cur = cur->next;
	}
	return q_len;
}

int head_of_queue(Node *node, int u_id){
	Node * cur = node;
	if(cur != NULL && cur->user_id == u_id){
		return 1;

	}
	return 0;
}

// Return the user ID at the head of the list
int user_q(Node *node){
	Node * cur = node;

	if (node == NULL)
	{
		printf( "Null list in user_q\n");
	}
	return cur->user_id;
}

// Return the class at the head of the list
int class_q(Node *node){
	Node * cur = node;
	if (node == NULL)
	{
		printf( "Null list in class_q\n");
	}
	return cur->a_class;
}

// Return the arrival time at the head of the list
double arrival_q(Node *node){
	Node * cur = node;
	if (node == NULL)
	{
		printf( "Null list in arrival_q\n");
	}
	return cur->arrival_t;
}

// Return the service time at the head of the list.
int service_q(Node *node){
	Node * cur = node;
	if (node == NULL)
	{
		printf( "Null list in service_q\n");
	}
	return cur->service_t;
}