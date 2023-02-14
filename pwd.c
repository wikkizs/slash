#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "pwd.h"
#define max 4096  

int f_pwd(int argc,char **argv){ 
    char *w=calloc(1,max*sizeof(char));
    if(w==NULL){
        perror("Error calloc");
        free(w);
        return 1;
    }
    if(argc>2){
        perror("Error:too many arguments");
        free(w);
        return 1;
    }
    if(argc==1 || (argc==2 && (strcmp(argv[1],"-L")==0))){
        strcpy(w,getenv("PWD"));
        strcat(w,"\n");
        write(STDOUT_FILENO,w,strlen(w));
        free(w);
        return 0;
    }else{
        if(strcmp(argv[1] ,"-P")==0){
            char *nw=getcwd(NULL,0);
            strcpy(w,nw);
            strcat(w,"\n");
            write(STDOUT_FILENO,w,strlen(w));
            free(nw);
            free(w);
            return 0 ;
        }else{
            perror("Error pwd: Invalid arguments");
            free(w);
            return 1;
        }
    }
    free(w);
    return 0 ;
}


