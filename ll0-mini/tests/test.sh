#!/bin/sh


for tf in *.c
do
	rf=${tf/test/result}; rf=${rf/\.c/\.txt}
	n=${tf/test/}; n=${n/\.c/}

	if [[ ! -f $rf ]];
	then
		echo "Test file $n not found"
		continue
	fi
	
	cc -ansi -Wall -o $n $tf
	t=$(cat $tf | "./$n")
	r=$(cat $rf)
	rm $n
	
	if [[ $t = $r ]]
	then
		echo "Test $n passed"
	else
		echo "Test $n failed"
	fi
done


ll='../ll'
for tf in *.l
do
	rf=${tf/test/result}; rf=${rf/\.l/\.txt}
	n=${tf/test/}; n=${n/\.l/}
	
	if [[ ! -f $rf ]];
	then
		echo "Test file $n not found"
		continue
	fi	
	
	t=$(cat $tf | $ll)
	r=$(cat $rf)
	
	if [[ $t = $r ]]
	then
		echo "Test $n passed"
	else
		echo "Test $n failed"
	fi
done


