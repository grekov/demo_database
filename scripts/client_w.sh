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
        if [ 1 -eq 1 ]
        then
                ./client put key1 val10000000000000000000000000000001 &
                ./client put key1 val11000000000000000011111144444442 &
                ./client put key2 val22222222222222222222222222222222 &
                ./client put key3 val33333333333333333333333333333332 &
                ./client put key3 val34444444444444555555555554444441 &
                ./client put key4 val45555555555555555555533344444441 &
                ./client put key5 val55555555555555555533366666633557 &
                ./client put key5 val57666666666666677777777665555558 &
        fi

        if [ 1 -eq 1 ]
        then
                ./client erase key1 &
                ./client erase key2 &
                ./client erase key3 &
                ./client erase key4 &
                ./client erase key5 &
        fi

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
