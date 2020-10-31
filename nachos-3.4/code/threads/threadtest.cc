// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "elevatortest.h"

// testnum is set in main.cc
int testnum = 3;

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
	printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
    }
}

void ShowThreadID(int tid)
{
    if(tid>=0)
        printf("create successful,thread id is %d, name is \"%s\"\n",currentThread->getThreadId(),currentThread->getName());
    else
    {
            printf("create failed\n");
    }
    
}

void ShowPriority()
{
    scheduler->Print();
}

void TimeSlice()
{
    for(int i=0;i<20;i++){
        //printf("%d %d %d %d\n",currentThread->getThreadId(),currentThread->getPriority(),stats->totalTicks,currentThread->getCPUtime());
        interrupt->OneTick();
        //printf("\n");
        ShowThreadsStatus();
        //if(currentThread->getCPUtime()%100==0)
        //    currentThread->Yield();
    }
    
}
//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest1()
{
    DEBUG('t', "Entering ThreadTest1");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, (void*)1);
    SimpleThread(0);
}

void ThreadTest2()
{
    DEBUG('t', "Entering ThreadTest2");
    
    for(int i=0;i<5;i++)
    {
        char *name=new char[16];
        sprintf(name,"%s%d\0","thread",i);
        
        Thread *t = new Thread(name);
        //t->Print();
        //printf(&(t->getName()));
        t->Fork(ShowThreadID, (void*)t->getThreadId());
    }
}

void ThreadTest3()
{
    DEBUG('t', "Entering cmd_ts");
    Thread *t = new Thread("ts");
    t->Fork(ShowThreadsStatus, (void*)t->getThreadId());
}

void ThreadTest4()
{
    DEBUG('t', "Entering ThreadTest4");
    for(int i=1;i<5;i++)
    {
        char *name=new char[16];
        sprintf(name,"%s%d\0","thread",i);
        
        //Thread *t = new Thread(name,5-i);
        Thread *t = new Thread(name);
        t->Fork(ShowPriority, (void*)t->getThreadId());
    }
}

void ThreadTest5()
{
    DEBUG('t', "Entering ThreadTest4");
    TimeSlice();
    Thread *t = new Thread("ts");
    t->Fork(ShowThreadsStatus, (void*)t->getThreadId());
    for(int i=1;i<5;i++)
    {
        char *name=new char[16];
        sprintf(name,"%s%d\0","thread",i);
        t = new Thread(name);
        //Thread *t = new Thread(name,5-i);
        //Thread *t = new Thread(name);
        t->Fork(TimeSlice, (void*)0);
    }
}

//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest()
{
    switch (testnum) {
    case 1:
	ThreadTest1();
    case 2:
	ThreadTest2();
    case 3:
	ThreadTest3();
	break;
    case 4:
    ThreadTest4();
    break;
    case 5:
    ThreadTest5();
    break;
    default:
	printf("No test specified.\n");
	break;
    }
}

