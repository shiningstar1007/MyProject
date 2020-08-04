# Lookaside list  
 
드라이버에서 PAGE_SIZE(일반적으로 4K)보다 작은 메모리를 빈번하게 할당하고 해제한다면   
메모리 단편화(fragmentation)가 발생하여 시스템 전체의 성능이 저하됩니다.  
이를 방지하게 위해 드라이버에서 동일한 크기의 구조체를 동적으로 사용하는 경우라면 ExAllocatePoolWithTag보다   
효율적으로 동작하는 Lookaside list를 사용할 수 있습니다.   

Pool과 Lookaside list의 기본적인 차이점은 Pool이 다양한 크기를 할당할 수 있는 반면에 Lookaside list는   
고정된 크기만 할당 할 수 있으며, Lookaside list는 스핀락을 사용하지 않기 때문에 더 빠르다는 장점이 있습니다.  

Lookaside list는 NT 4.0 시절에 사용하던 Zone Buffer의 개념을 보다 발전시킨 것입니다.  
Zone Buffer는 드라이버 초기화(예를 들어, DriverEntry) 루틴에서 일정량의 메모리를 할당하여   
이 영역으로부터 필요한 메모리를 할당 받게끔 합니다.   
최초에 확보한 Zone Buffer의 크기보다 더 많은 메모리가 필요한 경우 Zone Buffer를 확장할 수 있으나   
그 크기를 줄일 수는 없습니다. 따라서 드라이버가 Zone Buffer를 사용하는 피크(peak) 이후에   
메모리 사용이 줄어들 경우 Zone Buffer의 크기도 줄여야만 시스템 전체의 성능을 높일 수 있는데 이것이 불가능합니다.  
그래서 이런 단점을 극복한 것이 바로 Lookaside list 입니다.  

시스템은 Lookaside list에 대하여 시스템이 관리하는 리스트 depth를 가지고,   
이 depth는 리스트에 큐잉될 동일한 크기를 가진 메모리 엔트리의 최대 수를 나타냅니다.  
시스템은 드라이버가 Lookaside list를 사용하는 패턴에 따라 Lookaside list의 depth를 자동적으로 튜닝합니다.   
Lookaside list에 존재하는 큐잉된 메모리 엔트리를 빈번하게 사용하여 메모리 엔트리의 수가 어느 정도 이하로   
내려가게 되면(이것은 시스템 스레드인 밸런스 셋 관리자가 1초에 한번씩 검사함)   
시스템은 메모리 Pool로부터 추가적인 메모리를 할당받아 Lookaside list의 엔트리를 채웁니다.  
또한 드라이버가 Lookaside에서 할당받은 메모리 엔트리를 반환하여 엔트리의 수가 일정 이상 넘어가게 되면   
시스템은 Lookaside list의 엔트리 메모리를 메모리 Pool에 자동으로 반환합니다.  

참고 문헌 : Windows Internals