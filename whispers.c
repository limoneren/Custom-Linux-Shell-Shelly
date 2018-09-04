#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <mqueue.h>
#include <errno.h>
#include <time.h>
#include <string.h>

#define MSG_SIZE 100
#define QUEUE_NAME  "/q4_queue"
#define MAX_SIZE    1024
#define MSG_STOP    "exit"



int i;

void main(int argc, char *argv[]){

  pid_t pid;
  int n = 0;
  char message[MAX_SIZE];
  char buffer[MAX_SIZE];
  int randNum;
  
  mqd_t mq;
  struct mq_attr attr;
  int must_stop = 0;

  
  strcpy(message,argv[1]);
  for(i = 0; i < sizeof(message); i++){
    if(message[i] == NULL){
      break;
    }
    n++;
  }

  
  /* setting up queue attributes */
  attr.mq_flags = 0;
  attr.mq_maxmsg = 10;
  attr.mq_msgsize = MAX_SIZE;
  attr.mq_curmsgs = 0;
  
  /* create the message queue */
  mq = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, 0644, &attr);

  /* send the message */
  mq_send(mq, message, MAX_SIZE, 0);

  mq_close(mq);

  
  for(i = 0; i < n; i++){
    pid = fork();
    if(pid == 0){

      /* open the mail queue */
      mq = mq_open(QUEUE_NAME, O_RDWR);

      /* receive the message */
      memset(buffer, 0, MAX_SIZE);
      mq_receive(mq, buffer, MAX_SIZE, NULL);

      /* seeding the rand with the current time */
      srand(time(NULL) + i * 1000);
      randNum = rand() % 2;
      /* printf("rand num: %d\n",randNum); */
      
      if(randNum){ /* true with a probability of 0.5 */
	if(buffer[i] == 'Z'){
	  buffer[i] = 'A';
	} else if(buffer[i] == 'z'){
	  buffer[i] = 'a';
	} else {
	  buffer[i] = (char)((int)buffer[i] + 1);
	}
      }

      
      /* send the message */
      mq_send(mq, buffer, MAX_SIZE, 0);
         
      mq_close(mq);
      
      exit(0);
    } else if(pid > 0) {
      wait(NULL);
    } else {
      printf("Fork failed!");
    }
    
  }

  /* open m queue and read */
  mq = mq_open(QUEUE_NAME, O_RDWR);
  mq_receive(mq, buffer, MAX_SIZE, NULL);


  /* final modifies string */
  printf("%s\n", buffer);
  

}
