# Ian Conceicao
# 505153981
# IanCon234@gmail.com

exec = ./lab0
default:
	gcc lab0.c -o lab0 -Wall -Wextra

dist:
	tar -czvf lab0-505153981.tar.gz lab0.c Makefile backtrace.png breakpoint.png README

clean:
	rm -f lab0 lab0-505153981.tar.gz

check: default stdToStdout fileToFile goodExit badArg badInputFile badOutputFile catch segfault output

stdToStdout:
	echo "Output me \n yep you should" | $(exec); \
	if [ $$? -ne 0 ]; \
    then \
                echo "Error copying standard input to standard output. What a fail!" > errors.txt; \
    fi; 

fileToFile:
	echo "This should be in the output" > input.txt; \
	$(exec) --input=input.txt --output=output.txt; \
	if [ cmp -s "input.txt" "output.txt" ] ; \
	then \
		echo "Copying from one file to another does not work" > errors.txt; \
	fi;
	rm -f input.txt output.txt;

goodExit:
	echo "This should be in the output" > input.txt; \
	$(exec) --input=input.txt --output=output.txt; \
	if [ $$? -ne 0 ]; \
    then \
		echo "Bad exit code for a successful exit" > errors.txt; \
	fi;
	rm -f input.txt output.txt;

badArg:
	echo "Input does not matter" | $(exec) --dog; \
        if [ $$? -ne 1 ]; \
        then \
                echo "A bad argument was given, exit code should be 1 but is not!" > errors.txt; \
        fi;

badInputFile: #relies on that ridiculous file not existing in user's directory
	$(exec) --input=thisFileDoesNotExist.txt; \
	if [ $$? -ne 2 ]; \
    then \
                echo "A nonexistent file was given, but an error code of 1 was not outputted." > errors.txt; \
    fi;

badOutputFile:
	touch output.txt; chmod -rw output.txt; \
	$(exec) --output=output.txt; \
	if [ $$? -ne 3 ]; \
    then \
                echo "An output file error occured but an error number of 3 was not outputted" > errors.txt; \
    fi; \
	rm -f output.txt;
	
catch:
	$(exec) --segfault --catch; \
	if [ $$? -ne 4 ]; \
    then \
                echo "Catch is not outputting the correct error number of 4" > errors.txt; \
	fi;

segfault:
	$(exec) --segfault; \
        if [ $$? -ne  139 ]; \
        then \
                echo "A segmentation fault argument is not actually causing a segmentation fault" > errors.txt; \
        fi;

output:
	if [ -f errors.txt ]; \
		then \
			cat errors.txt; \
			rm -f errors.txt; \
		else \
			echo "Smoke test a success, passed all tests"; \
		fi;