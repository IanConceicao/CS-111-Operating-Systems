// Ian Conceicao
// 505153981
// IanCon234@gmail.com

#include<stdio.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stddef.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

void catch(){
	fprintf(stderr, "Error: Segmentation fault caught, exiting \n");
	exit(4);
}

int main(int argc, char *argv[]){
    static int input_flag = 0;
    static int output_flag = 0;
	static int segfault_flag = 0;
	static int catch_flag = 0;

    struct option long_options[] =
    {
        {"input",    required_argument, &input_flag,    1},
        {"output",   required_argument, &output_flag,   1},
        {"segfault", no_argument,       &segfault_flag, 1},
        {"catch",    no_argument,       &catch_flag,    1},
        {0,          0,                  0,             0}
    };

    char* output_file;
    char* input_file;

    int option_index = 0;
    int c;

    while(1){
        c = getopt_long(argc, argv, "", long_options, &option_index);

        if(c == -1){
            break;
        }
        if(c != 0){
            fprintf(stderr, "Error: Invalid arguments. Correct usage: [--input<file-name>] [--output<file-name>] [--segfault] [--catch]\n");
			exit(1);
        }
        else if(option_index == 0){ //input
            input_file = optarg;
        }
        else if(option_index == 1){ //output
            output_file = optarg;
        }
    }

    if(catch_flag)
        signal(SIGSEGV,catch);

    if(segfault_flag){
        int* nullPTR = NULL;
        *nullPTR = 42;
    }

    if(input_flag){
        int fd = open(input_file, O_RDONLY);
        if (fd >= 0) { //replace read fd to point to specified file
			close(0);
			dup(fd);
			close(fd);
		}
        else{
            fprintf(stderr,"Error with file: %s. %s \n", input_file, strerror(errno));
            exit(2);
        }
    }

    if(output_flag){
        int fd = creat(output_file, 0666);
        if(fd >= 0){
            close(1);
			dup(fd);
			close(fd);
        }
        else{
            fprintf(stderr, "Error with file: %s. %s \n",output_file, strerror(errno));
            exit(3);
        }
    }

    while(1){
        ssize_t sizeRead;
        ssize_t bufSize = 512;
        ssize_t writeOutput;
        char temp = 'a'; //Just so it compiles 'cleanly' with no warnings
        char* buf = &temp; 
        sizeRead = read(0, buf, bufSize);

        if(sizeRead < 0){
            if(input_flag)
                fprintf(stderr, "Error reading from the file: %s. %s \n", input_file, strerror(errno));
            else
                fprintf(stderr, "Error reading from standard input. %s \n", strerror(errno));
            exit(2);
        }
        if(sizeRead == 0)
            break;
        writeOutput = write(1,buf,sizeRead);
        if(writeOutput < 0){
            if(output_flag)
                fprintf(stderr, "Error writing to the file: %s. %s \n", output_file, strerror(errno));
            else
                fprintf(stderr, "Error writing to standard output. %s \n", strerror(errno));

            exit(3);
        }
    }

    exit(0);
}