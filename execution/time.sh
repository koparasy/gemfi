#!/bin/bash
my_child=$1
echo "$my_child"
while  kill -0 $my_child ;
do
  IFS=' '
  _time=$(ps -o stime,time $my_child)
  echo "$_time"
  array=($_time)
  string=${array[2]}
  OIFS=$IFS
  IFS=':'
  real_time=($string)
  hour=${real_time[0]}
  hour=`echo $hour|sed 's/^0*//'`
  min=${real_time[1]}
  min=`echo $min|sed 's/^0*//'`
  sec=${real_time[2]}
  sec=`echo $sec|sed 's/^0*//'`
  total_time=$((hour*3600+min*60+sec))
  IFS=$OIFS
  if [ "$total_time" -gt "14400" ]; then
    echo "kill my child"
    kill -15 $my_child
    cd start
    touch "killed"
    echo "Time $total_time" > killed
    cd ..
  fi
  IFS=$'\t'
  size=$(du -s ./start)
  _size=($size)
  actual_size=${_size[0]}
  IFS=$OIFS
  echo "size $actual_size"
  if [ "$actual_size" -gt "20000" ]; then
    echo "kill my child (size to big)"
    kill -15 $my_child
    if [ -d start ]; then
      cd start
      touch "oversized_experiments"
      echo "$actual_size" >oversized_experiments
      cd ..
    fi
  fi
  _users=$(users|wc -l)
  if [ "$_users" -gt "1" ]; then
    echo "someone else is using the computer"
    kill -15 $my_child
    exit
  else
    echo "Only me on the computer"
  fi
  sleep 60
done

