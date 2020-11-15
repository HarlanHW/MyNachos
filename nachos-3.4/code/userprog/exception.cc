// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------



void TLBAlgoFIFO(int virtAddr)
{
    int position1 = -1;
    unsigned int vpn;
    vpn = (unsigned) virtAddr / PageSize;
 		//寻找空的TLB数组
    for (int i = 0; i < TLBSize; i++) {
        if (machine->tlb[i].valid == FALSE) {
            position1 = i;
            break;
        }
    }
    // 如果满了，移除首项，然后把每一项往前移动，然后放在最后一项
    if (position1 == -1) {
        position1 = TLBSize - 1;
        for (int i = 0; i < TLBSize - 1; i++) {
            machine->tlb[i] = machine->tlb[i+1];
        }
    }
    //更新TLB
    machine->tlb[position1] = machine->pageTable[vpn];
}



//int position3 = 0;
void TLBAlgoClock(int virtAddr)
{
    //寻找那个use和valid都为0的位置，选取的顺序为(0,0)->(0,1)->(1,0)->(1,1)
    unsigned int vpn;
    vpn = (unsigned) virtAddr / PageSize;
    while (1) {
        machine->position %= TLBSize;
        if (machine->tlb[machine->position].valid == FALSE) {
            break;
        } 
        else{
            if (machine->tlb[machine->position].use) {
               //更新use的值
                machine->tlb[machine->position].use = FALSE;
                machine->position++;
            } 
            else 
                break; 
        }
    }
    //更新TLB
    machine->tlb[machine->position] = machine->pageTable[vpn];
    machine->tlb[machine->position].use = TRUE;
    machine->tlb[machine->position].valid=TRUE;
}
int NaivePageReplacement(int vpn)
{
    int pageFrame = -1;
    for (int temp_vpn = 0; temp_vpn < machine->pageTableSize, temp_vpn != vpn; temp_vpn++) {
        if (machine->pageTable[temp_vpn].valid) {
            if (!machine->pageTable[temp_vpn].dirty) {
                pageFrame = machine->pageTable[temp_vpn].physicalPage;
                break;
            }
        }
    }
    if (pageFrame == -1) { 
        for (int replaced_vpn = 0; replaced_vpn < machine->pageTableSize, replaced_vpn != vpn; replaced_vpn++) {
            if (machine->pageTable[replaced_vpn].valid) {
                machine->pageTable[replaced_vpn].valid = FALSE;
                pageFrame = machine->pageTable[replaced_vpn].physicalPage;
                //将页表写回磁盘
                OpenFile *vm = fileSystem->Open("VirtualMemory");
                vm->WriteAt(&(machine->mainMemory[pageFrame*PageSize]), PageSize, replaced_vpn*PageSize);
                delete vm;
                break;
            }
        }
    }
    return pageFrame;
}


TranslationEntry PageFaultHandler(int vpn)
{

    int pageFrame = machine->memoryMap->Find(); 
    if (pageFrame == -1) {
        pageFrame = NaivePageReplacement(vpn);
    }
    machine->pageTable[vpn].physicalPage = pageFrame;
    OpenFile *vm = fileSystem->Open("VirtualMemory"); 
    vm->ReadAt(&(machine->mainMemory[pageFrame*PageSize]), PageSize, vpn*PageSize);
    delete vm; 
    machine->pageTable[vpn].valid = TRUE;
    machine->pageTable[vpn].use = FALSE;
    machine->pageTable[vpn].dirty = FALSE;
    machine->pageTable[vpn].readOnly = FALSE;
    printf("%s %d\n",currentThread->getName(),currentThread->getThreadId());
    currentThread->space->Print(); //打印地址空间信息
    machine->memoryMap->Print();
}


void TLBMissHandler(int virtAddr)//处理页表失效
{
    unsigned int vpn;
    vpn = (unsigned) virtAddr / PageSize;

    TranslationEntry page = machine->pageTable[vpn];
    if (!page.valid) { 
        DEBUG('m',"\t=> Page miss\n");
        page = PageFaultHandler(vpn);
    }
    TLBAlgoClock(virtAddr);//处理快表失效
}





void ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    if (which == SyscallException) {
        if(type == SC_Halt){
            DEBUG('a', "Shutdown, initiated by user program.\n");
   	        interrupt->Halt();
        }
        else if(type==SC_Exit){
            if(currentThread->space!=NULL){
                DEBUG('a', "user program exit\n");
                delete currentThread->space;//这里会执行space的析构函数，释放位示图                
                currentThread->space = NULL;
                currentThread->Finish();
                //int nextPC=machine->ReadRegister(NextPCReg);
                //machine->WriteRegister(PCReg,nextPC);
                //interrupt->Halt();
            }         
        }	
    }

    else if(which == PageFaultException){
      //发生缺页则让TLBMissCount++
        //TLBMissCount++;
    	if (machine->tlb == NULL) { // linear page table page fault
            ASSERT(FALSE);
        } 
        else { 
            DEBUG('m',"=====>TLB miss,TranslateCount=%d,MissCount=%d\n",
            machine->TranslateCount,machine->TLBMissCount);
            int BadVAddr=machine->ReadRegister(BadVAddrReg);
            TLBMissHandler(BadVAddr);//TLB MISS测试
            //TLBAlgoFIFO(BadVAddr);//FIFO算法测试
            //TLBAlgoClock(BadVAddr);//CLOCK时钟算法测试
        }
        return;
    }
    else {
	printf("Unexpected user mode exception %d %d\n", which, type);
	ASSERT(FALSE);
    }
}



