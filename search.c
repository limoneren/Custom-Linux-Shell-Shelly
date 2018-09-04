#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <pwd.h>

char* buffer[10000];
int i = 0;
int found = 0;
const char *homedir;


void traverseDFS(const char *name, int depth, char *str)
{
  DIR *dir;
  struct dirent *entry;
  /*strcat(fileDirectory, name);
   */
  if (!(dir = opendir(name)))
    return;

  
  while ((entry = readdir(dir)) != NULL) { /* In a depth-first manner */


    if (entry->d_type == DT_DIR) { /* if the file is a directory */

      char path[1000];
      
      /* omit current and parent directories */
      if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
	continue;

      
      snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);


      if(strcmp(entry->d_name, str) == 0){
        found++;

        for(i = 0; i < depth; i++){
	  if(found == 1){
	    printf("%s/",buffer[i]);

	  }
	  
	}
	break;
	
      } else {
	
      }
      
      
      buffer[depth] = entry->d_name;
      traverseDFS(path, depth + 1,str);

      
    } else { /* if not a directory */

      if(strcmp(entry->d_name, str) == 0){
        found++;

        for(i = 0; i < depth; i++){
	  if(found == 1){
	    printf("%s/",buffer[i]);

	  }
	  
	}
		
	break;
      } else {
	
      }
      
    }
  }
  closedir(dir);
}

int main(int argc, char *argv[]) {

  printf("/home/");
  homedir = getpwuid(getuid())->pw_dir;
  
  traverseDFS(homedir, 0, argv[1]);
  if(found == 0) {
    printf(" is searched, but could not found.\n");
  } else {
     printf("%s\n",argv[1]);
  }
 
  return 0;
  
}
