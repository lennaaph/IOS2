//  proj2.c - 2. projekt IOS
//  VUT FIT 2023
//  Phamová Thu Tra, xphamo00

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/wait.h>

#define TYPE 4

void checkarguments(int argc, char *argv[]){
    if (argc != 6)
    {
        fprintf(stderr,"Invalid argument input\n");
        exit(1);
    }
    //checks if arguments are digits
    for (int i = 1; i < argc; i++)
    {        
        char *ch = argv[i];
        int length = strlen(argv[i]);
        for (int j = 0; j < length; j++)
        {
            if (!isdigit(ch[j]))
    		{
	    	    fprintf(stderr, "Arguments from the input are not written right\n");
                exit(1);
		    }
        }
    }
}

typedef struct shared{
    int nz;
    int nu;
    int tz;
    int tu;
    int f;

    int service[TYPE];
    bool closed;
    int line;

    sem_t *mutex;
    sem_t *wait3;
    sem_t *wait1;
    sem_t *wait2;
    sem_t *output;

    FILE *print_output;

}Shared_t;

int init(Shared_t *shared){
    if ((shared->mutex = sem_open("/xphamo00.mutex", O_CREAT, 0666, 1)) == SEM_FAILED) return 1;
    if ((shared->output = sem_open("/xphamo00.output", O_CREAT, 0666, 1)) == SEM_FAILED) return 1;
    if ((shared->wait1 = sem_open("/xphamo00.wait1", O_CREAT, 0666, 0)) == SEM_FAILED) return 1;
    if ((shared->wait2 = sem_open("/xphamo00.wait2", O_CREAT, 0666, 0)) == SEM_FAILED) return 1;
    if ((shared->wait3 = sem_open("/xphamo00.wait3", O_CREAT, 0666, 0)) == SEM_FAILED) return 1;

    return 0;
}

void destroy(Shared_t *shared){
    sem_close(shared->mutex);
    sem_close(shared->output);
    sem_close(shared->wait1);
    sem_close(shared->wait2);
    sem_close(shared->wait3);
    sem_unlink("/xphamo00.mutex");
    sem_unlink("/xphamo00.output");
    sem_unlink("/xphamo00.wait1");
    sem_unlink("/xphamo00.wait2");
    sem_unlink("/xphamo00.wait3");
}

// worker
void u_process(int i, Shared_t *shared){
    srand(time(NULL) * getpid());

    sem_wait(shared->output);
    fprintf(shared->print_output,"%d: U %d: started\n", ++shared->line, i);
    fflush(shared->print_output);
    sem_post(shared->output);

    int loop = true;
    while(loop){
        sem_wait(shared->mutex);

        // checks if queue is empty or not
        bool service_available = false;
        for (int i = 0; i < TYPE; i++) {
            if (shared->service[i] != 0) {
                service_available = true;
                break;
            }
        }
        
        // randomly choosing a non-empty queue
        int tmp;
        if (service_available) {
            do {
                tmp = rand() % 3 + 1;
            } while (shared->service[tmp] == 0);
            --shared->service[tmp];
        }

        // queue is non-empty
        if (service_available) {
            sem_post(shared->mutex);
            // 3 queues
            if (tmp == 1){
                sem_post(shared->wait1);
            } else if (tmp == 2) {
                sem_post(shared->wait2);
            } else if (tmp == 3) {    
                sem_post(shared->wait3);
            }
            
            sem_wait(shared->output);
            fprintf(shared->print_output,"%d: U %d: serving a service of type %d\n", ++shared->line, i, tmp);
            fflush(shared->print_output);
            sem_post(shared->output);

            usleep(1000 * (rand() % 10 + 1));

            sem_wait(shared->output);
            fprintf(shared->print_output,"%d: U %d: service finished\n", ++shared->line, i);
            fflush(shared->print_output);
            sem_post(shared->output);
        }
        // its open and queue is empty
        else if(!shared->closed && !service_available)
        {
            sem_wait(shared->output);
            fprintf(shared->print_output,"%d: U %d: taking break\n", ++shared->line, i);
            fflush(shared->print_output);
            sem_post(shared->output);

            sem_post(shared->mutex);

            usleep(1000 * (rand() % shared->tu + 1));

            sem_wait(shared->output);
            fprintf(shared->print_output,"%d: U %d: break finished\n", ++shared->line, i);
            fflush(shared->print_output);
            sem_post(shared->output);
        }
        // its closed and queue is empty
        else if(shared->closed && !service_available)
        {
            sem_wait(shared->output);
            fprintf(shared->print_output,"%d: U %d: going home\n", ++shared->line, i);
            fflush(shared->print_output);
            sem_post(shared->output);
            
            // end of process
            loop = false;
            sem_post(shared->mutex);
            return;
        }
    }
}

// customer
void z_process(int i, Shared_t *shared){
    srand(time(NULL) * getpid());
    sem_wait(shared->mutex);

    sem_wait(shared->output);
    fprintf(shared->print_output,"%d: Z %d: started\n", ++shared->line, i);
    fflush(shared->print_output);
    sem_post(shared->output);

    usleep(1000 * (rand() % (shared->tz + 1)));

    if (shared->closed)
    {
        sem_wait(shared->output);
        fprintf(shared->print_output,"%d: Z %d: going home\n", ++shared->line, i);
        fflush(shared->print_output);
        sem_post(shared->output);

        // end of process
        sem_post(shared->mutex);
        return;
    } else {
        int tmp = rand() % 3 + 1;
        ++shared->service[tmp];

        sem_wait(shared->output);
        fprintf(shared->print_output,"%d: Z %d: entering office for a service %d\n", ++shared->line, i, tmp);
        fflush(shared->print_output);
        sem_post(shared->output);

        // sending and waiting for signal
        sem_post(shared->mutex);
        // 3 semaphores for 3 queues
        if (tmp == 1){
            sem_wait(shared->wait1);
        } else if (tmp == 2) {
            sem_wait(shared->wait2);
        } else if (tmp == 3) {    
            sem_wait(shared->wait3);
        }
        
        sem_wait(shared->output);
        fprintf(shared->print_output,"%d: Z %d: called by office worker\n", ++shared->line, i);
        fflush(shared->print_output);
        sem_post(shared->output);

        usleep(1000 * (rand() % 10 + 1));

        sem_wait(shared->output);
        fprintf(shared->print_output,"%d: Z %d: going home\n", ++shared->line, i);
        fflush(shared->print_output);
        sem_post(shared->output);

        // end of process
        sem_post(shared->mutex);
        return;
    }
}

int main(int argc, char *argv[]){
    srand(time(NULL));
    checkarguments(argc,argv);
    // checks interval of each arguments
    int NZ = atoi(argv[1]);
    int NU = atoi(argv[2]);
    int TZ = atoi(argv[3]);
    int TU = atoi(argv[4]);
    int F = atoi(argv[5]);
    
    if ((NZ < 0) || (NU <= 0) || (TZ < 0) || (TZ > 10000) || (TU < 0) || (TU > 100) || (F < 0) || (F > 10000) )
    {
        fprintf(stderr,"Numbers do not belong to the interval\n");
        exit(1);
    }

    // initialization of shared memory
    Shared_t *shared;
    shared = mmap (NULL, sizeof(*shared), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    init(shared);

    // open file for writing
    if ((shared->print_output = fopen("proj2.out", "w")) == NULL){
        fprintf(stderr,"File for printing result out failed to open\n");
        exit(1);
    }

    // struct init
    shared->nz = NZ;
    shared->nu = NU;
    shared->tz = TZ;
    shared->tu = TU;
    shared->f = F;
    shared->closed = false;
    memset(shared->service, 0, sizeof(shared->service));

    // worker process
    for (int i = 1; i <= NU; i++){
        pid_t id = fork();
        if (id == 0){
            u_process(i, shared);
            exit(0);
        }
        else if (id < 0){
			fprintf(stderr, "Fork for u_process failed\n");
            fclose(shared->print_output);
            destroy(shared);
            munmap(shared, sizeof(*shared));
			exit(1);
		}
    }

    // customer process
    for (int i = 1; i <= NZ; i++){
        pid_t id = fork();
        if (id == 0){
            z_process(i, shared);
            exit(0);
        }
        else if (id < 0){
			fprintf(stderr, "Fork for z_process failed\n");
            fclose(shared->print_output);
            destroy(shared);
            munmap(shared, sizeof(*shared));
			exit(1);
		}
    }

    // main process
    usleep(1000 * (shared->f/2 + rand() % (shared->f/2 + 1)));
    sem_wait(shared->mutex);
        sem_wait(shared->output);
        shared->closed = true;
        fprintf(shared->print_output,"%d: closing\n", ++shared->line);
        fflush(shared->print_output);
        sem_post(shared->output);
    sem_post(shared->mutex);

    while(wait(NULL) > 0);

    // end destroy
    destroy(shared);
    fclose(shared->print_output);
    munmap(shared, sizeof(*shared));
    return 0;
}
