/* recursive terminate test */

#include <usloss.h>
#include <usyscall.h>
#include <phase1.h>
#include <phase2.h>
#include <phase3_usermode.h>
#include <stdio.h>

int Child1(void *);
int Child2(void *);
int Child2a(void *);
int Child2b(void *);
int Child2c(void *);

int sem1;


int start3(void *arg)
{
    int pid;
    int status;

    USLOSS_Console("start3(): started\n");

    Spawn("Child1", Child1, "Child1", USLOSS_MIN_STACK, 4, &pid);
    USLOSS_Console("start3(): spawned process %d\n", pid);

    Wait(&pid, &status);
    USLOSS_Console("start3(): child %d returned status of %d\n", pid, status);

    USLOSS_Console("start3(): done\n");
    return 0;
}


int Child1(void *arg) 
{
    int pid;
    int status;

    GetPID(&pid);
    USLOSS_Console("%s(): starting, pid = %d\n", arg, pid);

    Spawn("Child2", Child2, "Child2", USLOSS_MIN_STACK, 2, &pid);
    USLOSS_Console("%s(): spawned process %d\n", arg, pid);

    Wait(&pid, &status);
    USLOSS_Console("%s(): child %d returned status of %d\n", arg, pid, status);

    USLOSS_Console("%s(): done\n", arg);
    Terminate(9);
}

int Child2(void *arg) 
{
    int pid;

    GetPID(&pid);
    USLOSS_Console("%s(): starting, pid = %d\n", arg, pid);

    Spawn("Child2a", Child2a, "Child2a", USLOSS_MIN_STACK, 5, &pid);
    USLOSS_Console("%s(): spawned process %d\n", arg, pid);

    Spawn("Child2b", Child2b, "Child2b", USLOSS_MIN_STACK, 5, &pid);
    USLOSS_Console("%s(): spawned process %d\n", arg, pid);

    Spawn("Child2c", Child2c, "Child2c", USLOSS_MIN_STACK, 5, &pid);
    USLOSS_Console("%s(): spawned process %d\n", arg, pid);

    return 10;
}

int Child2a(void *arg) 
{
    USLOSS_Console("%s(): starting the code for Child2a\n", arg);
    Terminate(11);
}

int Child2b(void *arg) 
{
    USLOSS_Console("%s(): starting the code for Child2b\n", arg);
    return 12;
}

int Child2c(void *arg) 
{
    USLOSS_Console("%s(): starting the code for Child2c\n", arg);
    Terminate(13);
}

