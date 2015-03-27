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
all_exp="/scratch/experiments"
mutex="/scratch/script.lock"
experiments=(Fetch.txt Decode.txt IEW.txt LDS.txt)
ckpts=(fetch_ckpt.dmtcp decode_ckpt.dmtcp iew_ckpt.dmtcp lds_ckpt.dmtcp)
ckpt_dirs=(fetch decode iew lds)
# I need to assign differnt ports to each core therefore i use the coreid as a hash
x=$1
y=7778
port=$((x+y))
my_core="core$x"
export DMTCP_PORT=$port

#I am killing remained dmtcp coordinator which listen to that port
dmtcp_command -q
echo $my_core
echo "${experiments[*]}"
echo "${ckpts[*]}"
echo "${ckpt_dirs[*]}"

if [ ! -d "$my_core" ]; then 
  mkdir "$my_core"
fi

cd $my_core

if [ ! -d "$checkpoint_dir" ]; then 
  mkdir "$checkpoint_dir"
fi

for (( i = 0 ; i <${#ckpt_dirs[@]} ; i++))
do
  if [ ! -d "$checkpoint_dir/${ckpt_dirs[$i]}" ]; then 
    mkdir "$checkpoint_dir/${ckpt_dirs[$i]}"
  fi
done


if [ ! -d "$checkpoint_dir/maincheckpoint" ]; then 
    mkdir "$checkpoint_dir/maincheckpoint"
fi


#I need to check out whether I have arleady created a checkpoint



if [ ! -f "$checkpoint_dir/maincheckpoint/maincheckpoint.dmtcp" ]; then
  echo "$checkpoint_dir/maincheckpoint/maincheckpoint.dmtcp checkpoint does not exist creating"
dmtcp_checkpoint ./../../build/X86/gem5.opt --debug-flags=FaultInjection --remote-gdb-port=-1 -r -d start/ ../../configs/example/fs.py --caches --fi-in input -b test --text-start 0 -M 1 --switch-on-fault=1 --repeat-switch=1 --checkpoint-on-fault 1
  result=$?
  if [ "$result" -eq "1" ]; then
    echo "Checkpoint created">>"$my_core"
    #cp maincheckpoint.dmtcp "$checkpoint_dir/maincheckpoint/"
     #   cp -r ckpt_gem5.opt_* "$checkpoint_dir/maincheckpoint/"
     #   rm dmtcp_restart*
     #   rm maincheckpoint.dmtcp
     # rm -r ckpt_gem5.opt_*
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

for (( i = 0 ; i < ${#experiments[@]} ; i++))
do
  cur_exp="$all_exp/${experiments[$i]}"
  echo "$cur_exp"
  while [ -s "$cur_exp" ]
  do

    if [ ! -f input ]; then  
      echo "Lock taken"
      lockfile $mutex #take lock
      echo "Lock taken"
      exp=$(tail -n 1 $cur_exp)
      head -n -1 $cur_exp > temp.txt 
      mv temp.txt $cur_exp
      rm -f $mutex #free lock
      echo "lock released"
      echo -n "$exp">input
    fi
# Check if there is a checkpoint for this kind of faults
# if there is use it otherwise use the maincheckpoint

    if [ -f "${ckpts[$i]}" ]; then
      echo "Restoring from $ckpts[$i]"
#cp $checkpoint_dir/${ckpt_dirs[$i]}/${ckpts[$i]} . 
#     cp -r $checkpoint_dir/${ckpt_dirs[$i]}/ckpt_gem5.opt_* .
      dmtcp_restart ${ckpts[$i]} &
      pids=$!
      echo "Passing child id $i"
      ./../time.sh $pids
    else
      echo "restoring from maincheckpoint"
#      cp $checkpoint_dir/maincheckpoint/maincheckpoint.dmtcp .
#      cp -r $checkpoint_dir/maincheckpoint/ckpt_gem5.opt_* .
      dmtcp_restart maincheckpoint.dmtcp &
      pids=$!
      echo "Passing child id $i"
      ./../time.sh $pids
    fi
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
done
#trap "terminate $pids" SIGINT SIGTERM
echo "my experiments have finished"
exit


