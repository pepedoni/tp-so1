#!/bin/bash
set -eu

# DCC605: Userspace threading library programming assignment
# Autograding script

total=22
ecnt=0
start=7
stop=8
totalTestsRun=0
totalCasesRun=0

#Run script until stop
for (( i=start; i<=$stop; i++ ))
do
    
    if [ $i -le 6 ]
    then
        runTimes=1
    fi 

    if [ $i -eq 7 ] || [ $i -eq 8 ] || [ $i -eq 11 ] || [ $i -eq 12 ]
    then 
        runTimes=2
    fi

    if [ $i -eq 9 ] || [ $i -eq 10 ]
    then 
        runTimes=4
    fi

    for (( j=1; j<=$runTimes; j++ ))
    do
        totalCasesRun=$(( $totalCasesRun + 1 )) ;
        totalTestsRun=$(( $totalTestsRun + 1 )) ;
        
        if ! tests/test$i.sh ; 
        then 
            ecnt=$(( $ecnt + 1 )) ;
        fi
    done

done

echo "your code passes $(( $totalCasesRun - $ecnt )) of $totalCasesRun cases runned in $totalTestsRun tests"
rm -f dlist.o dccthread.o
rm -f test*.out test*.err
rm gcc.log
exit 0
