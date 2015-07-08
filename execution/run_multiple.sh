#!/bin/bash
terminate(){
  echo $1
  kill -15 $1
  echo "killing my childs"
  exit
}

#global variabled to handle storage units
export M5_PATH=/scratch/x86
checkpoint_dir="my_ckpts"
results="/scratch/results"
all_exp="/scratch/build/experiments/"
mutex="/scratch/build/script.lock"
# I need to assign differnt ports to each core therefore i use the coreid as a hash
x=$1
y=7778
port=$((x+y))
my_core="core$x"
export DMTCP_PORT=$port

#I am killing remained dmtcp coordinator which listen to that port
dmtcp_command -q
echo $my_core

if [ ! -d "$my_core" ]; then 
  mkdir "$my_core"
fi

cd $my_core


#I need to check out whether I have arleady created a checkpoint



if [ ! -f "maincheckpoint.dmtcp" ]; then
  echo "$checkpoint_dir/maincheckpoint/maincheckpoint.dmtcp checkpoint does not exist creating"
dmtcp_checkpoint ./../../build/X86/gem5.opt --debug-flags=FaultInjection --remote-gdb-port=-1 -r -d start/ ../../configs/example/fs.py --caches --fi-in input -b test --text-start 0 -M 1 --switch-on-fault=0 --repeat-switch=1 --checkpoint-on-fault 0  --file-type=1
  result=$?
  if [ "$result" -eq "1" ]; then
    echo "Checkpoint created">>"$my_core"
  else
    if [ -f dmtcp_restart*.sh ]; then
      rm dmtcp_restart*.sh
    fi
#    if [ -f ckpt_gem5.opt* ]; then
# rm ckpt_gem5.opt*
#    fi
    echo "ERROR IN CREATING THE CHECKPOINT $my_core"
  fi
fi

while true ;
do 
	echo "Going to take Lock $my_core"
    lockfile $mutex #take lock
    exp=($(ls $all_exp))
	echo "Lock TAKEN $my_core"
    if [ "${#exp[@]}" -eq "0" ]; then
      rm -f $mutex
      exit
    fi
    expx=$all_exp${exp[0]}
    echo "$expx"
    mv $expx input   
    rm -f $mutex #free lock
    echo "lock released $my_core"

    dmtcp_restart maincheckpoint.dmtcp &
    pids=$!
    echo "Passing child id $i"
    ./../time.sh $pids
    _name=$(echo -n  "$(/sbin/ifconfig | grep 'em0\|em1\|eth1\|eth0\|eth2\|eth3' | tr -s ' '  | cut -d ' ' -f5| sed 's/://g')$(date | sed 's/ //g')$my_core")
    echo  "$my_core storing $_name ..... " >>"$my_core"
    echo "$_name"
    mkdir "$_name"
    mv input "$_name"
    cp start/* "$_name"
    rm start/ApplicationOutput 
    mv "$_name" $results 
    echo "$ my_core .....results stored">>"$my_core"
done
#trap "terminate $pids" SIGINT SIGTERM
echo "my experiments have finished"
exit


