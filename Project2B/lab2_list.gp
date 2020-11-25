#! /usr/bin/gnuplot
#
# purpose:
#	 generate data reduction graphs for the multi-threaded list project
#
# input: lab2_list.csv
#	1. test name
#	2. # threads
#	3. # iterations per thread
#	4. # lists
#	5. # operations performed (threads x iterations x (ins + lookup + delete))
#	6. run time (ns)
#	7. run time per operation (ns)
#   8. average time spent waiting for a lock
# output:
#	lab2b_1.png ... throughput (total operations per second) vs number of threads for mutex and spin locks
#	lab2b_2.png ... 
#	lab2b_3.png ... 
#	lab2b_4.png ... 
#   lab2b_5.png ... 
# Note:
#	Managing data is simplified by keeping all of the results in a single
#	file.  But this means that the individual graphing commands have to
#	grep to select only the data they want.
#
#	Early in your implementation, you will not have data for all of the
#	tests, and the later sections may generate errors for missing data.
#

# general plot parameters
set terminal png
set datafile separator ","

#-----------------------------------------------------------------------------------------------
# 1 :: throughput (total operations per second) vs number of threads for mutex and spin locks  |
#-----------------------------------------------------------------------------------------------
set title "List-1: Throughput vs Number of Threads"
set xlabel "Number of Threads"
set xrange[1:25]
set ylabel "Throughput (operations/second)"
set logscale x 2
set logscale y 10
set output 'lab2b_1.png'

plot \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'Mutex' with linespoints lc rgb 'red', \
     "< grep 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'Spin-Lock' with linespoints lc rgb 'green'


#----------------------------------------------------------------------------
# 2 :: Mutex Mean Wait-For-Lock Time and Spin-Lock Mean Time-Per-Operation  |
#----------------------------------------------------------------------------

set title "List-2: Mutex Mean Wait-For-Lock Time and Mean Time-Per-Operation"
set xlabel "Number of Threads"
set xrange[1:25]
set ylabel "Time"
set logscale x 2
set logscale y 10
set output 'lab2b_2.png'

plot \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($8) \
	title 'Wait-For-Lock-Time' with linespoints lc rgb 'red', \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($7) \
	title 'Time-Per-Operation' with linespoints lc rgb 'green'

#----------------------------------------------------------------------------
# 3 :: Successful trials vs Number of threads for each sync method         |
#----------------------------------------------------------------------------

set title "List-3: Successful Trials vs Number of Threads for Each Sync Method with Yields"
set xlabel "Number of Threads"
set ylabel "Numbher of Successful Iterations "
set logscale x 2
set logscale y 10
set output 'lab2b_3.png'

plot \
    "< grep 'list-id-none' lab2b_list.csv | grep '^list-id-none'" using ($2):($3) \
	title 'Unprotected' with points lc rgb 'red', \
    "< grep 'list-id-m' lab2b_list.csv | grep '^list-id-m'" using ($2):($3) \
	title 'Mutex' with points lc rgb 'green', \
    "< grep 'list-id-s' lab2b_list.csv | grep '^list-id-s'" using ($2):($3) \
	title 'Spin-Lock' with points lc rgb 'blue'

#Points are painting over each other which is fine

#----------------------------------------------------------------------------
# 4 :: throughput vs. the number of threads for mutex with different list sizes |
#----------------------------------------------------------------------------

set title "List-4: Throughput vs Number of Threads for Partioned List and Mutex Locks"
set xlabel "Number of Threads"
set ylabel "Throughput (operations/second)"
set logscale x 2
set logscale y 10
set output 'lab2b_4.png'

plot \
    "< grep 'list-none-m,[0-9,\,]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
        title '1 list' with linespoints lc rgb 'red', \
	"< grep 'list-none-m,[0-9,\,]*,1000,4' lab2b_list.csv" using ($2):(1000000000/($7)) \
        title '4 lists' with linespoints lc rgb 'blue', \
	"< grep 'list-none-m,[0-9,\,]*,1000,8' lab2b_list.csv" using ($2):(1000000000/($7)) \
        title '8 lists' with linespoints lc rgb 'purple', \
	"< grep 'list-none-m,[0-9,\,]*,1000,16' lab2b_list.csv" using ($2):(1000000000/($7)) \
        title '16 lists' with linespoints lc rgb 'green'

#----------------------------------------------------------------------------
# 5 :: throughput vs. the number of threads for spin-lock with different list sizes |
#----------------------------------------------------------------------------

set title "List-5: Throughput vs Number of Threads for Partioned List and Spin-Locks"
set xlabel "Number of Threads"
set ylabel "Throughput (operations/second)"
set logscale x 2
set logscale y 10
set output 'lab2b_5.png'

plot \
    "< grep 'list-none-s,[0-9,\,]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
        title '1 list' with linespoints lc rgb 'red', \
	"< grep 'list-none-s,[0-9,\,]*,1000,4' lab2b_list.csv" using ($2):(1000000000/($7)) \
        title '4 lists' with linespoints lc rgb 'blue', \
	"< grep 'list-none-s,[0-9,\,]*,1000,8' lab2b_list.csv" using ($2):(1000000000/($7)) \
        title '8 lists' with linespoints lc rgb 'purple', \
	"< grep 'list-none-s,[0-9,\,]*,1000,16' lab2b_list.csv" using ($2):(1000000000/($7)) \
        title '16 lists' with linespoints lc rgb 'green'