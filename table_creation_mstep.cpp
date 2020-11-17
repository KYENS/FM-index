//************************************************************//
//															  //
//						 at 2020/11/16						  //
//						finish M variable, 					  //
//															  //
//															  //
//															  //
//															  //
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
#include "toolbox.h"
#include "ENCODE_L.h"
#include "ENCODE_F.h"
#include <string.h>
#define DOWN_SAMPLED_RATE 1

#define DEBUG 0
#define PRINT_FMINDEX_TABLE 1
//-----------------------Modify here---------------------------
//#define M_Fterm 3
//#define M_Lterm 2
char * read_data_file= "create_table_5step.txt";   // input DATA
//-------------------------------------------------------------
using namespace std;

//-----------------------DO NOT CHANGE--------------------------------------------

int read_count = 0;
int read_length = 0;

int **SA_Final;
char *L;
int NumOfF_term, NumOfL_term;

//Read file to get reads
char** inputReads(char *file_path, int *read_count, int *length){
    FILE *read_file = fopen(file_path, "r");
    int ch, lines=0;
    char **reads;
    do                                                                                                 
    {                                                                                                  
        ch = fgetc(read_file);                                                                            
        if (ch == '\n')                                                                                
            lines++;                                                                                   
    } while (ch != EOF);
    rewind(read_file);
    reads=(char**)malloc(lines*sizeof(char*));
    *read_count = lines;
    int i = 0;                                                                                         
    size_t len = 0;                                                                                    
    for(i = 0; i < lines; i++)                                                                         
    {
        reads[i] = NULL;
        len = 0;                                                                                
        getline(&reads[i], &len, read_file);
    }                                                                                                  
    fclose(read_file);
    int j=0;
    while(reads[0][j]!='\n')
        j++;
    *length = j+1;
    for(i=0;i<lines;i++)
        reads[i][j]='$';
    return reads;
}

//Rotate read by 1 character
void rotateRead(char *read, char *rotatedRead, int length){
    for(int i=0;i<length-1;i++)
        rotatedRead[i]=read[i+1];
    rotatedRead[length-1]=read[0];
}

//Generate Sufixes and their SA's for a read
char** generateSuffixes(char *read, int length, int read_id){
    char **suffixes=(char**)malloc(length*sizeof(char*));
    suffixes[0]=(char*)malloc(length*sizeof(char));
    for(int j=0;j<length;j++)
        suffixes[0][j]=read[j];
    for(int i=1;i<length;i++){
        suffixes[i]=(char*)malloc(length*sizeof(char));
        rotateRead(suffixes[i-1], suffixes[i], length);
    }
    return suffixes;
}

//Comparator for Suffixes
int compSuffixes(char *suffix1, char *suffix2, int length){
    int ret = 0;
    for(int i=0;i<length;i++){
        if(suffix1[i]>suffix2[i])
            return 1;
        else if(suffix1[i]<suffix2[i])
            return -1;
    }
    return ret;
}

// Defining comparator function as per the requirement
static int myCompare(const void *a, const void *b){
	return strcmp(*(const char **)a, *(const char **)b);
}
			
// Function to sort the array
void sort(const char *arr[], int n){
	// calling qsort function to sort the array
	// with the help of Comparator
	qsort(arr, n, sizeof(const char *), myCompare);
}
void sort_SA(const char *arr[], int n){
	// calling qsort function to sort the array
	// with the help of Comparator
	qsort(arr, n, sizeof(const char *), myCompare);
}


//Calculates the final FM-Index
int** makeFMIndex(char ***suffixes, int read_count, int read_length, int F_count[], char**L0){
    long i, j;

    SA_Final=(int**)malloc(read_count*read_length*sizeof(int*));
    for(i=0;i<read_count*read_length;i++)
        SA_Final[i]=(int*)malloc(2*sizeof(int));

    //Temporary storage for collecting together all suffixes
    char **temp_suffixes=(char**)malloc(read_count*read_length*sizeof(char*));

    //Initalization of temporary storage
    for(i=0;i<read_count;i++){
        for(j=0;j<read_length;j++){
            temp_suffixes[i*read_length+j]=(char*)malloc(read_length*sizeof(char));
            memcpy(&temp_suffixes[i*read_length+j], &suffixes[i][j],read_length*sizeof(char));
            SA_Final[i*read_length+j][0]=j;
            SA_Final[i*read_length+j][1]=i;
        }
    }
    
    char *temp=(char*)malloc(read_length*sizeof(char));
    
    int **L_count=(int**)malloc(read_length*read_count*sizeof(int*));
    for(i=0;i<read_length*read_count;i++){
        L_count[i]=(int*)malloc(4*sizeof(int));
        for(j=0;j<4;j++){
            L_count[i][j]=0;
        }
    }

	
	sort((const char**)temp_suffixes, read_count*read_length);

	sort_SA((const char**)SA_Final, read_count*read_length);
			

    free(temp);
    char this_F[M_Fterm];
	for(int i=0;i<M_Fterm;i++){
		this_F[i]= '$';
	}
	cout << this_F <<endl;
    j=0;
	
	if (DEBUG == 1){
		cout << "read_count = "  <<read_count <<endl;
		cout << "read_length = " <<read_length<<endl;
    }
	//Calculation of F_count's
	
    for(i=0;i<read_count*read_length;i++){
		
        int count=0;

		//?™é??¯ä»¥è®Šæ?ä¸‰å€‹å??„suffix æ¯”è?
        while(strncmp(temp_suffixes[i], this_F, M_Fterm)  == 0/*temp_suffixes[i][0]==this_F[0] && temp_suffixes[i][1]==this_F[1] && temp_suffixes[i][2]==this_F[2]*/){
            count++;i++;
        }

		if (DEBUG == 1){
			cout << temp_suffixes[i]/*temp_suffixes[i][0] << temp_suffixes[i][1] << temp_suffixes[i][2] */<< ',';
			cout << "index = " << j << endl ;
		}
		F_count[j]=j==0?count:count+1; //[j++] 1 step
		j++;
		while(j-1!=encode_Mchars_F(temp_suffixes[i],0)){
			j++;
			F_count[j]=0;
		}
		//printf("%d %d\n",i,count);
		if(j-1==encode_Mchars_F(temp_suffixes[i],0)){
			for(int k=0; k<M_Fterm; k++){
				this_F[k]=temp_suffixes[i][k];
			}
		}
		char end_suffix[M_Fterm];
		for(int k=0; k<M_Fterm; k++)
			end_suffix[k]='T';
		if(strncmp(end_suffix, temp_suffixes[i], M_Fterm)  == 0 /*temp_suffixes[i][0]==end_suffix[0] && temp_suffixes[i][1]==end_suffix[1] && temp_suffixes[i][2] ==end_suffix[2]*/)
            break;
		free(temp_suffixes[i]);
    }
	if(DEBUG == 1){
		int i ;
		for(i=0; i<NumOfF_term;i++)
		cout << "F_count[" << i << "] = " << F_count[i] <<endl;
		//cout << F_count[ 0] << ' ' << F_count[ 1] << ' ' << F_count[ 2] << ' '<< F_count[ 3] <<endl;
		//cout << F_count[ 4] << ' ' << F_count[ 5] << ' ' << F_count[ 6] << ' '<< F_count[ 7] <<endl;
		//cout << F_count[ 8] << ' ' << F_count[ 9] << ' ' << F_count[10] << ' '<< F_count[11] <<endl;
		//cout << F_count[12] << ' ' << F_count[13] << ' ' << F_count[14] << ' '<< F_count[15] <<endl;
		//cout << F_count[16] << ' ' << F_count[17] << ' ' << F_count[18] << ' '<< F_count[19] <<endl;
		//cout << F_count[20] << ' ' << F_count[21] << ' ' << F_count[22] << ' '<< F_count[23] <<endl;
	}
    //Calculation of L's and L_count's
    for(i=0;i<read_count*read_length;i++){
        for(int j=0;j<M_Lterm;j++){
			L0[j][i] = temp_suffixes[i][read_length-1-(M_Lterm-1)+j];

		}
		/*char ch = temp_suffixes[i][read_length-1];
        char ch0 = temp_suffixes[i][read_length-2];
		L0[0][i]=ch0;
		L0[1][i]=ch;*/
        //if(i>0){
        //    for(int k=0;k<4;k++)
        //        L_count[i][k]=L_count[i-1][k];
        //}
        //if(ch=='A')
        //    L_count[i][0]++;
        //else if(ch=='C')
        //    L_count[i][1]++;
        //else if(ch=='G')
        //    L_count[i][2]++;
        //else if(ch=='T')
        //    L_count[i][3]++;
    }
	if(PRINT_FMINDEX_TABLE){
		FILE *FMINDEX=fopen("FMindex.txt","w");
		for(i=0;i<read_count*read_length;i++)
        fprintf(FMINDEX,"%d , %s\n",i,temp_suffixes[i]);
	}
    return 0;//L_count;
}

//OCC table creation
void OCC_creation(/*int **OCC_1,*/ char **L_term, int read_length, int read_count){
	int OCC[NumOfL_term];
	for(int i=0;i<NumOfL_term;i++)
		OCC/*[0]*/[i] = 0;
	FILE *L_file = fopen("L_term.txt", "r");
	FILE *output_OCC = fopen("OCC.txt", "w");
	int index;
	for(long i=0;i<read_count*read_length;i++){
		/*for(int j=0;j<NumOfL_term;j++)
			if(i!=0)
				OCC[i][j] = OCC[i-1][j];*/
		fscanf(L_file,"%d",&index);
		OCC/*[i]*/[index]++;
		for(int k=0;k<NumOfL_term-1;k++)
			fprintf(output_OCC,"%d ",OCC/*[i]*/[k]);
		fprintf(output_OCC,"%d\n",OCC/*[i]*/[NumOfL_term-1]);
		/*switch(L_term[0][i]){
			case '$':
				switch(L_term[1][i]){
					case 'A':
						OCC[i][0]++;
					break;
					case 'C':
						OCC[i][1]++;
					break;
					case 'G':
						OCC[i][2]++;
					break;
					case 'T':
						OCC[i][3]++;
					break;
				}
			break;
			case 'A':
				switch(L_term[1][i]){
					case '$':
						OCC[i][4]++;
					break;
					case 'A':
						OCC[i][5]++;
					break;    
					case 'C':  
						OCC[i][6]++;
					break;    
					case 'G':  
						OCC[i][7]++;
					break;
					case 'T':
						OCC[i][8]++;
					break;
				}
			break;
			case 'C':
				switch(L_term[1][i]){
					case '$':
						OCC[i][9]++;
					break;
					case 'A':
						OCC[i][10]++;
					break;
					case 'C':
						OCC[i][11]++;
					break;
					case 'G':
						OCC[i][12]++;
					break;
					case 'T':
						OCC[i][13]++;
					break;
				}
			break;
			case 'G':
				switch(L_term[1][i]){
					case '$':
						OCC[i][14]++;
					break;
					case 'A':
						OCC[i][15]++;
					break;
					case 'C':
						OCC[i][16]++;
					break;
					case 'G':
						OCC[i][17]++;
					break;
					case 'T':
						OCC[i][18]++;
					break;
				}
			break;
			case 'T':
				switch(L_term[1][i]){
					case '$':
						OCC[i][19]++;
					break;
					case 'A':
						OCC[i][20]++;
					break;
					case 'C':
						OCC[i][21]++;
					break;
					case 'G':
						OCC[i][22]++;
					break;
					case 'T':
						OCC[i][23]++;
					break;
				}
			break;
		}*/
	}
	fclose(output_OCC);
	fclose(L_file);
}

void Write_file(int *F_count, char ** L_term, int read_length, int read_count){
	long i ,j;
	FILE *output_F_count = fopen("F_count.txt", "w");
	for(i=0;i<NumOfF_term;i++)
		fprintf(output_F_count,"%d\n",F_count[i]);
	fclose(output_F_count);
	
	FILE *output_L_term = fopen("L_term.txt", "w");
	for(i=0;i<read_length*read_count;i++){
		/*switch(L_term[0][i]){
			case '$':
				switch(L_term[1][i]){
					case 'A':
						fprintf(output_L_term,"0\n");
					break;
					case 'C':
						fprintf(output_L_term,"1\n");
					break;
					case 'G':
						fprintf(output_L_term,"2\n");
					break;
					case 'T':
						fprintf(output_L_term,"3\n");
					break;
				}
			break;
			case 'A':
				switch(L_term[1][i]){
					case '$':
						fprintf(output_L_term,"4\n");
					break;
					case 'A':
						fprintf(output_L_term,"5\n");
					break;    
					case 'C':  
						fprintf(output_L_term,"6\n");
					break;    
					case 'G':  
						fprintf(output_L_term,"7\n");
					break;
					case 'T':
						fprintf(output_L_term,"8\n");
					break;
				}
			break;
			case 'C':
				switch(L_term[1][i]){
					case '$':
						fprintf(output_L_term,"9\n");
					break;
					case 'A':
						fprintf(output_L_term,"10\n");
					break;
					case 'C':
						fprintf(output_L_term,"11\n");
					break;
					case 'G':
						fprintf(output_L_term,"12\n");
					break;
					case 'T':
						fprintf(output_L_term,"13\n");
					break;
				}
			break;
			case 'G':
				switch(L_term[1][i]){
					case '$':
						fprintf(output_L_term,"14\n");
					break;
					case 'A':
						fprintf(output_L_term,"15\n");
					break;
					case 'C':
						fprintf(output_L_term,"16\n");
					break;
					case 'G':
						fprintf(output_L_term,"17\n");
					break;
					case 'T':
						fprintf(output_L_term,"18\n");
					break;
				}
			break;
			case 'T':
				switch(L_term[1][i]){
					case '$':
						fprintf(output_L_term,"19\n");
					break;
					case 'A':
						fprintf(output_L_term,"20\n");
					break;
					case 'C':
						fprintf(output_L_term,"21\n");
					break;
					case 'G':
						fprintf(output_L_term,"22\n");
					break;
					case 'T':
						fprintf(output_L_term,"23\n");
					break;
				}
			break;
		}*/
		char *str=(char *)malloc(M_Lterm*sizeof(char));
		for(int j=0; j<M_Lterm; j++){
			str[j] =L_term[j][i];
		}
		fprintf(output_L_term,"%d\n",encode_Mchars_L(str,0));
		free(str);
	}
	fclose(output_L_term);
}

void Write_file_OCC(int **OCC){
	FILE *output_OCC = fopen("OCC.txt", "w");
	for(long i=0;i<read_count*read_length;i=i+DOWN_SAMPLED_RATE){
		for(int k=0;k<NumOfL_term-1;k++)
			fprintf(output_OCC,"%d ",OCC[i][k]);
		fprintf(output_OCC,"%d\n",OCC[i][NumOfL_term-1]);
	}
	fclose(output_OCC);
}
//-----------------------DO NOT CHANGE--------------------------------------------

int main(int argc, char *argv[]){

    char **reads = inputReads(read_data_file, &read_count, &read_length);//Input reads from file

    char ***suffixes=(char***)malloc(read_count*sizeof(char**));//Storage for read-wise suffixes
 	NumOfF_term = calculateNumofTerm(M_Fterm);
 	NumOfL_term = calculateNumofTerm(M_Lterm);
	int F_counts[NumOfF_term];
	for(int i=0;i<NumOfF_term;i++){
		F_counts[i]=0;
	}
    //-----------------------------Structures for correctness check----------------------------------------------
	char *L_term[M_Lterm];
	for(int i =0; i< M_Lterm; i++){
		L_term[i]=(char*)malloc(read_count*read_length*sizeof(char*));//Final storage for last column of sorted suffixes
	}
	//L_term[0]=(char*)malloc(read_count*read_length*sizeof(char*));//Final storage for last column of sorted suffixes
	//L_term[1]=(char*)malloc(read_count*read_length*sizeof(char*));//Final storage for last column of sorted suffixes
    
	//-----------------------------Structures for correctness check----------------------------------------------

    //Generate read-wise suffixes
    for(long i=0;i<read_count;i++){
        suffixes[i]=generateSuffixes(reads[i], read_length, i);
    }
   
    //Calculate finl FM-Index
    /*L_counts = */makeFMIndex(suffixes, read_count, read_length, F_counts, L_term/*L_term[0]*/);
	//---free memory----
	free(suffixes);
	free(reads);
	//---free memory----
	
	//OCC table creation
	//int *OCC[read_count*read_length];
	/*int **OCC=(int**)malloc(read_count*read_length*sizeof(int*));//[read_count*read_length];
	for(long i=0;i<read_count*read_length;i++)
		OCC[i] =(int*)malloc(NumOfL_term*sizeof(int));*///Final storage for last column of sorted suffixes
	//output file
	Write_file( F_counts, L_term, read_length, read_count);
	
	OCC_creation(/*OCC,*/ L_term, read_length, read_count);
	//Write_file_OCC(OCC);
	//print result
	if(DEBUG ==1){
		int i,j;
		for(i=0;i<read_count;i++)
			for(j=0;j<read_length;j++){
				//cout << suffixes[i][j] << endl;		
			}
		
		for(i=0;i<read_count*read_length;i++){
			//cout << L_term[0][i] << L_term[1][i] <<" , ";
			//for(j=0;j<NumOfL_term;j++)
			//cout << OCC[i][j] << " ";
			//cout<< endl;
			
		}
		/*for(j=0;j<NumOfL_term;j++)
		cout << OCC[read_count*read_length-1][j] << " ";
		cout << endl;
		int total;
		for(int i=0;i<M_Lterm;i++)
			total = total+ OCC[read_count*read_length-1][i];
			cout << "TOTAL : " << total<< endl;*/
	}
    


	
    return 0;
}
