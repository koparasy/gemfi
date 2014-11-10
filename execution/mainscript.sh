#!/bin/bash
terminate(){
  echo $1
  kill -15 $1
  echo "killing my childs"
  exit
}

start_new=$1
num_cores=$2

work_station="/srv/homes/koparasy/gem5campaings"
my_pid="$work_station/mypid.txt"
my_childs=""
echo $$>"$my_pid"

if [ ! -d "$work_station/results" ] ; then
  mkdir -p $work_station/results
fi

if [ $start_new -eq "1" ] ; then
  echo "Starting new"
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
