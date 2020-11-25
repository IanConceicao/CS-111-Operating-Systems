//NAME: Ian Conceicao
//EMAIL: IanCon234@gmail.com
//ID: 505153981

#include<stdio.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/wait.h>
#include <poll.h>

static int shell_flag = 0;
char *shellPath;

struct termios initialTermios;

pid_t pid;
int childToParent[2];//0 is read end, 1 is write end
int parentToChild[2];

char crlf[2] = {'\r','\n'};
int buffer_size = 256;
char ctrlC[2] = {'^', 'C'};
char ctrlD[2] = {'^', 'D'};
char newLine = '\n';
size_t char_size;


void fixTermios(){
    tcsetattr(STDIN_FILENO,TCSANOW,&initialTermios);
    if(shell_flag){
        int status;
        if(waitpid(pid, &status,0) < 0){
            fprintf(stderr, "Error: failure on waiting for state change of the shell");
            exit(1);
        }
        if(WIFEXITED(status)){
            int statusCode = WEXITSTATUS(status);
            int signalCode = status & 0x7f;
            fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", statusCode, signalCode);
        }

    }
}

void setUpChild(){
    
    close(parentToChild[1]); //Close write of parent to child
    close(childToParent[0]); //Close read of child to parent

    close(STDIN_FILENO);
    dup(parentToChild[0]);

    close(STDOUT_FILENO); 
    dup(childToParent[1]);

    close(STDERR_FILENO); 
    dup(childToParent[1]);

    close(parentToChild[0]); 
    close(childToParent[1]);
    
    char *argvector[2] = {shellPath, NULL};
    if(execvp(shellPath,argvector) < 0){
        fprintf(stderr,"Error: failed to exec child process as a shell\n");
        exit(1);
    }
}

void runParent(){
    close(parentToChild[0]);
    close(childToParent[1]);

    struct pollfd pollList[2];
    pollList[0].fd = STDIN_FILENO; //Keyboard
	pollList[1].fd = childToParent[0]; //Child input, input from shell
    pollList[0].events = POLLIN | POLLHUP | POLLERR;
    pollList[1].events = POLLIN | POLLHUP | POLLERR;

    int bytes_read;
    char buffer[buffer_size];
    char readMe;
    int timeout = 0;
    while(1){

        if(poll(pollList,2,timeout) < 0){
            fprintf(stderr, "Error: failed to poll\n");
            exit(1);
        }

        //Keyboard input
        if(pollList[0].revents && POLLIN){ 
           
            bytes_read = read(STDIN_FILENO,buffer,char_size*buffer_size);
            if(bytes_read < 0){
                fprintf(stderr, "Error: failed to read from standard input\n");
                exit(1);
            }

            for(int i=0; i<bytes_read;i++){
                readMe = buffer[i];
                switch(readMe){
                    case '\04': //ctrl D, or EOF
                        write(STDOUT_FILENO, &ctrlD, 2*char_size);
                        close(parentToChild[1]);
                        break;
                    case '\03': //Interrupt
                        write(STDOUT_FILENO, &ctrlC, 2*char_size);
                        if(kill(pid, SIGINT) < 0){
                            fprintf(stderr, "Error: Failed to kill process\n");
                            exit(1);
                        }
                        break;
                    case '\n':
                    case '\r':
                        write(STDOUT_FILENO, &crlf, 2*char_size);
                        write(parentToChild[1],&newLine,char_size);
                        break;
                    default:
                        write(STDOUT_FILENO, &readMe, char_size); //echo to stdout
                        write(parentToChild[1], &readMe, char_size); //write to shell
                        break;
                }
            }
            memset(buffer,0,bytes_read);
        }
        
        //Shell input
        if(pollList[1].revents && POLLIN){
            bytes_read = read(childToParent[0],buffer,char_size*buffer_size);
            if(bytes_read < 0){
                    fprintf(stderr, "Error: failed to read from shell\n");
                    exit(1);
            }

            for(int i=0; i <bytes_read; i++){
                readMe = buffer[i];
                
                switch(readMe){
                    case '\n':
                    write(STDOUT_FILENO,&crlf,2*char_size);
                    break;
                default:
                    write(STDOUT_FILENO,&readMe,char_size);
                    break;
                }
            }
            memset(buffer, 0, bytes_read);
        }
            if(pollList[1].revents & (POLLERR | POLLHUP))
    	        exit(0);
    }
    
}


void changeTerminalMode(){
    tcgetattr(STDIN_FILENO,&initialTermios);

    //Modify specific attributes:
    atexit(fixTermios); //revert to old state at end
    struct termios newState;
    tcgetattr(STDIN_FILENO,&newState);
    newState.c_iflag = ISTRIP;	/* only lower 7 bits	*/
	newState.c_oflag = 0;		/* no processing	*/
	newState.c_lflag = 0;		/* no processing	*/

    if(tcsetattr(0, TCSANOW,&newState) < 0){
        fprintf(stderr, "Error: Failed to set new terminal state\n");
        exit(1);
    }
}

void readCharacters(){

    char buffer[buffer_size];
    int bytes_read;

    do{
        bytes_read = read(STDIN_FILENO,buffer,char_size*buffer_size);
        if(bytes_read < 0){
            fprintf(stderr, "Error: failed to read from standard input\n");
            exit(1);
        }

        for(int i=0; i<bytes_read;i++){
            char readMe = buffer[i];
            switch(readMe){
                case '\04': //ctrl D
                    write(STDOUT_FILENO, &ctrlD, 2*char_size);
                    exit(0);
                    break;
                case '\r':
                case '\n':
                    write(STDIN_FILENO, &crlf, 2*char_size);
                    break;
                default:
                    write(STDOUT_FILENO, &buffer, char_size);
            }
        }
        memset(buffer,0,bytes_read);
    }
    while(bytes_read);
}

int main(int argc, char *argv[]){

    static struct option my_options[] = {
        {"shell", required_argument, NULL, 's'},
        {0,0,0,0}
    };

    int option_index;

    char c;
    while(1){
        c = getopt_long(argc,argv,"",my_options,&option_index);
        if(c==-1)
            break;
        else if(c=='s'){
            shell_flag=1;
            shellPath=optarg;
        }
        else{
            fprintf(stderr,"Error: unrecognized argument(s)\n");
            exit(1);
        }

    }
    
    char_size = sizeof(char);

    changeTerminalMode();

   if(pipe(childToParent) < 0 && shell_flag){
        fprintf(stderr,"Error: could not create pipes\n");
        exit(1);
    }
    if(pipe(parentToChild) < 0 && shell_flag){
        fprintf(stderr,"Error: could not create pipes\n");
        exit(1);
    }

    if(shell_flag){
        
        pid = fork();
        if(pid < 0){
            fprintf(stderr, "Error: Failed to fork the process\n");
            exit(1);
        }
        else if(pid==0){ //We are the child
            setUpChild();
        }
        else{//We are the parent
            runParent();
        }
    }
    else
        readCharacters();
    exit(0);
}