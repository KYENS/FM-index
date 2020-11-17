#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MAX_FILE_LEN 8192


typedef struct {
  int num;
  char str[MAX_FILE_LEN];
} Pair;

typedef struct {
  char F;
  char L;
}  FL_Pair;


//================================================
//				compare string
//================================================
int compare(const void *x, const void *y) {
  return strcmp(((Pair *)x)->str, ((Pair *)y)->str);
}
int compare_s(const void *x, const void *y) {
	return strcmp(*(const char **)x, *(const char **)y);
}

int compare_c(const void *x, const void *y) {
 // return strcmp(((FL_Pair*)x)->F,((FL_Pair*)x)->L);
  
   return *(char*)x - *(char*)y;

}

//================================================================================================================================================================

//		MAIN

//================================================================================================================================================================
int main(int argc, char *argv[]) {

	// Read File
	char s[MAX_FILE_LEN];
	int len;
	int read_count=0;
	int sample_rate = 40;
	FILE *f = fopen("../create_table_5step.txt", "r");
	FILE *f_L = fopen("../L_term_1step.txt", "r");
	FILE *f_OCC = fopen("../OCC_1step.txt", "r");
	FILE *f_S_OCC = fopen("../SAMPLED_OCC_1step.txt", "r");
	if(f == NULL) printf("File Opening Error!");
	while(fgets(s, MAX_FILE_LEN + 1, f)){
		//for(int l=0;l<strlen(s)-1;l++)
		if(s[strlen(s)-1]=='\n') s[strlen(s)-1]=NULL;
		read_count++;
		len = strlen(s);
		//printf("%s %d\n",s,read_count);
	}
	rewind(f);

	int** OCC_array=(int**)malloc(read_count*sizeof(int*));
	int** sampled_OCC_array=(int**)malloc(read_count*sizeof(int*));
	char* L_array_large=(char*)malloc(read_count*(len+1)*sizeof(char));
	for(int i=0;i<read_count*(len+1);i++){
		//fscanf(f_L,"%c",&L_array_large[i]);
		fgets(s, MAX_FILE_LEN + 1, f_L);
		L_array_large[i]=s[0];
		//printf("%c-%d\n",L_array_large[i],i);
	}


	OCC_array=(int**)malloc((len+1)*read_count*sizeof(int*));
	for(int j=0;j<(len+1)*read_count;j++) OCC_array[j]=(int*)malloc(4*sizeof(int));
	for(int i=0;i<read_count*(len+1);i++){
		for(int j=0;j<4;j++){
			fscanf(f_OCC,"%d",&OCC_array[i][j]);
		//	printf("%d ",OCC_array[i][j]);
		}
	}

	sampled_OCC_array=(int**)malloc(((len+1)*read_count/sample_rate+1)*sizeof(int*));
	for(int j=0;j<((len+1)*read_count/sample_rate)+1;j++) sampled_OCC_array[j]=(int*)malloc(4*sizeof(int));
	for(int i=0;i<((len+1)*read_count/sample_rate+1);i++){
		for(int j=0;j<4;j++){
			fscanf(f_S_OCC,"%d",&sampled_OCC_array[i][j]);
			//printf("%d ",sampled_OCC_array[i][j]);
		}
		//printf("\n");
	}

	
	fclose(f);
	fclose(f_L);
	fclose(f_OCC);
	fclose(f_S_OCC);
	
	//================================================================================================================================================================ 

	//QUERYING

	//================================================================================================================================================================

  

	//free(OCC_array);

	int query_size = strlen(argv[1]);
	int search_round = query_size - 1;
	int range_max, range_min;
	char query[query_size];
	strcpy(query,argv[1]);
	//printf("QUERY STRING: ");
	//scanf("%s",&query);
	

	FILE *ff=fopen("range_result_log_2.txt","w");
  
	int found_count=0;
		int not_found_flag = 0;
		int A_offset = read_count ;
		int C_offset = read_count + OCC_array[(len+1)*read_count-1][0];
		int G_offset = read_count + OCC_array[(len+1)*read_count-1][0] + OCC_array[(len+1)*read_count-1][1];
		int T_offset = read_count + OCC_array[(len+1)*read_count-1][0] + OCC_array[(len+1)*read_count-1][1] + OCC_array[(len+1)*read_count-1][2];

		//int k=0;
		int A_range_max=C_offset - 1;
		int C_range_max=G_offset - 1;
		int G_range_max=T_offset - 1;
		int T_range_max=read_count*(len+1)-1;
		
		/*while(L_array_large[A_offset]=='$')	A_offset++;
		while(L_array_large[C_offset]=='$')	C_offset++;
		while(L_array_large[G_offset]=='$')	G_offset++;
		while(L_array_large[T_offset]=='$')	T_offset++;
*/
		printf("A_offset:%d C_offset:%d G_offset:%d T_offset:%d\n",A_offset,C_offset,G_offset,T_offset);

		switch(query[query_size - 1]){
			case('A'):range_min = A_offset;
					range_max = A_range_max;
					break;
			case('C'):range_min = C_offset;
					range_max = C_range_max;
					break;
			case('G'):range_min = G_offset;
					range_max = G_range_max;
					break;
			case('T'):range_min = T_offset;
					range_max = T_range_max;
					break;			
		}

		//printf("%s\n",L_array[r]);
		//printf("A C G T\n");
		//for(int LL=0;LL<len/sample_rate+1;LL++)printf("%d %d %d %d \n",sampled_OCC_array[r][LL][0],sampled_OCC_array[r][LL][1],sampled_OCC_array[r][LL][2],sampled_OCC_array[r][LL][3]);
		printf("\n\n================================================\n");
		printf("initial range_min: %d, initial range_max: %d\n", range_min, range_max);

		for(int i = 0; i < search_round; i++){
		//printf("--------------------------------------------------\n");
			//char target_F = query[query_size - 1 - i];
			
			
			char target_L = query[query_size - 2 - i];
			fprintf(ff,"target_L: %c\n", target_L);
			int OCC_index;
			int offset;
			int update_range_min = -1;
			int update_range_max = -1;
			
			switch(target_L){
				case('A'):OCC_index = 0;
						offset = A_offset;
						break;
				case('C'):OCC_index = 1;
						offset = C_offset;
						break;
				case('G'):OCC_index = 2;
						offset = G_offset;
						break;
				case('T'):OCC_index = 3;
						offset = T_offset;
						break;			
			}
			//printf("%d %d\n",sampled_OCC_array[range_min/sample_rate][OCC_index],OCC_index);
			if(range_min % sample_rate <= sample_rate / 2){
				update_range_min = offset + sampled_OCC_array[range_min/sample_rate][OCC_index] - 1;
				for(int k = 1; k <= range_min % sample_rate; k++){
					//printf("%d == %c\n",sample_rate * (range_min/sample_rate) + k,target_L);

					if(L_array_large[sample_rate * (range_min/sample_rate) + k] == target_L){
					  update_range_min += 1;
					}
				}
			}
			else if(range_min % sample_rate > sample_rate / 2){
				if(range_min/sample_rate + 1 < len/sample_rate + 1){
				  update_range_min = offset + sampled_OCC_array[range_min/sample_rate + 1][OCC_index] - 1;
				  for(int k = 0; k < sample_rate - range_min % sample_rate; k++){
					if(L_array_large[sample_rate * (range_min/sample_rate + 1) - k] == target_L){
					  update_range_min -= 1;
					}
				  }
				}
				else if(range_min/sample_rate + 1 >= len/sample_rate + 1){
				  update_range_min = offset + sampled_OCC_array[range_min/sample_rate][OCC_index] - 1;
				  for(int k = 1; k <= range_min % sample_rate; k++){
					if(L_array_large[sample_rate * (range_min/sample_rate) + k] == target_L){
					  update_range_min += 1;
					}
				  }
				}
			}
			if(L_array_large[range_min] != target_L) update_range_min += 1;


			if(range_max % sample_rate <= sample_rate / 2){
				update_range_max = offset + sampled_OCC_array[range_max/sample_rate][OCC_index] - 1;
				for(int k = 1; k <= range_max % sample_rate; k++){
					if(L_array_large[sample_rate * (range_max/sample_rate) + k] == target_L){
					  update_range_max += 1;
					}
				}
			}
			else if(range_max % sample_rate > sample_rate / 2){
				if(range_max/sample_rate + 1 < len/sample_rate + 1){
				  update_range_max = offset + sampled_OCC_array[range_max/sample_rate + 1][OCC_index] - 1;
				  for(int k = 0; k < sample_rate - range_max % sample_rate; k++){
					if(L_array_large[sample_rate * (range_max/sample_rate + 1) - k] == target_L){
					  update_range_max -= 1;
					}
				  }
				}
				else if(range_max/sample_rate + 1 >= len/sample_rate + 1){
				  update_range_max = offset + sampled_OCC_array[range_max/sample_rate][OCC_index] - 1;
				  for(int k = 1; k <= range_max % sample_rate; k++){
					if(L_array_large[sample_rate * (range_max/sample_rate) + k] == target_L){
					  update_range_max += 1;
					}
				  }
				}
			}
			fprintf(ff,"updated range_min: %d, updated range_max: %d\n", update_range_min, update_range_max);

			/*if(update_range_min > update_range_max){
			  not_found_flag = 1;
			  break;
			}*/
			range_min = update_range_min;
			range_max = update_range_max;			
		}
		FILE *output=fopen("output_1step.txt","w");
		fprintf(output,"%d %d",range_min, range_max);
		fclose(output);
		
		fprintf(ff,"position of the query:\t");
		if(not_found_flag == 0){
			for(int i = range_min; i <= range_max; i++){
				//fprintf(ff,"%d  ", SA_array_large[i]);
				found_count++;
			}
			//fprintf(ff,"Range %d ~ %d", range_min+len*r,range_max+len*r);
		}
		else fprintf(ff,"not found");
		fprintf(ff,"\n");
	fprintf(ff,"TOTAL FOUND: %d \n",found_count);
	return 0;
}