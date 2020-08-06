# Context Switching(문맥전환) 

여러개의 프로세스들이 CPU를 점유하는 시간을 나눠 사용함으로써 동시에 실행되는 것처럼 보여지는데   
여기서 실행되는 프로세스에 대한 정보를 Context라고 하며, Context는 프로세스의 PCB(Process Control Block)에 표현되며,    
CPU레지스터의 값, 프로세스의 상태, 메모리 관리 정보등을 포함합니다.   

CPU 내에 존재하는 레지스터들은 현재 실행중인 프로세스에 관한 데이터들로 채워져 있으며, 인터럽트(Interrupt) 요청에 의해   
다음 프로세스를 실행하기 위해서는 CPU가 현재 프로세스의 상태를 저장하고,    
새로운 프로세스의 저장된 상태를 복구하고 변경하는 과정이 이루워지는데 이것을 문맥전환(Context Switching) 이라고 합니다.   

여기서 인터럽트(Interrupt)는 OS가 CPU를 현재 작업에서 빼내어 커널 루틴을 실행할 수 있게 해줍니다.   
여기서 문맥전환(Context Switching)이 이루워 질 때 현재 프로세스의 Context를 PCB(Process Control Block)에 저장하고   
이 후 스케줄 된 프로세스의 저장된 Context를 복구하게 됩니다.   
여기서 문맥전환(Context Switching)이 이루워지는 동안 CPU는 아무것도 하지 못함으로   
문맥전환(Context Switching) 시간은 순수한 오버헤드가 발생하게 됩니다.   
문맥전환(Context Switching)의 속도는 메모리의 속도, 반드시 저장되어야 하는 레지스터의 수, 특수 명령어의 존재에 좌우되므로   
그것을 저장하기 위해 얼만큼의 작업이 필요할지는 OS의 메모리 관리 기법에 의해 결정 됩니다.   

추가적으로 Thread도 동일하게 문맥전환(Context Switching)이 이루워지는데 프로세스보다는 가벼워서 훨씬 빠르게 이루워집니다.   
