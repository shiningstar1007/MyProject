# Debug-Mode : 메모리의 의미

“초기화 하지 않은 변수는 쓰레기 값으로 채워진다” 보통 C/C++을 처음 배울 때 듣던 이야기 중 하나입니다.  
하지만 디버그 빌드에 한하여 초기화 하지 않은 변수에 채워진 쓰레기 값이 어떤 의미 있는 값들이 있는데  
그 값들은 아래와 같습니다.  
  
|<center>Memory</center>|<center>설명</center>|
|:--------:|:--------:|
|*<center>0xCDCDCDCD</center>*| 할당된 Heap 메모리에 채워지는 값(초기화하지 않을 경우)|
|*<center>0xFDFDFDFD</center>*| 할당된 Heap의 경계(전,후)에 채워지는 값|
|*<center>0xDDDDDDDD</center>*| Free된 Heap 메모리에 채워지는 값|
|*<center>0xFEFEFEEE</center>*| Free된 Heap 메모리에 채워지는 값|
|*<center>0xCCCCCCCC</center>*| 스택에 할당된 메모리에 채워지는 값(초기화하지 않을 경우) <br> 해당 값은 어셈블리 _asm int 3(break)와 동일하여 이 영역을 접근하면 break point에 적중된다.|
  
예를 들어 메모리를 엑세스 할 때 에러가 발생할 때, 해당 메모리가  
- 0xCDCDCDCD로 채워져 있다면, 해당 메모리를 초기화 하지 않고 사용했을 가능성을 용의 선상에 올려 놓을수 있을 테고,  
- 0xFEEEFEEE로 채워져 있다면 이미 해제된 메모리를 참조하고 있음을,  
- 할당한 메모리의 전/후 경계 부분이 0xFDFDFDFD값이 아니라면 메모리의 범위를 넘어선 write동작을 했음을 유추 할 수 있을 듯 합니다.  

출처: https://thepassion.tistory.com/9 [좋은향기's 프로그램 블로그]