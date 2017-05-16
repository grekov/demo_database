#!/bin/bash

i=0
j=1

cd ../src

if [[ $? -ne 0 ]]
then
        exit 1
fi

if [[ ! -e client ]]
then
        echo "Executable file \"client\" does not exists."
        exit 1
fi

while [ $i -lt 1000 ]
do

        ./client get key1 &
        ./client get key2 &
        ./client get key3 &
        ./client get key4 &
        ./client get key5 &
        ./client list

        i=$[$i+1]
        j=$[$j+1]

        if [ $j -eq 500 ]
        then
                j=1
                echo "Go to sleep..."
                sleep 1
        fi

done

exit 0
