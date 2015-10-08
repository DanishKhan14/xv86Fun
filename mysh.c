#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>


typedef struct process{
                 int id;
                 char command[512];
}process;


static int IDcounter = 0;
static process* lis;
static FILE* input;
static int output;
static char str[1024];


//exclamation
char* exclamation(int i) {
    i = i-1;
	if(i < IDcounter && i > IDcounter-20 && i > -1){
        return lis[i % 20].command;
        
	}
	else{
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
    }
    return "";
}

//truncate white scpase
char *truncate_space(char *s)
{
    char *end;
    
    while(*s ==' '){
        s++;
    }
    
    end = s + strlen(s) - 1;
    if(!*end == ' '){
        return s;
    }
    while(end > s && *end == ' '){
        end--;
    }
    
    *(end+1) = '\0';
    return s;
}


//history
void history(){
    int i;
    if(IDcounter < 20){
        for(i = 0; i < IDcounter; i++){
            sprintf(str,"%d %s\n",lis[i].id,lis[i].command);
            write(1,str,strlen(str));
        }
    }
    else{
        int startId = (IDcounter) % 20;
        for(i = 0; i < 20; i++  ){
            sprintf(str,"%d %s\n",lis[startId % 20].id,lis[startId % 20].command);
            write(1,str,strlen(str));
            startId++;
        }
    }

}

int buildin(char * s){
    int rt = 0;
    //exit
    if(strcmp("exit",s) == 0){
        rt = 1;
        exit(0);
    }
    
    //hisotry
    if(strcmp("history",s) == 0){
        rt = 1;
        history();
    }
    
    //exclamation
    if (*s == '!') {
        rt = 1;
        int id  = 0;
        IDcounter--;
        if (IDcounter < 0) {
            IDcounter = 0;
        }
        
        if (*(s+1) == '\0') {
            id = IDcounter;
            char* redo = exclamation(id);
            buildin(redo);
        }
        else{
            id = atoi(s+1);
            char* redo = exclamation(id);
            buildin(redo);
        }
    }
    return rt;
    
}



int main (int argc, char* argv[]) {
    lis = (process *)malloc(20*sizeof(process));
    int batch = 0;
    input = stdin;
    output = STDOUT_FILENO;
    
    if(argc == 2){
        batch = 1;
        input = fopen(argv[1], "r");
        if (input == NULL) {
            char error_message[30] = "An error has occurred\n";
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);
        }
    }
    if (argc > 2) {
        char error_message[30] = "An error has occurred\n";
        write(STDERR_FILENO, error_message, strlen(error_message));
    }
    
    
    
    
    while(1){
        
        
        char userInput[512];
        process* temp;
        
        temp = (process* ) malloc(sizeof(process));
        //printf("%s", "mysh # ");
        if (batch < 1) {
            write(1, "mysh # ", strlen("mysh # "));
        }
        
        //scanf("%[^\n]",userInput);
        
        if(fgets(userInput, sizeof(userInput), input) == NULL){
            if(feof(input)){
                break;
            }else{
                char error_message[30] = "An error has occurred\n";
                write(STDERR_FILENO, error_message, strlen(error_message));
            }
        }
        
        if(batch == 1){
            write(1, userInput, strlen(userInput));
        }
        
        //fgets(userInput, sizeof(userInput), stdin);
        
        //truncate the newline character
        size_t ln = strlen(userInput) - 1;
        if (userInput[ln] == '\n')
            userInput[ln] = '\0';
        
        //store in process history
        temp->id = IDcounter+1;
        strcpy(temp->command,userInput);
        lis[IDcounter%20] = *temp;
        IDcounter++;
        
        //truncate space in front and after
        
        char* trim_input = truncate_space(userInput);
        
        
        
        //printf("%d",strcmp("history\n",userInput));
        
        if(buildin(trim_input)==0){
            int flag = 1;
            char * filename;
            char * ptr;
            char * star;
            star = trim_input;
            ptr = trim_input;
            int redirect = 0;
            
            int j;
            
            for (j = 0; j < strlen(trim_input); j++) {
                if (*ptr == ' ' && *(ptr-1) != ' ') {
                    flag++;
                }
                if (*ptr == '>') {
                    filename = ptr+1;
                    filename = truncate_space(filename);
                    redirect = 1;
                    *ptr = '\0';
                    ptr = strlen(trim_input);
                }
                ptr++;
                
            }
            
            if (redirect == 1) {
                    FILE* new = open(filename, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);
                    fflush(stdout);
                    int b = dup(1);
                    dup2(new, 1);
                    //close(new);
            }
            
            trim_input = truncate_space(userInput);
            star = trim_input;
            ptr = trim_input+1;
	    int q;
            for (q = 0; q < strlen(trim_input); q++) {
                if (*ptr == ' ' && *(ptr-1) != ' ') {
                    flag++;
                }
                ptr++;
                
            }
            
            ptr = trim_input+1;
            pid_t child_pid;
            char* child_argv[flag+1];
            int k = 0;
            int strlength = strlen(trim_input);
            
            for (ptr; ptr < strlength+trim_input+1; ptr++) {
                //printf("%d", strlen(trim_input));
                if(*ptr != '>'){
                    if (*ptr == ' ' && (*(ptr-1) != ' ') && (*(ptr-1) != '\0')) {
                        *ptr = '\0';
                        child_argv[k] = star;
                        //printf("%s", star);
                        star = ptr+1;
                        k++;
                    }
                    else if (*ptr == '\0') {
                        //printf("/%s", star);
                        child_argv[k] =  star;
                    }
                }
            }
            
            child_argv[flag] = NULL;
            
            
            
            
            int status;
        
            if((child_pid = fork())< 0){
                char error_message[30] = "An error has occurred\n";
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(1);
            }
            else if(child_pid == 0){
                if(execvp(child_argv[0], child_argv) < 0 ){
                    char error_message[30] = "An error has occurred\n";
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    exit(1);
                }
            }
            else{
                wait(&status);
            }
        
        }
                           
    }
    return 0;
}
