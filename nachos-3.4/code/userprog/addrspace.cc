// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"
#ifdef HOST_SPARC
#include <strings.h>
#endif

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void 
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical 
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------
/* 
AddrSpace::AddrSpace(OpenFile *executable)
{
    NoffHeader noffH;
    unsigned int i, size;

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && 
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
    	SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

// how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size 
			+ UserStackSize;	// we need to increase the size
						// to leave room for the stack
    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;

    ASSERT(numPages <= NumPhysPages);		// check we're not trying
						// to run anything too big --
						// at least until we have
						// virtual memory

    DEBUG('a', "Initializing address space, num pages %d, size %d\n", 
					numPages, size);
// first, set up the translation 
    pageTable = new TranslationEntry[numPages];
    for (i = 0; i < numPages; i++) {
    pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
	pageTable[i].physicalPage = machine->memoryMap->Find();
    DEBUG('a', "allocate Page %d\n", pageTable[i].physicalPage);
	pageTable[i].valid = TRUE;
	pageTable[i].use = FALSE;
	pageTable[i].dirty = FALSE;
	pageTable[i].readOnly = FALSE;  // if the code segment was entirely on 
					// a separate page, we could set its 
					// pages to be read-only
    
    //pageTable[i].phyaicalPage=machine->allocPhyPage();
    }
// zero out the entire address space, to zero the unitialized data segment 
// and the stack segment
    bzero(machine->mainMemory, size);
// then, copy in the code and data segments into memory
    if (noffH.code.size > 0) {
        DEBUG('a', "Initializing code segment, at 0x%x, size %d\n", 
			noffH.code.virtualAddr, noffH.code.size);
        executable->ReadAt(&(machine->mainMemory[noffH.code.virtualAddr]),
			noffH.code.size, noffH.code.inFileAddr);
    }
    if (noffH.initData.size > 0) {
        DEBUG('a', "Initializing data segment, at 0x%x, size %d\n", 
			noffH.initData.virtualAddr, noffH.initData.size);
        executable->ReadAt(&(machine->mainMemory[noffH.initData.virtualAddr]),
			noffH.initData.size, noffH.initData.inFileAddr);
    }
}
*/

AddrSpace::AddrSpace(OpenFile *executable)
{
    NoffHeader noffH;
    unsigned int i, size;
    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && 
        (WordToHost(noffH.noffMagic) == NOFFMAGIC))
        SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size + UserStackSize;   
    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;
		//上面的内容与原来的一样
  //用fileSystem创建VirtualMemory文件，运行nachos之后，会在在usrprog目录下面生成该文件
    bool success_create_vm = fileSystem->Create("VirtualMemory", size);
    ASSERT(numPages <= NumPhysPages);       // check we're not trying
    pageTable = new TranslationEntry[numPages];
    for (i = 0; i < numPages; i++) {
    pageTable[i].virtualPage = i;  
    pageTable[i].physicalPage = 0;//因为我们没有将用户程序内容装载进内存，所以physicalPage的值可以都设置为0
    pageTable[i].valid = FALSE;//表示没有从磁盘装载任何页面进内存
    pageTable[i].use = FALSE;
    pageTable[i].dirty = FALSE;
    pageTable[i].readOnly = FALSE;  
    }
		//初始化整个物理内存
    //bzero(machine->mainMemory, MemorySize);
  
    OpenFile *vm = fileSystem->Open("VirtualMemory");

    char *virtualMemory_temp;
    virtualMemory_temp = new char[size];//该数组主要是用于将用户程序的内容写入磁盘的中间过渡
    for (i = 0; i < size; i++)
        virtualMemory_temp[i] = 0;
    if (noffH.code.size > 0) {
        DEBUG('a', "\tCopying code segment, at 0x%x, size %d\n",
              noffH.code.virtualAddr, noffH.code.size);
        executable->ReadAt(&(virtualMemory_temp[noffH.code.virtualAddr]),
                           noffH.code.size, noffH.code.inFileAddr);
        vm->WriteAt(&(virtualMemory_temp[noffH.code.virtualAddr]),
                    noffH.code.size, noffH.code.virtualAddr*PageSize);
    }
    if (noffH.initData.size > 0) {
        DEBUG('a', "\tCopying data segment, at 0x%x, size %d\n",
              noffH.initData.virtualAddr, noffH.initData.size);
        executable->ReadAt(&(virtualMemory_temp[noffH.initData.virtualAddr]),
                           noffH.initData.size, noffH.initData.inFileAddr);
        vm->WriteAt(&(virtualMemory_temp[noffH.initData.virtualAddr]),
                    noffH.initData.size, noffH.initData.virtualAddr*PageSize);
    }

    delete vm; 
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
    //将当前线程占用的空间释放掉
    for (int i = 0; i < numPages; i++) {
        int n=pageTable[i].physicalPage;
        DEBUG('a', "free Page %d\n",n);
        machine->memoryMap->Clear(n);
    }
/* 
                    for(int i=0;i<machine->pageTableSize;i++){
                    int n=machine->pageTable[i].physicalPage;
                    if(machine->memoryMap->Test(n)){
                        DEBUG('a', "free Page %d\n",n);
                        machine->memoryMap->Clear(n);
                    }                        
                }
                */
    delete pageTable;
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);	

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------


//lab2
void AddrSpace::SaveState() 
{
    for(int i=0;i<TLBSize;i++)
        machine->tlb[i].valid=FALSE;
}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}


void AddrSpace::Print()
{
    int i=0;
    printf("=====Memory=====\n");
    printf("VPN\tPPN\tvalid\trdonly\tuse\tdirty\n");
    for(i=0;i<numPages;i++){
        int valid=pageTable[i].valid?1:0;
        int readOnly=pageTable[i].readOnly?1:0;
        int use=pageTable[i].use?1:0;
        int dirty=pageTable[i].dirty?1:0;

        printf("%d\t%d\t%d\t%d\t%d\t%d\n",
            pageTable[i].virtualPage,
            pageTable[i].physicalPage,
            valid,readOnly,use,dirty);
            
            
            
    }
}
/*int virtualPage;  	// The page number in virtual memory.
    int physicalPage;  	// The page number in real memory (relative to the
			//  start of "mainMemory"
    bool valid;         // If this bit is set, the translation is ignored.
			// (In other words, the entry hasn't been initialized.)
    bool readOnly;	// If this bit is set, the user program is not allowed
			// to modify the contents of the page.
    bool use;           // This bit is set by the hardware every time the
			// page is referenced or modified.
    bool dirty;    */