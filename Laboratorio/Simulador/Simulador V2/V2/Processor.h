#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "MainMemory.h"
#include "ProcessorBase.h"

#define INTERRUPTTYPES 10
#define CPU_SUCCESS 1
#define CPU_FAIL 0

// Inicio V2-ej2(a)
//enum PSW_BITS {POWEROFF_BIT=0, ZERO_BIT=1, NEGATIVE_BIT=2, OVERFLOW_BIT=3, EXECUTION_MODE_BIT=7};
enum PSW_BITS {POWEROFF_BIT=0, ZERO_BIT=1, NEGATIVE_BIT=2, OVERFLOW_BIT=3, EXECUTION_MODE_BIT=7, INTERRUPT_MASKED_BIT=15}; /* Enumerated type 
that connects bit positions in the PSW register with processor events and status */
// Fin V2-ej2(a)

// Inicio V2-ej1(a) 
//enum INT_BITS {SYSCALL_BIT=2, EXCEPTION_BIT=6};
enum INT_BITS {SYSCALL_BIT=2, EXCEPTION_BIT=6, CLOCKINT_BIT=9}; // Enumerated type that connects bit positions in the interruptLines with interrupt types
// Fin V2-ej1(a)

// Functions prototypes
void Processor_InitializeInterruptVectorTable(int);
int Processor_FetchInstruction();
void Processor_InstructionCycleLoop();
void Processor_DecodeAndExecuteInstruction();
void Processor_RaiseInterrupt(const unsigned int);
void Processor_ManageInterrupts();

char * Processor_ShowPSW();
int Processor_GetCTRL();
void Processor_SetCTRL(int);

// Inicio V2-ej5(d)
int Processor_GetRegisterD();
// Fin V2-ej5(d)

#endif
