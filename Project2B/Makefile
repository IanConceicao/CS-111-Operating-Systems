#NAME: Ian Conceicao
#EMAIL: IanCon234gmail.com
#ID: 505153981
.SILENT:

default:
	gcc -pthread -lrt  -Wall -Wextra -g SortedList.c  lab2_list.c -o lab2_list -lprofiler
	
dist: graphs
	tar -czvf lab2b-505153981.tar.gz SortedList.h SortedList.c lab2_list.c Makefile lab2b_list.csv profile.out lab2_list.gp README lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png 
	# TODO: Add png's
clean:
	rm -f lab2b-505153981.tar.gz lab2_list

profile: 
	rm -f profile.prof
	CPUPROFILE=profile.prof ./lab2_list --threads=12  --sync=s --iterations=1000
	pprof --text lab2_list profile.prof > profile.out
	pprof --list=doWork lab2_list profile.prof >> profile.out
	rm -f profile.prof


graphs: tests
	./lab2_list.gp

tests: default
	echo " " > lab2b_list.csv
	#-----------------------------------------------------------------------------------------------
	# 1/2 :: throughput (total operations per second) vs number of threads for mutex and spin locks  |
	#-----------------------------------------------------------------------------------------------
	echo "Starting test 1 and 2:  throughput (total operations per second) vs number of threads for mutex and spin locks"
	./lab2_list --iterations=1000 --sync=m --threads=1 >> lab2b_list.csv
	./lab2_list --iterations=1000 --sync=m --threads=2 >> lab2b_list.csv
	./lab2_list --iterations=1000 --sync=m --threads=4 >> lab2b_list.csv
	./lab2_list --iterations=1000 --sync=m --threads=8 >> lab2b_list.csv
	./lab2_list --iterations=1000 --sync=m --threads=12 >> lab2b_list.csv
	./lab2_list --iterations=1000 --sync=m --threads=16 >> lab2b_list.csv
	./lab2_list --iterations=1000 --sync=m --threads=24 >> lab2b_list.csv
	./lab2_list --iterations=1000 --sync=s --threads=1 >> lab2b_list.csv
	./lab2_list --iterations=1000 --sync=s --threads=2 >> lab2b_list.csv
	./lab2_list --iterations=1000 --sync=s --threads=4 >> lab2b_list.csv
	./lab2_list --iterations=1000 --sync=s --threads=8 >> lab2b_list.csv
	./lab2_list --iterations=1000 --sync=s --threads=12 >> lab2b_list.csv
	./lab2_list --iterations=1000 --sync=s --threads=16 >> lab2b_list.csv
	./lab2_list --iterations=1000 --sync=s --threads=24 >> lab2b_list.csv
	#-----------------------------------------------------------------------------------------------
	# 3 ::   |
	#-----------------------------------------------------------------------------------------------
	echo "Starting test 3 part 1, no synchronization, yield, 4 lists"
	-./lab2_list --yield=id --lists=4 --threads=1  --iterations=1   >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=1  --iterations=2   >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=1  --iterations=4   >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=1  --iterations=8   >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=1  --iterations=16   >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=4  --iterations=1   >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=4  --iterations=2   >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=4  --iterations=4   >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=4  --iterations=8   >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=4  --iterations=16   >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=8  --iterations=1   >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=8  --iterations=2   >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=8  --iterations=4   >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=8  --iterations=8   >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=12  --iterations=1   >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=12  --iterations=2   >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=12  --iterations=4   >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=12  --iterations=8   >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=12  --iterations=16   >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=16  --iterations=1   >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=16  --iterations=2   >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=16  --iterations=4   >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=16  --iterations=8   >> lab2b_list.csv
	-./lab2_list --yield=id --lists=4 --threads=16  --iterations=16   >> lab2b_list.csv

	./lab2_list --yield=id --sync=m--lists=4 --threads=1  --iterations=10   >> lab2b_list.csv
	./lab2_list --yield=id --sync=m --lists=4 --threads=1  --iterations=20   >> lab2b_list.csv
	./lab2_list --yield=id --sync=m --lists=4 --threads=1  --iterations=40   >> lab2b_list.csv
	./lab2_list --yield=id --sync=m --lists=4 --threads=1  --iterations=80   >> lab2b_list.csv
	./lab2_list --yield=id --sync=m --lists=4 --threads=4  --iterations=10  >> lab2b_list.csv
	./lab2_list --yield=id --sync=m --lists=4 --threads=4  --iterations=20   >> lab2b_list.csv
	./lab2_list --yield=id --sync=m --lists=4 --threads=4  --iterations=40   >> lab2b_list.csv
	./lab2_list --yield=id --sync=m --lists=4 --threads=4  --iterations=80   >> lab2b_list.csv
	./lab2_list --yield=id --sync=m --lists=4 --threads=8  --iterations=10  >> lab2b_list.csv
	./lab2_list --yield=id --sync=m --lists=4 --threads=8  --iterations=20   >> lab2b_list.csv
	./lab2_list --yield=id --sync=m --lists=4 --threads=8  --iterations=40   >> lab2b_list.csv
	./lab2_list --yield=id --sync=m --lists=4 --threads=8  --iterations=80   >> lab2b_list.csv
	./lab2_list --yield=id --sync=m --lists=4 --threads=12  --iterations=10  >> lab2b_list.csv
	./lab2_list --yield=id --sync=m --lists=4 --threads=12  --iterations=20   >> lab2b_list.csv
	./lab2_list --yield=id --sync=m --lists=4 --threads=12  --iterations=40   >> lab2b_list.csv
	./lab2_list --yield=id --sync=m --lists=4 --threads=12  --iterations=80   >> lab2b_list.csv
	./lab2_list --yield=id --sync=m --lists=4 --threads=16  --iterations=10  >> lab2b_list.csv
	./lab2_list --yield=id --sync=m --lists=4 --threads=16  --iterations=20   >> lab2b_list.csv
	./lab2_list --yield=id --sync=m --lists=4 --threads=16  --iterations=40   >> lab2b_list.csv
	./lab2_list --yield=id --sync=m --lists=4 --threads=16  --iterations=80   >> lab2b_list.csv

	./lab2_list --yield=id --sync=s--lists=4 --threads=1  --iterations=10   >> lab2b_list.csv
	./lab2_list --yield=id --sync=s --lists=4 --threads=1  --iterations=20   >> lab2b_list.csv
	./lab2_list --yield=id --sync=s --lists=4 --threads=1  --iterations=40   >> lab2b_list.csv
	./lab2_list --yield=id --sync=s --lists=4 --threads=1  --iterations=80   >> lab2b_list.csv
	./lab2_list --yield=id --sync=s --lists=4 --threads=4  --iterations=10  >> lab2b_list.csv
	./lab2_list --yield=id --sync=s --lists=4 --threads=4  --iterations=20   >> lab2b_list.csv
	./lab2_list --yield=id --sync=s --lists=4 --threads=4  --iterations=40   >> lab2b_list.csv
	./lab2_list --yield=id --sync=s --lists=4 --threads=4  --iterations=80   >> lab2b_list.csv
	./lab2_list --yield=id --sync=s --lists=4 --threads=8  --iterations=10  >> lab2b_list.csv
	./lab2_list --yield=id --sync=s --lists=4 --threads=8  --iterations=20   >> lab2b_list.csv
	./lab2_list --yield=id --sync=s --lists=4 --threads=8  --iterations=40   >> lab2b_list.csv
	./lab2_list --yield=id --sync=s --lists=4 --threads=8  --iterations=80   >> lab2b_list.csv
	./lab2_list --yield=id --sync=s --lists=4 --threads=12  --iterations=10  >> lab2b_list.csv
	./lab2_list --yield=id --sync=s --lists=4 --threads=12  --iterations=20   >> lab2b_list.csv
	./lab2_list --yield=id --sync=s --lists=4 --threads=12  --iterations=40   >> lab2b_list.csv
	./lab2_list --yield=id --sync=s --lists=4 --threads=12  --iterations=80   >> lab2b_list.csv
	./lab2_list --yield=id --sync=s --lists=4 --threads=16  --iterations=10  >> lab2b_list.csv
	./lab2_list --yield=id --sync=s --lists=4 --threads=16  --iterations=20   >> lab2b_list.csv
	./lab2_list --yield=id --sync=s --lists=4 --threads=16  --iterations=40   >> lab2b_list.csv
	./lab2_list --yield=id --sync=s --lists=4 --threads=16  --iterations=80   >> lab2b_list.csv

	#Last part!
	echo "Starting parts 4 and 3!"
	#Commented out tests are repeats
	# ./lab2_list  --sync=m --lists=1 --threads=1  --iterations=1000   >> lab2b_list.csv
	./lab2_list  --sync=m --lists=4 --threads=1  --iterations=1000   >> lab2b_list.csv
	./lab2_list  --sync=m --lists=8 --threads=1  --iterations=1000   >> lab2b_list.csv
	./lab2_list  --sync=m --lists=16 --threads=1  --iterations=1000   >> lab2b_list.csv
	# ./lab2_list  --sync=m --lists=1 --threads=2  --iterations=1000  >> lab2b_list.csv
	./lab2_list  --sync=m --lists=4 --threads=2  --iterations=1000   >> lab2b_list.csv
	./lab2_list  --sync=m --lists=8 --threads=2  --iterations=1000   >> lab2b_list.csv
	./lab2_list  --sync=m --lists=16 --threads=2  --iterations=1000   >> lab2b_list.csv
	# ./lab2_list  --sync=m --lists=1 --threads=4  --iterations=1000  >> lab2b_list.csv
	./lab2_list  --sync=m --lists=4 --threads=4  --iterations=1000   >> lab2b_list.csv
	./lab2_list  --sync=m --lists=8 --threads=4  --iterations=1000   >> lab2b_list.csv
	./lab2_list  --sync=m --lists=16 --threads=4  --iterations=1000   >> lab2b_list.csv
	# ./lab2_list  --sync=m --lists=1 --threads=8  --iterations=1000  >> lab2b_list.csv
	./lab2_list  --sync=m --lists=4 --threads=8  --iterations=1000   >> lab2b_list.csv
	./lab2_list  --sync=m --lists=8 --threads=8  --iterations=1000   >> lab2b_list.csv
	./lab2_list  --sync=m --lists=16 --threads=8  --iterations=1000   >> lab2b_list.csv
	# ./lab2_list  --sync=m --lists=1 --threads=12  --iterations=1000  >> lab2b_list.csv
	./lab2_list  --sync=m --lists=4 --threads=12  --iterations=1000   >> lab2b_list.csv
	./lab2_list  --sync=m --lists=8 --threads=12  --iterations=1000   >> lab2b_list.csv
	./lab2_list  --sync=m --lists=16 --threads=12  --iterations=1000   >> lab2b_list.csv


	# ./lab2_list  --sync=s --lists=1 --threads=1  --iterations=1000   >> lab2b_list.csv
	./lab2_list  --sync=s --lists=4 --threads=1  --iterations=1000   >> lab2b_list.csv
	./lab2_list  --sync=s --lists=8 --threads=1  --iterations=1000   >> lab2b_list.csv
	./lab2_list  --sync=s --lists=16 --threads=1  --iterations=1000   >> lab2b_list.csv
	# ./lab2_list  --sync=s --lists=1 --threads=2  --iterations=1000  >> lab2b_list.csv
	./lab2_list  --sync=s --lists=4 --threads=2  --iterations=1000   >> lab2b_list.csv
	./lab2_list  --sync=s --lists=8 --threads=2  --iterations=1000   >> lab2b_list.csv
	./lab2_list  --sync=s --lists=16 --threads=2  --iterations=1000   >> lab2b_list.csv
	# ./lab2_list  --sync=s --lists=1 --threads=4  --iterations=1000  >> lab2b_list.csv
	./lab2_list  --sync=s --lists=4 --threads=4  --iterations=1000   >> lab2b_list.csv
	./lab2_list  --sync=s --lists=8 --threads=4  --iterations=1000   >> lab2b_list.csv
	./lab2_list  --sync=s --lists=16 --threads=4  --iterations=1000   >> lab2b_list.csv
	# ./lab2_list  --sync=s --lists=1 --threads=8  --iterations=1000  >> lab2b_list.csv
	./lab2_list  --sync=s --lists=4 --threads=8  --iterations=1000   >> lab2b_list.csv
	./lab2_list  --sync=s --lists=8 --threads=8  --iterations=1000   >> lab2b_list.csv
	./lab2_list  --sync=s --lists=16 --threads=8  --iterations=1000   >> lab2b_list.csv
	# ./lab2_list  --sync=s --lists=1 --threads=12  --iterations=1000  >> lab2b_list.csv
	./lab2_list  --sync=s --lists=4 --threads=12  --iterations=1000   >> lab2b_list.csv
	./lab2_list  --sync=s --lists=8 --threads=12  --iterations=1000   >> lab2b_list.csv
	./lab2_list  --sync=s --lists=16 --threads=12  --iterations=1000   >> lab2b_list.csv
	echo "Finished all tests"
