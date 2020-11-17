#!/bin/bash 
#======================CHANGE STEP HERE========================
#READFILE='15chars_17.txt'   ## 16 for(41 42 43 44)  17 for(51 52 53 54) 20 for 55
READFILE='10K.clean.queries'
HEADER='query51.h'
BLANK=''
m_Fstep=5
m_Lstep=1
#======================CHANGE STEP HERE========================
g++ table_creation_1step.c -o table_creation_1step
g++ table_creation_mstep.cpp -o table_creation_mstep -include $HEADER   
./table_creation_1step
./table_creation_mstep

cd simple_query
echo "com shell:"
g++ query_1step.c  -o query_1step
g++ query_mstep.cpp  -o query_mstep -include ../$HEADER 
echo "-----start-----"
count=1
counter=1
cnt_for_pattern=1
echo $BLANK >'step_result.txt'

while read line;do
	echo "search query = $line"
	./que_1step.sh $line
	./que_mstep.sh $line
	lineB=`sed -n "$count"p output_1step.txt`;
	lineC=`sed -n "$count"p output_mstep.txt`;
	if [ "$lineB" == "$lineC" ] ;
	then
		echo "pass $cnt_for_pattern pattern."
		echo "index from 1step = $lineB"
		echo "index from mstep = $lineC"
		cnt_for_pattern=`expr $cnt_for_pattern + 1`
	else 
		echo "index from 1step = $lineB"
		echo "index from mstep = $lineC"
		echo "search query = $line"
		break
	fi
	echo $line:$lineC >>'mstep_result.txt'
	./local $counter
	counter=$((counter+1))
	echo "COUNTER:" $counter
done < $READFILE

g++ -o host -include ../$HEADER  pseudo_host.cpp
./host $READFILE  	
perl match_ans.pl
MAX_CHIP=`sed -n "$count"p traces/max_index.txt`;
echo $MAX_CHIP
perl local_index_to_row.pl $m_Fstep $m_Lstep $MAX_CHIP
echo "===END==="


