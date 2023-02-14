#define _DEFAULT_SOURCE

#include <unistd.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/wait.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include "pwd.h"
#include "cd.h"
#include "exit.h"


#define max 1000
#define maxDepth 9
#define CD 5863276
#define PWD 193502992
#define EXIT 6385204799

int val= 0;

// fonction hash prise d'internet
const unsigned long hash(const char *str) {
    unsigned long hash = 5381;  
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}

char * buildPrompt() {
    char *prompt=calloc(1,sizeof(char)*52);
    if(prompt==NULL){
        char *msg = "Erreur calloc\n" ;
        if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
            perror("Erreur ecriture dans le shell") ;
        return NULL;
    }
    char * cwd=getenv("PWD");
    char *v=calloc(1,4*sizeof(char));
    if(v==NULL){
        char *msg = "Erreur calloc\n" ;
        if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
            perror("Erreur ecriture dans le shell") ;
        free(prompt);
        return NULL;
    }
    sprintf(v,"%d",val);
    v[strlen(v)]='\0';
    if(val>0){
        strcat(prompt,"\001\033[91m\002[");
    }else{
        strcat(prompt,"\001\033[32m\002[");
    }
    if(val!=-1) {
        strcat(prompt, v);
    }else{
        strcat(prompt,"SIG");
    }
    strcat(prompt,"]");
    if ((strlen(cwd)+strlen(v)+strlen("$ ")+2)<=30){
        strcat(prompt,"\001\033[34m\002");
        strcat(prompt,cwd);
    }else{
        strcat(prompt,"\001\033[34m\002");
        strcat(prompt,"...");
        int length=51-strlen(prompt)-strlen("\001\033[00m\002$ ");
        strcat(prompt,cwd+strlen(cwd)-length);
    }
    strcat(prompt,"\001\033[00m\002$ ");
    prompt[51]='\0';
    free(v);
    return prompt;
}


char ** splitCommand(char *prompt){//, char *cmd[max]) {
    char **tab=calloc(max,sizeof(tab)*max);
    if(tab==NULL){
        char *msg = "Erreur calloc\n" ;
        if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
            perror("Erreur ecriture dans le shell") ;
        return NULL;
    }
    char *p =  strtok (prompt, " ");
    tab[0]=calloc(1,sizeof(char)*max);
    if(tab[0]==NULL){
        char *msg = "Erreur calloc\n" ;
        if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
            perror("Erreur ecriture dans le shell") ;
        free(tab);
        return NULL;
    }
    strcpy(tab[0],p);
    int i = 0;

    while (p != NULL)
    {
        p = strtok (NULL, " ");
        tab[++i]=p;
    }
    if(i!=0){
        tab[i-1][strlen(tab[i-1])]='\0';
    }
    return tab;
}


/*retoune un pointeur de pointeurs qui contient l'expansion de target qui peut etre 
'*' ou '*.extension' */
char ** Search(char *start, char *target){
    char ** tab=calloc(max,max*sizeof(char));
    if(tab==NULL){
        char *msg = "Erreur calloc\n" ;
        if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
            perror("Erreur ecriture dans le shell") ;
        return NULL;
    }
    DIR *rep;
    int i=0;
    if(strcmp(start,"")==0 || strlen(start)==0){
        rep = opendir (".");//on cherche dans le repertoire courant
    }else{
        rep = opendir (start);//on cherche dans le repertoire start
    }
    if(strcmp(target,"*")==0){
        if (rep != NULL){
            struct dirent * ent;
            while ((ent = readdir (rep)) != NULL){
                if((ent->d_name)[0]!='.'){
                    tab[i]=calloc(1,max*sizeof(char));
                    if(tab[i]==NULL){
                        char *msg = "Erreur calloc\n" ;
                        if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                            perror("Erreur ecriture dans le shell") ;
                        closedir(rep);
                        free(tab);
                        return NULL ;
                    }
                    strcpy(tab[i],ent->d_name);
                    i++;
                }
            }          
        }  
    if(i!=0) tab[i-1][strlen(tab[i-1])]='\0';
    closedir(rep);
    return tab;
    }else{
        char *search=calloc(1,(strlen(target))*sizeof(char));
        if(search==NULL){
            char *msg = "Erreur calloc\n" ;
            if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                perror("Erreur ecriture dans le shell") ;
            free(tab) ;
            closedir(rep);
            return NULL;
        }
        strcpy(search, target+1);
        search[strlen(search)]='\0';
        if (rep != NULL){
            struct dirent * ent;
            while ((ent = readdir (rep)) != NULL){
                char *n=calloc(1,sizeof(char)*(strlen(search)+1));
                if(n==NULL){
                    char *msg = "Erreur calloc\n" ;
                    if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                        perror("Erreur ecriture dans le shell") ;
                    free(tab);
                    free(search);
                    closedir(rep);
                    return NULL;
                }
                if(strlen(search)<=strlen(ent->d_name)){
                    strcpy(n,ent->d_name+(strlen(ent->d_name)-strlen(search)));
                }
                n[strlen(n)]='\0';
                if((ent->d_name)[0]!='.' && strcmp(n,search)==0){
                    tab[i]=calloc(1,sizeof(char)*max);
                    if(tab[i]==NULL){
                        char *msg = "Erreur calloc\n" ;
                        if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                            perror("Erreur ecriture dans le shell") ;
                        closedir(rep);
                        free(search);
                        free(tab) ;
                        free(n) ;
                        return NULL;
                    }
                    strcpy(tab[i],ent->d_name);
                    i++;
                }
                free(n);
            }
        }
        if(i==0){
            tab[i]=calloc(1,sizeof(char)*max);
            if(tab[i]==NULL){
                char *msg = "Erreur calloc\n" ;
                if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                    perror("Erreur ecriture dans le shell") ;
                free(tab);
                free(search);
                closedir(rep);
                return NULL;
            }
            strcpy(tab[i],target);
        }else{
            tab[i-1][strlen(tab[i-1])]='\0';
        }
        free(search);
        closedir(rep);
        return tab;
    }
}


//copie de search à quelques modifications pour fixer des erreurs de mémoire*/
char ** Search_cp(char *start, char *target2){
    char *target=realloc(target2,max*sizeof(char));
    if(target==NULL){
        char *msg = "Erreur calloc\n" ;
        if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
            perror("Erreur ecriture dans le shell") ;
        return NULL ;
    }
    char ** tab=calloc(max,max*sizeof(char));
    if(tab==NULL){
        char *msg = "Erreur calloc\n" ;
        if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
            perror("Erreur ecriture dans le shell") ;
        free(target) ;
        return NULL;
    }
    DIR *rep;
    int i=0;
    if(strcmp(start,"")==0 || strlen(start)==0){// * ou *.extension
        rep = opendir (".");
        if(rep==NULL){
            char *msg = "Erreur calloc\n" ;
            if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                perror("Erreur ecriture dans le shell") ;
            free(target);
            free(tab);
            return NULL;
        }
    }else{
        rep = opendir (start);
        if(rep==NULL){
            char *msg = "Erreur calloc\n" ;
            if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                perror("Erreur ecriture dans le shell") ;
            free(tab);
            free(target);
            return NULL;
        }
    }
    if(strcmp(target,"*")==0){ // *
        if (rep != NULL){
            struct dirent * ent;
            while ((ent = readdir (rep)) != NULL){
                if((ent->d_name)[0]!='.'){
                    tab[i]=calloc(1,max*sizeof(char));
                    if(tab[i]==NULL){
                        char *msg = "Erreur calloc\n" ;
                        if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                            perror("Erreur ecriture dans le shell") ;
                        free(tab);
                        free(target);
                        closedir(rep) ;
                        return NULL ;
                    }
                    strcpy(tab[i],ent->d_name);
                    i++;
                }
            }          
        }  
    
    if(i!=0) tab[i-1][strlen(tab[i-1])]='\0';
    free(target);
    closedir(rep);
    return tab;
    }else{
        char *search=calloc(1,(strlen(target))*sizeof(char));
        if(search==NULL){
            char *msg = "Erreur calloc\n" ;
            if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                perror("Erreur ecriture dans le shell") ;
            free(tab);
            closedir(rep);
            return NULL;
        }
        strcpy(search, target+1);
        search[strlen(search)]='\0';
        if (rep != NULL){
            struct dirent * ent;
            while ((ent = readdir (rep)) != NULL){
                char *n=calloc(1,sizeof(char)*(strlen(search)+1));
                if(n==NULL){
                    char *msg = "Erreur calloc\n" ;
                    if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                        perror("Erreur ecriture dans le shell") ;
                    free(tab);
                    free(search) ;
                    closedir(rep);
                    return NULL;
                }
                if(strlen(search)<=strlen(ent->d_name)){
                    strcpy(n,ent->d_name+(strlen(ent->d_name)-strlen(search)));
                }
                n[strlen(n)]='\0';
                if((ent->d_name)[0]!='.' && strcmp(n,search)==0){                        
                    tab[i]=calloc(1,sizeof(char)*max);
                    if(tab[i]==NULL){
                        char *msg = "Erreur calloc\n" ;
                        if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                            perror("Erreur ecriture dans le shell") ;
                        free(tab);
                        free(search);
                        closedir(rep) ;
                        return NULL ;
                    }
                    strcpy(tab[i],ent->d_name);
                    i++;
                }
                free(n);
            }
        }
        if(i==0){
            tab[0]=calloc(1,sizeof(char)*max);
            if(tab[0]==NULL){
                char *msg = "Erreur calloc\n" ;
                if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                    perror("Erreur ecriture dans le shell") ;
                free(tab);
                free(search);
                closedir(rep);
                return NULL;
            }
            strcpy(tab[i],target);
        }else{
            tab[i-1][strlen(tab[i-1])]='\0';
        }
        free(search);
        free(target);
        closedir(rep);
        return tab;
    }
}

//verifie si le path représenté par le pointeur s est fichier ou un repertoire valide
int Verif(char *s){
    DIR *cwd=opendir(s);
    int fd=open(s,O_RDONLY);
    if(cwd==NULL && fd<0){
        return -1;
    }else{
        closedir(cwd);
        close(fd);
        return 0;
    }
}

//verifie si le path représenté par le pointeur s est un repertoire valide(dans le cas de **)
int VerifEtoile(char *s){
    DIR *cwd=opendir(s);
    if(cwd==NULL){
        return -1;
    }else{
        closedir(cwd);
        return 0;
    }
}


int countSlash (char *s){
    int res=0;
    for(int i=0;i<strlen(s);i++){
        if(s[i]=='/') res++;
    }
    return res;
}

//compte le nombre de slash unitiles dans un pointeur, par ex uselessSlash(a///b)=2
int UselessSlash(char *s){
    int res=0;
    int i=0;
    while(i<strlen(s)){ 
        if(s[i]=='/'){
            int j=i+1;
            while(j < strlen(s) && s[j]=='/'){
                res++;
                j++;
            }
            i+=res+1;
        }else{
            i++;
        }
    }
    return res;
}

//verifie si le pointeur s contient quelques part un lien symbolique(pour **)
int checkSymLink(char *s){
    char *str=calloc(1,sizeof(char)*max);
    if(str==NULL){
        char *msg = "Erreur calloc\n" ;
        if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
            perror("Erreur ecriture dans le shell") ;
        return 1 ;
    }
    strcpy(str,s);
    int res=0;
    char *rest;
    char *token=strtok_r(str,"/",&rest);
    char *tmp=calloc(1,sizeof(char)*max);
    if(tmp==NULL){
        char *msg = "Erreur calloc\n" ;
        if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
            perror("Erreur ecriture dans le shell") ;
        free(str);
        return 1 ;
    }
    while(token!=NULL){
        if(strlen(tmp)==0){
            strcpy(tmp,token);
        }else{
            strcat(tmp,"/");
            strcat(tmp,token);
        }
        struct stat st;
        if(lstat(tmp,&st)==0) {
            if (S_ISLNK(st.st_mode)) res = -1;
            token = strtok_r(NULL, "/", &rest);
        }else{
            char *msg = "Erreur lstat\n" ;
            if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                perror("Erreur ecriture dans le shell") ;
            free(str);
            free(tmp) ;
            return 1 ;
        }
    }
    free(str);
    free(tmp);
    return res;
}


char ** treatJoker(char *str){
    char *s=calloc(1,sizeof(char)*(strlen(str)+1));
    if(s==NULL){
        char *msg = "Erreur calloc\n" ;
        if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
            perror("Erreur ecriture dans le shell") ;
        return NULL;
    }
    memmove(s,str,strlen(str)+1);
    s[strlen(s)]='\0';

    if(strcmp(s,"**/")==0){
        char **tab=calloc(max,max*sizeof(char));
        if(tab==NULL){
            char *msg = "Erreur calloc\n" ;
            if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                perror("Erreur ecriture dans le shell") ;
            free(s);
            return NULL;
        }
        int cmp=0;
        int b=0;
        char *x=calloc(1,sizeof(char)*max);
        if(x==NULL){
            char *msg = "Erreur calloc\n" ;
            if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                perror("Erreur ecriture dans le shell") ;
            free(tab);
            free(s);
            return NULL ;
        }
        for(int i=0;i<maxDepth;i++){
            if(b==0){
            strcpy(x,"*");}
            if(b!=0){
            strcat(x,"/*");}
            char **t=treatJoker(x);
            if(strstr(t[0],"*")==NULL){
                int j=0;
                while(t[j]!=NULL && j<max){
                    tab[cmp]=calloc(1,sizeof(char)*max);
                    if (tab[cmp]==NULL){
                        char *msg = "Erreur calloc\n" ;
                        if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                            perror("Erreur ecriture dans le shell") ;
                        free(tab);
                        free(t);
                        free(s);
                        free(x);
                        return NULL ;
                    }
                    strcpy(tab[cmp],t[j]);
                    cmp++;
                    free(t[j]);
                    j++;

                }
            }
            b++;
            free(t);
        }
        char **finalRes=calloc(max,max*sizeof(char));
        if(finalRes==NULL){char *msg = "Erreur calloc\n" ;
            if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                perror("Erreur ecriture dans le shell") ;
            free(tab);
            free(s);
           return NULL;
        }
        int pos=0;
        for(int k=0;k<cmp;k++){
            if(VerifEtoile(tab[k])==0){
                if(checkSymLink(tab[k])==0){
                finalRes[pos]=calloc(1,sizeof(char)*max);
                if(finalRes[pos]==NULL){
                    char *msg = "Erreur calloc\n" ;
                    if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                        perror("Erreur ecriture dans le shell") ;
                    free(tab);
                    free(s);
                    free(finalRes);
                    return NULL ;
                }
                strcpy(finalRes[pos],tab[k]);
                strcat(finalRes[pos],"/");
                pos++;}
            }
            free(tab[k]);
        }
        if(pos==0) {
            finalRes[0]=calloc(1,sizeof(char)*max);
            if(finalRes[0]==NULL){
                char *msg = "Erreur calloc\n" ;
                if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                    perror("Erreur ecriture dans le shell") ;
                free(tab);
                free(s);
                return NULL ;
            }
            strcpy(finalRes[0],str);
        }
        free(tab);
        free(s);
        free(x);
        return finalRes;
    }else if(strstr(s,"**")!=NULL && strstr(s,"/")!=NULL){// par ex **/*.c ou **/*/*/*/*.c
        char **tab=calloc(max,max*sizeof(char));
        if(tab==NULL){
            char *msg = "Erreur calloc\n" ;
            if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                perror("Erreur ecriture dans le shell") ;
            return NULL;
        }
        int tablen=0;
        char *rest;
        strtok_r(s,"/",&rest);
        char **tmp=treatJoker("**/");
        int cmp=0;
        while(tmp[cmp]!=NULL && cmp<max){
            char *a=calloc(1,sizeof(char)*max);
            if(a==NULL){
                char *msg = "Erreur calloc\n" ;
                if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                    perror("Erreur ecriture dans le shell") ;
                free(tmp);
                free(tab);
                return NULL;
            }
            strcpy(a,tmp[cmp]);
            strcat(a,rest);
            char **new=treatJoker(a);
            int k=0;
            while(new[k]!=NULL && strstr(new[k],"*")==NULL){
                tab[tablen]=calloc(1,sizeof(char)*max);
                if (tab[tablen]==NULL){
                    char *msg = "Erreur calloc\n" ;
                    if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                        perror("Erreur ecriture dans le shell") ;
                    free(tab);
                    free(tmp);
                    free(a);
                    return NULL ;
                }
                strcpy(tab[tablen],new[k]);
                tablen++;
                k++;
            }
            free(tmp[cmp]);
            int h=0;
            while(new[h]!=NULL && h<max) {
                free(new[h]);
                h++;
            }
            free(new);
            free(a);
            cmp++;
        }
        free(tmp);
        char **tmp2=treatJoker(rest);
        int m=0;
        while(tmp2[m]!=NULL && strstr(tmp2[m],"*")==NULL){
            tab[tablen]=calloc(1,sizeof(char)*max);
            if(tab[tablen]==NULL){
                char *msg = "Erreur calloc\n" ;
                if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                    perror("Erreur ecriture dans le shell") ;
                return NULL;
            }
            strcpy(tab[tablen],tmp2[m]);
            m++;
            tablen++;
        }
        int l=0;
        while(tmp2[l]!=NULL) {
            free(tmp2[l]);
            l++;
        }
        free(tmp2);

        if(tablen==0){
            tab[0]=calloc(1,sizeof(char)*max);
            if(tab[0]==NULL){
                char *msg = "Erreur calloc\n" ;
                if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                    perror("Erreur ecriture dans le shell") ;
                return NULL ;
            }
            strcpy(tab[0],str);
        }
        free(s);
        return tab;
    }
    else if((strcmp(s,"*")==0) || (s[0]=='*' && strstr(s,"/")==NULL)){//  * ou  *.extension
        return Search_cp("",s);
    }
    else if(strcmp(s,"*/")==0){
        char **a=Search("","*");
        char **res=calloc(max,max*sizeof(char));
        if(res==NULL){
            char *msg = "Erreur calloc\n" ;
            if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                perror("Erreur ecriture dans le shell") ;
            free(s);
            return NULL;
        }
        int j=0;
        int cmp=0;
        while(a[j]!=NULL && j<max){
            char *t=calloc(1,sizeof(char)*max);
            if(t==NULL){
                char *msg = "Erreur calloc\n" ;
                if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                    perror("Erreur ecriture dans le shell") ;
                free(res);
                free(a);
                return NULL;
            }
            strcpy(t,a[j]);
            strcat(t,"/");
            if(Verif(t)==0){
                res[cmp]=t;
                cmp++;
            }
            j++;
        }
      if(res[0]==NULL) res[0]=str;
      free(a);
      free(s);
      return res;
    }
    else{
        int tablen=0;
        int tmplen=0;
        char ** tab =calloc(max,sizeof(char)*max);
        if(tab==NULL){
            char *msg = "Erreur calloc\n" ;
            if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                perror("Erreur ecriture dans le shell") ;
            return NULL;
        }
        //for(int f=0;f<max;f++) tab[f]=malloc(max*sizeof(char));
        char *tmp=calloc(1,max*sizeof(char)); //ls a/b/* -> a/ a/b  
        if(tab==NULL){
            char *msg = "Erreur calloc\n" ;
            if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                perror("Erreur ecriture dans le shell") ;
            free(s);
            return NULL;
        }
        if(s[0]=='/'){
            strcpy(tmp,"/");
            tmplen++;
        }
        int nbtoken=0;
        char *rest;
        char *token=strtok_r(s,"/",&rest);
        while(token!=NULL){
            if(strstr(token,"*")==NULL){//le token ne contient pas * 
                if(tablen==0){//le tableau est vide: au debut
                    if(nbtoken>=1){
                        strcat(tmp,"/");
                        tmplen++;
                    }
                    strcat(tmp,token);
                    tmplen+=strlen(token);
                }else{
                    for(int i=0;i<tablen ;i++){
                        strcat(tab[i],"/");
                        strcat(tab[i],token);
                    }
                }
            }
            else{//le token contient *
                int j=0;
                if(tablen==0){//le tableau est vide
                    char ** val=Search(tmp,token);
                    if(strcmp(tmp,"")==0 || tmplen==0){
                        while(val[j]!=NULL && j<max){
                            tab[j]=calloc(1,sizeof(char)*max);
                            if(tab[j]==NULL){
                                char *msg = "Erreur calloc\n" ;
                                if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                                    perror("Erreur ecriture dans le shell") ;
                                free(val) ;
                                return NULL ;
                            }
                            strcpy(tab[j],val[j]);
                            free(val[j]);
                            j++;
                            tablen++;
                        }
                    }else{
                        while(val[j]!=NULL && j<max){ 
                            tab[j]=calloc(1,sizeof(char)*max);
                            if(tab[j]==NULL){
                                char *msg = "Erreur calloc\n" ;
                                if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                                    perror("Erreur ecriture dans le shell") ;
                                return NULL ;
                            }
                            strcpy(tab[j],tmp);
                            if(strcmp(tmp,"/")!=0){
                               strcat(tab[j],"/");
                            }        
                            strcat(tab[j],val[j]);
                            free(val[j]);
                            j++;
                            tablen++;
                        }
                    }
                    free(val);
                }else{
                    int ct=tablen;
                    for(int k=0;k<ct;k++){
                        char **new=Search(tab[k],token);
                        int tr=0;
                        while(new[tr]!=NULL && tr<max){
                            tab[tablen]=calloc(1,sizeof(char)*max);
                            if(tab[tablen]==NULL){
                                char *msg = "Erreur calloc\n" ;
                                if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                                    perror("Erreur ecriture dans le shell") ;
                                free(new);
                                return NULL;
                            }
                            strcpy(tab[tablen],tab[k]);
                            strcat(tab[tablen],"/");
                            strcat(tab[tablen],new[tr]);
                            free(new[tr]);
                            tablen++;
                            tr++;
                        }
                        free(new);
                    }
                }
            }
            token=strtok_r(NULL,"/",&rest);
            nbtoken++;
        
    }
    if(tablen!=0){
        tab[tablen-1][strlen(tab[tablen-1])]='\0';
    }

    char **finalRes=calloc(max,max*sizeof(char));
    if(finalRes==NULL){
        char *msg = "Erreur calloc\n" ;
        if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
            perror("Erreur ecriture dans le shell") ;
        free(s);
        free(tmp);
        free(tab);
        return NULL;
    }
    int cmp=0;
    for(int v=0;v<tablen;v++){
        if(Verif(tab[v])==0 && ((countSlash(tab[v])==countSlash(str)) || 
            countSlash(tab[v])==(countSlash(str)-UselessSlash(str)))){
            finalRes[cmp]=calloc(1,sizeof(char)*max);
            if(finalRes[cmp]==NULL){
                char *msg = "Erreur calloc\n" ;
                if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                    perror("Erreur ecriture dans le shell") ;
                free(finalRes);
                return NULL ;
            }
            strcpy(finalRes[cmp],tab[v]);
            cmp++;
        }
        free(tab[v]);
    }
    if(cmp==0 || tablen==0){
        finalRes[0]=calloc(1,sizeof(char)*max);
        if(finalRes[0]==NULL){
            char *msg = "Erreur calloc\n" ;
            if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                perror("Erreur ecriture dans le shell") ;
            free(finalRes);
            return NULL ;
        }
        strcpy(finalRes[0],str);
        cmp++;
    }
    free(tab);
    free(s);
    free(tmp);
    if(cmp!=0) finalRes[cmp-1][strlen(finalRes[cmp-1])]='\0';
    return finalRes;
}
}

    

//faire l'expansion de l'etoile et double etoile si besoin
char ** interpretCommand(int length, char **cmd2){
    char **cmd =realloc(cmd2,max*max*sizeof(cmd2));
    if(cmd==NULL){
        char *msg = "Erreur calloc\n" ;
        if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
            perror("Erreur ecriture dans le shell") ;
        return NULL;
    }
    char **res=calloc(max,max*sizeof(char));
    if(res==NULL){
        char *msg = "Erreur calloc\n" ;
        if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
            perror("Erreur ecriture dans le shell") ;
        free(res);
        return NULL;
    }
    else{
        if(strstr(cmd[0],"*")!=NULL){
            char **tb=treatJoker(cmd[0]);
            res[0]=calloc(1,sizeof(char)*max);
            if(res[0]==NULL){
                char *msg = "Erreur calloc\n" ;
                if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                    perror("Erreur ecriture dans le shell") ;
                free(tb);
                return NULL ;
            }
            strcpy(res[0],tb[0]);
            int q=0;
            while(tb[q]!=NULL && q<max) {
                free(tb[q]);
                q++;
            }
            free(tb);
        }else{
            res[0]=calloc(1,sizeof(char)*max);
            if(res[0]==NULL){
                char *msg = "Erreur calloc\n" ;
                if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                    perror("Erreur ecriture dans le shell") ;
                free(res);
                return NULL ;
            }
            strcpy(res[0],cmd[0]);
        }
        int j=0;
        for(int i=1;i<length;i++){
            if(strstr(cmd[i],"*")!=NULL){
                char** tmp=treatJoker(cmd[i]);
                int t=0;
                while(tmp[t]!=NULL && t<max){
                    res[j+1]=calloc(1,sizeof(char)*max);
                    if(res[j+1]==NULL){
                        char *msg = "Erreur calloc\n" ;
                        if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                            perror("Erreur ecriture dans le shell") ;
                        free(res);
                        free(tmp);
                        return NULL ;
                    }
                    strcpy(res[j+1],tmp[t]);
                    free(tmp[t]);
                    t++;
                    j++;
                }
                free(tmp);
            }else{
                res[j+1]=calloc(1,sizeof(char)*max);
                if(res[j+1]==NULL){
                    char *msg = "Erreur calloc\n" ;
                    if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                        perror("Erreur ecriture dans le shell") ;
                    free(res);
                    return NULL ;
                }
                strcpy(res[j+1],cmd[i]);
                j++;
            }
        }
        if(j!=0){
            res[j-1][strlen(res[j-1])]='\0';
        }
        free(cmd[0]);
        free(cmd);
        return res;
    }
}



//executer les commandes externes
void execExtern(int length, char **cmd) {
  pid_t pid;
  int status;
  pid = fork();
  if (pid < 0) exit(1);
  else if (!pid){
      struct sigaction sa = {};
      sa.sa_handler = SIG_DFL ;
      sigaction(SIGTERM,&sa,NULL) ;
      sigaction(SIGINT,&sa,NULL) ;
      execvp(cmd[0],cmd);
      exit(127);
  }
  if (wait(&status) < 0){
    exit(1);
  }
  if(WIFEXITED(status)) {
      if (WEXITSTATUS(status) > 0) {
          val = 1;
      } else {
          val = 0;
      }
  }
  else{
      write(STDERR_FILENO,"Processus Complété\n", strlen("Processus Complété\n")) ;
      val =-1 ;
  }
}


void executeCommand(int length, char** cmd) {
    char * cmd_name = cmd[0];
    if(cmd[0]==NULL){
        exit(0);
    }
    switch(hash(cmd_name)) {
    case CD: 
        val=f_cd(length,cmd);
        break;
    case PWD:
       val=f_pwd(length,cmd);
        break;
    case EXIT:
        f_exit(length,val,cmd);
        break;
    default:
        execExtern(length,cmd);
    }
}

//retoune un pointeur de pointeur qui contient les positions des redirections dans tab
char ** getPos(int length,char **tab){
    char **res=calloc(max,max*sizeof(char));
    if(res==NULL){
        char *msg = "Erreur calloc\n" ;
        if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
            perror("Erreur ecriture dans le shell") ;
        return NULL ;
    }
    int pos=0;
    for (int i=0;i<length;i++){
        if(strstr(tab[i],"|")!=NULL || strstr(tab[i],">")!=NULL || strstr(tab[i],"<")!=NULL){
            res[pos]=malloc(max*sizeof(char));
            if(res[pos]==NULL){
                char *msg = "Erreur calloc\n" ;
                if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                    perror("Erreur ecriture dans le shell") ;
                free(res);
                return NULL;
            }
            sprintf(res[pos],"%d",i);
            pos++;
        }
    }
    return res;
}


//verifie s'il y a une redirections à faire
int ContainsRedirections(int length,char **tab){
    int res=-1;
    for (int i=0;i<length;i++){
        if(strstr(tab[i],">")!=NULL || strstr(tab[i],"<")!=NULL){
            res=i;
        }
    }
    return res;
}

//retoune un pointeur de pointeurs avec les positions des tubes dans tab
char ** getPipesPos(int length,char **tab){
    char **res=calloc(max,max*sizeof(char));
    int pos=0;
    for (int i=0;i<length;i++){
        if(strcmp(tab[i],"|")==0){
            res[pos]=malloc(max*sizeof(char));
            if(res[pos]==NULL){
                char *msg = "Erreur malloc\n" ;
                if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                    perror("Erreur ecriture dans le shell") ;
                free(res);
                return NULL ;
            }
            sprintf(res[pos],"%d",i);
            pos++;
        }
    }
    return res;
}

//verifie si tab contient des tubes
int ContainsPipes(int length,char **tab){
    int res=-1;
    for (int i=0;i<length;i++){
        if(strcmp(tab[i],"|")==0){
            res=i;
        }
    }
    return res;
}


//traiter les redirections
void treatRedirections(char **cmd,char **tab){
        int i=0;
        while(tab[i]!=NULL && i<max){
        int p=atoi(tab[i]);
        char *sym=cmd[p];
        char *filename=cmd[p+1];
        if(strcmp(sym,">>")==0){
            int fd=open(filename,O_CREAT | O_WRONLY | O_APPEND, 0666);
            if(fd==-1){
                char *msg = "Erreur open\n" ;
                if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                    perror("Erreur ecriture dans le shell") ;
            }
            dup2(fd, 1);
            close(fd);
        }else if(strcmp(sym,">|")==0){
            int fd=open(filename,O_CREAT | O_WRONLY | O_TRUNC, 0666);
            if(fd==-1){
                char *msg = "Erreur open\n" ;
                if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                    perror("Erreur ecriture dans le shell") ;
            }
            dup2(fd, 1);
            close(fd);

        }else if(strcmp(sym,">")==0){
            int fd = open(filename, O_WRONLY | O_CREAT | O_EXCL, 0666);
            if(fd<0) {
                val=1;
                char *msg="\001\033[36m\002Slash: error occured while executing this command\n\001\033[00m\002";
                write(STDERR_FILENO,msg,strlen(msg));            
            }
            else{
            dup2(fd, 1);
            close(fd);
        }
        }else if(strcmp(sym,"<")==0){
            int fd=open(filename,O_RDONLY);
            if(fd==-1){
                char *msg = "Erreur open\n" ;
                if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                    perror("Erreur ecriture dans le shell") ;
            }
            dup2(fd, 0);
            close(fd);
        }else if(strcmp(sym,"2>")==0){
            int fd=open(filename,O_WRONLY|O_CREAT|O_EXCL,0666);
            if(fd<0) {
                val=1;
                char *msg="\001\033[36m\002Slash: error occured while executing this command\n\001\033[00m\002";
                write(STDERR_FILENO,msg,strlen(msg));            
            }else{
            dup2(fd, 2);
            close(fd);
        }
        }else if(strcmp(sym,"2>|")==0){
            int fd=open(filename,O_CREAT | O_WRONLY | O_TRUNC, 0666);
            if(fd==-1){
                char *msg = "Erreur open\n" ;
                if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                    perror("Erreur ecriture dans le shell") ;
            }
            dup2(fd, 2);
            close(fd);
        }else if(strcmp(sym,"2>>")==0){
            int fd=open(filename,O_CREAT | O_WRONLY | O_APPEND, 0666);
            if(fd==-1){
                char *msg = "Erreur open\n" ;
                if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                    perror("Erreur ecriture dans le shell") ;
            }
            dup2(fd, 2);
            close(fd);
        }
        i++;
    }
}

//verifier s'il y a des redirections à faire
void treat(int length,char **cmd){
    char **tab=getPos(length,cmd);
    int cmp=0;
    while(tab[cmp]!=NULL && cmp<max){
        cmp++;
    }
    if(cmp==0){//s'il n y a pas de redirections, on execute la commande directement
        free(tab);
        executeCommand(length,cmd);
    }else{//sinon on traite les redirections
        int copy1=dup(0);
        int copy2=dup(1);
        int copy3=dup(2);
        int len=0;
        char **cmd2=calloc(max,sizeof(char)*max);
        if(cmd2==NULL){
            char *msg = "Erreur calloc\n" ;
            if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                perror("Erreur ecriture dans le shell") ;
        }
        for(int i=0;i<atoi(tab[0]);i++){
            cmd2[i]=cmd[i];
            len++;
        }
        treatRedirections(cmd,tab);
        if(val!=1){
        executeCommand(len,cmd2);
        free(cmd2);
        int k=0;
        while(tab[k]!=NULL && k<max){
            int a=atoi(tab[k]);
            if(strcmp(cmd[a],">")==0 || strcmp(cmd[a],">>")==0 || strcmp(cmd[a],">|")==0){
                dup2(copy2,1);
            }
            if(strcmp(cmd[a],"2>")==0 || strcmp(cmd[a],"2>>")==0 || strcmp(cmd[a],"2>|")==0){
                dup2(copy3,2);
            }
            if(strcmp(cmd[a],"<")==0){
                dup2(copy1,0);
            }
            k++;
        }
    }
        for(int a=0;a<cmp;a++) free(tab[a]);
        free(tab);
    }
}

int tabLen(char **tab){
    int i=0;
    while(tab[i]!=NULL && i<max) i++;
    return i; 
}

//traiter le cas de tubes ou pipelines
void treatMultiplesPipe(char ***cmd){
    int fd[2];
    pid_t pid;
    int fdd = 0;
    while (*cmd != NULL) {
        if (pipe(fd)==-1){
            char *msg = "Erreur pipe\n" ;
            if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                perror("Erreur ecriture dans le shell") ;
        }
        if ((pid = fork()) == -1) {
            char *msg = "Erreur fork\n" ;
            if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                perror("Erreur ecriture dans le shell") ;
            exit(1);
        }
        else if (pid == 0) {
            dup2(fdd, 0);
            if (*(cmd + 1) != NULL) {
                dup2(fd[1], 1);
            }
            close(fd[0]);
            int taille = tabLen(*cmd);
            treat(taille , *cmd) ;
            exit(val);
        }
        else {
            wait(NULL);
            close(fd[1]);
            fdd = fd[0];
            cmd++;
        }
    }
}


//verifier s'il y a des tubes
void treatPipes(int length , char **cmd){
    int p=ContainsPipes(length,cmd);
    if(p<0) treat(length,cmd); //s'il n y a pas de tubes, on passe directement à la redirections
    else{//sinon
        char** pos=getPipesPos(length,cmd);
        int cmp=0;
        while(pos[cmp]!=NULL && cmp<max){
            cmp++;
        }
        char *** tab=calloc(max,max*sizeof(char));
        if(tab == NULL){
            char *msg = "Erreur calloc\n" ;
            if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                perror("Erreur ecriture dans le shell") ;
            free(pos);
        }
        int i=0;
        int tablen=0;
        for(int st=0;st<cmp;st++){
            int a=atoi(pos[st]);
            char **cmd2=calloc(max,max*sizeof(char));
            if(tab == NULL){
                char *msg = "Erreur calloc\n" ;
                if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg)){
                    perror("Erreur ecriture dans le shell") ;
                }
                free(pos);
            }
            for(int j=i;j<a;j++){
                cmd2[j-i]=cmd[j];
            }
            tab[tablen]=cmd2;
            tablen++;
            i=a+1;
        }
        char **t=calloc(max,sizeof(char)*max);
        if(t ==NULL){
            char *msg = "Erreur calloc\n" ;
            if(write(STDERR_FILENO,msg,strlen(msg))< strlen(msg))
                perror("Erreur ecriture dans le shell") ;
            free(pos);
            free(tab);
        }
        int p2=atoi(pos[cmp-1])+1;
        int m=p2;
        while(cmd[m]!=NULL && m<max){
            t[m-p2]=cmd[m];
            m++;
        }
        tab[tablen]=t;
        treatMultiplesPipe(tab);
        for(int s=0;s<cmp;s++) free(pos[s]);
        free(pos);
        for(int t=0;t<tablen;t++) free(tab[t]);
        free(tab);
        free(t);
    }
}


int main(int argc,char **argv) {
    struct sigaction sa_ignore;
    sa_ignore.sa_handler = SIG_IGN;
    sigemptyset(&sa_ignore.sa_mask);
    sa_ignore.sa_flags = 0;

    sigaction(SIGINT, &sa_ignore, NULL);
    sigaction(SIGTERM, &sa_ignore, NULL);

    rl_outstream = stderr;

    if(argc > 1){
        perror("Error slash : too many arguments");
        return 1;
    }
    char **cmd;
    while(1) {
        char *prompt = buildPrompt ();
        char *line = readline(prompt);
        if(line!=NULL){
            if(strcmp(line, "") != 0){
                using_history();
                add_history (line);
                cmd = splitCommand(line); 
                free(prompt);
                int i=0;
                int j = 0 ;
                while(cmd[i]!= NULL) {
                    i++;
                }
                cmd = interpretCommand(i, cmd);
                while(cmd[j]!= NULL) {
                    j++;
                }
                treatPipes(j,cmd);
                int k=0;
                while(cmd[k]!=NULL){
                    free(cmd[k]);
                    k++;
                }
                free(cmd);
                free(line);
            }
        }else{
            printf("\n");
            f_exit(1,val,NULL);
        }
    }
}