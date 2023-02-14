#define _DEFAULT_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "cd.h"

#define max 4096


//verifie si le pointeur en argument est un path valide
int isValidPathDir(char *start){
	DIR *cwd=opendir(start);
	if(cwd==NULL){
		free(cwd);
		return -1;
	}else{
		free(cwd);
		return 0;
	}
}

//changer le repertoire courant
int setRepCourant(int LorP, char * nwd,char* init){
	char *owd;
	char rp[strlen(nwd)+1];
	strcpy(rp,nwd);
	if((owd=getenv("PWD"))<0){
		perror("getcwd error");
		return 1;
	}
    if(LorP){
        if(isValidPathDir(rp)<0){
        	return setRepCourant(0,init,init);
        }else{
        	if(setenv("OLDPWD",owd,1)<0){
            perror("Error: cannot change 'OLDPWD'");
            return 1;
	    }
        	chdir(rp);
        	setenv("PWD",rp,1);
        }
    }else{
    	if(isValidPathDir(rp)<0){
    		write(STDOUT_FILENO,"cd: No such file or directory\n",strlen("cd: No such file or directory\n"));
    		return 1;
        }
        else{
    		if(setenv("OLDPWD",owd,1)<0){
                perror("Error: cannot change 'OLDPWD'");
                return 1;
	        }
	        if(chdir(rp)<0){
    		    perror("Error");
    		    return 1;
    	    }else{
    		    char *nd=getcwd(NULL,0);
    		    if(setenv("PWD",nd,1)<0){
    			    perror("fail pwd");
    			    free(nd);
    			    return 1;
    		    }
    		    free(nd);
    		}
    	}
    }
    return 0;
}




//concaténer x et y et simplifier les . et .. s'ils existent
char * simplifierPath(char *x ,char *y){
	int lx=strlen(x);
	int ly=strlen(y);
    char *X=calloc(1,sizeof(char)*max);
    char *Y=calloc(1,sizeof(char)*max);
    if(X==NULL || Y==NULL){
    	perror("Error malloc");
    	free(X);
    	free(Y);
    	return NULL;
    }else{
    	for(int i=0;i<lx;i++){
    	X[i]=x[i];
        }
        for(int j=0;j<ly;j++){
    	Y[j]=y[j];
        }

        /*****************verifie*****************/
        if(X[lx-1]=='/'){
            X[lx-1]='\0';
            lx--;
        }
        if(X[0]=='/'){
           ly--;
        }
        /***************verfie********************/
        char * new=calloc(1,sizeof(char)*(lx+ly+3));
        if(new==NULL){
        	perror("Error calloc");
        	free(X);
        	free(Y);
        	free(new);
        	return NULL;
        }
        memmove(new,X,lx);
        memmove(new+lx,"/",1);
        if(Y[0]=='/'){
            memmove(new+lx+1,Y+1,ly+1);
        }else{
            memmove(new+lx+1,Y,ly+1);
        }
        new[lx+ly+2]='\0';
        /**************verifie**********************/
        char *rest;
        char *simplified=calloc(1,(max*sizeof(char)));
        if(simplified==NULL){
        	perror("Error calloc");
        	free(new);
        	free(X);
        	free(Y);
        	free(simplified);
        	return NULL;
        }
        /***********verifie***************/
        memmove(simplified,"/",1);
        int length=1;
        int lastLength=0;
        char *token=strtok_r(new,"/",&rest);
        while(token!=NULL){
            if(strcmp(token,".")==0 || (strcmp(simplified,"/")==0 && strcmp(token,"..")==0)){
               token=strtok_r(NULL,"/",&rest);
               continue;
            }
            /***********verifie********************/
            else if(strcmp(token,"..")!=0){
                if(simplified[length-1]!='/'){
                    memmove(simplified+length,"/",1);
                    length++;
                }
                memmove(simplified+length,token,strlen(token));
                length+=strlen(token);
                memmove(simplified+length+1,"/",1);
                lastLength=strlen(token);
            }else{
                char *copy=simplified;
                if(copy[strlen(copy)-1]=='/'){
                    copy[strlen(copy)-1]='\0';
                }
                char* occ=strrchr(copy,'/');
                int indice=occ-copy;
                lastLength=length-indice-1;
                simplified[length-lastLength]='\0';
                length-=lastLength;
            }
            token=strtok_r(NULL,"/",&rest);
        }
        if(simplified[length-1]=='/'){
        	simplified[length-1]='\0';
        }else{
        simplified[length]='\0';
    }
        free(X);
        free(Y);
        free(new);
        return simplified;
    }
}


int checkPath(char *directory, char *path){
	char * newPath=simplifierPath(directory,path);//le path simplifié
	if(newPath==NULL){
		perror("Error simplifierpath");
		free(newPath);
		return 1;
	}
	int a=setRepCourant(1,newPath,path);
	free(newPath);
	return a;
}

int f_cd(int argc, char **argv){
	char *cwd;
	
	if(argc==1){ //cd sans argument -> $HOME
	   	   if((cwd=getenv("HOME"))==NULL){
	   	   	   perror("cd: Error Home is not defined");
	   	   	   return 1;
	   	   }
	   	   return setRepCourant(0,cwd,"");
	}
	else if(argc==2){ //-L par défaut -> logique
	   		char path[max];//length
	        strcpy(path, argv[1]);
	   		if(strcmp(path,"-")==0){//cd [-L] -
	   			cwd=getenv("OLDPWD");
	   			if(cwd==NULL || strlen(cwd)==0){
	   				perror("cd: Error OLDPWD is not defined");
	   				return 1;
	   			}
	   			return setRepCourant(1,cwd,"");
	   		}else if(strcmp(path,"-L")==0 || strcmp(path,"-P")==0){
	   			if((cwd=getenv("HOME"))==NULL){
	   	   	    perror("cd: Error Home is not defined");
	   	   	    return 1;
	   	   	    }
	   	        return setRepCourant(1,cwd,"");
	   		}else{
	   			if(path[0]=='/'){
	   				return checkPath("/",path);
	   			}
	   			if((cwd=getenv("PWD"))==NULL){
	   				perror("cd: Error PWD");
	   				return 1;
	   			}
	   			return checkPath(cwd,path);
	   		}
	   	}
	else if(argc==3){
	   		char path[max];
	        strcpy(path, argv[2]);
	        if(strcmp(argv[1],"-L")==0){//Logique
	        	if(strcmp(path,"-")==0){//cd [-L] -
	   			    if((cwd=getenv("OLDPWD"))==NULL){
	   				    perror("cd: Error OLDPWD is not defined");
	   				    return 1;
	   				}
	   			    return setRepCourant(1,cwd,"");
	   		    }else{
	   			    if(path[0]=='/'){
	   				   return setRepCourant(1,path,""); 
	   				}
	   				if((cwd=getenv("PWD"))==NULL){
	   					perror("cd: Error PWD");
	   					return 1;
	   				}
	   				return checkPath(cwd,path);
	   			}
	        }
	        else if(strcmp(argv[1],"-P")==0){//Physique
	        	if(strcmp(path,"-")==0){//cd [-P] -
	   			    if((cwd=getenv("OLDPWD"))==NULL){
	   				    perror("cd: Error OLDPWD is not defined");
	   				    return 1;
	   				}
	   			    return setRepCourant(0,cwd,"");
	   		    }else{
	   				return setRepCourant(0,path,"");
	   			}
	        }
	        else{
	        	perror("cd: Invalid arguments");
	        	return 1;
	        }
	    }else{

	    	perror("cd: Error too many arguments");
	    	return 1;

	}
	return 0;
}

