#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MAX_FILE_LEN 101


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

//================================================
// 				create_suffix_array
//================================================

static void create_suffix_array(char *str, int len, char *F, char *L, int *SA,char **str_array) {
  // add $ -> len+1
	Pair p[len + 1]; 
	char strs[len + 1];
	char tmp;
	sprintf(strs, "%s%s", str, "$");
  

	// Create_char_pair		//printf("-- Before Sort --\n");
	for (int i = 0; i < len + 1; i++) {
		p[i].num = i;
		const char *ctmp = strs;
		strcpy(p[i].str, ctmp);

		tmp = strs[0];
		int j;
		for (j = 0; j  < len; j++) 
			strs[j] = strs[j + 1];
		strs[j] = tmp;
	}

	// Quick_sort
	qsort(p, len + 1, sizeof(Pair), compare);

	// Create suffix array  	//printf("-- After Sort --\n");
	memset(L,NULL,sizeof(L));
	memset(F,NULL,sizeof(F));
	for(int i=0;i<len+1;i++)memset(str_array[i],NULL,sizeof(char*));
	//printf("%d %s\n",0,L);

	for (int i = 0; i < len + 1; i++) {
		SA[i] = p[i].num;
		sprintf(str_array[i], "%s", p[i].str);
		// suffix add
		if (!L) sprintf(L, "%c", p[i].str[len]);
		else	sprintf(L, "%s%c", L, p[i].str[len]);

		if (!F)sprintf(F, "%c", p[i].str[0]);
		else   sprintf(F, "%s%c", F, p[i].str[0]);
		//printf("%d-P-%s==%d\n",i,p[i].str,p[i].num);
	//	if(i==len)printf("%d--%s\n",i,F);
	}
}
//================================================
// 				create_occ_array
//================================================

static void create_occ(int read_count, int len,int sample_rate,int** sampled_OCC,int** OCC,char *L) {
	int OCC_A = 0;
	int OCC_C = 0;
	int OCC_G = 0;
	int OCC_T = 0;
  
	//printf("A,C,G,T\n");
	for(int i = 0; i < (len+1)*read_count; i++){
		switch(L[i]){
			case('A'):OCC_A = OCC_A + 1;
			break;
			case('C'):OCC_C = OCC_C + 1;
			break;			
			case('G'):OCC_G = OCC_G + 1;
			break;
			case('T'):OCC_T = OCC_T + 1;
			break;			
			
		}
		OCC[i][0] = OCC_A;
		OCC[i][1] = OCC_C;
		OCC[i][2] = OCC_G;
		OCC[i][3] = OCC_T;
		//printf("%d,%d,%d,%d\n",OCC_A,OCC_C,OCC_G,OCC_T);
		if(i % sample_rate == 0){
			//printf("%d  %d\n",i,i/sample_rate);
			for(int j = 0; j < 4; j++){
				sampled_OCC[i/sample_rate][j] = OCC[i][j];
			}
		}
	}
}	

//================================================================================================================================================================

//		MAIN

//================================================================================================================================================================
int main() {


	// Read File
	char s[MAX_FILE_LEN];
	int len;
	int read_count=0;
	int sample_rate = 40;
	FILE *f = fopen("create_table_5step.txt", "r");
	if(f == NULL) printf("File Opening Error!");
	while(fgets(s, MAX_FILE_LEN + 1, f)){
		//for(int l=0;l<strlen(s)-1;l++)
		if(s[strlen(s)-1]=='\n') s[strlen(s)-1]=NULL;
		len = strlen(s);
		read_count++;
		//printf("%s %d\n",s,read_count);
	}
	rewind(f);
	printf("read_count= %d, read_count= %d  \n",read_count,len);
	//read_count=14000;

	char* L_array_large=(char*)malloc(read_count*(len+1)*sizeof(char));
	char** STR_array_large=(char**)malloc(read_count*(len+1)*sizeof(char*));
	//int* SA_array_large=(int*)malloc(read_count*(len+1)*sizeof(int));
	//memset(L_array,0,sizeof(L_array));
	for(int i=0;fgets(s, MAX_FILE_LEN + 1, f);i++){		
		char* F_array=(char*)malloc((len+1)*sizeof(char));
		char* L_array=(char*)malloc((len+1)*sizeof(char));
		int*  SA_array=(int*)malloc((len+1)*sizeof(int));
		char** STR_array=(char**)malloc((len+1)*sizeof(char*));
		for(int j=0;j<len+1;j++)STR_array[j]=(char*)malloc((len+1)*sizeof(char));
		
		if(s[strlen(s)-1]=='\n') s[strlen(s)-1]=NULL;
		create_suffix_array(s, len, F_array, L_array, SA_array,STR_array);

		for(int j=0;j<len+1;j++)L_array_large[i*(len+1)+j]=L_array[j];
		//for(int j=0;j<len+1;j++)SA_array_large[i*(len+1)+j]=SA_array[j]+(len+1)*i;
		for(int j=0;j<len+1;j++)STR_array_large[i*(len+1)+j]=STR_array[j];

		free(F_array);
		free(L_array);
		free(SA_array);
		//for(int j=0;j<len+1;j++)free(STR_array[j]);
		//free(STR_array);
	}
	printf("CK000");
	// Quick_sort

	int** OCC_array=(int**)malloc(read_count*sizeof(int*));
	int** sampled_OCC_array=(int**)malloc(read_count*sizeof(int*));
	

	Pair* p=(Pair*)malloc((len + 1)*read_count*sizeof(Pair));
	// Create_char_pair		//printf("-- Before Sort --\n");
	for (int i = 0; i < (len + 1)*read_count; i++) {
		//p[i].num = SA_array_large[i];
		strcpy(p[i].str, STR_array_large[i]);
	}
	qsort(p, (len + 1)*read_count, sizeof(Pair), compare);
	for(int i=0;i<(len + 1)*read_count;i++){
		//F_array_large[i]=p[i].str[0];
		L_array_large[i]=p[i].str[len];
		//SA_array_large[i]=p[i].num;
		//printf("%c\n",L_array_large[i]);
	}

	OCC_array=(int**)malloc((len+1)*read_count*sizeof(int*));
	for(int j=0;j<(len+1)*read_count;j++) OCC_array[j]=(int*)malloc(4*sizeof(int));

	sampled_OCC_array=(int**)malloc(((len+1)*read_count/sample_rate+1)*sizeof(int*));
	for(int j=0;j<((len+1)*read_count/sample_rate)+1;j++) sampled_OCC_array[j]=(int*)malloc(4*sizeof(int));
	//printf("%d  OO\n",(len+1)*read_count/sample_rate);
	create_occ(read_count,len,sample_rate,sampled_OCC_array,OCC_array, L_array_large);

	
	//for(int i=0;i<read_count*(len+1);i++)printf("%d-F:%c,L:%c SA:%d\n",i,F_array_large[i],L_array_large[i],SA_array_large[i]);
	fclose(f);
	
	
	FILE *f_L = fopen("L_term_1step.txt", "w");
	if(f_L == NULL) printf("File Opening Error!");
	for(int i=0;i<read_count*(len+1);i++)fprintf(f_L,"%c\n",L_array_large[i]);
	fclose(f_L);
	
	FILE *f_OCC = fopen("OCC_1step.txt", "w");
	if(f_OCC == NULL) printf("File Opening Error!");
	for(int i=0;i<read_count*(len+1);i++){
		for(int j=0;j<4;j++)
			fprintf(f_OCC,"%d\t",OCC_array[i][j]);
		fprintf(f_OCC,"\n");
	}
	fclose(f_OCC);
	
	FILE *f_SAMPLED_OCC = fopen("SAMPLED_OCC_1step.txt", "w");
	if(f_SAMPLED_OCC == NULL) printf("File Opening Error!");
	for(int i=0;i<((len+1)*read_count/sample_rate+1);i++){
		for(int j=0;j<4;j++)
			fprintf(f_SAMPLED_OCC,"%d\t",sampled_OCC_array[i][j]);
		fprintf(f_SAMPLED_OCC,"\n");
	}
	fclose(f_SAMPLED_OCC);


	return 0;
}