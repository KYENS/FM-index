HEADER='query71.h'
READFILE='../../10K.clean.queries'
F_FILE='../../F_counts/suffixes.sorted.7.F_counts'
m_Fstep=7
m_Lstep=1
g++ -o host -include ../$HEADER pseudo_host.cpp
./host $READFILE $F_FILE
MAX_CHIP=`sed -n "$count"p traces/max_index.txt`;
echo $MAX_CHIP
perl local_index_to_row.pl $m_Fstep $m_Lstep $MAX_CHIP
echo "===END==="
 

HEADER='query72.h'
F_FILE='../../F_counts/suffixes.sorted.7.F_counts'
m_Fstep=7
m_Lstep=2
g++ -o host -include ../$HEADER pseudo_host.cpp
./host $READFILE $F_FILE
MAX_CHIP=`sed -n "$count"p traces/max_index.txt`;
echo $MAX_CHIP
perl local_index_to_row.pl $m_Fstep $m_Lstep $MAX_CHIP
echo "===END==="


HEADER='query73.h'
F_FILE='../../F_counts/suffixes.sorted.7.F_counts'
m_Fstep=7
m_Lstep=3
g++ -o host -include ../$HEADER pseudo_host.cpp
./host $READFILE $F_FILE
MAX_CHIP=`sed -n "$count"p traces/max_index.txt`;
echo $MAX_CHIP
perl local_index_to_row.pl $m_Fstep $m_Lstep $MAX_CHIP
echo "===END==="
 

HEADER='query91.h'
F_FILE='../../F_counts/suffixes.sorted.9.F_counts'
m_Fstep=9
m_Lstep=1
g++ -o host -include ../$HEADER pseudo_host.cpp
./host $READFILE $F_FILE
MAX_CHIP=`sed -n "$count"p traces/max_index.txt`;
echo $MAX_CHIP
perl local_index_to_row.pl $m_Fstep $m_Lstep $MAX_CHIP
echo "===END==="
 

HEADER='query92.h'
F_FILE='../../F_counts/suffixes.sorted.9.F_counts'
m_Fstep=9
m_Lstep=2
g++ -o host -include ../$HEADER pseudo_host.cpp
./host $READFILE $F_FILE
MAX_CHIP=`sed -n "$count"p traces/max_index.txt`;
echo $MAX_CHIP
perl local_index_to_row.pl $m_Fstep $m_Lstep $MAX_CHIP
echo "===END==="
 

HEADER='query93.h'
F_FILE='../../F_counts/suffixes.sorted.9.F_counts'
m_Fstep=9
m_Lstep=3
g++ -o host -include ../$HEADER pseudo_host.cpp
./host $READFILE $F_FILE
MAX_CHIP=`sed -n "$count"p traces/max_index.txt`;
echo $MAX_CHIP
perl local_index_to_row.pl $m_Fstep $m_Lstep $MAX_CHIP
echo "===END==="
 
