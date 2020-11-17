int Combination(int m, int n){
	int i=0;
	int result=1;
	for(i=0;i<n;i++){
		result= result * (m-i);
	}
	for(i=0;i<n;i++){
		result= result / (n-i);
	}
	return result;
}
int calculateNumofTerm(int M){
	
	int i=0;
	int result=1;
	for(i=0;i<M;i++){
		result = result * 5;
	}
	int temp = 1;
	for(i=0;i<M-1;i++){
		result = result - Combination(M,M-i)*temp;
		temp = temp*4;
	}
	return result;
}