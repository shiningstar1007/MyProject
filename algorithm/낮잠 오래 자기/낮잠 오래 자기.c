#include <stdio.h>
#include <math.h>
#define MAX_BUF_SIZE 100

void main(){
	int time[MAX_BUF_SIZE][4];
	int nap[MAX_BUF_SIZE][2];	//시간차
	int spl[MAX_BUF_SIZE][4];	//시간
	int s,i,j; int sum=0;
	int count=0;
	int day=1;
	int test;
	
	scanf("%d",&test);
	while(test!=0){
		scanf("%d",&s);
		for(i=0; i<s; i++){
			for(j=0; j<4; j++){
				scanf("%d%c",&time[i][j]);
			}
			fflush(stdin);
		}

		for(i=0; i<s-1; i++){
			if(time[i+1][0]<time[i][0]){
				spl[i][0]=time[i+1][0]; spl[i][1]=time[i+1][1];
				spl[i][2]=time[i+1][2]; spl[i][3]=time[i+1][3];
				time[i+1][0]=time[i][0]; time[i+1][1]=time[i][1];
				time[i+1][2]=time[i][2]; time[i+1][3]=time[i][3];
				time[i][0]=spl[i][0]; time[i][1]=spl[i][1];
				time[i][2]=spl[i][2]; time[i][3]=spl[i][3];
			}
		}

		for(i=0; i<s; i++){
			if(10>time[i][0]) continue;
			if(i==(s-1)){
				spl[count][0]=time[i][2]; spl[count][1]=time[i][3];
				if(time[i][2]==17) nap[count][0]=0;
				else nap[count][0]=(18-time[i][2]);
				if(time[i][3]==0) nap[count][1]=0;
				else nap[count][1]=(60-time[i][3]); 
				count++;
				continue;
			}
			if(time[i][2]==time[i+1][0] && time[i][3]==time[i+1][1]) continue;
			else{
				spl[count][0]=time[i][2]; spl[count][1]=time[i][3];
				nap[count][0]=time[i][2]-time[i+1][0];
				nap[count][1]=abs(time[i][3]-time[i+1][1]);
				count++;
			}
		}

		for(i=0; i<count; i++){
			if(nap[i][0] > nap[i+1][0]) sum=i;
		}


		if(nap[sum][0]==0){
			printf("Day #%d : the longest nap starts at %d:%02d and will last for %02d minutes.\n",day++,spl[sum][0],spl[sum][1],nap[sum][1]);
		} 
		else {
			printf("Day #%d : the longest nap starts at %d:%02d and will last for %d hours and %d minutes.\n", day++, spl[sum][0], spl[sum][1], nap[sum][0], nap[sum][1]);
		}
		
		test--;
	}
}
