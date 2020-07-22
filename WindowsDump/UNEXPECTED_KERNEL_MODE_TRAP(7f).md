# UNEXPECTED_KERNEL_MODE_TRAP (7f)

오랜만에 덤프 분석을 하게 되었는데 Kernel Stack Overflow 관련해서 좋은 예가 있어서

정리를 해보려고 합니다.

https://docs.microsoft.com/en-us/windows-hardware/drivers/debugger/bug-check-0x7f--unexpected-kernel-mode-trap

BUGCHECK_STR: 0x7f_8 이 나오면 스택 오버플로우 일 수 있다고 MSDN에 검색해보면 아주 친절하게

나와 있더군요. 그리고 한가지 더는 하드웨어 문제라고도 하니 둘 중 하나라고 생각을 하고 접근을 할 필요가

있을거 같습니다. 그러면 여기서 에러코드는 0x7f 이고 첫번째 인자가 0000000000000008 인걸로 봐서는

스택 오버 플로우를 먼저 의심하고 접근을 해볼 수 있을 거 같습니다.

그러면 덤프 분석을 시작해 보겠습니다.

 

__UNEXPECTED_KERNEL_MODE_TRAP (7f) <- BSOD가 발생한 코드__  
This means a trap occurred in kernel mode, and it's a trap of a kind  
that the kernel isn't allowed to have/catch (bound trap) or that  
is always instant death (double fault). The first number in the  
bugcheck params is the number of the trap (8 = double fault, etc)  
Consult an Intel x86 family manual to learn more about what these  
traps are. Here is a portion of those codes:  
If kv shows a taskGate  
use .tss on the part before the colon, then kv.  
Else if kv shows a trapframe  
use .trap on that value  
Else  
.trap on the appropriate frame will show where the trap was taken  
(on x86, this will be the ebp that goes with the procedure KiTrap)  
Endif  
kb will then show the corrected stack.  
Arguments:  
__Arg1: 0000000000000008, EXCEPTION_DOUBLE_FAULT__  
Arg2: 0000000080050033  
Arg3: 00000000000406f8  
Arg4: fffff88001482416  

Debugging Details:
BUGCHECK_STR: 0x7f_8

DEFAULT_BUCKET_ID: WIN7_DRIVER_FAULT

PROCESS_NAME: System

CURRENT_IRQL: 2

ANALYSIS_VERSION: 6.3.9600.17237 (debuggers(dbg).140716-0327) amd64fre

LAST_CONTROL_TRANSFER: from fffff80001cf2f69 to fffff80001ce4ba0

STACK_TEXT:  
fffff88002174768 fffff80001cf2f69 : 000000000000007f 0000000000000008 0000000080050033 00000000000406f8 : nt!KeBugCheckEx  
fffff88002174770 fffff80001cefa4b : 0000000000000000 0000000000000000 0000000000000000 0000000000000000 : nt!KiBugCheckDispatch+0x69  
fffff880021748b0 fffff88001482416 : 0000000000000000 0000000000000000 0000000000000000 0000000000000000 : nt!KiDoubleFaultAbort+0x28b  
fffff8800b9c0f10 fffff88001486c7a : fffffab0f6061008 fffffab0227bea10 0000000000000000 0000000000000000 : ql2300+0x69416 <- BSOD 문제가 발생한 위치  
fffff8800b9c1050 fffff8800136c113 : fffffab021f17600 fffffab0227bea10 0000000000000000 0000000000000000 : ql2300+0x6dc7a  
fffff8800b9c10d0 fffff80001c0d5f3 : 0000000000000000 0000000000000000 0000000000000000 fffff88005af60b8 : storport!RaidpAdapterContinueScatterGather+0xc3  
fffff8800b9c1140 fffff8800136be7a : fffff88005af6010 fffffab02493b010 000000000000003c 0000000000000000 : hal!HalBuildScatterGatherList+0x203  
fffff8800b9c11b0 fffff8800136baf7 : 0000000000000001 fffffab02147fb10 0000000000000001 fffffab02493b088 : storport!RaUnitStartIo+0x28a  
fffff8800b9c1230 fffff8800136b71d : 000000002493b001 fffff8800138a110 0000000300040000 0000000000000000 : storport!RaUnitScsiIrp+0x397  
fffff8800b9c12f0 fffff88000c5f273 : 0000000000000000 0000000000000001 fffffab0227bea68 fffffab0258baf08 : storport!RaDriverScsiIrp+0x5d  
fffff8800b9c1330 fffff88000c5d374 : fffffab021f8f680 fffffab02493b010 0000000000000000 0000000000000000 : rosempio!DsmWriteEvent+0x3e3f3  
fffff8800b9c13f0 fffff88000c39fc4 : fffffab021f8f680 fffffab02493b010 fffffab02493b010 fffffab020906010 : rosempio!DsmWriteEvent+0x3c4f4  
fffff8800b9c1670 fffff88000c39649 : fffffab021f8f680 fffffab02493b010 fffffab0258baf08 0000000000000000 : rosempio!DsmWriteEvent+0x19144  
fffff8800b9c16d0 fffff88000c39ea8 : fffffab021f8f680 fffffab02493b010 0000000000000000 fffffab0227bea68 : rosempio!DsmWriteEvent+0x187c9  
fffff8800b9c1790 fffff88000c3272e : fffffab021f8f680 fffffab02493b010 0000000100000000 0000000000000000 : rosempio!DsmWriteEvent+0x19028  
fffff8800b9c17f0 fffff88000c525db : fffffab021f8f680 fffffab02493b010 fffff8800b9c1888 0000000000000001 : rosempio!DsmWriteEvent+0x118ae  
fffff8800b9c1830 fffff88000c42f28 : fffffab02283b3d0 fffffab02493b010 fffffab02493b010 0000000000000000 : rosempio!DsmWriteEvent+0x3175b  
fffff8800b9c18c0 fffff88000c42df5 : fffffab02283b3d0 fffffab02493b010 fffff88000000000 0000000000000000 : rosempio!DsmWriteEvent+0x220a8  
fffff8800b9c1920 fffff88000c43059 : fffffab02283b3d0 fffffab02493b010 fffff88005af6390 0000000000000000 : rosempio!DsmWriteEvent+0x21f75  
fffff8800b9c1980 fffff88000c41e13 : fffffab02283b3d0 fffffab02493b010 fffffab0f6061008 fffff88001486c7a : rosempio!DsmWriteEvent+0x221d9  
fffff8800b9c19d0 fffff88000c417c3 : fffffab02283b3d0 fffffab02493b010 0000000000000001 fffffab0227be8f0 : rosempio!DsmWriteEvent+0x20f93  
fffff8800b9c1a20 fffff88000c3274b : fffffab02283b3d0 fffffab02493b010 0000000000002000 fffffab11c6ae010 : rosempio!DsmWriteEvent+0x20943  
fffff8800b9c1a80 fffff88001801445 : fffffab02283b3d0 fffffab02493b010 fffffab0227be8f0 0000004530035000 : rosempio!DsmWriteEvent+0x118cb  
fffff8800b9c1ac0 fffff88001813f06 : fffffab0f9504010 fffffab02283b3d0 fffffab000080000 fffffab000000001 : CLASSPNP!ServiceTransferRequest+0x355  
fffff8800b9c1b60 fffff88001805bbc : fffffab0f950db50 fffff8800216b101 fffffab0f950dc10 0000000000000000 : CLASSPNP!ClasspServiceIdleRequest+0x26  
fffff8800b9c1b90 fffff80001c8f801 : 0000000000000000 0000000000000000 0000000000000000 0000000000002000 : CLASSPNP! ?? ::FNODOBFM::string'+0x4bb3  
fffff8800b9c1c10 fffff88000c52671 : fffffab02200aa10 fffffab000000100 0000000000000000 0000000000000000 : nt!IopfCompleteRequest+0x341  
fffff8800b9c1d00 fffff88000c42f28 : fffffab02283b3d0 fffffab020906010 fffffab020906010 0000000000000000 : rosempio!DsmWriteEvent+0x317f1   
fffff8800b9c1d90 fffff88000c42df5 : fffffab02283b3d0 fffffab020906010 fffff88000000000 0000000000000000 : rosempio!DsmWriteEvent+0x220a8  
fffff8800b9c1df0 fffff88000c43059 : fffffab02283b3d0 fffffab020906010 fffff88003d2e390 0000000000000000 : rosempio!DsmWriteEvent+0x21f75  
fffff8800b9c1e50 fffff88000c41e13 : fffffab02283b3d0 fffffab020906010 fffffab0f5aa8008 fffff88001486c7a : rosempio!DsmWriteEvent+0x221d9  
fffff8800b9c1ea0 fffff88000c417c3 : fffffab02283b3d0 fffffab020906010 0000000000000001 fffffab0249685f0 : rosempio!DsmWriteEvent+0x20f93  
fffff8800b9c1ef0 fffff88000c3274b : fffffab02283b3d0 fffffab020906010 0000000000002000 fffffab11c695ab0 : rosempio!DsmWriteEvent+0x20943  
fffff8800b9c1f50 fffff88001801445 : fffffab02283b3d0 fffffab020906010 fffffab0249685f0 000000452f09f000 : rosempio!DsmWriteEvent+0x118cb  
fffff8800b9c1f90 fffff88001813f06 : fffffab0f9504010 fffffab02283b3d0 fffffab000080000 fffffab000000001 : CLASSPNP!ServiceTransferRequest+0x355  
fffff8800b9c2030 fffff88001805bbc : fffffab0f950db50 fffff8800216b101 fffffab0f950dc10 0000000000000000 : CLASSPNP!ClasspServiceIdleRequest+0x26  
fffff8800b9c2060 fffff80001c8f801 : 0000000000000000 0000000000000000 0000000000000000 0000000000002000 : CLASSPNP! ?? ::FNODOBFM::string'+0x4bb3  
fffff8800b9c20e0 fffff88000c52671 : fffffab02200aa10 fffffab000000100 0000000000000000 0000000000000000 : nt!IopfCompleteRequest+0x341 
fffff8800b9c21d0 fffff88000c42f28 : fffffab02283b3d0 fffffab02498d990 fffffab02498d990 0000000000000000 : rosempio!DsmWriteEvent+0x317f1  
fffff8800b9c2260 fffff88000c42df5 : fffffab02283b3d0 fffffab02498d990 fffff88000000000 0000000000000000 : rosempio!DsmWriteEvent+0x220a8  
fffff8800b9c22c0 fffff88000c43059 : fffffab02283b3d0 fffffab02498d990 fffff88005af6390 0000000000000000 : rosempio!DsmWriteEvent+0x21f75  
fffff8800b9c2320 fffff88000c41e13 : fffffab02283b3d0 fffffab02498d990 fffffab0f6061008 fffff88001486c7a : rosempio!DsmWriteEvent+0x221d9  
fffff8800b9c2370 fffff88000c417c3 : fffffab02283b3d0 fffffab02498d990 0000000000000001 fffffab02498de60 : rosempio!DsmWriteEvent+0x20f93  
fffff8800b9c23c0 fffff88000c3274b : fffffab02283b3d0 fffffab02498d990 0000000000002000 fffffab11c693010 : rosempio!DsmWriteEvent+0x20943  
fffff8800b9c2420 fffff88001801445 : fffffab02283b3d0 fffffab02498d990 fffffab02498de60 000000452ec8b000 : rosempio!DsmWriteEvent+0x118cb  
fffff8800b9c2460 fffff88001813f06 : fffffab0f9504010 fffffab02283b3d0 fffffab000080000 fffffab000000001 : CLASSPNP!ServiceTransferRequest+0x355  
fffff8800b9c2500 fffff88001805bbc : fffffab0f950db50 fffff8800216b101 fffffab0f950dc10 0000000000000000 : CLASSPNP!ClasspServiceIdleRequest+0x26  
fffff8800b9c2530 fffff80001c8f801 : 0000000000000000 0000000000000000 0000000000000000 0000000000002000 : CLASSPNP! ?? ::FNODOBFM::string'+0x4bb3  
fffff8800b9c25b0 fffff88000c52671 : fffffab02200aa10 fffffab000000100 0000000000000000 0000000000000000 : nt!IopfCompleteRequest+0x341  
fffff8800b9c26a0 fffff88000c42f28 : fffffab02283b3d0 fffffab02266a010 fffffab02266a010 0000000000000000 : rosempio!DsmWriteEvent+0x317f1   
fffff8800b9c2730 fffff88000c42df5 : fffffab02283b3d0 fffffab02266a010 fffff88000000000 0000000000000000 : rosempio!DsmWriteEvent+0x220a8  
fffff8800b9c2790 fffff88000c43059 : fffffab02283b3d0 fffffab02266a010 fffff88003d2e390 0000000000000000 : rosempio!DsmWriteEvent+0x21f75  
fffff8800b9c27f0 fffff88000c41e13 : fffffab02283b3d0 fffffab02266a010 fffffab0f5aa8008 fffff88001486c7a : rosempio!DsmWriteEvent+0x221d9  
fffff8800b9c2840 fffff88000c417c3 : fffffab02283b3d0 fffffab02266a010 0000000000000001 fffffab0226a5010 : rosempio!DsmWriteEvent+0x20f93  
fffff8800b9c2890 fffff88000c3274b : fffffab02283b3d0 fffffab02266a010 0000000000002000 fffffab11c69bab0 : rosempio!DsmWriteEvent+0x20943  
fffff8800b9c28f0 fffff88001801445 : fffffab02283b3d0 fffffab02266a010 fffffab0226a5010 000000452ed89000 : rosempio!DsmWriteEvent+0x118cb  
fffff8800b9c2930 fffff88001813f06 : fffffab0f9504010 fffffab02283b3d0 fffffab000080000 fffffab000000001 : CLASSPNP!ServiceTransferRequest+0x355  
fffff8800b9c29d0 fffff88001805bbc : fffffab0f950db50 fffff8800216b101 fffffab0f950dc10 0000000000000000 : CLASSPNP!ClasspServiceIdleRequest+0x26  
fffff8800b9c2a00 fffff80001c8f801 : 0000000000000000 0000000000000000 0000000000000000 0000000000002000 : CLASSPNP! ?? ::FNODOBFM::string'+0x4bb3  
fffff8800b9c2a80 fffff88000c52671 : fffffab02200aa10 fffffab000000100 0000000000000000 0000000000000000 : nt!IopfCompleteRequest+0x341  
fffff8800b9c2b70 fffff88000c42f28 : fffffab02283b3d0 fffffab022579350 fffffab022579350 0000000000000000 : rosempio!DsmWriteEvent+0x317f1  
fffff8800b9c2c00 fffff88000c42df5 : fffffab02283b3d0 fffffab022579350 fffff88000000000 0000000000000000 : rosempio!DsmWriteEvent+0x220a8  
fffff8800b9c2c60 fffff88000c43059 : fffffab02283b3d0 fffffab022579350 fffff88005af6390 0000000000000000 : rosempio!DsmWriteEvent+0x21f75  
fffff8800b9c2cc0 fffff88000c41e13 : fffffab02283b3d0 fffffab022579350 fffffab0f6061008 fffff88001486c7a : rosempio!DsmWriteEvent+0x221d9  
fffff8800b9c2d10 fffff88000c417c3 : fffffab02283b3d0 fffffab022579350 0000000000000001 fffffab020a032d0 : rosempio!DsmWriteEvent+0x20f93  
fffff8800b9c2d60 fffff88000c3274b : fffffab02283b3d0 fffffab022579350 0000000000002000 fffffab11c6a7ab0 : rosempio!DsmWriteEvent+0x20943  
fffff8800b9c2dc0 fffff88001801445 : fffffab02283b3d0 fffffab022579350 fffffab020a032d0 000000452ee04000 : rosempio!DsmWriteEvent+0x118cb  
fffff8800b9c2e00 fffff88001813f06 : fffffab0f9504010 fffffab02283b3d0 fffffab000080000 fffffab000000001 : CLASSPNP!ServiceTransferRequest+0x355  
fffff8800b9c2ea0 fffff88001805bbc : fffffab0f950db50 fffff8800216b101 fffffab0f950dc10 0000000000000000 : CLASSPNP!ClasspServiceIdleRequest+0x26  
fffff8800b9c2ed0 fffff80001c8f801 : 0000000000000000 0000000000000000 0000000000000000 0000000000002000 : CLASSPNP! ?? ::FNODOBFM::string'+0x4bb3  
fffff8800b9c2f50 fffff88000c52671 : fffffab02200aa10 fffffab000000100 0000000000000000 0000000000000000 : nt!IopfCompleteRequest+0x341  
fffff8800b9c3040 fffff88000c42f28 : fffffab02283b3d0 fffffab0226c5820 fffffab0226c5820 0000000000000000 : rosempio!DsmWriteEvent+0x317f1   
fffff8800b9c30d0 fffff88000c42df5 : fffffab02283b3d0 fffffab0226c5820 fffff88000000000 0000000000000000 : rosempio!DsmWriteEvent+0x220a8  
fffff8800b9c3130 fffff88000c43059 : fffffab02283b3d0 fffffab0226c5820 fffff88003d2e390 0000000000000000 : rosempio!DsmWriteEvent+0x21f75  
fffff8800b9c3190 fffff88000c41e13 : fffffab02283b3d0 fffffab0226c5820 fffffab0f5aa8008 fffff880`01486c7a : rosempio!DsmWriteEvent+0x221d9  

STACK_COMMAND: kb

FOLLOWUP_IP:
ql2300+69416
fffff880`01482416 899424c0000000 mov dword ptr [rsp+0C0h],edx

SYMBOL_STACK_INDEX: 3

SYMBOL_NAME: ql2300+69416 <- BSOD 문제가 발생한 위치

FOLLOWUP_NAME: MachineOwner

MODULE_NAME: ql2300

IMAGE_NAME: ql2300.sys <- BSOD를 발생시킨 드라이버

DEBUG_FLR_IMAGE_TIMESTAMP: 5a1dd291

FAILURE_BUCKET_ID: X64_0x7f_8_ql2300+69416

BUCKET_ID: X64_0x7f_8_ql2300+69416

ANALYSIS_SOURCE: KM

FAILURE_ID_HASH_STRING: km:x64_0x7f_8_ql2300+69416

FAILURE_ID_HASH: {0f9ebaaf-2e04-8097-fea0-1da0b10cd026}

 

__!analyze -v 명령을 사용하면 이렇게 친절하게 분석을 해주네요.  
일단 이거만 봐서는 잘 모르겠고 현재 스레드의 상태를 확인해 보도록 하겠습니다.__
 
kd> !thread  
THREAD fffffab0f950db50 Cid 0004.144c Teb: 0000000000000000 Win32Thread: 0000000000000000 RUNNING on processor 4  
Not impersonating  
DeviceMap fffff8a000008710  
Owning Process fffffab0205a5b00 Image: System  
Attached Process N/A Image: N/A  
Wait Start TickCount 5466514 Ticks: 0  
Context Switch Count 83790 IdealProcessor: 4  
UserTime 00:00:00.000  
KernelTime 00:00:00.000  
Win32 Start Address rosempio!DsmWriteEvent (0xfffff88000c41c50)  
Stack Init fffff8800b9c6c70 Current fffff8800b9c1a50  
Base fffff8800b9c7000 Limit fffff8800b9c1000 Call 0  
Priority 8 BasePriority 8 UnusualBoost 0 ForegroundBoost 0 IoPriority 2 PagePriority 5  
Child-SP RetAddr : Args to Child : Call Site  
fffff88002174768 fffff80001cf2f69 : 000000000000007f 0000000000000008 0000000080050033 00000000000406f8 : nt!KeBugCheckEx  
fffff88002174770 fffff80001cefa4b : 0000000000000000 0000000000000000 0000000000000000 0000000000000000 : nt!KiBugCheckDispatch+0x69  
fffff880021748b0 fffff88001482416 : 0000000000000000 0000000000000000 0000000000000000 0000000000000000 : nt!KiDoubleFaultAbort+0x28b (TrapFrame @ fffff880021748b0)  
fffff8800b9c0f10 fffff88001486c7a : fffffab0f6061008 fffffab0227bea10 0000000000000000 0000000000000000 : ql2300+0x69416  
fffff8800b9c1050 fffff8800136c113 : fffffab021f17600 fffffab0227bea10 0000000000000000 0000000000000000 : ql2300+0x6dc7a  
fffff8800b9c10d0 fffff80001c0d5f3 : 0000000000000000 0000000000000000 0000000000000000 fffff88005af60b8 : storport!RaidpAdapterContinueScatterGather+0xc3  
fffff8800b9c1140 fffff8800136be7a : fffff88005af6010 fffffab02493b010 000000000000003c 0000000000000000 : hal!HalBuildScatterGatherList+0x203  
fffff8800b9c11b0 fffff8800136baf7 : 0000000000000001 fffffab02147fb10 0000000000000001 fffffab02493b088 : storport!RaUnitStartIo+0x28a  
fffff8800b9c1230 fffff8800136b71d : 000000002493b001 fffff8800138a110 0000000300040000 0000000000000000 : storport!RaUnitScsiIrp+0x397  
fffff8800b9c12f0 fffff88000c5f273 : 0000000000000000 0000000000000001 fffffab0227bea68 fffffab0258baf08 : storport!RaDriverScsiIrp+0x5d  
fffff8800b9c1330 fffff88000c5d374 : fffffab021f8f680 fffffab02493b010 0000000000000000 0000000000000000 : rosempio!DsmWriteEvent+0x3e3f3  
fffff8800b9c13f0 fffff88000c39fc4 : fffffab021f8f680 fffffab02493b010 fffffab02493b010 fffffab020906010 : rosempio!DsmWriteEvent+0x3c4f4  
fffff8800b9c1670 fffff88000c39649 : fffffab021f8f680 fffffab02493b010 fffffab0258baf08 0000000000000000 : rosempio!DsmWriteEvent+0x19144  
fffff8800b9c16d0 fffff88000c39ea8 : fffffab021f8f680 fffffab02493b010 0000000000000000 fffffab0227bea68 : rosempio!DsmWriteEvent+0x187c9  
fffff8800b9c1790 fffff88000c3272e : fffffab021f8f680 fffffab02493b010 0000000100000000 0000000000000000 : rosempio!DsmWriteEvent+0x19028  
fffff8800b9c17f0 fffff88000c525db : fffffab021f8f680 fffffab02493b010 fffff8800b9c1888 0000000000000001 : rosempio!DsmWriteEvent+0x118ae  
fffff8800b9c1830 fffff88000c42f28 : fffffab02283b3d0 fffffab02493b010 fffffab02493b010 0000000000000000 : rosempio!DsmWriteEvent+0x3175b  
fffff8800b9c18c0 fffff88000c42df5 : fffffab02283b3d0 fffffab02493b010 fffff88000000000 0000000000000000 : rosempio!DsmWriteEvent+0x220a8  
fffff8800b9c1920 fffff88000c43059 : fffffab02283b3d0 fffffab02493b010 fffff88005af6390 0000000000000000 : rosempio!DsmWriteEvent+0x21f75  
fffff8800b9c1980 fffff88000c41e13 : fffffab02283b3d0 fffffab02493b010 fffffab0f6061008 fffff88001486c7a : rosempio!DsmWriteEvent+0x221d9  
fffff8800b9c19d0 fffff88000c417c3 : fffffab02283b3d0 fffffab02493b010 0000000000000001 fffffab0227be8f0 : rosempio!DsmWriteEvent+0x20f93  
fffff8800b9c1a20 fffff88000c3274b : fffffab02283b3d0 fffffab02493b010 0000000000002000 fffffab11c6ae010 : rosempio!DsmWriteEvent+0x20943  
fffff8800b9c1a80 fffff88001801445 : fffffab02283b3d0 fffffab02493b010 fffffab0227be8f0 0000004530035000 : rosempio!DsmWriteEvent+0x118cb  
fffff8800b9c1ac0 fffff88001813f06 : fffffab0f9504010 fffffab02283b3d0 fffffab000080000 fffffab000000001 : CLASSPNP!ServiceTransferRequest+0x355  
fffff8800b9c1b60 fffff88001805bbc : fffffab0f950db50 fffff8800216b101 fffffab0f950dc10 0000000000000000 : CLASSPNP!ClasspServiceIdleRequest+0x26  
fffff8800b9c1b90 fffff80001c8f801 : 0000000000000000 0000000000000000 0000000000000000 0000000000002000 : CLASSPNP! ?? ::FNODOBFM::string'+0x4bb3  
fffff8800b9c1c10 fffff88000c52671 : fffffab02200aa10 fffffab000000100 0000000000000000 0000000000000000 : nt!IopfCompleteRequest+0x341  
fffff8800b9c1d00 fffff88000c42f28 : fffffab02283b3d0 fffffab020906010 fffffab020906010 0000000000000000 : rosempio!DsmWriteEvent+0x317f1  
fffff8800b9c1d90 fffff88000c42df5 : fffffab02283b3d0 fffffab020906010 fffff88000000000 0000000000000000 : rosempio!DsmWriteEvent+0x220a8  
fffff8800b9c1df0 fffff88000c43059 : fffffab02283b3d0 fffffab020906010 fffff88003d2e390 0000000000000000 : rosempio!DsmWriteEvent+0x21f75  
fffff8800b9c1e50 fffff88000c41e13 : fffffab02283b3d0 fffffab020906010 fffffab0f5aa8008 fffff88001486c7a : rosempio!DsmWriteEvent+0x221d9  
fffff8800b9c1ea0 fffff88000c417c3 : fffffab02283b3d0 fffffab020906010 0000000000000001 fffffab0249685f0 : rosempio!DsmWriteEvent+0x20f93  
fffff8800b9c1ef0 fffff88000c3274b : fffffab02283b3d0 fffffab020906010 0000000000002000 fffffab11c695ab0 : rosempio!DsmWriteEvent+0x20943  
fffff8800b9c1f50 fffff88001801445 : fffffab02283b3d0 fffffab020906010 fffffab0249685f0 000000452f09f000 : rosempio!DsmWriteEvent+0x118cb  
fffff8800b9c1f90 fffff88001813f06 : fffffab0f9504010 fffffab02283b3d0 fffffab000080000 fffffab000000001 : CLASSPNP!ServiceTransferRequest+0x355  
fffff8800b9c2030 fffff88001805bbc : fffffab0f950db50 fffff8800216b101 fffffab0f950dc10 0000000000000000 : CLASSPNP!ClasspServiceIdleRequest+0x26  
fffff8800b9c2060 fffff80001c8f801 : 0000000000000000 0000000000000000 0000000000000000 0000000000002000 : CLASSPNP! ?? ::FNODOBFM::string'+0x4bb3  
fffff8800b9c20e0 fffff88000c52671 : fffffab02200aa10 fffffab000000100 0000000000000000 0000000000000000 : nt!IopfCompleteRequest+0x341  
fffff8800b9c21d0 fffff88000c42f28 : fffffab02283b3d0 fffffab02498d990 fffffab02498d990 0000000000000000 : rosempio!DsmWriteEvent+0x317f1  
fffff8800b9c2260 fffff88000c42df5 : fffffab02283b3d0 fffffab02498d990 fffff88000000000 00000000`00000000 : rosempio!DsmWriteEvent+0x220a8  
 

여기서 가장 중요하게 봐야 하는거는  
__Stack Init fffff8800b9c6c70 Current fffff8800b9c1a50    
Base fffff8800b9c7000 Limit fffff8800b9c1000__    
입니다. 이거는 스택 범위 값을 나타냅니다.  
보통은 스레드가 실행되면 ESP 레지스터 값은 항상 Stack Init (fffff8800b9c6c70)과 Limit (fffff8800b9c1000)  
사이에 있어야 합니다. 일반적으로 ESP 레지스터 값은 Current (fffff8800b9c1a50) 와 상대적으로 유사합니다.  
그리고 Current 값은 Stack Init 값과 Limit 값 사이에 있는걸 볼 수 있습니다.  
그렇다면 현재 스택 정보를 저장한 레지스터 상태를 봐야 합니다. 


kd>.trap fffff880021748b0  
NOTE: The trap frame does not contain all registers. Some register values may be zeroed or incorrect.  
rax=0000000000000002 rbx=0000000000000000 rcx=0000000000000001 rdx=0000000000000001  
rsi=0000000000000000 rdi=0000000000000000 rip=fffff88001482416 rsp=fffff8800b9c0f10 rbp=0000000000000200  
r8=fffffab0f615a084 r9=0000000000000000 r10=fffff88005af60b8 r11=0000000000000000 r12=0000000000000000  
r13=0000000000000000 r14=0000000000000000 r15=0000000000000000  
iopl=0 nv up ei pl nz na pe nc ql2300+0x69416: fffff88001482416 899424c0000000  
mov dword ptr [rsp+0C0h],edx ss:0018:fffff880`0b9c0fd0=???????? 
 

__ESP는 64bit에서 rsp로 통합되어 사용되니 rsp를 확인하면 됩니다.__ 

Stack Init fffff8800b9c6c70 Current fffff8800b9c1a50  
Base fffff8800b9c7000 Limit fffff8800b9c1000  
rsp=fffff8800b9c0f10  

여기서 rsp 값을 확인 하면 (fffff8800b9c0f10) 로 이 값이 범위를 벗어나는 것으로 보여지면서 스택 오버플로우가 의심 되는것으로 보여지네요.  
그렇다면 이제 실제 스택 메모리를 얼마나 사용하는지를 확인해보면 되겠습니다.  
보통 실제로 스택 사용량을 확인하는 방법은 ChildEBP 의 변화량을 보는 방법이 있는데  
그걸 다 어느 세월에 계산하면서 일일히 체크를 할까요...  
그래서 대신 계산을 해주는 kf 명령어를 사용하면 스택 메모리 사용량을 보여줍니다.  
위에서 콜스택이 엄청 길어서 인자를 100으로 주었고 거기서 간추려보겠습니다  
 

kd> kf 100

|  <center>Memory</center> |  <center>Child-SP</center> |  <center>RetAddr</center> |  <center>Call Site</center> |
|:--------:|:--------:|:--------:|:--------:|
|**<center> </center>** | <center>fffff88002174768</center> |*<center>fffff80001cf2f69</center>* |*<center>nt!KeBugCheckEx</center>* |
|**<center>8</center>** | <center>fffff88002174770</center> |*<center>fffff80001cefa4b</center>* |*<center>nt!KiBugCheckDispatch+0x69</center>* |
|**<center>140</center>** | <center>fffff880021748b0</center> |*<center>fffff88001482416</center>* |*<center>nt!KiDoubleFaultAbort+0x28b</center>* |
|**<center>984c660</center>** | <center>fffff8800b9c0f10</center> |*<center>fffff88001486c7a</center>* |*<center>ql2300+0x69416</center>* |
|**<center>140</center>** | <center>fffff8800b9c1050</center> |*<center>fffff8800136c113</center>* |*<center>ql2300+0x6dc7a</center>* |


ql2300+69416 여기서 보니까 스택 메모리를 무려 984c660이나 사용을 하네요...  
보통 커널 스택 메모리는 x86의 경우는 12K 이고 x64의 경우는 16K 입니다.  
(Windows internals 에서 확인)
이로써 ql2300+69416 여기에서 무려 984c660 이나 되는 대량의 스택 메모리를  
사용하여 문제가 발생하였다는것을 확인 할 수 있게 되었습니다.  
