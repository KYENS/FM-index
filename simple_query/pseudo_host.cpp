#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <queue>
#include <map>
#include <iomanip>
#include <cmath>
#include <stdlib.h>
#include<sys/time.h>
#include "../toolbox.h"
#include "../ENCODE_L.h"
#include "../ENCODE_F.h"

using namespace std;

#define MAX_FILE_LEN 8192
#define BATCH_SIZE 128
#define QUEUE_LEN 10



#define DEBUG 0
#define DOWN_SAMPLED_RATE 5
//#define NUM_OF_ENCODE_TERM 24
//#define M_Fterm 4
//../#define M_Lterm 4



#define NumOfDRAM 128

//====GLOBAL VARIABLES======
int len;
int sample_rate;
//===OBJECT DECLARATION===



int read_count = 0;
int NumOfF_term, NumOfL_term;

int *chip;
int *local_index;

int *arr;
int Num_L_in_chip[NumOfDRAM];
int Num_L_cross_chip[NumOfDRAM];
int cross_index[NumOfDRAM-1];

int  * L_term ;
int  **OCC;
int  *F_counts;

char * read_L_file	= "../../L_terms/suffixes.1.L_terms";   		// input DATA
char * read_OCC_file= "../../Occ/suffixes.1.Occ";   	  		// input DATA
char * read_F_file;//	= "../../F_counts/suffixes.sorted.7.F_counts";   	  	// input DATA	

//Read file to get reads
int* inputReads(char *file_path, int *read_count){
    FILE *read_file = fopen(file_path, "r");
    int ch, lines=0;
    int *reads;
    do                                                                                                 
    {                                                                                                  
        ch = fgetc(read_file);                                                                            
        if (ch == '\n')                                                                                
            lines++;                                                                                   
    } while (ch != EOF);
    rewind(read_file);
    reads=(int*)malloc(lines*sizeof(int));
    *read_count = lines;
    
	int i = 0;                                                                                                                                                                             
    for(i = 0; i < lines; i++)                                                                         
    {
		fscanf(read_file,"%d",&reads[i]);
    }                                                                                                  
    fclose(read_file);
    return reads;
}

//OCC table creation
int** OCC_creation(char *file_path){
	FILE *read_file = fopen(file_path, "r");
	int **OCC =(int**)malloc(read_count*sizeof(int*));
	for(int i=0;i<read_count;i++)
		OCC[i] =(int*)malloc(NumOfL_term*sizeof(int));
	for(int i=0;i<read_count;i++)
		for(int j=0;j<NumOfL_term;j++){
			fscanf(read_file, "%d", &OCC[i][j]);
		}
	fclose(read_file);
	return OCC;
}

int OCC_creation_for_huge_data(int search_index, int L_int){
	FILE *read_file = fopen(read_OCC_file, "r");
	int result;
	for(int i=0;i<search_index;i++){
		for(int j=0;j<NumOfL_term;j++){
			fscanf(read_file, "%d", &result);
		}
	}
	for(int j=0;j<=L_int;j++){
		fscanf(read_file, "%d", &result);
	}
	fclose(read_file);
	return result;
}

//F_count read from file
int* F_creation(char *file_path){
	ifstream infile;
	string line;
	infile.open(file_path);
	cout<<file_path<<endl;
	if(infile.is_open())cout<<"OPEN F SUCCESS"<<endl;	
	int* F =(int*)malloc(NumOfF_term*sizeof(int));
	for(int i=0;i<NumOfF_term;i++)
		infile>>F[i];
	infile.close();
	return F;
}

void read_queries(vector <string> &queries,char* path){
	ifstream infile;
	string line;
	infile.open(path);
	if(infile.is_open())cout<<"OPEN SUCCESS"<<endl;	
	while(getline(infile,line)){
		len = line.length()+1;// len +'$'
		read_count++;
		queries.push_back(line);
		//cout<<line<<endl;
	}
	infile.close();
	

}


void group_creation(int M_F, int M_L, int ptr){
	int first =1;
	int i;
	first = pow(4,M_F-1);
	for(i=0;i<first;i++)
		arr[ptr+i]=0;
	ptr=ptr+first;
	
	int numofterm=calculateNumofTerm(M_F-1);
	for(i=0;i<numofterm/*calculateNumofTerm(M_F-1)*/*4;i++)
	arr[ptr+i]=1;	
	
	if(M_L > 1){
		for(i=0;i<4;i++)
			group_creation(M_F-1,M_L-1,ptr+numofterm/*calculateNumofTerm(M_F-1)*/*i);
	}

}
//產生chip和local的table
int mapping_table_creation(int max_L_per_chip, int*local_index){
	int mapping_table_success = 0;
	//int cross_index[NumOfDRAM-1];
	int i = 0, j = 0;
	int total = 0;
	if(DEBUG==1)
		cout<< "----------Mapping table creation start ----------" << endl;
	while(total <= max_L_per_chip && j <= NumOfDRAM-1 && i < NumOfF_term){
		chip[i] = j;
		local_index[i] = total;
		if(DEBUG ==1)	printf("chip[%d] = %d, local_index[%d]=%d\n",i,j,i,total);
		i = i + 1;
		if(i < NumOfF_term ) total = total + F_counts[i];
		if(total > max_L_per_chip){
			cross_index[j]= i - 2;
			i = i - 1;
			j = j + 1;
			total = 0;
		}
	}

	if(DEBUG ==1)	cout << "how many L_term corresponding F_term can be put? " << i <<endl;
	//------ if放不下則顯示無法query警示    ------//
	if(i!=NumOfF_term){
		mapping_table_success = 0;
		cout << endl << "mapping table create fail !! error 1: total capacity not enough" << endl << endl;
	}
	//------ else放得下則可開始query sig=1  ------//
	else{
		mapping_table_success = 1;
		cout << endl << "mapping table create success!" << endl << endl;
	}
	return mapping_table_success;
}

//計算每個chip有幾條cross chip的資料 以及 是從哪個F_term後換chip
void cross_chip_table_creation(int *local_index, int *F_counts){
	int i=0;
	int bound=pow(4,M_Fterm-M_Lterm);
	int NumofTerm=calculateNumofTerm(M_Fterm-M_Lterm);
	if(DEBUG==1)
		cout<< "----------cross chip table creation start ----------" << endl;
	while(i!=NumOfF_term){
		if(arr[i]==0){
			int j = i;
			//這個while的用意是同一組可以跨多chip
			while(chip[j]!=chip[i+bound-1]){
				//這個while用來找出跨chip的index
				while(chip[j]==chip[j+1] && j < (i+bound-1)){
					j=j+1;
				}
				if(j!=(i+bound-1)){
					for(int k=j+1;k>i;k--)
						Num_L_cross_chip[chip[j+1]]=Num_L_cross_chip[chip[j+1]]+F_counts[k];
					j++;
				}
			}
			i=i+bound;
		}
		else{
			int j = i;
			while(chip[j]!=chip[i+NumofTerm-1]){
				while(chip[j]==chip[j+1] && j < (i+NumofTerm-1)){
					j=j+1;
				}
				if(j!=(i+NumofTerm-1)){
					for(int k=j+1;k>i;k--)
						Num_L_cross_chip[chip[j+1]]=Num_L_cross_chip[chip[j+1]]+F_counts[k];
					j++;
				}
			}
			i=i+NumofTerm;
		}
	}
	if(DEBUG==1)
		cout<< "----------cross chip table creation end ----------" << endl << endl;
}

//送入chip所需的diff資訊
int create_diff(int F_int){
	int j=0; //j會指到每一組的第一個($AA、$CA...)
	int diff=0;
	int bound=pow(4,M_Fterm-M_Lterm);
	while(j <= cross_index[chip[F_int]-1]+1 && chip[F_int]!=0){
		if(arr[j]==0){
			j=j+bound;
		}
		else{
			j=j+calculateNumofTerm(M_Fterm-M_Lterm);
		}
		if(F_int < j)
			diff=1;
	}
	//cout << "diff = " << diff <<endl;
	return diff;
}
//chip內計算
int OCC_recover(int index, int L_int, int F_int){
	/*int global_index = 0; //global_index = 目標cip之前有存多少L資訊+local_index
	if(chip[F_int]==0){
		global_index = index;
	}
	else{
		for(int i=0;i<chip[F_int];i++){
			global_index = global_index + local_index[cross_index[i]] + F_counts[cross_index[i]+1];
		}
		global_index = global_index + index;
	}
	cout << global_index << endl;
	*/
	//使用OCC做計算每個row的OCC記下不包含第一個L的global_OCC 
	int row_in_chip = index/DOWN_SAMPLED_RATE;
	//經過加工(實際上黑盒子取的還是global之中的，這裡的用意只是表示可以用local index來做計算)
	int order_in_row= index%DOWN_SAMPLED_RATE;
	int search_index;
	int OCC_in_chip;
	if(chip[F_int]==0 && row_in_chip==0){
		search_index= -1;
		OCC_in_chip = 0;
	}
	else{
		//取出 sampled point 的值，array做的事 black box in
		search_index=row_in_chip*DOWN_SAMPLED_RATE-1;
		
		for(int i=0;i<chip[F_int];i++){
			search_index = search_index + local_index[cross_index[i]] + F_counts[cross_index[i]+1];
		}
		//取出 sampled point 的值，array做的事 black box out
		OCC_in_chip = OCC_creation_for_huge_data(search_index,L_int);//OCC[search_index][L_int];
	}
	for(int i=0;i<=order_in_row;i++){
		if(L_term[search_index+1+i]==L_int){
			OCC_in_chip = OCC_in_chip + 1;
		}
	}
	return OCC_in_chip;//OCC[global_index][L_int]; 
}
void distributed_chip(int &max, int &min, int L_int, int F_int, int diff){
	int local_min;
	int local_max;
	//diff若是1,max和min需要修正 
	if(diff==1){
		local_min = min - Num_L_cross_chip[chip[F_int]]-1 ;
		local_max = max - Num_L_cross_chip[chip[F_int]];
	}
	else{
		local_min = min -1;
		local_max = max;
	}
	//更新min &max
	//cout<<"local_max1:"<<local_max<<endl;
	//cout<<"local_min1:"<<local_min<<endl;
	local_min = OCC_recover(local_min, L_int, F_int)+1;
	local_max = OCC_recover(local_max, L_int, F_int);
	//cout<<"local_max2:"<<local_max<<endl;
	//cout<<"local_min2:"<<local_min<<endl;

	min = local_min;
	max = local_max;
}

void cal_L_in_chip(){
	int j = 0;//order of chip
	if(DEBUG==1)
		cout<< "----------number of L store in each chip ----------" << endl;
	for(int i=0; i<NumOfF_term; i++){
		if(i!=NumOfF_term-1){
			Num_L_in_chip[j] = Num_L_in_chip[j] + F_counts[i+1];
		}
		else{
			Num_L_in_chip[j] = read_count;
			for(int k=0;k<NumOfDRAM-1;k++)
				Num_L_in_chip[j] = Num_L_in_chip[j] - Num_L_in_chip[k];
		}
		if(i==cross_index[j])
			j++;
	}
	if(DEBUG==1){
		for(int i=0;i<NumOfDRAM; i++)
			cout<< Num_L_in_chip[i] << " , ";
		cout << endl<<endl;
	}
}
//-----------------------DO NOT CHANGE--------------------------------------------
int get_F_int(string queries){
	string search_F=queries.substr(queries.size()-M_Fterm,M_Fterm);
	char strF[M_Fterm];
	for(int i=0;i<M_Fterm;i++){
		strF[i]=search_F[i];
	}
    return encode_Mchars_F(strF,0);	
}

int get_L_int(string queries){
	string search_L=queries.substr(queries.size()-M_Fterm-M_Lterm,M_Lterm);
	char strL[M_Lterm];//={search_L.c_str()};
	for(int i=0;i<M_Lterm;i++){
		strL[i]=search_L[i];
	}
	return encode_Mchars_L(strL,0);	
}

class query{
	public:
		string query_string;
		string orig_string;
		int max;
		int min;
		int L_int;
		int F_int;
		int diff;
		int search_round;
		bool is_in_queue;
		query(int a,int b,int c,int d,int e,string s){
			max=a;
			min=b;
			L_int=c;
			F_int=d;
			diff=e;
			search_round=(s.size()-M_Fterm)/M_Lterm;
			query_string=s;
			orig_string=s;
			is_in_queue=false;
		}
		
		void update(){
			//cout<<"SEARCH: "<<search_round<<" "<<NumOfL_term<<endl;
			if(search_round>1){
				query_string.erase(query_string.size()-M_Lterm,query_string.size()-1);	
				//cout<<"UPDATING  "<<query_string <<  "\n";
				L_int=get_L_int(query_string);
				F_int=get_F_int(query_string);
			}
			search_round--;

		}
};

//====MAIN FUNCTION=====
int main(int argc, char** argv){
	int CHIP_CYCLE[NumOfDRAM]={0};

	//===========================================
	// Read File
	//===========================================
	vector <string> queries;
	//-----read strings to be queried from file-----	
	
	read_queries(queries,argv[1]);	
	
	//-----read F, L, OCC tables-----

	NumOfF_term = calculateNumofTerm(M_Fterm);
 	NumOfL_term = calculateNumofTerm(M_Lterm);
	
	chip		=(int*)malloc(NumOfF_term*sizeof(int));
	local_index	=(int*)malloc(NumOfF_term*sizeof(int));
	arr			=(int*)malloc(NumOfF_term*sizeof(int));
	group_creation(M_Fterm, M_Lterm, 0);
	cout<<"before L creation\n";
	//L_term OCC F_table reads   from file
    L_term	= inputReads(read_L_file, &read_count);
	//OCC    	= OCC_creation(read_OCC_file);
	cout<<"before F creation\n";
	//read_F_file=argv[2];
	//cout<<read_F_file;

	F_counts= F_creation(argv[2]);
	cout<<"after F creation\n";
	
	//------ host mapping table creation    ------//
	//------ 先算平均要有幾條               ------//
	int L_per_chip = read_count/NumOfDRAM;
	//------ 每個chip可以放的數>=實際放的數 ------//
	int max_L_per_chip = L_per_chip*1.39;
	int mapping_table_success = mapping_table_creation(max_L_per_chip, local_index);
	if(mapping_table_success == 1){
		cross_chip_table_creation(local_index, F_counts);
		cal_L_in_chip();
	}
	else{
			cout<<"FAILED\n";
			return 0;
	}

	//==============================================================================

	cout<<"QUERY NUM:"<<queries.size()<<endl;

	//for(int i=0;i<queries.size();i++)CHIP_CYCLE[]
	//==============================================================================
	//QUERY PART
	//==============================================================================
	bool searching=true;
	
	int query_pointer=BATCH_SIZE;
	
    vector <query*> query_batch;
    vector <queue<query*> > QUEUE_LIST;
	vector <int>Q_empty_counter;
	vector <int>Q_len;
	ofstream OUT;
	OUT.open("host_query.txt");
	QUEUE_LIST.resize(NumOfDRAM);
	Q_len.resize(NumOfDRAM);
	Q_empty_counter.resize(NumOfDRAM);
	query_batch.resize(BATCH_SIZE);
	
	ofstream out[NumOfDRAM];
	for(int i=0;i<NumOfDRAM;i++){
		char I[10];
		sprintf(I, "%d", i);
		string trace(I);
		trace="traces/"+trace+".txt";
		out[i].open(trace.c_str());
	}
	//-------initial batch--------------
	for(int i=0;i<BATCH_SIZE&i<queries.size();i++){
		int F_int=get_F_int(queries[i]);
		int L_int=get_L_int(queries[i]);
		query* temp = new query(read_count,0,L_int,F_int,0,queries[i]);
		query_batch[i]=temp;
	}
	//----------------------------------
	int count=0;
	//=======STARTING QUERY===========
	int clk;
	for (clk=0;searching;clk++){
		//cout<<"CLK-"<<clk<<endl;
		for(int i=0;i<BATCH_SIZE&i<queries.size();i++){
			
			//-----replace with new query string in the batch if this query is done--
			if(query_batch[i]->search_round<1){
				if(query_pointer>=queries.size())continue;
				int F_int=get_F_int(queries[query_pointer]);
				int L_int=get_L_int(queries[query_pointer]);

				query* temp = new query(read_count,0,L_int,F_int,0,queries[query_pointer++]);
				query_batch[i]=temp;
			}			
			//============================================================
			if(!query_batch[i]->is_in_queue){
				query_batch[i]->is_in_queue=true;
				QUEUE_LIST[chip[query_batch[i]->F_int]].push(query_batch[i]);

			}
		}
		int empty_counter=0;
		for(int i=0;i<NumOfDRAM;i++){
			if(QUEUE_LIST[i].size()==0)
				empty_counter++;
		}
		if(empty_counter==NumOfDRAM){
			searching=false;
		}


		//------------------ chip內計算------------------ 

		for(int i=0;i<NumOfDRAM;i++){
			if(!QUEUE_LIST[i].empty()){
				Q_len[i]+=QUEUE_LIST[i].size();
				CHIP_CYCLE[i]++;

				//============GENERATE query data======================	
				if(QUEUE_LIST[i].front()->orig_string!=QUEUE_LIST[i].front()->query_string) 
					QUEUE_LIST[i].front()->diff=create_diff(QUEUE_LIST[i].front()->F_int);

				//------ Mapping 到該去的 (chip,local_index)--//
				if(QUEUE_LIST[i].front()->orig_string==QUEUE_LIST[i].front()->query_string){
					QUEUE_LIST[i].front()->min = local_index	[QUEUE_LIST[i].front()->F_int];	//start point
					if(QUEUE_LIST[i].front()->F_int!=NumOfF_term-1)
						QUEUE_LIST[i].front()->max = local_index	[QUEUE_LIST[i].front()->F_int] + F_counts	[QUEUE_LIST[i].front()->F_int+1]-1;
					else{
						int sum = read_count;
						for(int cnt=0; cnt<NumOfF_term ;cnt++)
							sum = sum - F_counts[cnt];
						QUEUE_LIST[i].front()->max = local_index	[QUEUE_LIST[i].front()->F_int] +sum-1;
					}				
				}else{
					//------------------ chip內計算------------------ 
					//chip內需要知道，各種F_term(前兩字)的local_index才可放入這一塊
					if(QUEUE_LIST[i].front()->diff==0){
						int size=QUEUE_LIST[i].front()->query_string.size();
						string F_cnt_s=QUEUE_LIST[i].front()->query_string.substr(size-M_Fterm,M_Fterm);
						//cout<<"CHECK  "<< F_cnt_char<<"\n";
						char F_cnt_char[M_Fterm];
//						for(int x=0;x<M_Fterm;x++)
	//						F_cnt_char[x]=F_cnt_s[x];
						
						for(int k=0;k<M_Fterm;k++){
							if(M_Lterm>k)
								F_cnt_char[k]=F_cnt_s[k];
							else if(M_Lterm==k)
								F_cnt_char[k]='$';
							else
								F_cnt_char[k]='A';
						}

						int F_cnt=encode_Mchars_F(F_cnt_char,0);
						QUEUE_LIST[i].front()->min = local_index	[F_cnt] + QUEUE_LIST[i].front()->min -1;
						QUEUE_LIST[i].front()->max = local_index	[F_cnt] + QUEUE_LIST[i].front()->max -1;
					}
					else{//to be continued
						QUEUE_LIST[i].front()->min = QUEUE_LIST[i].front()->min -1;
						QUEUE_LIST[i].front()->max = QUEUE_LIST[i].front()->max -1;
					}
					//------------------ chip內計算------------------ 
				}
				//============GENERATE query data======================	
				//distributed_chip(QUEUE_LIST[i].front()->max, QUEUE_LIST[i].front()->min, QUEUE_LIST[i].front()->L_int ,QUEUE_LIST[i].front()->F_int, QUEUE_LIST[i].front()->diff);
	         	out[i]<<"CHIP:"<< i<<",LOCAL_MIN:"<< QUEUE_LIST[i].front()->min <<",LOCAL_MAX:"<<  QUEUE_LIST[i].front()->max <<",L_INT:"<<  QUEUE_LIST[i].front()->L_int<<endl;
				QUEUE_LIST[i].front()->update();

				QUEUE_LIST[i].front()->is_in_queue=false;
				if(QUEUE_LIST[i].front()->search_round<1){
					string L_cnt_s=QUEUE_LIST[i].front()->query_string.substr(QUEUE_LIST[i].front()->query_string.size()-M_Lterm-M_Fterm,M_Lterm);
					char L_cnt_char[M_Fterm];
					for(int k=0;k<M_Fterm;k++){
						if(M_Lterm>k)
							L_cnt_char[k]=L_cnt_s[k];
						else if(M_Lterm==k)
							L_cnt_char[k]='$';
						else
							L_cnt_char[k]='A';						
					}
					int  L_cnt=encode_Mchars_F(L_cnt_char,0);
					int total=0;
					for(int global_index_cnt = 0;global_index_cnt<=L_cnt;global_index_cnt++){
						total=total+F_counts[global_index_cnt];					
					}
					OUT<<QUEUE_LIST[i].front()->orig_string<<": "<<QUEUE_LIST[i].front()->min+total-1 <<" "<< QUEUE_LIST[i].front()->max+total-1<<endl;

					count++;
				}
				QUEUE_LIST[i].pop();
			}else{
				Q_empty_counter[i]++;
			}

		}
		//------------------ chip內計算------------------ 
		//cout<<clk<<endl;
	}
	int max_cycle=0;
	int max_cycle_id=0;
	for(int i=0;i<NumOfDRAM;i++){
		if(CHIP_CYCLE[i]>max_cycle){
			max_cycle=CHIP_CYCLE[i];
			max_cycle_id=i;
		}
	}
	ofstream o_end;
	o_end.open("traces/max_index.txt");
	o_end<<max_cycle_id<<endl;
	for(int i=0;i<NumOfDRAM;i++)
		cout<<"CHIP ID:"<<i <<"CHIP_CYCLE:"<<CHIP_CYCLE[i]<<"\tEMPTY_CYCLE:"<<Q_empty_counter[i]<<"\tEMPTY_RATE:"<<100*((float)Q_empty_counter[i]/(float)CHIP_CYCLE[i])<<"% \tAVG QUEUE LEN:"<<((float)Q_len[i]/(float)clk)<<endl;
	cout<<"===================================================\n";
}










