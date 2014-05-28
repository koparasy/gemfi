my_core=$1


all_exp=$EXP_FILE
mutex=$EXP_LOCK
_results=$EXP_RESULTS_FILE
gem5bin=$GEM5_BINARY
gemconfig=$GEM5CONFIGS

cd $my_core

while [ -s "$all_exp" ]
do
        lockfile $mutex #take lock

        exp=$(tail -n 1 $all_exp)
        head -n -1 $all_exp > temp.txt
        mv temp.txt $all_exp
        rm -f $mutex #free lock

        echo -n "$exp" >input
	cat input
	$gem5bin -r -d start --debug-flags=FaultInjection $gemconfig/fs.py --kernel=/home/koparasy/kernels/x86/vmlinux --fi-in input -b test -n 4 --switch-on-fault 0 -M 1
	
        _name=$(/sbin/ifconfig | grep 'eth0' | tr -s ' '  | cut -d ' ' -f5| sed 's/://g')
        _name+="$(/sbin/ifconfig | grep 'eth1' | tr -s ' '  | cut -d ' ' -f5| sed 's/://g')"
        
        _name+=$(date | sed 's/ //g')
        _name+=$my_core
        
        mkdir "$_name"
        mv input "$_name"
        cp start/* "$_name"

        cp -r "$_name" $_results
        result=$?
        
        if [ "$result" -eq "0" ]; then
          rm -r $_name
          echo "$my_core .....results stored"
        else
	  echo "$ problem in storing results : $_name exiting this simulation..."
          exit
        fi

done

