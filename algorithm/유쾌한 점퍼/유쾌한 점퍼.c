#include <stdio.h>
#include <string.h>
#include <math.h>
#define MAX_BUF_SIZE 4096

void main(){
	int i, j, Count, Num = 0;
	int n[MAX_BUF_SIZE];
	
	scanf("%d",&count);

	for(i=0; i<count; i++) {
		scanf("%d",&n[i]);
	}

	for(i=0; i<count-1; i++) {
		if(abs(n[i]-n[i+1])>count) Num++; //break;
		
	}

	if(Num!=0) printf("Not Jolly\n");
	else printf("Jolly\n");
}