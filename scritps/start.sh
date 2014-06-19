
array=(0 0 0 0 0 0 0 0 0 0 0 0 0 0 0)
if [ ! -d "$HOME/results" ]; then
	mkdir -p $HOME/results/RegisterInjectedFault/int/propagated/correct
	mkdir -p $HOME/results/RegisterInjectedFault/int/propagated/crashed
	mkdir -p $HOME/results/RegisterInjectedFault/int/propagated/strict
	mkdir -p $HOME/results/RegisterInjectedFault/int/propagated/altered
	mkdir -p $HOME/results/RegisterInjectedFault/int/non_propagated
	
	mkdir -p $HOME/results/RegisterInjectedFault/float/propagated/correct
	mkdir -p $HOME/results/RegisterInjectedFault/float/propagated/strict
	mkdir -p $HOME/results/RegisterInjectedFault/float/propagated/crashed
	mkdir -p $HOME/results/RegisterInjectedFault/float/propagated/altered
	mkdir -p $HOME/results/RegisterInjectedFault/float/non_propagated
	
	mkdir -p $HOME/results/GeneralFetchInjectedFault/correct
	mkdir -p $HOME/results/GeneralFetchInjectedFault/strict
	mkdir -p $HOME/results/GeneralFetchInjectedFault/crashed
	mkdir -p $HOME/results/GeneralFetchInjectedFault/altered
	
	mkdir -p $HOME/results/IEWStageInjectedFault/correct
	mkdir -p $HOME/results/IEWStageInjectedFault/strict
	mkdir -p $HOME/results/IEWStageInjectedFault/crashed
	mkdir -p $HOME/results/IEWStageInjectedFault/altered
	
	mkdir -p $HOME/results/PCInjectedFault/correct
	mkdir -p $HOME/results/PCInjectedFault/strict
	mkdir -p $HOME/results/PCInjectedFault/crashed
	mkdir -p $HOME/results/PCInjectedFault/altered
	
	mkdir -p $HOME/results/LoadStoreInjectedFault/correct
	mkdir -p $HOME/results/LoadStoreInjectedFault/strict
	mkdir -p $HOME/results/LoadStoreInjectedFault/crashed
	mkdir -p $HOME/results/LoadStoreInjectedFault/altered
	
	mkdir -p $HOME/results/RegisterDecodingInjectedFault/correct
	mkdir -p $HOME/results/RegisterDecodingInjectedFault/strict
	mkdir -p $HOME/results/RegisterDecodingInjectedFault/crashed
	mkdir -p $HOME/results/RegisterDecodingInjectedFault/altered
	echo "Creation of tree directories done"
fi


for (( i = 0 ; i < 16 ; i++ ))
do
	if [ ! -d ./core$i ]
	then
		mkdir core$i
	else
		if [ ! -f core$i/maincheckpoint.dmtcp ]
		then
			array[$i]="0";
		else
			array[$i]="maincheckpoint.dmtcp";
		fi

		if [ -f core$i/ckpt*.txt ]
		 then
		    array[$i]=`ls core$i/ckpt*.dmtcp -v -1|tail -1`
		fi
	fi
done


for (( i = 0 ; i < 16 ; i++ ))
do
echo ${array[$i]}
./run.sh "core$i" ${array[$i]} &
done

echo "waiting my childs"
wait
echo "my childs finished"

cd ~/results
 local _name=$(/sbin/ifconfig | grep 'eth0' | tr -s ' '  | cut -d ' ' -f5| sed 's/://g')
  _name+="$(/sbin/ifconfig | grep 'eth1' | tr -s ' '  | cut -d ' ' -f5| sed 's/://g')"
  _name+=$(date | sed 's/ //g')
  
tar -cjf _name.tar.bz2 *
touch finished












