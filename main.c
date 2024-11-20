#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>

// Structure for coffee order
typedef struct {
    int order_number;
    char drink_name[50];
    int prep_time;  // preparation time in seconds
} Order;

// Shared order queue
#define MAX_ORDERS 5  // Maximum number of orders that can be in queue
Order order_queue[MAX_ORDERS];
int in = 0, out = 0;
sem_t empty, full, mutex;

// Customer (Producer) function
void* customer(void* arg) {
    // Menu of drinks with preparation times
    char* drinks[] = {"Espresso", "Latte", "Cappuccino", "Americano"};
    int prep_times[] = {2, 4, 3, 2};  // preparation time in seconds
    
    int customer_num = *((int*)arg);
    
    // Each customer places 2 orders
    for(int i = 0; i < 2; i++) {
        sem_wait(&empty);    // Wait if queue is full
        sem_wait(&mutex);    // Get exclusive access to queue
        
        // Create new order
        int drink_choice = rand() % 4;  // Randomly choose a drink
        Order new_order = {
            .order_number = (customer_num * 10) + i,
            .prep_time = prep_times[drink_choice]
        };
        strcpy(new_order.drink_name, drinks[drink_choice]);
        
        // Add order to queue
        order_queue[in] = new_order;
        printf("Customer %d ordered: %s (Order #%d)\n", 
               customer_num, new_order.drink_name, new_order.order_number);
        
        in = (in + 1) % MAX_ORDERS;
        
        sem_post(&mutex);    // Release queue access
        sem_post(&full);     // Signal new order available
        
        sleep(rand() % 3);   // Wait 0-2 seconds before next order
    }
    return NULL;
}

// Barista (Consumer) function
void* barista(void* arg) {
    int barista_num = *((int*)arg);
    
    while(1) {
        sem_wait(&full);     // Wait if no orders
        sem_wait(&mutex);    // Get exclusive access to queue
        
        // Get order from queue
        Order current_order = order_queue[out];
        printf("Barista %d preparing: %s (Order #%d)\n", 
               barista_num, current_order.drink_name, current_order.order_number);
        
        out = (out + 1) % MAX_ORDERS;
        
        sem_post(&mutex);    // Release queue access
        sem_post(&empty);    // Signal space available in queue
        
        // Simulate drink preparation
        sleep(current_order.prep_time);
        printf("Barista %d completed Order #%d: %s\n", 
               barista_num, current_order.order_number, current_order.drink_name);
        
        // Exit condition: if processed 6 orders (2 orders × 3 customers)
        static int orders_completed = 0;
        orders_completed++;
        if(orders_completed >= 6) break;
    }
    return NULL;
}

int main() {
    // Initialize random seed
    srand(time(NULL));
    
    // Initialize semaphores
    sem_init(&empty, 0, MAX_ORDERS);  // Queue starts empty
    sem_init(&full, 0, 0);            // Queue starts with no orders
    sem_init(&mutex, 0, 1);           // Binary semaphore for queue access
    
    printf("=== Coffee Shop Order System ===\n");
    printf("Queue Size: %d orders\n", MAX_ORDERS);
    printf("Drinks Available: Espresso (2s), Latte (4s), Cappuccino (3s), Americano (2s)\n\n");
    
    // Create threads for customers and baristas
    pthread_t customer_threads[3], barista_threads[2];
    int customer_nums[3] = {1, 2, 3};
    int barista_nums[2] = {1, 2};
    
    // Create customer threads
    for(int i = 0; i < 3; i++) {
        pthread_create(&customer_threads[i], NULL, customer, &customer_nums[i]);
    }
    
    // Create barista threads
    for(int i = 0; i < 2; i++) {
        pthread_create(&barista_threads[i], NULL, barista, &barista_nums[i]);
    }
    
    // Wait for all threads to complete
    for(int i = 0; i < 3; i++) {
        pthread_join(customer_threads[i], NULL);
    }
    for(int i = 0; i < 2; i++) {
        pthread_join(barista_threads[i], NULL);
    }
    
    // Cleanup
    sem_destroy(&empty);
    sem_destroy(&full);
    sem_destroy(&mutex);
    
    printf("\n=== Coffee Shop Closed ===\n");
    return 0;
}
