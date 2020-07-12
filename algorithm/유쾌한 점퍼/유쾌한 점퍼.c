#include <stdio.h>
#include <string.h>
#include <math.h>
#define MAX 3000

void main(){

	int count;
	int i,j;
	int n[MAX];
	int a=0;

	
	scanf("%d",&count);

	for(i=0; i<count; i++){
		scanf("%d",&n[i]);
	}

	for(i=0; i<count-1; i++){
		if(abs(n[i]-n[i+1])>count) a++; //break;
		
	}

	if(a!=0) printf("Not Jolly\n");

	else printf("Jolly\n");
	
	


}