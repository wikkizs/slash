#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "exit.h"

void  f_exit(int argc,int val, char *argv[]) {
    if (argc >2) {
        perror("Exit: Error too many arguments");
        exit(EXIT_FAILURE);
    }
    if(argc==1){
        exit(val);
    }else{
        int ret=atoi(argv[1]);
        exit(ret);
    }
    
    exit(0);
}