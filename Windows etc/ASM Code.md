# ASM Code
## xor eax, eax
- xor(Exclusive OR) - 두 오퍼랜드의 비트들에 대해 논리 XOR를 수행하여 그 결과를 목적지 오퍼랜드에 저장한다.
- XOR 연산은 두 값 중 한 값만 참인 경우 참으로 간주하므로 eax값을 0으로 만드는 코드이다.
- 같은 값을 xor하면 모든 비트가 같으므로 1이 나올수 없다.
- 결국 mov eax, 0 과 같은 결과가 나온다는 얘기인데, 위와 같은 코드를 사용하는 이유는 xor연산이 cost & opcode size가 작기 때문인듯 하다.  
0041CD4E B8 00 00 00 00   mov         eax,0   
0041CD53 33 C0              xor         eax,eax

## mov edi, edi
- 2byte 의 NOP코드와 같다.
- XP-SP2이후부터, 함수 prolog의 첫번째 명령으로 추가되었다.
- 이는 마이크로소프트가 재부팅 없이 패치를 수행(hot-fix)하기 위해 추가한 것으로
- XP-SP2이전에는 prolog의 2byte(why2?)+이어지는 명령의 2byte를 교체해야 했으나, 
- XP-SP2이후에는 위의 두바이트 명령이 추가됨으로써 함수의 prolog가 정확히 5바이트로 변경되었으며, 
이로 인해 인라인 후킹이 더 쉬워졌다.(다른 코드로의 점프에 필요한 바이트수가 5byte)

## jmp WHook!ILT+360(?MyMessageBoxWYGHPAUHWND__PB_W1IZ) (00b3716d)
- ILT( Incremental Link Table ) /INCREMENTAL 옵션을 주어 링크 했을때, 링커는 함수 주소들의 배열(ILT)을 생성한다. 
그리고 함수를 호출하기 위해 해당 슬롯을 통해 함수를 직접 호출한다.
ILT+360은 해당배열의 360번째 슬롯의 주소를 의미한다

## add byte ptr [eax],al
- 이 명령의 기계어 코드를 보면 0000이다. 
이 명령이 반복적으로 계속되는 경우, 유효한 기계어 코드라기 보다는 0으로 채워진 메모리를 디스어셈블러가 이 코드로 번역했다고 보는 게 맞을 거 같다

## pushad
- 모든 레지스터를 저장

## popad
- 모든 레지스터를 복구

출처: https://thepassion.tistory.com/115 [좋은향기's 프로그램 블로그]