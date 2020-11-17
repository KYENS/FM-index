//************************************************************//
//															  //
//					Version 4, at 2020/11/14				  //
//	finish M variable, Note: query_size = M_Fterm+M_Lterm*n.  //
//			search a query through different chips,			  //
//	remember to revise query_size when Search a new query,	  //
//															  //
//		finished m-step										  //
//  														  //
//															  //
//************************************************************//
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <cmath>
#include <stdlib.h>
#include <sys/time.h>
#include "../toolbox.h"
#include "../ENCODE_L.h"
#include "../ENCODE_F.h"  
#include <string.h>
#include <math.h>

#define DEBUG 0

//-----------------------Modify here---------------------------
#define DOWN_SAMPLED_RATE 5
//#define M_Fterm 3
//#define M_Lterm 2 
#define NumOfDRAM 128
char * read_L_file	= "../L_term.txt";   		// input DATA
char * read_OCC_file= "../OCC.txt";   	  		// input DATA
char * read_F_file	= "../F_count.txt";   	  	// input DATA
// max_L_per_chip should be modify for different capacity.
//------------------------------------------------------------

using namespace std;


int read_count = 0;
int NumOfF_term, NumOfL_term;

int *chip;
int *local_index;

int *arr;
int F_int;
int Num_L_in_chip[NumOfDRAM];
int Num_L_cross_chip[NumOfDRAM];
int cross_index[NumOfDRAM-1];

int  * L_term ;
int  **OCC;
int  *F_counts;
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
	FILE *read_file = fopen(file_path, "r");
	int* F =(int*)malloc(NumOfF_term*sizeof(int));
	for(int i=0;i<NumOfF_term;i++)
		fscanf(read_file, "%d", &F[i]);
	fclose(read_file);
	return F;
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
int create_diff(){
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
	cout << "diff = " << diff <<endl;
	return diff;
}

//chip內計算
int OCC_recover(int index, int L_int){
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

void distributed_chip(int *max, int *min, int L_int, int diff){
	int local_min;
	int local_max;
	//diff若是1,max和min需要修正 
	if(diff==1){
		local_min = *min - Num_L_cross_chip[chip[F_int]]-1 ;
		local_max = *max - Num_L_cross_chip[chip[F_int]];
	}
	else{
		local_min = *min -1;
		local_max = *max;
	}
	//更新min &max
	//cout<<"local_max1:"<<local_max<<endl;
	//cout<<"local_min1:"<<local_min<<endl;
	local_min = OCC_recover(local_min, L_int)+1;
	local_max = OCC_recover(local_max, L_int);
	//cout<<"local_max2:"<<local_max<<endl;
	//cout<<"local_min2:"<<local_min<<endl;

	*min = local_min;
	*max = local_max;
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
	
	ofstream outL;
	outL.open("Num_L_in_chip.txt");
	for(int i=0;i<NumOfDRAM; i++)
		outL<< Num_L_in_chip[i] << ",";
	outL.close();
}
//-----------------------DO NOT CHANGE--------------------------------------------

int main(int argc, char *argv[]){

	NumOfF_term = calculateNumofTerm(M_Fterm);
 	NumOfL_term = calculateNumofTerm(M_Lterm);

	chip		=(int*)malloc(NumOfF_term*sizeof(int));
	local_index	=(int*)malloc(NumOfF_term*sizeof(int));
	arr			=(int*)malloc(NumOfF_term*sizeof(int));
	group_creation(M_Fterm, M_Lterm, 0);
	//L_term reads   from file
    L_term	= inputReads(read_L_file, &read_count);
	//OCC table read from file
	//OCC    	= OCC_creation(read_OCC_file);
	//F_counts read  from file
	F_counts= F_creation(read_F_file);
	
	//------ host mapping table creation    ------//
	//------ 先算平均要有幾條               ------//
	int L_per_chip = read_count/NumOfDRAM;
	//------ 每個chip可以放的數>=實際放的數 ------//
	//int max_L_per_chip = 4000;
	/*int largest=0, record;
	for(int i=0;i<NumOfF_term;i++){
		if(F_counts[i]>=largest){
			largest=F_counts[i];
			record=i;
		}
	}
	cout << "largest num = " << largest <<endl;*/
	int max_L_per_chip = L_per_chip*5;
	int mapping_table_success = mapping_table_creation(max_L_per_chip, local_index);
	//int cross_index[NumOfDRAM-1];

	//  NOTE: 記得要table也要標示跨chip有多少個，
	//  這個資料要存在chip裡 (使用global做運算)。
	if(mapping_table_success == 1){
		cross_chip_table_creation(local_index, F_counts);
//		if(DEBUG==1)	cal_L_in_chip();
		cal_L_in_chip();
	
		if(DEBUG==1){
			int i = 0;
			cout<< "---------- cross chip table ----------" <<endl;
			for(i=0;i<NumOfDRAM;i++)
				cout << Num_L_cross_chip[i] << " , ";
			cout<<endl;
			cout<< "---------- cross chip table ----------" <<endl<<endl;
		}
		//------         chip QUERY PART        ------//
		//QUERYING
		int query_size = strlen(argv[1]);
		cout << query_size <<endl;
		int search_round = (query_size - M_Fterm)/M_Lterm;//第一輪不算,最後一輪不算
		int range_max = read_count;
		int range_min = 0;
		char query[query_size];
		strcpy(query, argv[1]);
		char search_F[M_Fterm] ;
		char search_L[M_Lterm] ;
		char F_cnt_char[M_Fterm];
		char L_cnt_char[M_Fterm];
		for(int i=0;i<search_round;i=i+1){
		//------  先有query資訊 F_term & L_term ------//
			for(int k=0;k<M_Fterm;k++){
				search_F[k] = query[query_size-1-(M_Fterm-1-k)-M_Lterm*i];
			}
			for(int k=0;k<M_Lterm;k++){
				search_L[k] = query[query_size-1-(M_Fterm-1-k)-M_Lterm*i-M_Lterm];
			}

			if(DEBUG==1){
				cout <<endl;
				cout <<"search_round = " 	<< i << " : ";
				cout << "search_F = " 		<< search_F << " , ";
				cout << "search_L = " 		<< search_L << " , ";
			}
				F_int=encode_Mchars_F(search_F,0);
			int L_int=encode_Mchars_L(search_L,0);
			if(DEBUG==1){
				cout << "F_int = " 		<< F_int << " , ";
				cout << "L_int = " 		<< L_int << endl;
			}
			
			int diff;
			if(i==0){
				diff=0;
			}
			else{
				diff = create_diff();
			}
		//------ Mapping 到該去的 (chip,local_index)--//
			if(i==0){
				range_min = local_index	[F_int];	//start point
				if(F_int!=NumOfF_term-1)
					range_max = local_index	[F_int] + F_counts	[F_int+1]-1;
				else{
					int sum = read_count;
					for(int cnt=0; cnt<NumOfF_term ;cnt++)
						sum = sum - F_counts[cnt];
					range_max = local_index	[F_int] +sum-1;
				}
				
			}
			else{
				//------------------ host內計算------------------ 
				//chip內需要知道，各種F_term(前兩字)的local_index才可放入這一塊
				if(diff==0){
					for(int k=0;k<M_Fterm;k++){
						if(M_Lterm>k)
							F_cnt_char[k]=search_F[k];
						else if(M_Lterm==k)
							F_cnt_char[k]='$';
						else
							F_cnt_char[k]='A';
					}
					int F_cnt=encode_Mchars_F(F_cnt_char,0);
					cout<<F_cnt<<"->LOCAL IDX::"<<local_index	[F_cnt]<<"F CNT::"<<F_cnt<<endl;
					range_min = local_index	[F_cnt] + range_min -1;
					range_max = local_index	[F_cnt] + range_max -1;
				}
				else{//to be continued
					range_min = range_min -1;
					range_max = range_max -1;
				}
	
				//------------------ host計算------------------ 
			}
	
			cout << "start postion min = "<< range_min <<" , ";
			cout << "max = " << range_max << endl;
	
		//  Search begin
			int *max= &range_max;
			int *min= &range_min;
	
			//------------------ chip內計算------------------ 
			cout<<"MAX:"<<*max   <<", MIN:"<<*min <<", L_int:"<<L_int <<", F_int:"<<F_int <<", DIFF:"<<diff <<", local index:"<<local_index[F_int]<<endl;
			distributed_chip(max, min, L_int, diff);
			
			//------------------ chip內計算------------------ 
			cout << "end postion min = "<< *min <<" , ";
			cout << "max = " << *max  << endl;
			
			//-----------------以下for對答案-----------------
			for(int k=0;k<M_Fterm;k++){
				if(M_Lterm>k)
					L_cnt_char[k]=search_L[k];
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
			if(i==search_round-1){
				FILE *output=fopen("output_mstep.txt","w");
				fprintf(output,"%d %d",total+*min-1, total+*max-1);
				fclose(output);
			}
			//-----------------以上for對答案-----------------
			total=0;
			cout << "------------------------------------------------------------ "<<endl<<endl<<endl;
			
		//  NOTE: systemC上 chip是int表示才能共用function，	//
		//  local_index會是bit表示是真正要送進chip的資料。	//
	
		}
	}
	else{
		FILE *output=fopen("output_mstep.txt","w");
		fclose(output);
	}
	//print result
    if(DEBUG ==1){
		int i,j;
		for(i=0;i<read_count;i++){
			//cout << "L_term[" << i << "] = " << L_term[i] <<endl;
		}
		cout << "last OCC: ";
		for(int j=0;j<NumOfL_term;j++){
			cout << OCC[read_count-1][j] << " ";
		}
		cout << endl;
		int total = 0;
		for(int j=0;j<NumOfL_term;j++){
			total = total +OCC[read_count-1][j ];
		}
		
		cout << "TOTAL : " << total<< endl;
	}
    //
	return 0 ;
}
