#include <phase3.h>
#include <usloss.h>
#include <phase2.h>
#include <phase1.h>
#include <stdlib.h>

int printDebug = 0;

typedef struct fakePCB {
    int pid;
    int (*func)(void *);
    void *param;
} process;

process shadowTable[MAXPROC];

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

// this one is used as a helper, as it is called in psrEnable and psrDisable
// * makes sure that the current mode is in kernel mode
// Taken from Phase1
void enforceKernelMode(char *caller)
{
    if ((USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE) == 0)
    {
        // USLOSS_Console("Error: Not in Kernel Mode \n");
        USLOSS_Console("ERROR: Someone attempted to call %s while in user mode!\n", caller);
        USLOSS_Halt(1);
    }
}

static void checkArgs(USLOSS_Sysargs *args){
    if(args == NULL){
        USLOSS_Console("Args Struct Null\n");
        USLOSS_Halt(1);
    }
}

static void trampoline(){
    //Setting user mode
    unsigned int psr = USLOSS_PsrGet();
    USLOSS_PsrSet(psr & ~USLOSS_PSR_CURRENT_MODE);



    //Restoring Previous PSR
    USLOSS_PsrSet(psr);
}

static void spawnHandler(USLOSS_Sysargs *args){
    if(printDebug == 1){
        USLOSS_Console("Spawn Handler\n");
    }
    enforceKernelMode("SpawnHandler");
    checkArgs(&args);
    printArgs(args);
    int retval = spork((char *) args->arg5, (int (*)(void *)) args->arg1, args->arg2, 
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
    USLOSS_Console("I HAVEN'T DONE SEMCREATE YET\n");
    USLOSS_Halt(1);
}

static void semPHandler(USLOSS_Sysargs *args){
    if(printDebug == 1){
        USLOSS_Console("SemP Handler\n");
    }
    enforceKernelMode("semPHandler");
    checkArgs(&args);
    USLOSS_Console("I HAVEN'T DONE SEMP YET\n");
    USLOSS_Halt(1);
}

static void semVHandler(USLOSS_Sysargs *args){
    if(printDebug == 1){
        USLOSS_Console("SemV Handler\n");
    }
    enforceKernelMode("SemV Handler\n");
    checkArgs(&args);
    USLOSS_Console("I HAVEN'T DONE SEMV YET\n");
    USLOSS_Halt(1);
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
    systemCallVec[syscallNum](args); // note: in phase2, these are all nullsys
}

//For any syscall num we aren't supposed to implement
static void badCallHandler(USLOSS_Sysargs *args){
    USLOSS_Console("Syscall we aren't supposed to implement called");
}

void phase3_init(){
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