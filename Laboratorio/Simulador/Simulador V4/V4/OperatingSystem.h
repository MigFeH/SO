#ifndef OPERATINGSYSTEM_H
#define OPERATINGSYSTEM_H

#include "ComputerSystem.h"
#include <stdio.h>


#define SUCCESS 1
#define PROGRAMDOESNOTEXIST -1
#define PROGRAMNOTVALID -2

#define NOFREEENTRY -3
#define TOOBIGPROCESS -4

#define NOPROCESS -1

// Inicio V2-ej5(c)
#define SLEEPINGQUEUE
// Fin V2-ej5(c)

// Number of queues of ready to run processes, initially one queue...
// Inicio V1-ej11(a)
//#define NUMBEROFQUEUES 1
//enum TypeOfReadyToRunProcessQueues { ALLPROCESSESQUEUE }; 

#define NUMBEROFQUEUES 2
enum TypeOfReadyToRunProcessQueues { USERPROCESSQUEUE /* mayor prio para los procesos user que los daemon por estar primero en el enumerado */, DAEMONSQUEUE };
// Fin V1-ej11(a)

// Contains the possible type of programs
enum ProgramTypes { USERPROGRAM=100, DAEMONPROGRAM }; 

// Enumerated type containing all the possible process states
enum ProcessStates { NEW, READY, EXECUTING, BLOCKED, EXIT};

// Enumerated type containing the list of system calls and their numeric identifiers
// Inicio V1-ej12
//enum SystemCallIdentifiers { SYSCALL_END=3, SYSCALL_PRINTEXECPID=5 };

// Inicio V2-ej5(f)
//enum SystemCallIdentifiers { SYSCALL_END=3, SYSCALL_YIELD=4, SYSCALL_PRINTEXECPID=5 };
enum SystemCallIdentifiers { SYSCALL_END=3, SYSCALL_YIELD=4, SYSCALL_PRINTEXECPID=5 , SYSCALL_SLEEP=7}; // agregamos una nueva llamada al sistema
// Fin V2-ej5(f)

// Fin V1-ej12

// Inicio V4-ej5(a)
// Partitions configuration definition
#define MEMCONFIG // in OperatingSystem.h
// Fin V4-ej5(a)

// Inicio V4-ej6(d)
#define MEMORYFULL -5 // In OperatingSystem.h
// Fin V4-ej6(d)


// A PCB contains all of the information about a process that is needed by the OS
typedef struct {
	int busy;
	int initialPhysicalAddress;
	int processSize;
	int copyOfSPRegister;
	int state;
	int priority;
	int copyOfPCRegister;
	unsigned int copyOfPSWRegister;
	int programListIndex;
	
	// Inicio V1-ej11(a)
	int queueID; // para que cada proceso conozca la cola a la que pertenece
	// Fin V1-ej11(a)

	// Inicio V1-ej13(e)
	int copyOfAccumulatorRegister; // copia del registro acumulador
	int copyOfARegister; // CAMBIO HECHO EN LA V2
	int copyOfBRegister; // CAMBIO HECHO EN LA V2
	// Fin V1-ej13(e)

	// Inicio V2-ej5(a)
	int whenToWakeUp; // agregamos un campo adicional
	// Fin V2-ej5(a)

	// Inicio V4-ej8
	int partitionIndex; // almacenamos el indice de la particion que tiene asignada para que la liberacion sobre esta sea mas facil de realizar 
	// Fin V4-ej8
} PCB;

// These "extern" declaration enables other source code files to gain access
// to the variable listed


// Functions prototypes
void OperatingSystem_Initialize(int);
void OperatingSystem_InterruptLogic(int);
int OperatingSystem_ShortTermScheduler();
void OperatingSystem_Dispatch(int);
#endif
