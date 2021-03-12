#!/bin/bash

pushd "$(dirname "$0")"

function exec {
    MAX_REPEAT=$1
    OUTPUT=$2

    if [ ! -d ./output ]; then
        mkdir output
    fi
    if [ ! -f ./output/$OUTPUT ]; then
        touch ./output/$OUTPUT
        echo "${@:3}" >> ./output/$OUTPUT
    fi

    REPEAT=$( expr $MAX_REPEAT + 1 - $( wc -l < ./output/$OUTPUT ) )
    echo Exec plan: $REPEAT
    echo Command: ${@:3}
    for (( it=0; it<$REPEAT; it++ ))
    do

        echo "$(expr $it + 1) of $REPEAT"
        ${@:3} >> ./output/$OUTPUT

    done
}

exec 30 pi_acc_mc_100_100000.txt ./bin/pi_acc_mc  100 100000
exec 30 pi_acc_mc_1000_10000.txt ./bin/pi_acc_mc  1000 10000
exec 30 pi_acc_mc_10_1000000.txt ./bin/pi_acc_mc  10 1000000

exec 30 pi_acc_1e9.txt ./bin/pi_acc  1000000000
exec 10 pi_seq_1e9.txt ./bin/pi_seq  1000000000

exec 30 pi_acc_1e10.txt ./bin/pi_acc 10000000000
exec 30  pi_seq_1e10.txt ./bin/pi_seq 10000000000

exec 6  pi_acc_1e11.txt ./bin/pi_acc 100000000000
exec 6  pi_seq_1e11.txt ./bin/pi_seq 100000000000

exec 30 mandel_acc_strict_i100_xy1000.txt ./bin/mandel_acc_strict -i 100 -xy 1000
exec 30 mandel_seq_i100_xy1000.txt ./bin/mandel_seq -i 100 -xy 1000

exec 30 mandel_acc_strict_i6000_xy6000.txt ./bin/mandel_acc_strict -i 6000 -xy 6000
exec 1  mandel_seq_i6000_xy6000.txt ./bin/mandel_seq -i 6000 -xy 6000
