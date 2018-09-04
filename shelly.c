/*
0;95;0c * shelly interface program

KUSIS ID: 0054129 PARTNER NAME: Eren Limon
KUSIS ID: 0050464 PARTNER NAME: Mehmet Samed Bicer

*/

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>

#define MAX_LINE       80 /* 80 chars per line, per command, should be enough. */

int parseCommand(char inputBuffer[], char *args[],int *background,int *is_redirected, char *file_name[]);
char *removeDQMFromString(char *str);
void traverseDFS(const char *name, int depth, char *str);

int main(void)
{
  char inputBuffer[MAX_LINE];         /* buffer to hold the command entered */
  int background;                     /* equals 1 if a command is followed by '&' */
  char *args[MAX_LINE/2 + 1];        /* command line (of 80) has max of 40 arguments */
  pid_t child;            /* process id of the child process */
  int status;           /* result from execv system call*/
  int shouldrun = 1;

  char *file_name[128];
  int args_length = 0;
  int is_redirected; /* equals 1 if i/o redirection is enabled */
  int i, upper;
  int output_id;
  FILE *script_file;
  int script_mode_on = 0;
  int inputBufferLength = 0;
  int scriptLine = 0;
  char scriptFileName[255];

  //for bookmark and crontab
  FILE *fp;
  FILE *fptr;
  FILE *cronptr;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  char keys[100][255];  // keys
  char values[100][255];  // values
  int j, argc;
  char key[255];
  char value[255];
  char* wf;
  char name[255];
  char eval[255] = "eval ";
  char args_rest[255] = "";
  char buffer[255] = "";
  char username[255] = "";
  char path[255] = "/home/";
  char cron[255] = "echo \"0 9 * * * /usr/bin/curl wttr.in/Istanbul.png >> /home/";
  int bookmark_counter = 0;
  int key_found = 0;
 
  cuserid(&username[0]);
  //printf("%s\n",username );
  strcat(path, username);
  strcat(path, "/.mybookmarks");
  //printf("path:%s\n",path );
  
  fp = fopen(path, "a+");
  if (fp == NULL)
    exit(EXIT_FAILURE); 
  
  i=0;
  while ((read = getline(&line, &len, fp)) != -1) {

      strcpy(key, strtok(line, " "));
      //printf("key: %s\n", key );
      strcpy(keys[i], key);
      strcpy(value, strtok(NULL, ""));
      
      wf = removeDQMFromString(&value[0]);
      strcpy(values[i], wf);
      //printf("keys %d:%s\n", i, keys[i]);
      //printf("value %d:%s\n", i, wf);
      bookmark_counter++;
      i++;
  } // end of the while 


  while (shouldrun){            /* Program terminates normally inside setup */
    background = 0;
    is_redirected = 0;
    inputBufferLength = 0;
    args_length = 0;
    /*for(i = 0; i < sizeof(inputBuffer); i++){
      inputBuffer[i] = NULL; default value for char array, clearing inputBuffer 
    }*/
    
    
    shouldrun = parseCommand(inputBuffer,args,&background,&is_redirected, file_name);       /* get next command */

    /*printf("zero %s",file_name[0]);
      printf("first %s",file_name[1]);*/
    /*
    if(strcmp(args[1], "script") == 0){
      script_mode_on = 1;
      printf("on");
    }*/

    if(strncmp(inputBuffer,"script",6) == 0){

      script_mode_on = 1;
      strcpy(scriptFileName, args[1]);
      //printf("%s\n",scriptFileName );
      script_file = fopen(scriptFileName,"w+");
      if(script_file == NULL){

      	printf("Error occured while creating the script file.");
      	exit(1);
      }
      
      fclose(script_file);
      continue;

    } // end of the script

    if(strcmp(args[0],"wforecast") == 0){
      
      cronptr = fopen("cronJobs", "a+");
      if(cronptr == NULL){
          printf("Error while opening file\n!");
          exit(1);
      }

      argc=0;
      while(args[argc] != NULL){
          argc++;
      }      

      if(argc != 2) {
        printf("Please use command as following: wforecast \"filename\"\n");
      }else{
       
        system("crontab -l >> cronJobs");
        strcat(cron, username);
        strcat(cron, "/Desktop/");
        strcat(cron, args[1]);
        strcat(cron, "\"");
        strcat(cron, " >> cronJobs");
        //printf("cron-string: %s\n",cron );
        system(cron);
        system("crontab cronJobs");
        system("rm cronJobs");

      }

      fclose(cronptr);

    } // end of the wforecast



    if(strcmp(args[0],"bookmark") == 0){

      argc=0;
      while(args[argc] != NULL){
          argc++;
      }

      if(argc < 3) {
        printf("Please provide valid bookmark name or command \n");
      } 

      if(argc >= 3) {
        
        // open file
        fptr = fopen(path, "a+");
        if(fptr == NULL){
          printf("Error!");
          exit(1);
        }

        if(strcmp(args[1],"-r") == 0){
          //printf("remove mode\n");
          fclose(fptr);

            for (i = 0; i < 100; i++){
              if (strcmp(args[2], keys[i]) == 0){
                //printf("value found :%s\n", values[i]);
                strcpy(keys[i], "");    //deleting key
                strcpy(values[i], "");  //deleting value
              } 
            }

            fptr = fopen(path, "w+");

            for (i = 0; i < 100; i++){
              if(strcmp(keys[i], "") == 0){
                //printf("blank line found: %d\n",i );
              }else{
                fprintf(fptr,"%s ", keys[i]);
                fprintf(fptr,"%s", values[i]);
              }
            }

            fclose(fptr);          

        }else{   

          for (i = 0; i < 100; i++){
            //printf("keys %d:  %s\n",i, keys[i] );
            if (strcmp( args[1], keys[i] ) == 0){
              key_found = 1;
              break;
            }else{
              key_found = 0;
            } 
          }

          if(key_found){
            printf("There is already a bookmark entry with specified key \"%s\".\n", args[1]);
            printf("Please specify another key for your bookmark.\n");
          }else{

            i = 1;
            while(i < argc){

              if(i == argc -1 ){
                fprintf(fptr,"%s\n", args[i]);
                strcpy(buffer, args[i]);
              }else{
                fprintf(fptr,"%s ", args[i]);
                
                if(i == 1){
                  strcpy(buffer, args[i]);
                  //printf("args 1 %s\n",args[i] );
                  //strcat(buffer, " ");
                  strcpy(keys[bookmark_counter], buffer);
                  //printf("keys.%s\n", keys[bookmark_counter]);
                  strcpy(buffer, "");
                }else{
                  strcpy(buffer, args[i]);
                  strcat(buffer, " ");
                }
                  
              }

              strcat(args_rest, buffer);
              //printf("each time %d :%s\n", i, args_rest ); //true rest args
              i++;
            }
            
            //printf("args_rest :%s\n", args_rest ); 
            strcpy(values[bookmark_counter], args_rest);
            //printf("values.%s\n", values[bookmark_counter]);
            bookmark_counter++;
            strcpy(args_rest, "");
            fclose(fptr);

          }

        }
            /*  test purposes
            for (i = 0; i < 100; i++){
              printf("keys - values %d:  %s - %s\n",i, keys[i], values[i] );
            }
            */ 

      } // end of if(argc >= 3)

    } // end of the bookmark

    if (strncmp(inputBuffer, "exit", 4) == 0){
      if(script_mode_on == 1){
        script_mode_on = 0;
      } else {
        shouldrun = 0;     /* Exiting from myshell*/
      } 
    }


    /*for(i = 0; i < sizeof(inputBuffer); i++){
      if(inputBuffer[i]==NULL){
          break;
       }
       inputBufferLength++;
    }

    printf("\n%d\n",inputBufferLength);

    for(i = 0; i < inputBufferLength; i++){
      printf("\n%c", inputBuffer[i]);
      }
    */

    /*for(i = 0; i < sizeof(args); i++){
       if(args[i]==NULL){
	 break;
       }
       args_length++;
    }
    printf("\n%d\n",args_length);
    for(i = 0; i < args_length; i++){
      printf("\n%s\n\n",args[i]);
    } */

    if(script_mode_on == 1){

      script_file = fopen(scriptFileName,"a");
      for(i = 0; i < sizeof(args); i++){
        if(args[i]==NULL){
	       break;
        }
       args_length++;
      }
      
      for(i = 0; i < args_length; i++){
      	fprintf(script_file,args[i]);
      	fprintf(script_file," ");
      }

      fprintf(script_file,"\n");
      fclose(script_file);
      /* execvp(args[0],args);*/
      /*continue;*/
    }

    if (shouldrun) {
      /*
	After reading user input, the steps are 
	(1) Fork a child process using fork()
	(2) the child process will invoke execv()
	(3) if command included &, parent will invoke wait()
      */

      child = fork();

      if(child < 0){ /* error occured */
	
      	fprintf(stderr, "Fork failed");
      	return 1;
	
      }else if(child == 0) { /* child process */

	/*printf("\n%s\n",args[0]);
	printf("\n%s\n",args[2]);
	*/

	
	/*for(i = 0; i < sizeof(args); i++){
	  if(args[i]==NULL){
	    break;
	  }
	  args_length++;
	  }*/

	/*printf("\n%d\n",args_length);*/

      if(is_redirected == 1) {

        if(strcmp(file_name[0],">>") == 0){
          output_id = open(file_name[1],O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
        }else{
          output_id = open(file_name[1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);  
        }
         
        dup2(output_id,1);

        execvp(args[0],args);

        close(output_id);

      }else{ /* all commands except redirection */
            
        if(script_mode_on == 1){
  	     
          output_id = open(scriptFileName, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
          dup2(output_id,1);
          execvp(args[0],args);
          close(output_id);  

  	    }else if (execvp(args[0],args) == -1){

          for (i = 0; i < 100; i++){
            if (strcmp( args[0], keys[i] ) == 0){

              //printf("child except redirection\n");
              //printf("value found in key array:%s\n", values[i]);
              strcat(eval, values[i]);
              //printf("eval command :%s\n", eval);
              system(eval);
              strcpy(eval, "eval ");
            } 
          }
        }else {
      	     execvp(args[0],args);
      	}
      	   
      	
      }
	     
      exit(0);
	
      } else { /* parent process */

      	if(background != 1) { /* if not a background process, parent waits until termination of the child */
      	  wait(NULL); /* same as waitpid(-1, NULL, 0)  */
        }
	
      }

    }

  }
  return 0;
}

/** 
 * The parseCommand function below will not return any value, but it will just: read
 * in the next command line; separate it into distinct arguments (using blanks as
 * delimiters), and set the args array entries to point to the beginning of what
 * will become null-terminated, C-style strings. 
 */

int parseCommand(char inputBuffer[], char *args[],int *background, int *is_redirected, char *file_name[])
{
  int length,/* # of characters in the command line */
    i,/* loop index for accessing inputBuffer array */
    start,/* index where beginning of next command parameter is */
    ct,        /* index of where to place the next parameter into args[] */
    command_number;/* index of requested command number */

  int ct2;
  int redirected_helper = 0;
  ct = 0;
  ct2 = 0;

  /* read what the user enters on the command line */
  do {
    printf("shelly>");
    fflush(stdout);
    length = read(STDIN_FILENO,inputBuffer,MAX_LINE);
    /*printf("\n%d\n",length);*/
  }
  while (inputBuffer[0] == '\n'); /* swallow newline characters */

  /**
   *  0 is the system predefined file descriptor for stdin (standard input),
   *  which is the user's screen in this case. inputBuffer by itself is the
   *  same as &inputBuffer[0], i.e. the starting address of where to store
   *  the command that is read, and length holds the number of characters
   *  read in. inputBuffer is not a null terminated C-string. 
   */
  start = -1;
  if (length == 0)
    exit(0);            /* ^d was entered, end of user command stream */

  /** 
   * the <control><d> signal interrupted the read system call 
   * if the process is in the read() system call, read returns -1
   * However, if this occurs, errno is set to EINTR. We can check this  value
   * and disregard the -1 value 
   */

  if ( (length < 0) && (errno != EINTR) ) {
    perror("error reading the command");
    exit(-1);           /* terminate with error code of -1 */
  }

  /**
   * Parse the contents of inputBuffer
   */

  for (i=0;i<length;i++) {
    /* examine every character in the inputBuffer */

    switch (inputBuffer[i]){
    case ' ':
    case '\t' :               /* argument separators */
      if(start != -1){
	if(redirected_helper == 0){
	  args[ct] = &inputBuffer[start];    /* set up pointer */
	  ct++;
	} else {
	  file_name[ct2] = &inputBuffer[start];
	  ct2++;
	 }
      }
      inputBuffer[i] = '\0';  
      start = -1;
      break;

    case '\n':                 /* should be the final char examined */
      if (start != -1){
	if(redirected_helper == 0){
	  args[ct] = &inputBuffer[start];
	  ct++;
	 } else {
	   file_name[ct2] = &inputBuffer[start];
	   ct2++;
	 }
      }
      inputBuffer[i] = '\0';
      
      args[ct] = NULL; /* no more arguments to this command */
      break;

    default :             /* some other character */
      if (start == -1)
	start = i;
      if (inputBuffer[i] == '&') {
	*background  = 1;
	inputBuffer[i-1] = '\0';
      }
      if (inputBuffer[i] == '>'){
	*is_redirected = 1;
	redirected_helper = 1;
      }
    } /* end of switch */
  }    /* end of for */

  /**
   * If we get &, don't enter it in the args array
   */

  if (*background)
    args[--ct] = NULL;

  args[ct] = NULL; /* just in case the input line was > 80 */

  return 1;

} /* end of parseCommand routine */

// a fuction that remove double duotation mark from given string
char *removeDQMFromString(char *str)
{

  int i,j;
  i = 0;
  while(i<strlen(str)){
      if (str[i]=='"'){
        for (j=i; j<strlen(str); j++)
          str[j]=str[j+1];
      }
      else i++;
  }
  return str;

} //end of removeDQMFromString
