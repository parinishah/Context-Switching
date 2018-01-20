#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <iostream>
#include <queue>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>	
#include "header.h"
#include "threadQueueStructure.h"
/*#include "fetchInstruction.h"
#include "decodeInstruction.h"
*/
using namespace std;
static int process_left[2];
time_t current_time;

//Queue of the process
struct queue_process{
	char *filename;
	struct queue_process *q;
};
struct queue_process *process_q[2],*head[2],*ptrQ[2];

threadAttribute t[2];   //Structure array which contain the attributes of the thread 
pthread_mutex_t running_mutex = PTHREAD_MUTEX_INITIALIZER;      //Initalizing the mutex variable  
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

void signal_handling (int signal){ //Handling the Signal

	if(SIGUSR1 == signal){
		pause();
	}
	if(SIGUSR2 == signal){
		kill(signal, SIGCONT);
	}
}

void error (int en, char *msg){      //Printing the error
    printf("%d %s",en,msg);
}

static void stack_attributes(pthread_attr_t *attr, char *prefix,int id){    //this function prints the stack attributes of the thread
    int s;
    size_t stack_size, guard_size;
    void *stack_addr;
    
    s = pthread_attr_getguardsize(attr, &guard_size);   //return the guard size
    if (s != 0)
        error(s, "error: thread attribute getguardsize fail");
    
    printf("%sGuard size of thread ID %d          = %d bytes\n", prefix,pthread_self(), guard_size);

    if(id == 0){
        t[0].nguard_size = guard_size;
    }
    else{
        t[1].nguard_size = guard_size;
    }

    s = pthread_attr_getstack(attr, &stack_addr, &stack_size);  //return the stack address
    if (s != 0)
        error(s, "pthread_attr_getstack");

    printf("%sStack address of thread ID %d       = %p", prefix,pthread_self(), stack_addr);

    if(id == 0){
        t[0].nstack_addr = stack_addr;
        t[0].nstack_size = stack_size;
    }
    else{
        t[1].nstack_addr = stack_addr;
        t[1].nstack_size = stack_size;
    }

    if (stack_size > 0)
        printf(" (EOS = %p)", (char *) stack_addr + stack_size);
    printf("\n");

    printf("%sStack size of thread ID %d        = 0x%x (%d) bytes\n",
            prefix,pthread_self(), stack_size, stack_size);
}

static void thread_attributes(pthread_t thread, char *prefix,int id){   //Display the thread attributes
    
    int s;
    pthread_attr_t attr;
    pthread_key_t key;
    void *value;
    
    sched_param param;
    int priority;
    int policy;
    int ret;

    /* scheduling parameters of target thread */
    ret = pthread_getschedparam (pthread_self(), &policy, &param);
    /* schedule_priority contains the priority of the thread */
    printf("\tPolicy of thread ID %d             = %s\n",pthread_self(),    //Printing the thread attribute
            (policy == SCHED_FIFO)  ? "SCHED_FIFO" :
            (policy == SCHED_RR)    ? "SCHED_RR" :
            (policy == SCHED_OTHER) ? "SCHED_OTHER" :
            "???");

    if(id == 0){    //Priority of the thread
        t[0].npolicy = policy;
    }
    else{
        t[1].npolicy = policy;
    }

    printf("\tPriority of thread ID %d           = %d\n",pthread_self(),param.__sched_priority);
    /* key previously created */
    if(id == 0){
        t[0].npriority = param.__sched_priority;
    }
    else{
        t[1].npriority = param.__sched_priority;
    }

//    value = pthread_getspecific(key);
    printf("\tThread ID           = %d\n",pthread_self());
    if(id == 0){
        t[0].nthread_id = pthread_self();
    }
    else{
        t[1].nthread_id = pthread_self();
    }

    s = pthread_getattr_np(thread, &attr);
    if (s != 0)
        error(s, "Error: Thread get attribute fail");

    stack_attributes(&attr, prefix,id);

    s = pthread_attr_destroy(&attr);
    if (s != 0)
        error(s, "Error: Thread attribute destroy");
}


void *ins_read(void * pv){   //Thread will start its execution from here [process execution]

	pthread_t thread = (pthread_t) pv;
	pause();
        int i=0,process_status;
        FILE *f;
        thread_attributes(pthread_self(), "\t",thread);
        
        if((int)thread == 1){       //Store the attributes of the thread in the file
            f = fopen("proc/1/attr.txt","w");
            fprintf(f,"\nThread ID      = %d",t[0].nthread_id);
            fprintf(f,"\nPolicy         = %d",t[0].npolicy);
            fprintf(f,"\nPriority       = %d",t[0].npriority);
            fprintf(f,"\nStack Size     = %x",t[0].nstack_size);
            fprintf(f,"\nStack Size     = 0x%x (%d)bytes",t[0].nstack_addr,t[0].nstack_size);
            fprintf(f,"\nGuard Size     = %d",t[0].nguard_size);
            fprintf(f,"\nStack address  = %p \n", t[0].nstack_addr);
            fclose(f);
        }
	else{
            f = fopen("proc/2/attr.txt","w");
            fprintf(f,"\nThread ID      = %d",t[1].nthread_id);
            fprintf(f,"\nPolicy         = %d",t[1].npolicy);
            fprintf(f,"\nPriority       = %d",t[1].npriority);
            fprintf(f,"\nStack Size     = %x",t[1].nstack_size);
            fprintf(f,"\nStack Size     = 0x%x (%d)bytes",t[1].nstack_addr,t[1].nstack_size);
            fprintf(f,"\nGuard Size     = %d",t[1].nguard_size);
            fprintf(f,"\nStack address  = %p \n", t[1].nstack_addr);
            fclose(f);
        }

        while(1){   //fetching the instruction from the file 
	    if(process_left[thread] == 0)
		exit(1);

	    printf("\nThread in while : %d",thread);
	    printf("\nFile in while : %s",head[thread]->filename);
            process_status = fetch(head[thread]->filename,(int)(thread+1));
	    
            printf("\nProc Status %d",process_status);
	    ptrQ[thread] = head[thread];		
	    head[thread]=head[thread]->q;
	    free(ptrQ[thread]);
	    process_left[thread]--;
	    printf("\nProcess left in thread %d : %d\n\n", thread,process_left[thread]);
            sleep(1);
        }
	pthread_exit(NULL);
}

void sigintHandler(int sig_num)
{
    signal(SIGINT, sigintHandler);
    fflush(stdout);
}

int main()
{
    int s;

    process_q[0]=NULL;
    process_q[1]=NULL;
    head[0]=NULL;
    head[1]=NULL;
    ptrQ[0]=NULL;
    ptrQ[1]=NULL;

    process_left[0] = 3;
    process_left[1] = 3;
    
    process_q[0]=(struct queue_process *)malloc(sizeof(struct queue_process));    //Adding the process in the queue
    head[0] = process_q[0];
    process_q[0]->filename="file_1.txt";

    process_q[0]->q = (struct queue_process *)malloc(sizeof(struct queue_process));
    process_q[0] = process_q[0]->q;
    process_q[0]->filename="file_2.txt";

    process_q[0]->q = (struct queue_process *)malloc(sizeof(struct queue_process));
    process_q[0] = process_q[0]->q;
    process_q[0]->filename="file_3.txt";
    process_q[0]->q = head[0];

    process_q[1]=(struct queue_process *)malloc(sizeof(struct queue_process));    //Adding the process in the queue
    head[1] = process_q[1];
    process_q[1]->filename="file_4.txt";

    process_q[1]->q = (struct queue_process *)malloc(sizeof(struct queue_process));
    process_q[1] = process_q[1]->q;
    process_q[1]->filename="file_5.txt";

    process_q[1]->q = (struct queue_process *)malloc(sizeof(struct queue_process));
    process_q[1] = process_q[1]->q;
    process_q[1]->filename="file_6.txt";
    process_q[1]->q = head[1];
    
    	struct sigaction signal_action;     //creating the signal identifier and signal handler
	memset(&signal_action, 0, sizeof(signal_action));
	signal_action.sa_handler = signal_handling;

	sigaction(SIGUSR1, &signal_action, NULL);
	sigaction(SIGUSR2, &signal_action, NULL);

	pthread_t threads[2];
	threads[0] = 0;
	threads[1] = 0;

	pthread_t thread = 0;
    
      	for(thread = 0; thread < 2; thread++){  //creating the thread

		int thread_creation_success = pthread_create(&threads[thread], NULL, ins_read,(void *) thread);
		if(thread_creation_success){

			perror("Error : Problem creating thread");
			exit(1);
	    	}
	}

	sleep(1);
	for(thread = 0;thread<=2; thread++){    //Managing the execution of the thread
                                                //Scheduling the thread in round robin fashion
		//printf("Thread value %d\n\n",thread);
		if(thread == 2){
		//printf("Thread value %d\n\n",thread);
		thread = 0;
		}
		
		if(signal(SIGINT,sigintHandler)==SIG_ERR)
		{
			continue;
		}

//		printf("Before Sleep\n");
		int thread_resume_success = pthread_kill(threads[thread], SIGUSR2); //resume the thread
		if (thread_resume_success){

			perror("Error : Problem resuming thread");
			exit(1);
		}

  		sleep(1);   //thread will work for 4 second
//                printf("After sleep\n");
		
		int thread_block_success = pthread_kill(threads[thread], SIGUSR1); //blocking the thread
		if(thread_block_success){

			perror("Error : Problem blocking thread");
			exit(1);
		}

	}
}
