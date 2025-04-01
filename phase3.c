//Phase3.c code by Alec Schmitt and Luckie Musngi

#include <phase3.h>
#include <usloss.h>
#include <phase2.h>
#include <phase1.h>
#include <stdlib.h>
#include <phase3_usermode.h>
#include <phase3_kernelInterfaces.h>

int printDebug = 0;

//A struct just containing a function's pointer and params
typedef struct fakePCB {
    int (*func)(void *);
    void *param;
} process;

p//rocess shadowTable[MAXPROC];

//Semaphore struct with its value and the mailbox used for blocking
typedef struct semaphore{
    int value;
    int mbox;
} sem;


int nextSem;
//Array allocated for all semaphore structs
sem semTable[MAXSEMS];

//Called by most functions, prints out values of arg struct either as ints or pointers
static void printArgs(USLOSS_Sysargs * args){
    if(printDebug != 1){
        return;
    }
    int ints = 1;
    if(ints == 0){
        USLOSS_Console("=====================\n");
        USLOSS_Console("==  Printing Args  ==\n");
        USLOSS_Console("=====================\n");
        USLOSS_Console("==  Number: %d\n", args->number);
        USLOSS_Console("==  Arg1: %p\n", args->arg1);
        USLOSS_Console("==  Arg2: %p\n", args->arg2);
        USLOSS_Console("==  Arg3: %p\n", args->arg3);
        USLOSS_Console("==  Arg4: %p\n", args->arg4);
        USLOSS_Console("==  Arg5: %p\n", args->arg5);
        USLOSS_Console("=====================\n");
    }else{
        USLOSS_Console("=====================\n");
        USLOSS_Console("==  Printing Args  ==\n");
        USLOSS_Console("=====================\n");
        USLOSS_Console("==  Number: %d\n", args->number);
        USLOSS_Console("==  Arg1: %d\n", args->arg1);
        USLOSS_Console("==  Arg2: %d\n", args->arg2);
        USLOSS_Console("==  Arg3: %d\n", args->arg3);
        USLOSS_Console("==  Arg4: %d\n", args->arg4);
        USLOSS_Console("==  Arg5: %d\n", args->arg5);
        USLOSS_Console("=====================\n");
    }
}

// * makes sure that the current mode is in kernel mode
// Halts program and returns error if not in kernel mode
// Taken from our Phase1 code
void enforceKernelMode(char *caller)
{
    if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) == 0)
    {
        // USLOSS_Console("Error: Not in Kernel Mode \n");
        USLOSS_Console("ERROR: Someone attempted to call %s while in user mode!\n", caller);
        USLOSS_Halt(1);
    }
}

//Makes sure pointer to args struct is not NULL
static void checkArgs(USLOSS_Sysargs *args){
    if(args == NULL){
        USLOSS_Console("Args Struct Null\n");
        USLOSS_Halt(1);
    }
}

//Creates a semaphore on the kernel level
//Returns 0 if successful, -1 if invalid args or no more room for new semaphores
int kernSemCreate(int value, int *semaphore){
    enforceKernelMode("KernSemCreate");
    if(nextSem == MAXSEMS || value < 0 || &semaphore == NULL){
        return -1;
    }
    semTable[nextSem].value = value;
    semTable[nextSem].mbox = -1;
    *semaphore = nextSem++;
    return 0;
}

//Decrements a semaphore, or if semaphore value is 0 blocks using mailbox until 
//semaphore is incremented
//Returns 0 if successful, -1 if invalid values
int kernSemP(int semaphore){
    enforceKernelMode("KernSemP");
    if(semaphore < 0 || semaphore > MAXSEMS || semTable[semaphore].value == -1){
        return -1;
    }
    if(semTable[semaphore].value == 0){
        if(semTable[semaphore].mbox == -1){
            semTable[semaphore].mbox = MboxCreate(MAXSLOTS, 0);
        }
        if(printDebug == 1){
            USLOSS_Console("KernSemP %d Blocking at Mailbox %d\n", semaphore, semTable[semaphore].mbox);
        }
        MboxRecv(semTable[semaphore].mbox, NULL, 0);
    }
    semTable[semaphore].value--;
    return 0;
}

//Increments a semaphore value on the kernel level, returns -1 if invalid values and
//0 if successful. Performs conditional send to wake up any blocked kernSemP functions
int kernSemV(int semaphore){
    enforceKernelMode("KernSemV");
    if(semaphore < 0 || semaphore > MAXSEMS || semTable[semaphore].value == -1){
        return -1;
    }
    semTable[semaphore].value++;
    if(semTable[semaphore].mbox != -1){
        if(printDebug == 1){
            USLOSS_Console("KernSemV %d Sending to Mailbox %d\n", semaphore, semTable[semaphore].mbox);
        }
        int retval = MboxCondSend(semTable[semaphore].mbox, NULL, 0);
        if(printDebug == 1){
            USLOSS_Console("CondSend Retval: %d\n", retval);
        }
        // if(semTable[semaphore].value == 1){
        //     MboxSend(semTable[semaphore].mbox, NULL, 0);
        // }
    }
    return 0;
}

//Trampoline for spawn kernel function, reads in a struct from a mailbox,
//switches to user mode and calls the function in the struct
void trampoline(int mID){
    if(printDebug == 1){
        USLOSS_Console("trampoline called\n");
    }
    process newProcess;
    if(MboxCondRecv(mID, &newProcess, sizeof(process)) == -2){
        USLOSS_Console("Trampoline not given function to call\n");
        return;
    }
    MboxRelease(mID);
    //Setting user mode
    unsigned int psr = USLOSS_PsrGet();
    USLOSS_PsrSet(psr & ~USLOSS_PSR_CURRENT_MODE);
    Terminate(newProcess.func(newProcess.param));
    //Restoring Previous PSR
    USLOSS_PsrSet(psr);
}

//Sporks the trampoline function with its arg parameter as the mbox
//to read in the function pointer.
//Returns 0 on arg4 if spork works and -1 otherwise
static void spawnHandler(USLOSS_Sysargs *args){
    if(printDebug == 1){
        USLOSS_Console("Spawn Handler\n");
    }
    enforceKernelMode("SpawnHandler");
    checkArgs(&args);
    printArgs(args);
    process spawned;
    spawned.func = args->arg1;
    spawned.param = args->arg2;
    int * mboxID = MboxCreate(1, sizeof(process));
    MboxCondSend(mboxID, &spawned, sizeof(process));
    int retval = spork((char *) args->arg5, trampoline, mboxID, 
        (int) args->arg3, (int)args->arg4);
    if(retval == -1){
        args->arg4 = (void *) -1;
    }else{
        args->arg4 = (void *) 0;
    }
    args->arg1 = (void *) retval;
    printArgs(args);
}

static void waitHandler(USLOSS_Sysargs *args){
    if(printDebug == 1){
        USLOSS_Console("Wait Handler\n");
    }
    enforceKernelMode("WaitHandler");
    checkArgs(&args);
    printArgs(args);
    int * status = 0;
    int pid = join(&status);
    //USLOSS_Console("Joined\n");
    if(pid == -2){
        args->arg4 = (void *) -2;
        //USLOSS_Console("If\n");
    }else{
        //USLOSS_Console("Else\n");
        args->arg1 = (void *)(long) pid;
        //USLOSS_Console("Status: %d\n", status);
        args->arg2 = (void *)(long) status;
        //USLOSS_Console("status\n");
        args->arg4 = (void *)(long) 0;
    }
    printArgs(args);
}

static void terminateHandler(USLOSS_Sysargs *args){
    if(printDebug == 1){
        USLOSS_Console("Terminate Handler\n");
    }
    enforceKernelMode("TerminateHandler");
    checkArgs(&args);
    printArgs(args);
    int * status = args->arg1;
    int retval = 0;
    while(retval != -2){
        retval = join(&status);
        //USLOSS_Console("Looping, retval: %d, Status Ptr: %p\n", retval, status);
    }
    //USLOSS_Console("End Loop\n");
    quit((int) args->arg1);
    printArgs(args);
}

static void semCreateHandler(USLOSS_Sysargs *args){
    if(printDebug == 1){
        USLOSS_Console("SemCreate Handler\n");
    }
    enforceKernelMode("SemCreateHandler");
    checkArgs(&args);
    printArgs(&args);
    //int id;
    args->arg4 = kernSemCreate(args->arg1, &args->arg1);
    // if(args->arg4 == 0){
    //     args->arg1 = id;
    // }
    printArgs(&args);
}

static void semPHandler(USLOSS_Sysargs *args){
    if(printDebug == 1){
        USLOSS_Console("SemP Handler\n");
    }
    enforceKernelMode("semPHandler");
    checkArgs(&args);
    args->arg4 = kernSemP(args->arg1);
    if(printDebug == 1){
        USLOSS_Console("SemP Handler Done\n");
    }
}

static void semVHandler(USLOSS_Sysargs *args){
    if(printDebug == 1){
        USLOSS_Console("SemV Handler\n");
    }
    enforceKernelMode("SemV Handler\n");
    checkArgs(&args);
    args->arg4 = kernSemV(args->arg1);
    if(printDebug == 1){
        USLOSS_Console("SemV Handler Done\n");
    }
}

//I don't think this is needed
/*
static void kernHandler(USLOSS_Sysargs *args){
    if(printDebug == 1){
        USLOSS_Console("KermSem Func Handler\n");
    }
}*/

static void getTODHandler(USLOSS_Sysargs *args){
    if(printDebug == 1){
        USLOSS_Console("GetTimeOfDay Handler\n");
    }
    enforceKernelMode("getTODHandler");
    checkArgs(&args);
    args->arg1 = (void *) currentTime();
}

static void getPIDHandler(USLOSS_Sysargs *args){
    if(printDebug == 1){
        USLOSS_Console("getPID Handler\n");
    }
    enforceKernelMode("GetPIDHandler");
    checkArgs(&args);
    args->arg1 = (void *) getpid();
}

static void dumpHandler(USLOSS_Sysargs *args){
    if(printDebug == 1){
        USLOSS_Console("Dump Processes Handler\n");
    }
    enforceKernelMode("dumpHandler");
    checkArgs(&args);
    dumpProcesses();
}

// for syscall interrupts
// type is ignored (it's always USLOSS_SYSCALL_INT)
// arg is a pointer to a USLOSS_Sysargs struct
// from phase2 code
void intHandlerSyscall(int type, void *arg)
{
    //enforceKernelMode("intHandlerSyscall");

    // cast the pointer to a USLOSS_Sysargs pointer
    USLOSS_Sysargs *args = (USLOSS_Sysargs*)arg;

    // get the system call number
    int syscallNum = args->number;

    if (syscallNum < 0 || syscallNum >= USLOSS_MAX_SYSCALLS)
    {
        USLOSS_Console("syscallHandler(): Invalid syscall number %d\n", syscallNum);
        USLOSS_Halt(1);
    }

    // call the system call
    systemCallVec[syscallNum](args);
}

//For any syscall num we aren't supposed to implement
static void badCallHandler(USLOSS_Sysargs *args){
    USLOSS_Console("Syscall we aren't supposed to implement called");
}

void phase3_init(){
    for(int i = 0; i < MAXSEMS; i++){
        semTable[i].mbox = -1;
        semTable[i].value = -1;
    }
    nextSem = 0;
    //totalSem = 0;
    for (int i = 0; i < USLOSS_MAX_SYSCALLS; i++)
    { 
        systemCallVec[i] = badCallHandler;
    }
    systemCallVec[SYS_SPAWN] = spawnHandler;
    systemCallVec[SYS_WAIT] = waitHandler;
    systemCallVec[SYS_TERMINATE] = terminateHandler;
    systemCallVec[SYS_SEMCREATE] = semCreateHandler;
    systemCallVec[SYS_SEMP] = semPHandler;
    systemCallVec[SYS_SEMV] = semVHandler;
    systemCallVec[SYS_GETTIMEOFDAY] = getTODHandler;
    systemCallVec[SYS_GETPID] = getPIDHandler;
    systemCallVec[SYS_DUMPPROCESSES] = dumpHandler;

    USLOSS_IntVec[USLOSS_SYSCALL_INT] = intHandlerSyscall;
}

void phase3_start_service_processes(){
    
}