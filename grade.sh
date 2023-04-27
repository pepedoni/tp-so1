#!/bin/bash
set -eu

# DCC605: Userspace threading library programming assignment
# Autograding script
testsToRun=(0 1 2 3 4 5 6 7 8 9 10)
total=23
ecnt=0
totalTestsRun=0
totalCasesRun=0

#Run script until stop
for i in ${testsToRun[*]}
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
        echo "Executando teste $i pela $j vez"  
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
