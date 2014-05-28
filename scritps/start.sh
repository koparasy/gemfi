array=(0 0 0 0 0 0 0 0 0 0 0 0 0 0 0)
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
