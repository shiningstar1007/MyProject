#include <stdio.h>
#define MAX_DATA_BUF 100

void main(){
	int TestCount;				 //테스트 갯수
	int Date;							 //날짜
	int PoliticalCount;		 //정당수
	int Num[MAX_DATA_BUF]; //휴일일수
	int Holiday = 0;			 //동맹휴업일수
	int i,j,k = 0;
	int a;
	
	scanf("%d", &TestCount);

	for(a = 0; a < TestCount; a++){
		scanf("%d %d",&Date, &PoliticalCount);

		for(i = 0; i < PoliticalCount; i++){
			scanf("%d", &Num[i]);
		}

		for(i = 0; i <= Date; i++){
			for(j = 0; j < PoliticalCount; j++){
				if((6 + k) == i) {
					k += 7; 
					break;
				}

				if((i % 7) == 0) break;

				if((i % Num[j]) == 0){
					Holiday++;
					break;
				}
			}
		}

		printf("%d\n", Holiday);
	}
}