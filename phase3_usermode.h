/*
 * This file contains the function definitions for the library interfaces
 * to the USLOSS system calls.
 */
#ifndef _PHASE3_USERMODE_H
#define _PHASE3_USERMODE_H

// Phase 3 -- User Function Prototypes
extern int  Spawn(char *name, int (*func)(void*), void *arg, int stack_size,
                  int priority, int *pid);
extern int  Wait(int *pid, int *status);
extern void Terminate(int status) __attribute__((__noreturn__));
extern void GetTimeofDay(int *tod);
extern void GetPID(int *pid);
extern int  SemCreate(int value, int *semaphore);
extern int  SemP(int semaphore);
extern int  SemV(int semaphore);

   // NOTE: No SemFree() call, it was removed

extern void DumpProcesses(void);

#endif
