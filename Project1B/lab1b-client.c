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
#include <sys/socket.h>
#include <netinet/in.h>
#include <zlib.h> 
#include <netdb.h>
#include <fcntl.h>

//Flags
static int port_flag;
static int compress_flag;
static int log_flag;


//File descriptors
int logFD;
int socketFD;

char* crlf = "\r\n";
char newLine = '\n';

char readMe;
int buffer_size;
size_t char_size;

struct termios initialTermios;

const int SENT = 1;
const int RECEIVED = 0;

void badSystemCall(char* customMessage){
    fprintf(stderr, "%s: %s \n",customMessage,strerror(errno));
    exit(1);
}

//** TERMINAL SETTING AND RESETTING


void resetTerminal(){
    tcsetattr(STDIN_FILENO,TCSANOW,&initialTermios);
}

void changeTerminalMode(){
    tcgetattr(STDIN_FILENO,&initialTermios);

    //Modify specific attributes:
    atexit(resetTerminal); //revert to old state at end
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


void writeLog(char* data, int type){
    if(type == SENT){
         // SENT # BYTES: <data>
        char* SENTmsg = "SENT ";
        if(write(logFD,SENTmsg,strlen(SENTmsg))<0)
            badSystemCall("Error writing to log file");
    }
    else{ 
        // RECEIVED # BYTES: <data>
        char* RECEIVEDmsg = "RECEIVED ";
        if(write(logFD,RECEIVEDmsg,strlen(RECEIVEDmsg))<0)
            badSystemCall("Error writing to log file");
    }
    
    int bytes = strlen(data);
    char bytesString[256];
    sprintf(bytesString, "%d", bytes);

    if(write(logFD,bytesString,strlen(bytesString))<0)
        badSystemCall("Error writing to log file");


    char* BYTES = " BYTES: ";
    if(write(logFD,BYTES,strlen(BYTES))<0)
        badSystemCall("Error writing to file");


    if(write(logFD,data,bytes)<0)
        badSystemCall("Error writing to file");
    char* nl = "\n";
    if(write(logFD,nl,strlen(nl))<0)
        badSystemCall("Error writing to file");
}


void writeLogUnsigned(unsigned char* data, int type){
    if(type == SENT){
         // SENT # BYTES: <data>
        char* SENTmsg = "SENT ";
        if(write(logFD,SENTmsg,strlen(SENTmsg))<0)
            badSystemCall("Error writing to log file");
    }
    else{ 
        // RECEIVED # BYTES: <data>
        char* RECEIVEDmsg = "RECEIVED ";
        if(write(logFD,RECEIVEDmsg,strlen(RECEIVEDmsg))<0)
            badSystemCall("Error writing to log file");
    }
    
    int bytes = strlen((char *)data);
    char bytesString[256];
    sprintf(bytesString, "%d", bytes);

    if(write(logFD,bytesString,strlen(bytesString))<0)
        badSystemCall("Error writing to log file");


    char* BYTES = " BYTES: ";
    if(write(logFD,BYTES,strlen(BYTES))<0)
        badSystemCall("Error writing to file");


    if(write(logFD,data,bytes)<0)
        badSystemCall("Error writing to file");
    char* nl = "\n";
    if(write(logFD,nl,strlen(nl))<0)
        badSystemCall("Error writing to file");
}

void readAndWrite(){
    
    struct pollfd pollList[2];
    pollList[0].fd = STDIN_FILENO; //Keyboard
	pollList[1].fd = socketFD; //Socket
    pollList[0].events = POLLIN | POLLHUP | POLLERR;
    pollList[1].events = POLLIN | POLLHUP | POLLERR;

    int bytes_read;
    char buffer[buffer_size];
    int timeout = 0;
    while(1){

        if(poll(pollList,2,timeout) < 0)
            badSystemCall("failed to poll");
        
        //Keyboard input
        if(pollList[0].revents && POLLIN){ 
           
            bytes_read = read(STDIN_FILENO,buffer,char_size*buffer_size);
            if(bytes_read < 0){
                fprintf(stderr, "Error: failed to read from standard input\n");
                exit(1);
            }

            if(log_flag && bytes_read)
                writeLog(buffer, SENT);
            int i;
            for(i=0; i<bytes_read;i++){
                readMe = buffer[i];
                switch(readMe){
                    case '\04': //ctrl D, or EOF
                        write(STDOUT_FILENO, "^C", 2*char_size); //Echo
                        write(socketFD, &readMe, char_size); //write to server
                        break;
                    case '\03': //Interrupt ^C
                        write(STDOUT_FILENO, "^C", 2*char_size); //Echo
                        write(socketFD, &readMe, char_size); //write to server
                        break;
                    case '\n':
                    case '\r':
                        write(STDOUT_FILENO, crlf, 2*char_size);
                        write(socketFD,&newLine,char_size);
                        break;
                    default:
                        write(STDOUT_FILENO, &readMe, char_size); //echo to stdout
                        write(socketFD, &readMe, char_size); //write to server
                        break;
                }
            }
            memset(buffer,0,bytes_read);
        }
        
        //Server input
        if(pollList[1].revents && POLLIN){
            bytes_read = read(socketFD,buffer,char_size*buffer_size);
            if(bytes_read < 0){
                    fprintf(stderr, "Error: failed to read from shell\n");
                    exit(1);
            }
            if(log_flag && bytes_read)
                writeLog(buffer,RECEIVED);

            if(bytes_read == 0) //Socket is closed
                exit(0);
            int i;
            for(i=0; i <bytes_read; i++){
                readMe = buffer[i];
                
                switch(readMe){
                    case '\r':
                    case '\n':
                        write(STDOUT_FILENO,crlf,2*char_size);
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

z_stream toServer;
z_stream fromServer;

void startCompression(){
    toServer.zalloc = Z_NULL;
    toServer.zfree = Z_NULL;
    toServer.opaque = Z_NULL;
    toServer.avail_out = buffer_size;


    if(deflateInit(&toServer,Z_DEFAULT_COMPRESSION) != Z_OK){
        badSystemCall("Error setting up z stream");
        exit(1);
    }
}

void endCompression(){
    fromServer.zalloc = Z_NULL;
    fromServer.zfree = Z_NULL;
    fromServer.opaque = Z_NULL;
    fromServer.avail_out = buffer_size;


    if(inflateInit(&fromServer) != Z_OK){
        fprintf(stderr, "Eror setting up z stream");
        exit(1);
    }
}

void compressedReadAndWrite(){

  
    struct pollfd pollList[2];
    pollList[0].fd = STDIN_FILENO; //Keyboard
	pollList[1].fd = socketFD; //Socket
    pollList[0].events = POLLIN | POLLHUP | POLLERR;
    pollList[1].events = POLLIN | POLLHUP | POLLERR;

    int bytes_read;
    unsigned char buffer[buffer_size];
    unsigned char compressedBuffer[buffer_size];
    unsigned char uncompressedBuffer[buffer_size];
    int timeout = 0;
    while(1){


        if(poll(pollList,2,timeout) < 0)
            badSystemCall("failed to poll");
        
        //Keyboard input
        if(pollList[0].revents && POLLIN){ 
           
            bytes_read = read(STDIN_FILENO,buffer,char_size*buffer_size);
            if(bytes_read < 0){
                fprintf(stderr, "Error: failed to read from standard input\n");
                exit(1);
            }

            
            startCompression();

            toServer.avail_in = bytes_read;
            toServer.next_in = buffer;

            toServer.next_out = compressedBuffer;

            do{
                if(deflate(&toServer, Z_SYNC_FLUSH) != Z_OK){
                    fprintf(stderr, "Eror deflating compression stream to server");
                    exit(1);
                }
            } 
            while(toServer.avail_in > 0);
            
            int numCompressed = buffer_size - toServer.avail_out;
            write(socketFD,&compressedBuffer,numCompressed);

            if(log_flag && bytes_read) 
                writeLogUnsigned(compressedBuffer, SENT);

            deflateEnd(&toServer);

            
            int i;
            for(i=0; i<bytes_read;i++){
                readMe = buffer[i];
                switch(readMe){
                    case '\04': //ctrl D, or EOF
                        write(STDOUT_FILENO, "^C", 2*char_size); //Echo
                        break;
                    case '\03': //Interrupt ^C
                        write(STDOUT_FILENO, "^C", 2*char_size); //Echo
                        break;
                    case '\n':
                    case '\r':
                        write(STDOUT_FILENO, crlf, 2*char_size);
                        break;
                    default:
                        write(STDOUT_FILENO, &readMe, char_size); //echo to stdout
                        break;
                }
            }
            memset(buffer,0,bytes_read);
            memset(compressedBuffer,0,numCompressed);
        }
        
        //Server input
        if(pollList[1].revents && POLLIN){
            bytes_read = read(socketFD,buffer,char_size*buffer_size);
            if(bytes_read < 0){
                    fprintf(stderr, "Error: failed to read from shell\n");
                    exit(1);
            }

            if(bytes_read == 0) //Socket is closed
                exit(0);


            endCompression();

            fromServer.avail_in = bytes_read;
            fromServer.next_in = buffer;
            fromServer.next_out = uncompressedBuffer;

            if(log_flag && bytes_read)
                writeLogUnsigned(buffer, RECEIVED);

            do{
                if(inflate(&fromServer, Z_SYNC_FLUSH) != Z_OK){
                    fprintf(stderr, "Error inflating buffer that reads from the server");
                    exit(1);
                }
            } while(fromServer.avail_in > 0);

            inflateEnd(&fromServer);

            int numUncompressed = buffer_size - fromServer.avail_out;
            int i;
            for(i=0; i < numUncompressed; i++){
                readMe = uncompressedBuffer[i];
                
                switch(readMe){
                    case '\r':
                    case '\n':
                        write(STDOUT_FILENO,crlf,2*char_size);
                        break;
                    default:
                        write(STDOUT_FILENO,&readMe,char_size);
                        break;
                }
            }
            memset(buffer, 0, bytes_read);
            memset(uncompressedBuffer,0,numUncompressed);

        }
            if(pollList[1].revents & (POLLERR | POLLHUP))
    	        exit(0);
    }



}



int main(int argc, char *argv[]){
    
    buffer_size = 256;
    char_size = sizeof(char);

    port_flag = 0;
    compress_flag = 0;
    log_flag = 0;
    char* portOfServer;
    char* logFile;

    char* correctUsage = "Usage: lab1b-client [--port][--log][--compress]\n";

    static const struct option long_options[] = {
        {"port", required_argument, NULL, 'p'},
        {"log", required_argument, NULL, 'l'},
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
            case 'l':
                log_flag=1;
                logFile=optarg;
                break;
            case 'c':
                compress_flag=1;
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

    if(log_flag){
        logFD = creat(logFile,0666);
        if(logFD < 0){
            badSystemCall("Error opening/creating log file:");
        }
    }

    struct hostent* server;
    struct sockaddr_in serverAddress;

    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if(socketFD < 0)
        badSystemCall("Error opening socket: ");
    
    server = gethostbyname("127.0.0.1");
    if(!server)
        badSystemCall("Error finding host server: ");
    
    bzero((char *) &serverAddress, sizeof(serverAddress));    
    serverAddress.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serverAddress.sin_addr.s_addr, server->h_length);
    serverAddress.sin_port = htons(atoi(portOfServer));

    if(connect(socketFD, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0)
        badSystemCall("Error connecting to server");

    changeTerminalMode();

    if(compress_flag)
        compressedReadAndWrite();
    else
        readAndWrite();

    exit(0);
}