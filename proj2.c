// project IOS VUT FIT - Pošta (synchronizace procesů)
// author: Gleb Litvinchuk
// login: xlitvi02
// date: 23.04.2023
/* description: In the system we have 3 types of processes: (0) main process, (1) postal clerk and (2) customer.
                Each customer goes to the post office to handle one of three types of requests: letter services, parcels, money services.
                Each request is uniquely identified by a number (letters:1, parcels:2, money services:3).
                After When the request arrives, it is placed in the queue according to the activity to be handled.
                Each clerk serves all the queues (he selects one of the queues at random each time).
                If there are no customers currently waiting, the clerk takes a short break.
                After the post office closes, the clerks finish serving all customers in the queue and after all queues are cleared, they go home.
                Any customers who arrive after the post office closes leave home (tomorrow is also a day).*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/wait.h>

FILE *proj2out = NULL; // file for output

// semaphores
sem_t *queue1 = NULL; // semaphore for the first queue
sem_t *queue2 = NULL; // semaphore for the second queue
sem_t *queue3 = NULL; // semaphore for the third queue
sem_t *queue_j1 = NULL; // semaphore to enter first queue
sem_t *queue_j2 = NULL; // semaphore to enter second queue
sem_t *queue_j3 = NULL; // semaphore to enter third queue
sem_t *mutex = NULL; // semaphore for mutual exclusion

// global variables
int jednicka = 1;
int *cond = &jednicka; // condition open or close post office
int *count = NULL;
int *queue_1 = 0;
int *queue_2 = 0;
int *queue_3 = 0;
int *served = NULL; // number of served customers
int NZ, NU, TZ, TU, F; // input arguments

// function for error
void err(int err_code){
    switch (err_code){
        case 1: // wrong number of arguments
            fprintf(stderr, "Error: Wrong number of arguments\n");
            exit(1);
        case 2: // erroneous argument
            fprintf(stderr, "Error: Erroneous argument\n");
            exit(1);
        case 3: // error in process creation
            fprintf(stderr, "Error: Error in process creation\n");
            exit(1);
        case 4: // error in semaphores creation
            fprintf(stderr, "Error: Semaphores creation error\n");
            exit(1);
        case 5: // error in semaphores deletion
            fprintf(stderr, "Error: Error when deleting a semaphores\n");
            exit(1);
        case 6: // error in shared memory allocation
            fprintf(stderr, "Error: Unknown error\n");
            exit(1);
        case 7: // error in shared memory allocation
            fprintf(stderr, "Error: Shared memory allocation error\n");
            exit(1);
        case 8: // error in file
            fprintf(stderr, "Error: File error\n");
            exit(1);
        case 9: // error in id number
            fprintf(stderr, "Error: Id number\n");
            exit(1);
        case 0: // normal exit
            exit(0);
        default:
            break;
    }
}

void arg_err() { // function for checking input arguments
    if (NZ <= 0) {
        err(2);
    }
    if (NU <= 0) {
        err(2);
    }
    if (TZ < 0 || TZ > 10000) {
        err(2);
    }
    if (TU < 0 || TU > 100) {
        err(2);
    }
    if (F < 0 || F > 10000) {
        err(2);
    }
    return;
}

// function for initialization
void init(){
    if ((proj2out = fopen("proj2.out", "w")) == NULL){
        err(8);
    }

    // shared memory initialization
    if((cond = mmap(NULL, sizeof(cond), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0)) == MAP_FAILED ||
       (count = mmap(NULL, sizeof(count), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0)) == MAP_FAILED ||
       (queue_1 = mmap(NULL, sizeof(queue_1), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0)) == MAP_FAILED ||
       (queue_2 = mmap(NULL, sizeof(queue_2), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0)) == MAP_FAILED ||
       (queue_3 = mmap(NULL, sizeof(queue_3), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0)) == MAP_FAILED ||
       (served = mmap(NULL, sizeof(served), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0)) == MAP_FAILED){
        err(7); // error in shared memory allocation
    }

    //inicialization of semaphores
    if((queue1 = mmap(NULL, sizeof(queue1), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0)) == MAP_FAILED ||
       (queue2 = mmap(NULL, sizeof(queue2), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0)) == MAP_FAILED ||
       (queue3 = mmap(NULL, sizeof(queue3), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0)) == MAP_FAILED ||
       (queue_j1 = mmap(NULL, sizeof(queue_j1), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0)) == MAP_FAILED ||
       (queue_j2 = mmap(NULL, sizeof(queue_j2), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0)) == MAP_FAILED ||
       (queue_j3 = mmap(NULL, sizeof(queue_j3), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0)) == MAP_FAILED ||
       (mutex = mmap(NULL, sizeof(mutex), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0)) == MAP_FAILED){
        err(7); // error in shared memory allocation
    }

    if((sem_init(queue1, 1, 0)) == -1 ||
       (sem_init(queue2, 1, 0)) == -1 ||
       (sem_init(queue3, 1, 0)) == -1 ||
       (sem_init(queue_j1, 1, 1)) == -1 ||
       (sem_init(queue_j2, 1, 1)) == -1 ||
       (sem_init(queue_j3, 1, 1)) == -1 ||
       (sem_init(mutex, 1, 1)) == -1){ // inicialization of semaphores
        err(4); // error in semaphores creation
    }

    setbuf(proj2out, NULL);
}

// function for cleaning
void clean(){
    if(proj2out != NULL){ // closing file
        fclose(proj2out);
    }

    // releasing shared memory
    munmap(&(*cond), sizeof(cond)); // release of shared memory
    munmap(count, sizeof(count));
    munmap(queue_1, sizeof(queue_1));
    munmap(queue_2, sizeof(queue_2));
    munmap(queue_3, sizeof(queue_3));
    munmap(served, sizeof(served));


    // release of semaphores
    if(sem_destroy(queue1) == -1 ||
       sem_destroy(queue2) == -1 ||
       sem_destroy(queue3) == -1 ||
       sem_destroy(queue_j1) == -1 ||
       sem_destroy(queue_j2) == -1 ||
       sem_destroy(queue_j3) == -1 ||
       sem_destroy(mutex) == -1){
        err(5);
    }
}

/* Customer() function to create a customer process
 * Parameters: id - customer serial number
 */
void customer(int id){

    //Po spuštění vypíše: A: Z idZ: started
    sem_wait(mutex); // waiting for mutex
    fprintf(proj2out, "%d: Z %d: started\n", ++(*count), id); // printing to file
    sem_post(mutex); // releasing mutex

    //Následně čeká pomocí volání usleep náhodný čas v intervalu <0,TZ>
    usleep((rand() % (TZ + 1)) * 1000); // waiting for random time

    //Pokud je pošta uzavřena. Vypíše: A: Z idZ: going home. Proces končí
    if(*cond == 0){ // if post office is closed
        sem_wait(mutex);
        fprintf(proj2out, "%d: Z %d: going home\n", ++(*count), id);
        sem_post(mutex);
        exit(0);
    }

    /* Pokud je pošta otevřená, náhodně vybere činnost X---číslo z intervalu <1,3>
     * Vypíše: A: Z idZ: entering office for a service X
     * Zařadí se do fronty X a čeká na zavolání úředníkem.
     * Vypíše: Z idZ: called by office worker
     * Následně čeká pomocí volání usleep náhodný čas v intervalu <0,10> (synchronizace s
     * úředníkem na dokončení žádosti není vyžadována).
     * Vypíše: A: Z idZ: going home
     * Proces končí*/
    srand(time(NULL) * getpid());
    int x = (rand() % 3) + 1;
    sem_wait(mutex);
    fprintf(proj2out, "%d: Z %d: entering office for a service %d\n", ++(*count), id, x);
    sem_post(mutex);
    switch(x){
        case 1:
            *queue_1 += 1; // adding customer to queue
            sem_wait(queue1); // waiting for clerk
            sem_wait(mutex); // waiting for mutex
            fprintf(proj2out, "%d: Z %d: called by office worker\n", ++(*count), id); // printing to file
            sem_post(mutex); // releasing mutex
            usleep((rand() % 11) * 1000); // waiting for random time
            sem_wait(mutex); // waiting for mutex
            fprintf(proj2out, "%d: Z %d: going home\n", ++(*count), id); // printing to file
            sem_post(mutex); // releasing mutex
            exit(0);
        case 2:
            *queue_2 += 1; // all the same as in case 1
            sem_wait(queue2);
            sem_wait(mutex);
            fprintf(proj2out, "%d: Z %d: called by office worker\n", ++(*count), id);
            sem_post(mutex);
            usleep((rand() % 11) * 1000);
            sem_wait(mutex);
            fprintf(proj2out, "%d: Z %d: going home\n", ++(*count), id);
            sem_post(mutex);
            exit(0);
        case 3:
            *queue_3 += 1; // all the same as in case 1
            sem_wait(queue3);
            sem_wait(mutex);
            fprintf(proj2out, "%d: Z %d: called by office worker\n", ++(*count), id);
            sem_post(mutex);
            usleep((rand() % 11) * 1000);
            sem_wait(mutex);
            fprintf(proj2out, "%d: Z %d: going home\n", ++(*count), id);
            sem_post(mutex);
            exit(0);
    }
    if(proj2out != NULL){ // closing file
        fclose(proj2out);
    }
}

/* Function office_worker() to create a worker process of a certain type
 * Parameters: id - serial number of the worker
 */
void office_worker(int id) {

    //Po spuštění vypíše: A: U idU: started
    sem_wait(mutex);
    fprintf(proj2out, "%d: U %d: started\n", ++(*count), id);
    sem_post(mutex);
    //[začátek cyklu]
    while (true) { // infinite loop
        srand(time(NULL) * getpid()); // random seed
        int x = (rand() % 3) + 1; // random number from 1 to 3
        switch (x) {
            case 1:
                if (*queue_1 > 0) { // if there is a customer in queue
                    *queue_1 -= 1; // decrementing queue
                    sem_post(queue1); // signaling customer
                    sem_wait(mutex); // waiting for mutex
                    fprintf(proj2out, "%d: U %d: serving a service of type %d\n", ++(*count), id, x); // printing to file
                    sem_post(mutex); // releasing mutex
                    usleep((rand() % (TU + 1)) * 1000); // waiting for random time
                    sem_wait(mutex); // waiting for mutex
                    fprintf(proj2out, "%d: U %d: service finished\n", ++(*count), id); // printing to file
                    sem_post(mutex); // releasing mutex
                }
                break;
            case 2:
                if (*queue_2 > 0) {
                    *queue_2 -= 1; // all the same as in case 1
                    sem_post(queue2);
                    sem_wait(mutex);
                    fprintf(proj2out, "%d: U %d: serving a service of type %d\n", ++(*count), id, x);
                    sem_post(mutex);
                    usleep((rand() % (TU + 1)) * 1000);
                    sem_wait(mutex);
                    fprintf(proj2out, "%d: U %d: service finished\n", ++(*count), id);
                    sem_post(mutex);
                }
                break;
            case 3:
                if (*queue_3 > 0) {
                    *queue_3 -= 1; // all the same as in case 1
                    sem_post(queue3);
                    sem_wait(mutex);
                    fprintf(proj2out, "%d: U %d: serving a service of type %d\n", ++(*count), id, x);
                    sem_post(mutex);
                    usleep((rand() % (TU + 1)) * 1000);
                    sem_wait(mutex);
                    fprintf(proj2out, "%d: U %d: service finished\n", ++(*count), id);
                    sem_post(mutex);
                }
                break;
        }
        /*Pokud v žádné frontě nečeká zákazník a pošta je otevřená vypíše
         * Vypíše: A: U idU: taking break
         * Následně čeká pomocí volání usleep náhodný čas v intervalu <0,TU>
         * Vypíše: A: U idU: break finished
         * Pokračuje na [začátek cyklu]*/
        if ((*queue_1 == 0) & (*queue_2 == 0) & (*queue_3 == 0) & ((*cond) == 1)) { // if there is no customer in queue and post office is open
            sem_wait(mutex);
            fprintf(proj2out, "%d: U %d: taking break\n", ++(*count), id);
            sem_post(mutex);
            usleep((rand() % (TU + 1)) * 1000);
            sem_wait(mutex);
            fprintf(proj2out, "%d: U %d: break finished\n", ++(*count), id);
            sem_post(mutex);
        }
        /*Pokud v žádné frontě nečeká zákazník a pošta je zavřená vypíše
         * Vypíše: A: U idU: going home
         * Ukončí se*/
        if ((*queue_1 <= 0) & (*queue_2 <= 0) & (*queue_3 <= 0) & ((*cond) == 0)) { // if there is no customer in queue and post office is closed
            sem_wait(mutex);
            fprintf(proj2out, "%d: U %d: going home\n", ++(*count), id);
            sem_post(mutex);
            exit(0);
        }
    }
}

// Main() function to process arguments and create processes
int main(int argc, char **argv){
    // Checking and saving arguments
    if (argc != 6) {
        err(1);
    }

    NZ = atoi(argv[1]); // Number of customers
    NU = atoi(argv[2]); // Number of office workers
    TZ = atoi(argv[3]); // Max time between customer arrivals
    TU = atoi(argv[4]); // Max time of service
    F = atoi(argv[5]); // Max number of customers in queue
    arg_err(NZ, NU, TZ, TU, F); // Checking arguments

    // Initializing shared memory and semaphores
    init();

    *cond = 1; // Post office is open

    // Creating customer processes
    for (int i = 1; i <= NZ; i++) {
        pid_t pid = fork(); // Creating a child process
        if (pid == 0) {
            customer(i);
            exit(0);
        } else if (pid < 0) {
            clean();
            err(9);
        }
    }
    // Creating office worker processes
    for (int i = 1; i <= NU; i++) {
        pid_t pid = fork(); // Creating a child process
        if (pid == 0) {
            office_worker(i);
            exit(0);
        } else if (pid < 0) {
            clean();
            err(9);
        }
    }

    /*Hlavní proces vytváří ihned po spuštění NZ procesů zákazníků a NU procesů úředníků.
     * Čeká pomocí volání usleep náhodný čas v intervalu <F/2,F>
     * Vypíše: A: closing
     * Poté čeká na ukončení všech procesů, které aplikace vytváří.
     * Jakmile jsou tyto procesy ukončeny, ukončí se i hlavní proces s kódem (exit code) 0.*/
    usleep((rand() % (F/2 + 1) + F/2) * 1000);
    *cond = 0;
    sem_wait(mutex);
    fprintf(proj2out, "%d: closing\n", ++(*count)); // Post office is closed
    sem_post(mutex);

    while(wait(NULL) > 0); // wait for all processes to finish
    clean(); // clean shared memory and semaphores
    err(0);

    return 0;
}