//NAME: Ian Conceicao
//EMAIL: IanCon234@gmail.com
//ID: 505153981

#define _GNU_SOURCE
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
#include <sys/socket.h>
#include <netinet/in.h>
//#include <zlib.h> Need to download this package
#include <netdb.h>
#include <fcntl.h>
#include <zlib.h> 
#include <stdio.h>

//Flags
static int port_flag;
static int compress_flag;

char* crlf = "\r\n";
char* ctrlC = "^C";
char* ctrlD = "^D";
char newLine = '\n';
int buffer_size;
size_t char_size;

char readMe;

int socketFD;

struct termios initialTermios;

const int SENT = 1;
const int RECEIVED = 0;

pid_t pid;
int childToParent[2];//0 is read end, 1 is write end
int parentToChild[2];

static int shell_flag;
char *shellPath;

void badSystemCall(char* customMessage){
    fprintf(stderr, "%s: %s \n",customMessage,strerror(errno));
    exit(1);
}

z_stream toClient;
z_stream fromClient;

void startCompression(){
    toClient.zalloc = Z_NULL;
    toClient.zfree = Z_NULL;
    toClient.opaque = Z_NULL;
    toClient.avail_out = buffer_size;


    if(deflateInit(&toClient,Z_DEFAULT_COMPRESSION) != Z_OK){
        badSystemCall("Error setting up z stream");
        exit(1);
    }
}

void endCompression(){
    fromClient.zalloc = Z_NULL;
    fromClient.zfree = Z_NULL;
    fromClient.opaque = Z_NULL;
    fromClient.avail_out = buffer_size;


    if(inflateInit(&fromClient) != Z_OK){
        fprintf(stderr, "Eror setting up z stream");
        exit(1);
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


void readCharacters(){

    char buffer[buffer_size];
    int bytes_read;

    do{
        bytes_read = read(socketFD,buffer,char_size*buffer_size);
        if(bytes_read < 0){
            fprintf(stderr, "Error: failed to read from standard input\n");
            exit(1);
        }
        int i;
        for(i=0; i<bytes_read;i++){
            readMe = buffer[i];
            switch(readMe){
                case '\04': //ctrl D
                    write(socketFD, ctrlD, 2*char_size);
                    exit(0);
                    break;
                case '\03': //ctrl C
                    write(socketFD, ctrlC, 2*char_size);
                    exit(0);
                    break;
                case '\r':
                case '\n':
                    write(socketFD, crlf, 2*char_size);
                    break;
                default:
                    write(socketFD, buffer, char_size);
            }
        }
        memset(buffer,0,bytes_read);
    }
    while(bytes_read);
}

void compressedReadCharacters(){

    unsigned char buffer[buffer_size];
    int bytes_read;
    unsigned char compressedBuffer[buffer_size];
    unsigned char uncompressedBuffer[buffer_size];

    do{
        bytes_read = read(socketFD,buffer,char_size*buffer_size);
        if(bytes_read < 0){
            fprintf(stderr, "Error: failed to read from standard input\n");
            exit(1);
        }

        endCompression();

        fromClient.avail_in = bytes_read;
        fromClient.next_in = buffer;
        fromClient.next_out = uncompressedBuffer;

        do{
            if(inflate(&fromClient, Z_SYNC_FLUSH) != Z_OK){
                fprintf(stderr, "Error inflating buffer that reads from the client stream");
                exit(1);
            }
        } while(fromClient.avail_in > 0);

        inflateEnd(&fromClient);

        int numUncompressed = buffer_size - fromClient.avail_out;

        char toWrite[256] = "";
        int i;
        for(i=0; i<numUncompressed;i++){
            readMe = uncompressedBuffer[i];
            switch(readMe){
                case '\04': //ctrl D
                    strcat(toWrite, ctrlD);
                    //write(socketFD, ctrlD, 2*char_size);
                    exit(0);
                    break;
                case '\03': //ctrl C
                    strcat(toWrite, ctrlC);
                   // write(socketFD, ctrlC, 2*char_size);
                    exit(0);
                    break;
                case '\r':
                case '\n':
                    strcat(toWrite, crlf);
                    //write(socketFD, crlf, 2*char_size);
                    break;
                default:
                    strcat(toWrite, &readMe);
                    //write(socketFD, buffer, char_size);
                    break;
            }
        }


        startCompression();

        toClient.avail_in = sizeof(toWrite);
        toClient.next_in = (unsigned char*)toWrite;

        toClient.next_out = compressedBuffer;

        do{
            if(deflate(&toClient, Z_SYNC_FLUSH) != Z_OK){
                fprintf(stderr, "Eror deflating compression stream to server");
                exit(1);
            }
        } 
        while(toClient.avail_in > 0);
        
        int numCompressed = buffer_size - toClient.avail_out;
        write(socketFD,&compressedBuffer,numCompressed);

        deflateEnd(&toClient);

        memset(buffer,0,bytes_read); //pre compressed stuff
        memset(uncompressedBuffer,0,numUncompressed); //Uncompressed stuff
        memset(compressedBuffer,0,numCompressed); //Compressed to send again back

    }
    while(bytes_read);
}


void runParent(){
    close(parentToChild[0]);
    close(childToParent[1]);

    struct pollfd pollList[2];
    pollList[0].fd = socketFD; //socket
	pollList[1].fd = childToParent[0]; //Child input, input from shell
    pollList[0].events = POLLIN | POLLHUP | POLLERR;
    pollList[1].events = POLLIN | POLLHUP | POLLERR;

    int bytes_read;
    char buffer[buffer_size];
    int timeout = 0;
    while(1){

        if(poll(pollList,2,timeout) < 0){
            fprintf(stderr, "Error: failed to poll\n");
            exit(1);
        }

        //Keyboard input
        if(pollList[0].revents && POLLIN){ 
            bytes_read = read(socketFD,buffer,char_size*buffer_size);
            if(bytes_read < 0)
                badSystemCall("Failed to read from standard input");
            int i;
            for(i=0; i<bytes_read;i++){
                readMe = buffer[i];
                fprintf(stdout,"Server is reading from client: %c \n", readMe);

                switch(readMe){
                    case '\04': //ctrl D, or EOF
                        close(parentToChild[1]);
                        break;
                    case '\03': //Interrupt ctrl C
                        if(kill(pid, SIGINT) < 0){
                            fprintf(stderr, "Error: Failed to kill process\n");
                            exit(1);
                        }
                        break;
                    case '\n':
                    case '\r':
                        write(parentToChild[1],&newLine,char_size);
                        break;
                    default:
                        //(parentToChild[1],"ls\n",4);
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
            int i;
            for(i=0; i <bytes_read; i++){
                readMe = buffer[i];
                fprintf(stdout,"Server is reading from the shell: %c \n", readMe);

                switch(readMe){
                    case '\r':
                    case '\n':
                    write(socketFD,crlf,2*char_size);
                    break;
                default:
                    write(socketFD,&readMe,char_size);
                    break;
                }
            }
            memset(buffer, 0, bytes_read);
        }
            if(pollList[1].revents & (POLLERR | POLLHUP))
    	        exit(0);
    }
    
}


void compressedRunParent(){
    close(parentToChild[0]);
    close(childToParent[1]);

    struct pollfd pollList[2];
    pollList[0].fd = socketFD; //socket
	pollList[1].fd = childToParent[0]; //Child input, input from shell
    pollList[0].events = POLLIN | POLLHUP | POLLERR;
    pollList[1].events = POLLIN | POLLHUP | POLLERR;

    int bytes_read;
    char unsigned buffer[buffer_size];
    unsigned char compressedBuffer[buffer_size];
    unsigned char uncompressedBuffer[buffer_size];

    int timeout = 0;
    while(1){

        if(poll(pollList,2,timeout) < 0){
            fprintf(stderr, "Error: failed to poll\n");
            exit(1);
        }

        //Keyboard input
        if(pollList[0].revents && POLLIN){ 
            bytes_read = read(socketFD,buffer,char_size*buffer_size);
            if(bytes_read < 0)
                badSystemCall("Failed to read from standard input");

            endCompression();

            fromClient.avail_in = bytes_read;
            fromClient.next_in = buffer;
            fromClient.next_out = uncompressedBuffer;

            do{
                if(inflate(&fromClient, Z_SYNC_FLUSH) != Z_OK){
                    fprintf(stderr, "Error inflating buffer that reads from the client stream");
                    exit(1);
                }
            } while(fromClient.avail_in > 0);

            inflateEnd(&fromClient);

            int numUncompressed = buffer_size - fromClient.avail_out;
            int i;
            for(i=0; i<numUncompressed;i++){
                    readMe = uncompressedBuffer[i];

                    switch(readMe){
                        case '\04': //ctrl D, or EOF
                            close(parentToChild[1]);
                            break;
                        case '\03': //Interrupt ctrl C
                            if(kill(pid, SIGINT) < 0){
                                fprintf(stderr, "Error: Failed to kill process\n");
                                exit(1);
                            }
                            break;
                        case '\n':
                        case '\r':
                            write(parentToChild[1],&newLine,char_size);
                            break;
                        default:
                            //(parentToChild[1],"ls\n",4);
                            write(parentToChild[1], &readMe, char_size); //write to shell
                            break;
                    }
                }
                memset(uncompressedBuffer,0,numUncompressed); //Uncompressed stuff
                memset(buffer,0,bytes_read);
        }
        
        //Shell input
        if(pollList[1].revents && POLLIN){
            bytes_read = read(childToParent[0],buffer,char_size*buffer_size);

            if(bytes_read < 0){
                    fprintf(stderr, "Error: failed to read from shell\n");
                    exit(1);
            }

            char toWrite[256] = "";
            int i;
            for(i=0; i <bytes_read; i++){ //change 1 to bytes_read
                readMe = buffer[i];
                fprintf(stdout,"Server is reading from the shell: %c \n", readMe);

                switch(readMe){
                    case '\r':
                    case '\n':
                    //write(socketFD,crlf,2*char_size);
                    strcat(toWrite,crlf);
                    break;
                default:
                    //write(socketFD,&readMe,char_size);
                    strcat(toWrite,&readMe);
                    break;
                }
            }


            startCompression();

            toClient.avail_in = sizeof(toWrite);

            toClient.next_in = (unsigned char*)toWrite;

            toClient.next_out = compressedBuffer;

            do{
                if(deflate(&toClient, Z_SYNC_FLUSH) != Z_OK){
                    fprintf(stderr, "Error deflating compression stream to server");
                    exit(1);
                }
            } 
            while(toClient.avail_in > 0);
            
            int numCompressed = buffer_size - toClient.avail_out;
            write(socketFD,&compressedBuffer,numCompressed);

            deflateEnd(&toClient);
            memset(compressedBuffer,0,numCompressed); //Compressed to send again back
            memset(buffer, 0, bytes_read);
        }

        if(pollList[1].revents & (POLLERR | POLLHUP))
            exit(0);

    }




}

void End()
{
    if(shell_flag){
        int shellStatus = 0;

        if (waitpid(pid, &shellStatus, 0) < 0)
            badSystemCall("Error waiting for the shell");
        
        int signalCode = shellStatus & 0x7f;
        int statusCode = WEXITSTATUS(shellStatus);
        fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", signalCode, statusCode);
    }
    close(parentToChild[1]);
    close(childToParent[0]);

	close(socketFD);
}

int main(int argc, char* argv[]){
    atexit(End);
    buffer_size = 256;
    char_size = sizeof(char);

    port_flag = 0;
    compress_flag = 0;
    shell_flag = 0;
    char* portOfServer;

    char* correctUsage = "Usage: lab1b-server [--port][--compress]\n";

    static const struct option long_options[] = {
        {"port", required_argument, NULL, 'p'},
        {"shell", required_argument, NULL, 's'},
        {"compress", no_argument, NULL, 'c'},
        {0, 0, 0, 0}
    };

    char c;
    int optionIndex=0;
    while(1){
        c = getopt_long(argc,argv,"",long_options,&optionIndex);

        if(c==-1)
            break;
        
        switch(c){
            case 'p':
                port_flag=1;
                portOfServer=optarg;
                break;
            case 'c':
                compress_flag=1;
                break;
            case 's':
                shell_flag=1;
                shellPath=optarg;
                break;
            default:
                fprintf(stderr, "%s \n", correctUsage);
                exit(1);
        }
    }


   if(!port_flag){
        fprintf(stderr, "Error: --port argument is required. \n");
        fprintf(stderr, "%s \n", correctUsage);
        exit(1);
    }

    int socketFDTemp = socket(AF_INET, SOCK_STREAM, 0);
    if(socketFDTemp < 0)
        badSystemCall("Failed to open socket");

    //Server stuff
    struct sockaddr_in serverAddress;
    struct sockaddr_in clientAddress;
    bzero((char *) &serverAddress, sizeof(serverAddress));    
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(atoi(portOfServer));
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if(bind(socketFDTemp, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0)
        badSystemCall("Failed to bind socket");
    
    listen(socketFDTemp, 5);
    socklen_t  clientLength = sizeof(clientAddress);

    socketFD = accept(socketFDTemp, (struct sockaddr *) &clientAddress, &clientLength);
    if(socketFD < 0)
        badSystemCall("Failed to accept client");
    

    //Pipes
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
        if(pid < 0)
            badSystemCall("Failed to fork process");
        
        else if(pid==0){ //We are the child
            setUpChild();
        }
        else{//We are the parent
            if(compress_flag)
                compressedRunParent();
            else
                runParent(); //Talks to shell
        }
    }
    else{
        if(compress_flag)
            compressedReadCharacters();
        else
            readCharacters(); //Not shell, just spits back
    }

    close(socketFDTemp);
    exit(0);
}