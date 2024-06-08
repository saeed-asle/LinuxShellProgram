#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#define INPUTSIZE 511
pid_t pid;
pid_t child_pid;
//signals for ctrl z and for chaild withh parents
void sigtstp_handler(int sig) {
    if (child_pid > 0) {
        kill(pid, SIGTSTP);
    }
    else{
    }
}
void sigchld_handler(int sig) {
    int status;
    waitpid(-1, &status, WNOHANG);
}
int how_many_commands(char input[]){//get numbers of words in onput
    int count=1;
    char r;
    if(strstr(input,"|")){
        r='|';
    }
    else{
        r=';';
    } 
    for (int i = 0; input[i] != '\0'; i++){
        if (input[i] == r){
            count+=1;
        }    
    }
  return count;
}
int count_word(char* input){//get numbers of words in onput
    char* token;
    int count=0;
    char temp[strlen(input)+1];
    strcpy(temp,input);
    token=strtok(temp," ");
    while(token!=NULL){
        token=strtok(NULL," ");  
        count+=1;
    }
    token=NULL;
    return count;
}
int totalSizeLenOfCommands(char** arrWords){//get numbers of words in command
    int index=0;
    int count=0;
    while(arrWords[index]!=NULL){
        count+=strlen(arrWords[index]);
        index++;
    } 
    return index;
}
char*** take_part1(char* const input,int count){//split the commands
    char*** command=(char***)malloc(count*sizeof(char**));
    if(command==NULL){
       perror("ERR!!!\n");
       exit(1);
    }
    int c=0;int i=0;
    char* subtok1;
    char* subtok2;
    char* r;
    if(strstr(input,"|")){
        r="|\n";
    }
    else{
        r=";\n";
    }    
    char* tok = strtok_r(input, r, &subtok1);
    while (tok != NULL) {
        int x=count_word(tok);        
        if(strchr(tok,'\a')){
            char *e = strchr(tok, '\a');
            int index = (int)(e - tok);
               tok[index]=';' ;
        }

        command[i]=(char**)malloc((x)*sizeof(char*)+1);
        if(command[i]==NULL){
           perror("ERR!!!\n");
           exit(1);
        }
        if(strchr(tok,'"')!=NULL && strchr(tok,'$')==NULL){
            char* subtok = strtok_r(tok, " ",&subtok2);     
            command[i][c]=subtok;
            c+=1; 
            subtok = strtok_r(NULL, "\"",&subtok2);
            command[i][c]=subtok;
            subtok = strtok_r(NULL, "\"",&subtok2);
            while (subtok != NULL) {
                strcat(command[i][c],subtok);
                subtok = strtok_r(NULL, "\"",&subtok2); 
            }
            c+=1;
            subtok=NULL;
        }
        else{
            char* subtok = strtok_r(tok, " ",&subtok2);
            while (subtok != NULL) { 
                command[i][c]=subtok;
                subtok = strtok_r(NULL, " ",&subtok2);   
                c+=1;
            }
            subtok=NULL;
        }
        command[i][c]=NULL;
        c=0;
        i+=1;
        tok = strtok_r(NULL, r, &subtok1);
    }  
    subtok1=NULL;
    subtok2=NULL;
    tok=NULL;
    return command;
}
typedef struct {
    char *key;
    char *value;
} KeyValuePair;
int main (){
    signal(SIGTSTP, sigtstp_handler);
    signal(SIGCHLD, sigchld_handler);
    char input_check[INPUTSIZE+1]="";
    input_check[0]='\0';
    char ***res;
    char path [512];
    getcwd (path, 512);
    int number_of_ALLcommand=0;
    int number_of_total_ALLargs=0;
    int number_of_command=0;
    int number_of_total_args=0;
    int count_exit=1;
    int numPairs=1;
    int status;
    int type;
    KeyValuePair *pairs = malloc(numPairs * sizeof(KeyValuePair));
    if(pairs==NULL){
       perror("ERR!!!\n");
       exit(1);
    }
    while(1){
        printf ("#cmd:%d|#args:%d@%s>",number_of_ALLcommand,number_of_total_ALLargs,path);    
        while (fgets(input_check, INPUTSIZE+1, stdin) != NULL) {
            if (input_check[strlen(input_check) - 1] != '\n') {
                printf ("#cmd:%d|#args:%d@%s>",number_of_ALLcommand,number_of_total_args,path); 
                int c;
                while ((c = getchar()) != '\n' && c != EOF);
                continue;
            }
            break;
        }
        if(strstr(input_check,"|")){
            type=0;
        }
        else{
            type=1;
        }
        char c_string[512]  ;
        char new_string[512] = "";
        int inside_quotes = 0;
        strcpy(c_string,input_check);
        for (int i = 0; i < strlen(c_string); i++) {// If the current character is a quote, toggle the inside_quotes flag
            if (c_string[i] == '\"') {
                inside_quotes = !inside_quotes;
            }  
            if (c_string[i] == ';' && inside_quotes) {// If the current character is a semicolon and inside quotes, append "\a" to the new string
                strcat(new_string, "\a");
            }
            else { // Otherwise, append the current character to the new string
                char str[2];
                str[0] = c_string[i];
                str[1] = '\0';
                strcat(new_string, str);
            }
        }
        number_of_command=how_many_commands(new_string);
        res=take_part1(new_string,number_of_command);                             
        if(count_exit==3 ){
            for (int i = 0; i < numPairs-1; i++) {
                free(pairs[i].key);
                free(pairs[i].value);
            }
            pairs->value=NULL;
            pairs->key=NULL;
            free(pairs);
            for (int i = 0; i < number_of_command; i++) {
                free(res[i]);
            }
            free(res);
            res=NULL;
            break;
        }
        else if(strcmp(input_check, "\n") == 0){
            count_exit+=1;    
        }
        else{
            int back=0;
            int index=0;
            int output_fd;
            int pipes[2*(number_of_command-1)];
            if (type!=0){
                goto not_pipe;
            }
            for (int i = 0; i < number_of_command-1; i++) {
                if (pipe(pipes + i*2) < 0) {
                    perror("pipe");
                    exit(EXIT_FAILURE);
                }
            }
            int in = 0;
            int out = 0;
            not_pipe:
            count_exit=0;
            for(int i=0;i<number_of_command;i++){
                int place_check=totalSizeLenOfCommands(res[i]);
                int finshed=0;
                if(strcmp(res[i][0],"bg")==0){//execute thhat we stoped if there
                        if (child_pid != 0) {
                        printf("%d\n",child_pid);
                        kill(child_pid, SIGCONT);
                        child_pid = 0;
                        count_exit=0;

                    }
                    continue;
                }
                else if(strcmp(res[i][0],"cd")==0){//cd not supported
                    count_exit=0;
                    finshed=1;
                    printf("command not supported (Yet)\n");
                }
                else if(totalSizeLenOfCommands(res[i])>10){ 
                    finshed=1;
                    printf("ERR\n");
                }
                else if(strstr(res[i][0],"=")){//save in struct
                    count_exit=0;
                    finshed=1;
                    char* token;
                    char temp[strlen(res[i][0])+1];
                    char *e = strstr(res[i][0], "=");
                    int index = (int)(e - res[i][0]);
                    char key[index];
                    char value[strlen(res[i][0])-index];
                    strcpy(temp,res[i][0]);
                    token=strtok(temp,"=");
                    strcpy(key,token);
                    token=strtok(NULL,"\0");
                    strcpy(value,token);
                    int exit_key=0;
                    for (int j = 0; j < numPairs-1; j++) {
                        if(strcmp(pairs[j].key,key)==0){
                            exit_key=1;
                            pairs[j].value=strdup(value);
                            break;
                        }
                    }
                    if(exit_key==0){
                        pairs[numPairs-1].key=strdup(key);
                        pairs[numPairs-1].value=strdup(value);
                        numPairs+=1;
                        pairs = realloc(pairs, numPairs * sizeof(KeyValuePair));
                        if(pairs==NULL){
                            perror("ERR!!!\n");
                            exit(1);
                        }
                    }
                    token=NULL;
                    e=NULL;
                }
                if(strstr(res[i][place_check-1],"&")){//back groud order
                    if (strlen(res[i][place_check-1])==1){
                        res[i][place_check-1]=NULL;
                        back=2;
                    }
                    else{
                        res[i][place_check-1][strlen(res[i][place_check-1])-1]='\0';
                        back=2;
                    }
                }
                if(finshed==0){//for $ we have many possible ways thhat $ in string
                    count_exit=0;
                    int search =0;
                    while(search<place_check && res[i][search]!=NULL){
                        if(strstr(res[i][search],"$") && strlen(res[i][search])>1){
                            if(res[i][search][0]=='$' && strchr(res[i][search], '\"')!=NULL){
                                char *quote = strchr(res[i][search], '\"');
                                char *dollar_sign = strchr(res[i][search], '$');
                                int substring_length = quote - dollar_sign;
                                char substring[substring_length];
                                strncpy(substring, res[i][search], substring_length);
                                substring[substring_length] = '\0';
                                res[i][search][strlen(res[i][search])-1]='\0';
                                for(int j = 0; j < numPairs-1; j++) {
                                    if((strcmp(pairs[j].key,substring+1))==0){
                                        res[i][search]=strdup(strcat(pairs[j].value,res[i][search]+substring_length+1));
                                        finshed=1;
                                        break;
                                    }
                                }
                                if(finshed==1 ){
                                    finshed=0;
                                }
                                else{
                                    res[i][search]=strdup(res[i][search]+substring_length+1);
                                }
                            }
                            if(strchr(res[i][search], '$')!=NULL && strchr(res[i][search], '\"')!=NULL){
                                char *e = strstr(res[i][search], "$");
                                int index = (int)(e - res[i][search]);
                                char substring[index-1];
                                strncpy(substring, res[i][search], index-1);
                                substring[index-1] = '\0';
                                 for(int j = 0; j < numPairs-1; j++) {
                                    if((strcmp(pairs[j].key,res[i][search]+index+1))==0){
                                        res[i][search]=strdup(strcat(substring+1,pairs[j].value));
                                        finshed=1;
                                        break;
                                    }
                                }
                                if(finshed==1 ){
                                    finshed=0;
                                }
                                else{
                                    res[i][search]=strdup(substring+1);
                                }
                            } 
                            if(res[i][search][0]== '$'){
                                 for(int j = 0; j < numPairs-1; j++) {
                                    if(strcmp(pairs[j].key,res[i][search]+1)==0){
                                        res[i][search]=strdup(pairs[j].value);
                                        finshed=1;
                                        break;
                                    }
                                }
                                if(finshed==1 ){
                                    finshed=0;
                                }
                                else{
                                    res[i][search]=strdup(" ");
                                }
                            } 
                            else{
                                char *e = strstr(res[i][search], "$");
                                int index = (int)(e - res[i][search]);
                                char substring[index];
                                strncpy(substring, res[i][search], index);
                                substring[index] = '\0';
                                 for(int j = 0; j < numPairs-1; j++) {
                                    if((strcmp(pairs[j].key,res[i][search]+index+1))==0){
                                        res[i][search]=strdup(strcat(substring,pairs[j].value));
                                        finshed=1;
                                        break;
                                    }
                                }
                                if(finshed==1 ){
                                    finshed=0;
                                }
                                else{
                                    res[i][search]=strdup(substring);
                                }
                            }
                        }
                        search+=1;
                    }
                }
                if(finshed==0){//file order
                    for (int j = 0; j< totalSizeLenOfCommands(res[i]); j++){
                        if(strstr(res[i][j],">")){
                            if (strlen(res[i][j])==1){
                                res[i][j]=NULL;
                                back=1;
                                index=j+1;
                            }
                            else{
                                res[i][j]=res[i][j]+1;
                                index=j;
                                back=1;
                                
                            }
                        }  
                    } 
                } 
                if(finshed==0){//execute the commands or pipes
                    if (type==0){
                        if (i != 0) {
                            in = pipes[(i-1)*2];
                        }
                        if (i != number_of_command-1) {
                            out = pipes[i*2+1];
                        }
                    }
                    if(back==1){
                        if (strcmp(res[i][index]," ")==0){
                            continue;///
                        }
                        output_fd = open(res[i][index], O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                        if (output_fd == -1) {
                            printf("Error: could not open output file\n");
                            exit(1);
                        }
                    }
                    pid = fork();
                    if (pid == -1) {
                        perror("fork");
                        exit(EXIT_FAILURE);
                    }
                    else if(pid == 0){
                        if (type==0){
                            if(i!=0 ){
                                dup2(in, STDIN_FILENO);
                                close(in); 
                            } 
                            if(i!=number_of_command-1 ){
                                dup2(out, STDOUT_FILENO);
                                close(out);
                            }
                        }
                        if(back==1){
                            close(STDOUT_FILENO);
                            dup2(output_fd, STDOUT_FILENO);
                            close(output_fd);
                        }
                        execvp(res[i][0], res[i]);
                        perror("execvp"); 
                        exit(EXIT_FAILURE);        
                    } 
                    else if(back==1) {
                        close(output_fd);
                    }
                    else if(type==0) {
                        if (i != 0 ) {
                            close(in);
                        }
                        if (i != number_of_command-1 ) {
                            close(out);
                        }
                    }
                    number_of_ALLcommand+=1;
                    number_of_total_ALLargs+=totalSizeLenOfCommands(res[i]);
                }
                if (back==0 && finshed==0){//parents
                    waitpid(pid, &status, WUNTRACED);
                    if (WIFSTOPPED(status)) {
                        child_pid = pid;
                    }
                }
            }    
            for (int i = 0; i < number_of_command; i++) {//free memory
                free(res[i]);
            }
            free(res);
            res=NULL;
        }
    }         
    return 0;
}








