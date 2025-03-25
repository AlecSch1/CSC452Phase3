#include <phase3.h>
#include <usloss.h>


void (* systemCallVec[USLOSS_MAX_SYSCALLS])(USLOSS_Sysargs *args);

// for syscall interrupts
// type is ignored (it's always USLOSS_SYSCALL_INT)
// arg is a pointer to a USLOSS_Sysargs struct
// from phase2 code
void intHandlerSyscall(int type, void *arg)
{
    enforceKernelMode("intHandlerSyscall");

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

void phase3_init(){
    
}

void phase3_start_service_processes(){
    
}