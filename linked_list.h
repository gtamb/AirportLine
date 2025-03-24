#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_

typedef struct Node Node;


struct Node{
    int user_id;
    int a_class;
    double arrival_t;
    int service_t;
    Node * next;
};

Node * enqueue(Node* head, int n_id, int n_class, double n_arrival, int n_service);
Node * dequeue(Node* head);
void printList(struct Node *node);
int head_of_queue(Node * head, int u_id);
int user_q(Node *node);
int class_q(Node *node);
double arrival_q(Node *node);
int service_q(Node *node);


#endif