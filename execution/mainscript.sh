#!/bin/bash
terminate(){
  echo $1
  kill -15 $1
  echo "killing my childs"
  exit
}

start_new=$1
num_cores=$2

workstation="/local/koparasy/"
my_pid="$workstation/mypid.txt"
my_childs=""
echo $$>"$my_pid"

if [ ! -d "$workstation/results" ] ; then
  mkdir -p $workstation/results
fi

if [ $start_new -eq "1" ] ; then
  cp -r $HOME/test.rcS ../configs/boot/test.rcS
  echo "Starting new"
  rm -r /home/dinos/results/*
  cp -r "$HOME/x86/disks/x86root.img" "$workstation/disks/x86root.img"
  rm -r core*
  for (( i = 0 ; i < $num_cores; i++))
  do
    mkdir core$i
  done
fi

my_childs=""
for (( i = 0 ; i < $num_cores ; i++)) 
do
	./run.sh $i &							# run.sh is responsible for 
	my_childs="$my_childs $!" 	# the experiments of a single core in each workstation
	
done
echo $my_childs>my_childs
trap "terminate $my_childs" SIGINT SIGTERM
wait

my_childs=""
