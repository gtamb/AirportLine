#define _GNU_SOURCE
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <errno.h>
#include <sys/time.h>
#define NTHREADS 5
#include "linked_list.h"
#define NQUEUE 2

Node* b_head = NULL; // head of business queue
Node* e_head = NULL; //head of economy queue

double e_wait; //total economy time
double b_wait; //total business time

void *thread_function(void *); //adding thread

pthread_mutex_t lock_b; //mutex declaration business
pthread_mutex_t lock_e; // economy mutex
pthread_mutex_t clerk_lock; // lock for the clerk to get head of queue

int NClerks = 5; // number of clerks
static struct timeval start_time; // simulation start time

pthread_cond_t new_customer = PTHREAD_COND_INITIALIZER; //convar for if there is a customer that needs attention in the queue


struct clerk_info{
    int clerk_id;
    int c_status;
};

struct custom_info{
    int user_id;
    int a_class;
    int service_t;
    int arrival_t;
};

struct timeval init_t;
double overall_wait_t;
int customers_served = 0; // updated by clerks whne they dequeue a customer from either queue
int total_customers = 0; // total customers determined at beginning of file

int queue_length[NQUEUE]; // the number of customers for economy [0] and business[1]
int queue_status[NQUEUE] = {-1, -1}; // if the queue is ready to be served, -1 for idle, clerk number otherwise

// queue 0 economy
// queue 1 business

// file 
    // number of customers
    // (customer id):(0 or 1 for queue),(int arrival time),(int service time),(\n)
    // time: 0.2s to 2, 6s to 60
struct custom_info customers[50];
struct clerk_info clerks[5];

double getCurSystemTime(){
	
	struct timeval cur_time;
	double cur_secs, init_secs;
	
	init_secs = (start_time.tv_sec + (double) start_time.tv_usec / 1000000.0);
	
	gettimeofday(&cur_time, NULL);
	cur_secs = (cur_time.tv_sec + (double) cur_time.tv_usec / 1000000.0);
	
	return cur_secs - init_secs;
}

void * customer_enters(void * customer_info){

    struct custom_info * p_myInfo = (struct custom_info *) customer_info;

    // Multiple for 10ths of a second to microseconds
    usleep(p_myInfo->arrival_t*100000.0);

    pthread_mutex_t selected_lock;

    int cur_queue = p_myInfo->a_class;
    
    if(cur_queue == 0){
        selected_lock = lock_e;

        
    }else{
        selected_lock = lock_b;

    }


    double queue_entry_time = getCurSystemTime();

     //locks the passed resouce
    pthread_mutex_lock(&selected_lock);
    {
    

        // after print enter which queue
        printf("Customer %d arrived \n", p_myInfo->user_id);
        if(cur_queue == 0){
            
            e_head = enqueue(e_head, p_myInfo->user_id, p_myInfo->a_class, queue_entry_time, p_myInfo->service_t);
        }
        else{
            b_head = enqueue(b_head, p_myInfo->user_id, p_myInfo->a_class, queue_entry_time, p_myInfo->service_t);

        }
        queue_length[cur_queue]++;
        printf("Customer %d entered queue %1d at %.2f seconds, and the length of the queue is %2d. \n", p_myInfo->user_id, cur_queue, queue_entry_time, queue_length[cur_queue]);
        pthread_cond_signal(&new_customer);

        
    
        
    }
    pthread_mutex_unlock(&selected_lock);

    pthread_exit(NULL);

    return NULL;
     
}

void *clerk_entry(void * clerk_i){


    struct clerk_info * cur_clerk = (struct clerk_info *) clerk_i;
    int selected_qid;
    Node * selected_queue;
    int customer_i = 0; //user_id var
    
    double queue_entry_time;
    int service_i = 0; //service time var
    int do_nothing = 0; // for threads when idle and there are no more customers left

    printf("clerk %d started working \n",cur_clerk->clerk_id );
    


    while(1){

        // check if there are any customers in either business or economy

        // if no more customer break from loop
        if(customers_served == total_customers){
            break;
        }

        // Only check queues within mutex to ensure atomic operation...
        pthread_mutex_lock(&clerk_lock);
        {
            // If both queues are currently empty
            if (queue_length[0]+queue_length[1] == 0)
            {
               // printf("Idle clerk %d waiting...\n", cur_clerk->clerk_id);

                // Wait until a customer shows up 
                    //or signal when no more cutomers appear
                pthread_cond_wait(&new_customer, &clerk_lock);

                // Check if another clerk handled the last customer already...
                if (customers_served == total_customers)
                {
                    // We should not attempt to process
                    do_nothing = 1;
                }

            }

            // If we didn't run out of cusotmers
            if (!do_nothing)
            {
                // Start serving the next customer
              
                //business has priority so check it first
                if (queue_length[1] != 0){

                    selected_qid = 1;
                    selected_queue = b_head;

            
                } //if there are no customers in business
                else{
                    selected_qid = 0;
                    selected_queue = e_head;
                }


                //change clerk status from idle
                cur_clerk->c_status = 1;
                queue_status[selected_qid] = cur_clerk->clerk_id; // queue is now being served by a clerk 

                //get the info from the head node before dequeue
                customer_i = user_q(selected_queue);
                queue_entry_time = arrival_q(selected_queue);
                service_i = service_q(selected_queue);

                printf("Clerk %d started working on customer %d from queue %d \n", cur_clerk->clerk_id, customer_i, selected_qid);
            
                if(selected_qid == 0){
                    e_head = dequeue(e_head);
                }
                else{
                    b_head = dequeue(b_head);
                }
                queue_length[selected_qid]--;   

                // Increment the number of customers served    
                customers_served++;
            }
                   
        }
        pthread_mutex_unlock(&clerk_lock);

        // If we are just exiting...
        if (do_nothing)
        {
            // All done
            float cur_time = getCurSystemTime();
            printf("After %.2f seconds Clerk %1d is done taking care of customers\n", cur_time, cur_clerk->clerk_id);

           // printf("Clerk %d exiting from idle.\n", cur_clerk->clerk_id);
            pthread_exit(NULL);
        }

                
        double start_of_service = getCurSystemTime();
        double wait_time = start_of_service - queue_entry_time;

        // add to total wait time for each queue
        if(selected_qid == 0){
            e_wait += wait_time;
        }
        else{
            b_wait += wait_time;
        }
        overall_wait_t += wait_time;
        printf("Clerk %1d starts serving customer %2d, with start time %.2f. \n", cur_clerk->clerk_id, customer_i, start_of_service);
        printf("Customer %2d had a waiting time of %.2f seconds \n", customer_i, wait_time);

        // Put thread to sleep for service period
        usleep(service_i*100000);

        // Get actual end of service time
        double end_of_service = getCurSystemTime();
        printf("Clerk %1d finished serving customer %d, with end time %.2f \n", cur_clerk->clerk_id, customer_i, end_of_service);
        
        cur_clerk->c_status = -1;
        
        // wake up idle clerks if finished with last customer
        if (customers_served == total_customers)
        {
        
            pthread_cond_broadcast(&new_customer);
            
        }
    }
    
    float cur_time = getCurSystemTime();
    printf("After %.2f seconds Clerk %1d is done taking care of their final customer %d\n", cur_time, cur_clerk->clerk_id, customer_i);
   
    // If we are here we were the first clerk to finish the last customer
    pthread_exit(NULL);
    return NULL;
}


void sim_stats(int t_num, int b_num, int e_num){
    float avg_time = 0;
    float avg_b_time = 0;
    float avg_e_time = 0;

    printf("\n--------------------------------\n");
    printf("The total wait time for %d customers was %.2f seconds \n", t_num, overall_wait_t);
    if(t_num != 0){
        avg_time = overall_wait_t/t_num;
        
        if (e_num != 0){
            avg_e_time = e_wait/e_num;
        }
        if(b_num != 0){
            avg_b_time = b_wait/b_num;
        }
       
    }
    printf("The average wait time for all customers was %.2f\n", avg_time);

    printf("The %d Business class customers spent %.2f seconds waiting\n", b_num, b_wait);
    printf("The average waiting time for Business class was %.2f seconds\n",avg_b_time);

    printf("The %d Economy class customers spent %.2f seconds waiting\n", e_num, e_wait);
    printf("The average waiting time for Economy class was %.2f seconds\n",avg_e_time);
    printf("\n--------------------------------\n\n");


}
int main(int argc, char* argv[]){

    FILE * f_p;

    gettimeofday(&start_time, NULL); // record simulation start time
    // read the file, 
    
    if(argc != 2){
        fprintf(stderr, "Missing filename\n");
        return 1;
    }

    // printf(argv[0]);
    f_p = fopen(argv[1], "r");

    if(f_p == NULL){

        perror("Failed to open file, try again\n");
        return 1;
    }
   
        // first line is number of customers
    
    char f_buff[4];
    char c_buff[100];
    total_customers = atoi(fgets(f_buff, 4, f_p));
    printf("Total customers: %d \n", total_customers);

    int t_id;
    int t_class;
    int t_service;
    int t_arrival;
    int c_index =0;;
    int b_num = 0;
    int e_num = 0;
    

    while(fgets(c_buff, sizeof(c_buff), f_p)){
        // 5:1,7,40
        sscanf(c_buff, "%d:%d,%d,%d", &t_id, &t_class, &t_arrival, &t_service);
        
         if(t_arrival < 0 || t_service <0){
            perror("Invalid input: negative number for time \n");
            exit(0);

         }
         if(t_class == 0){
             e_num++;
         }
         else{
             b_num++;
         }

        //create master list of all customers, not in order
        customers[c_index].user_id = t_id;
        customers[c_index].a_class = t_class;
        customers[c_index].arrival_t = t_arrival;
        customers[c_index].service_t = t_service;
        c_index++;

    }
    for (int i = 0; i < 5; i++){
        clerks[i].clerk_id = i;
        clerks[i].c_status = -1;
    }

    pthread_t clerkId[NTHREADS];
    pthread_t customId[total_customers];
    //initialize mutex locks for business and economy queues
    if(pthread_mutex_init(&lock_b, NULL) != 0){
        printf("\n mutex init failed\n");
        return -1;
    }	
    if(pthread_mutex_init(&lock_e, NULL) != 0){
        printf("\n mutex init failed\n");
        return -1;
    }
    if(pthread_mutex_init(&clerk_lock, NULL) != 0){
        printf("\n mutex init failed\n");
        return -1;
    }
    if(pthread_cond_init(&new_customer, NULL)!=0){
        printf("\n convar init failed\n");
        return -1;
    }
    

    for(int i = 0; i < NClerks; i++){ // number of clerks
        if(pthread_create(&clerkId[i], NULL, clerk_entry, (void *)&clerks[i])) // clerk_info: passing the clerk information (e.g., clerk ID) to clerk thread
            printf("can't create clerk thread %d\n", i);
    }

    //create customer thread
    for(int i = 0; i < total_customers; i++){ // number of customers
        if(pthread_create(&customId[i], NULL, customer_enters, (void *)&customers[i]) !=0)//custom_info: passing the customer information (e.g., customer ID, arrival time, service time, etc.) to customer thread
            printf("can't create customer thread %d\n", i);	
    }
    // wait for all customer threads to terminate
    for(int k = 0; k < 5; k++){
        pthread_join(clerkId[k], NULL);
    }
    for (int j = 0; j< total_customers; j++){
        pthread_join(customId[j], NULL);
    }
    
    if(pthread_mutex_destroy(&lock_b)!= 0){
        printf("\n mutex destroy failed\n");
        return -1;
    }
    if(pthread_mutex_destroy(&lock_e) !=0){
        printf("\n mutex destroy failed\n");
        return -1;
    }
    if(pthread_mutex_destroy(&clerk_lock)!= 0){
        printf("\n mutex destroy failed\n");
        return -1;
    }
    if(pthread_cond_destroy(&new_customer)!= 0){
        printf("\n convar destroy failed\n");
        return -1;
    }

    fclose(f_p);

    //print out final stats for simulation
    sim_stats(total_customers, b_num, e_num);


    return 0;

}
