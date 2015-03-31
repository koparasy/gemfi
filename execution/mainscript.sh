#!/bin/bash
terminate(){
  echo $1
  kill -15 $1
  echo "killing my childs"
  exit
}

start_new=$1
num_cores=$2
delete_core=$3

workstation="/scratch"
my_pid="$workstation/mypid.txt"
my_childs=""
echo $$>"$my_pid"

if [ ! -d "$workstation/results" ] ; then
  mkdir -p $workstation/results
fi

if [ $delete_core -eq "1" ]; then
rm -r core*
for (( i = 0 ; i < $num_cores; i++))
do
    mkdir core$i
done
fi

if [ $start_new -eq "1" ] ; then
  echo "Starting new"
  cp -r "$workstation/build/x86" "$workstation"

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
