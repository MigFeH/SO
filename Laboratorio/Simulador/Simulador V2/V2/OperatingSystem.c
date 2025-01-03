#include "Simulator.h"
#include "OperatingSystem.h"
#include "OperatingSystemBase.h"
#include "MMU.h"
#include "Processor.h"
#include "Buses.h"
#include "Heap.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>

// Functions prototypes
void OperatingSystem_PCBInitialization(int, int, int, int, int);
void OperatingSystem_MoveToTheREADYState(int);
void OperatingSystem_Dispatch(int);
void OperatingSystem_RestoreContext(int);
void OperatingSystem_SaveContext(int);
void OperatingSystem_TerminateExecutingProcess();
int OperatingSystem_LongTermScheduler();
void OperatingSystem_PreemptRunningProcess();
int OperatingSystem_CreateProcess(int);
int OperatingSystem_ObtainMainMemory(int, int);
int OperatingSystem_ShortTermScheduler();
int OperatingSystem_ExtractFromReadyToRun();
void OperatingSystem_HandleException();
void OperatingSystem_HandleSystemCall();
void OperatingSystem_PrintReadyToRunQueue();
// Inicio V2-ej1(b)
void OperatingSystem_HandleClockInterrupt();
// Fin V2-ej1(b)

// Inicio V2-ej5(f)
void OperatingSystem_MoveToTheBlockedState(int);
// Fin V2-ej5(f)

// Inicio V2-ej6(a)
//void OperatingSystem_ExtractFromSleepingQueue(int);
int OperatingSystem_ExtractFromSleepingQueue(int); // Cambio hecho en el V2-ej6(b)
// Fin V2-ej6(a)

// The process table
// PCB processTable[PROCESSTABLEMAXSIZE];
PCB * processTable;

// Size of the memory occupied for the OS
int OS_MEMORY_SIZE=32;

// Address base for OS code in this version
int OS_address_base; 

// Identifier of the current executing process
int executingProcessID=NOPROCESS;

// Identifier of the System Idle Process
int sipID;

// Initial PID for assignation (Not assigned)
int initialPID=-1;

// Begin indes for daemons in programList
// int baseDaemonsInProgramList; 

// Array that contains the identifiers of the READY processes
// heapItem readyToRunQueue[NUMBEROFQUEUES][PROCESSTABLEMAXSIZE];
heapItem *readyToRunQueue[NUMBEROFQUEUES];

// Inicio V1-ej11(a)
//int numberOfReadyToRunProcesses[NUMBEROFQUEUES]={0};
int numberOfReadyToRunProcesses[NUMBEROFQUEUES]={0,0};
char * queueNames [NUMBEROFQUEUES]={"USER","DAEMONS"};
// Fin V1-ej11(a)

// Variable containing the number of not terminated user processes
int numberOfNotTerminatedUserProcesses=0;

// char DAEMONS_PROGRAMS_FILE[MAXFILENAMELENGTH]="teachersDaemons";

int MAINMEMORYSECTIONSIZE = 60;

extern int MAINMEMORYSIZE;

int PROCESSTABLEMAXSIZE = 4;

// Inicio V1-ej10(a)
char * statesNames [5]={"NEW","READY","EXECUTING","BLOCKED","EXIT"};
// Fin V1-ej10(a)

// Inicio V2-ej1(e)
int numberOfClockInterrupts = 0;
// Fin V2-ej1(e)

// Inicio V2-ej5(b)
// Heap with blocked processes sort by when to wakeup
heapItem *sleepingProcessesQueue;
int numberOfSleepingProcesses=0;
// Fin V2-ej5(b)

// Initial set of tasks of the OS
void OperatingSystem_Initialize(int programsFromFileIndex) {
	
	int i, selectedProcess;
	FILE *programFile; // For load Operating System Code
	
// In this version, with original configuration of memory size (300) and number of processes (4)
// every process occupies a 60 positions main memory chunk 
// and OS code and the system stack occupies 60 positions 

	OS_address_base = MAINMEMORYSIZE - OS_MEMORY_SIZE;

	MAINMEMORYSECTIONSIZE = OS_address_base / PROCESSTABLEMAXSIZE;

	if (initialPID<0) // if not assigned in options...
	{
		// Inicio V1-ej8
		initialPID=PROCESSTABLEMAXSIZE-1;
		// Fin V1-ej8
	}
	// Space for the processTable
	processTable = (PCB *) malloc(PROCESSTABLEMAXSIZE*sizeof(PCB));
	
	// Space for the ready to run queues (one queue initially...)
	// Inicio V1-ej11(c)
	//readyToRunQueue[ALLPROCESSESQUEUE] = Heap_create(PROCESSTABLEMAXSIZE);
	readyToRunQueue[USERPROCESSQUEUE] = Heap_create(PROCESSTABLEMAXSIZE); // cola de listos para procesos del usuario
	readyToRunQueue[DAEMONSQUEUE] = Heap_create(PROCESSTABLEMAXSIZE); // cola de listos para procesos del sistema (daemon)
	// Fin V1-ej11(c)

	// Inicio V2-ej5(b)
	sleepingProcessesQueue = Heap_create(PROCESSTABLEMAXSIZE); // creamos la cola de prioridad para tantos procesos como pueda soportar el sistema (PROCESSTABLEMAXSIZE) de la misma forma que para las colas de listos
	// Fin V2-ej5(b)

	programFile=fopen("OperatingSystemCode", "r");
	if (programFile==NULL){
		// Show red message "FATAL ERROR: Missing Operating System!\n"
		ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,99,SHUTDOWN,"FATAL ERROR: Missing Operating System!\n");
		exit(1);		
	}

	// Obtain the memory requirements of the program
	int processSize=OperatingSystem_ObtainProgramSize(programFile);

	// Load Operating System Code
	OperatingSystem_LoadProgram(programFile, OS_address_base, processSize);
	
	// Process table initialization (all entries are free)
	for (i=0; i<PROCESSTABLEMAXSIZE;i++){
		processTable[i].busy=0;
		processTable[i].programListIndex=-1;
		processTable[i].initialPhysicalAddress=-1;
		processTable[i].processSize=-1;
		processTable[i].copyOfSPRegister=-1;
		processTable[i].state=-1;
		processTable[i].priority=-1;
		processTable[i].copyOfPCRegister=-1;
		processTable[i].copyOfPSWRegister=0;
		processTable[i].programListIndex=-1;
	}
	// Initialization of the interrupt vector table of the processor
	Processor_InitializeInterruptVectorTable(OS_address_base+2);
		
	// Include in program list all user or system daemon processes
	OperatingSystem_PrepareDaemons(programsFromFileIndex);
	
	// Create all user processes from the information given in the command line
	OperatingSystem_LongTermScheduler();

	// Inicio V1-ej14
	if(numberOfNotTerminatedUserProcesses == 0)
	{
		// finalizamos la simulacion
		OperatingSystem_ReadyToShutdown();
	}
	// Fin V1-ej14

	if (strcmp(programList[processTable[sipID].programListIndex]->executableName,"SystemIdleProcess")
		&& processTable[sipID].state==READY) {
		// Show red message "FATAL ERROR: Missing SIP program!\n"
		ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,99,SHUTDOWN,"FATAL ERROR: Missing SIP program!\n");
		exit(1);		
	}

	// At least, one process has been created
	// Select the first process that is going to use the processor
	selectedProcess=OperatingSystem_ShortTermScheduler();

	Processor_SetSSP(MAINMEMORYSIZE-1);

	// Assign the processor to the selected process
	OperatingSystem_Dispatch(selectedProcess);

	// Initial operation for Operating System
	Processor_SetPC(OS_address_base);

	// Inicio V2-ej3(a)
	OperatingSystem_PrintStatus(); // Llamamos a la funcion
	// Fin V2-ej3(a)
}

// The LTS is responsible of the admission of new processes in the system.
// Initially, it creates a process from each program specified in the 
// 			command line and daemons programs
int OperatingSystem_LongTermScheduler() {
  
	int PID, i,
		numberOfSuccessfullyCreatedProcesses=0;
	
	for (i=0; programList[i]!=NULL && i<PROGRAMSMAXNUMBER ; i++) {
		PID=OperatingSystem_CreateProcess(i);

		// Inicio V1-ej4(b)
		if(PID == NOFREEENTRY) // creacion de proceso fallida por no haber entrada libre en la entrada de procesos...
		{
			ComputerSystem_DebugMessage(TIMED_MESSAGE,103,ERROR,programList[i]->executableName);
			// Inicio V1-ej14
			numberOfSuccessfullyCreatedProcesses--;
			// Fin V1-ej14
		}
		// Fin V1-ej4 (b)

		// Inicio V1-ej5(b)
		else if(PID == PROGRAMDOESNOTEXIST) // creacion de proceso fallida por no existir el programa a ejecutar...
		{
			ComputerSystem_DebugMessage(TIMED_MESSAGE,104,ERROR,programList[i]->executableName,"--- it does not exist ---");
			// Inicio V1-ej14
			numberOfSuccessfullyCreatedProcesses--;
			// Fin V1-ej14
		}
		// Fin V1-ej5(b)

		else if(PID == PROGRAMNOTVALID) // creacion de proceso fallida por no tener un tamanio o prioridad valida...
		{
			ComputerSystem_DebugMessage(TIMED_MESSAGE,104,ERROR,programList[i]->executableName,"--- invalid priority or size ---");
			// Inicio V1-ej14
			numberOfSuccessfullyCreatedProcesses--;
			// Fin V1-ej14
		}

		// Inicio V1-ej6(b)
		else if(PID == TOOBIGPROCESS) // creacion de proceso fallida porque el tamanio del programa excede el delimitado por MAINMEMORYSIZE
		{
			ComputerSystem_DebugMessage(TIMED_MESSAGE,105,ERROR,programList[i]->executableName);
			// Inicio V1-ej14
			numberOfSuccessfullyCreatedProcesses--;
			// Fin V1-ej14
		}
		// Fin V1-ej6(b)

		else // creacion de proceso lograda
		{
			numberOfSuccessfullyCreatedProcesses++;
			if (programList[i]->type==USERPROGRAM) 
				numberOfNotTerminatedUserProcesses++;
			// Move process to the ready state
			OperatingSystem_MoveToTheREADYState(PID);
			// Inicio V2-ej3(a)
			OperatingSystem_PrintStatus(); // CASO: PLP creo un proceso nuevo ; Llamamos a la funcion
			// Fin V2-ej3(a)
		}
		
	}

	// Return the number of succesfully created processes
	return numberOfSuccessfullyCreatedProcesses;
}

// This function creates a process from an executable program
int OperatingSystem_CreateProcess(int indexOfExecutableProgram) {
  
	int PID;
	int processSize;
	int loadingPhysicalAddress;
	int priority;
	FILE *programFile;
	PROGRAMS_DATA *executableProgram=programList[indexOfExecutableProgram];

	// Obtain a process ID
	PID=OperatingSystem_ObtainAnEntryInTheProcessTable();
	
	// Inicio V1-ej4(a)
	if(PID == NOFREEENTRY) // el PID es igual al valor que indica que no hay entrada libre en la tabla de procesos...
	{
		return PID;
	}
	// Fin V1-ej4(a)

	// Check if programFile exists
	programFile=fopen(executableProgram->executableName, "r");
	
	// Inicio V1-ej5(b)
	if(programFile == NULL) // si el programa a ejecutar no existe en el directorio del ejecutable Simulador...
	{
		return PROGRAMDOESNOTEXIST;
	}
	// Fin V1-ej5(b)

	// Obtain the memory requirements of the program
	processSize=OperatingSystem_ObtainProgramSize(programFile);

	// Fin V1-ej6(a)
	if(processSize >= MAINMEMORYSECTIONSIZE) // comprobamos que el tamanio del programa no exceda el tamanio maximo predefinido
	{
		return TOOBIGPROCESS; // le devolvemos a OperatingSystem_LongTermScheduler() el valor que nos indica que el tamanio del programa excede el limite establecido en MAINMEMORYSIZE
	}
	// Fin V1-ej6(a)

	// Obtain the priority for the process
	priority=OperatingSystem_ObtainPriority(programFile);

	// Inicio V1-ej5(c)
	if(processSize == PROGRAMNOTVALID || priority == PROGRAMNOTVALID) // si el programa a ejecutar no tiene un tamanio o prioridad valido...
	{
		return PROGRAMNOTVALID;
	}
	// Fin V1-ej5(c)
	
	// Obtain enough memory space
 	loadingPhysicalAddress=OperatingSystem_ObtainMainMemory(processSize, PID);

	// Load program in the allocated memory
	// Inicio V1-ej7(a)
	int resultado = OperatingSystem_LoadProgram(programFile, loadingPhysicalAddress, processSize); // guardamos en la variable resultado si se ha cargado el programa correctamente o no
	if(resultado == TOOBIGPROCESS)
	{
		return TOOBIGPROCESS;
	}
	// Fin V1-ej7(a)

	// PCB initialization
	OperatingSystem_PCBInitialization(PID, loadingPhysicalAddress, processSize, priority, indexOfExecutableProgram);

	return PID;
}


// Main memory is assigned in chunks. All chunks are the same size. A process
// always obtains the chunk whose position in memory is equal to the processor identifier
int OperatingSystem_ObtainMainMemory(int processSize, int PID) {

 	if (processSize>MAINMEMORYSECTIONSIZE)
		return TOOBIGPROCESS;
	
 	return PID*MAINMEMORYSECTIONSIZE;
}


// Assign initial values to all fields inside the PCB
void OperatingSystem_PCBInitialization(int PID, int initialPhysicalAddress, int processSize, int priority, int processPLIndex) {

	processTable[PID].busy=1;
	processTable[PID].initialPhysicalAddress=initialPhysicalAddress;
	processTable[PID].processSize=processSize;
	processTable[PID].copyOfSPRegister=initialPhysicalAddress+processSize;
	processTable[PID].state=NEW;
	processTable[PID].priority=priority;
	processTable[PID].programListIndex=processPLIndex;

	// Inicio V1-ej10(b)
	ComputerSystem_DebugMessage(TIMED_MESSAGE,111,SYSPROC,PID,programList[processPLIndex]->executableName); // mostramos por pantalla el mensaje que indica que el proceso se ha creado (pasa al estado NEW)
	// Fin V1-ej10(b)

	// Daemons run in protected mode and MMU use real address
	if (programList[processPLIndex]->type == DAEMONPROGRAM) {
		processTable[PID].copyOfPCRegister=initialPhysicalAddress;
		processTable[PID].copyOfPSWRegister= ((unsigned int) 1) << EXECUTION_MODE_BIT;
		// Inicio V1-ej11(c)
		processTable[PID].queueID = DAEMONSQUEUE;
		// Fin V1-ej11(c)
	} 
	else {
		processTable[PID].copyOfPCRegister=0;
		processTable[PID].copyOfPSWRegister=0;
		// Inicio V1-ej11(c)
		processTable[PID].queueID = USERPROCESSQUEUE;
		// Fin V1-ej11(c)
	}

	// Inicio V1-ej13(e)
	processTable[PID].copyOfAccumulatorRegister = 0;
	processTable[PID].copyOfARegister = 0; // CAMBIO HECHO EN LA V2
	processTable[PID].copyOfBRegister = 0; // CAMBIO HECHO EN LA V2
	// Fin V1-ej13(e)

	// Inicio V2-ej5(a)
	processTable[PID].whenToWakeUp = -1;
	// Fin V2-ej5(a)
}


// Move a process to the READY state: it will be inserted, depending on its priority, in
// a queue of identifiers of READY processes
void OperatingSystem_MoveToTheREADYState(int PID) {
	// Inicio V1-ej11(c)
	if (Heap_add(PID, readyToRunQueue[processTable[PID].queueID/*ALLPROCESSESQUEUE*/],QUEUE_PRIORITY ,&(numberOfReadyToRunProcesses[processTable[PID].queueID/*ALLPROCESSESQUEUE*/]))>=0) {
	// Fin V1-ej11(c)

		// Inicio V1-ej10(b)
		int anteriorEstado = processTable[PID].state; // guardamos el anterior estado del proceso
		processTable[PID].state=READY;
		int indiceEnLaListaDeProgramas = processTable[PID].programListIndex; // obtenemos el indice del proceso en la lista de programas
		ComputerSystem_DebugMessage(TIMED_MESSAGE,110,SYSPROC,PID,programList[indiceEnLaListaDeProgramas]->executableName,statesNames[anteriorEstado],statesNames[processTable[PID].state]); // mostramos por pantalla el mensaje que indica que el proceso pasa de un estado a otro
		// Fin V1-ej10(b)
	}
	
	// Inicio V2-ej4
	
	// Inicio V1-ej9(b)
	//OperatingSystem_PrintReadyToRunQueue(); // comentamos la llamada (el V2-ej4)
	// Fin V1-ej9(b)
	
	// Fin V2-ej4
}


// The STS is responsible of deciding which process to execute when specific events occur.
// It uses processes priorities to make the decission. Given that the READY queue is ordered
// depending on processes priority, the STS just selects the process in front of the READY queue
int OperatingSystem_ShortTermScheduler() {
	
	int selectedProcess;

	selectedProcess=OperatingSystem_ExtractFromReadyToRun();
	
	return selectedProcess;
}


// Return PID of more priority process in the READY queue
int OperatingSystem_ExtractFromReadyToRun() {
  
	int selectedProcess=NOPROCESS;

	// Inicio V1-ej11(c)
	for(int i = 0; i < NUMBEROFQUEUES; i++) // recorremos todas las colas
	{
		selectedProcess = Heap_poll(readyToRunQueue[i], QUEUE_PRIORITY, &(numberOfReadyToRunProcesses[i])); // recogemos el proceso de mayor prioridad en la cola i
		if(selectedProcess != NOPROCESS) // si el proceso recogido es distinto de NOPROCESS...
		{
			return selectedProcess; // retornamos el proceso recogido
		}
	}
	//selectedProcess=Heap_poll(readyToRunQueue[ALLPROCESSESQUEUE],QUEUE_PRIORITY ,&(numberOfReadyToRunProcesses[ALLPROCESSESQUEUE]));
	// Fin V1-ej11(c)

	// Return most priority process or NOPROCESS if empty queue
	return selectedProcess; 
}


// Function that assigns the processor to a process
void OperatingSystem_Dispatch(int PID) {

	// The process identified by PID becomes the current executing process
	executingProcessID=PID;

	// Inicio V1-ej10(b)
	int anteriorEstado = processTable[PID].state; // guardamos el anterior estado del proceso
	processTable[PID].state=EXECUTING; // Change the process' state
	int indiceEnLaListaDeProgramas = processTable[PID].programListIndex; // obtenemos el indice del proceso en la lista de programas
	ComputerSystem_DebugMessage(TIMED_MESSAGE,110,SYSPROC,PID,programList[indiceEnLaListaDeProgramas]->executableName,statesNames[anteriorEstado],statesNames[processTable[PID].state]); // mostramos por pantalla el mensaje que indica que el proceso pasa de un estado a otro
	// Fin V1-ej10(b)

	// Modify hardware registers with appropriate values for the process identified by PID
	OperatingSystem_RestoreContext(PID);
}


// Modify hardware registers with appropriate values for the process identified by PID
void OperatingSystem_RestoreContext(int PID) {
  
	// New values for the CPU registers are obtained from the PCB
	Processor_PushInSystemStack(processTable[PID].copyOfPCRegister);
	Processor_PushInSystemStack(processTable[PID].copyOfPSWRegister);
	Processor_SetRegisterSP(processTable[PID].copyOfSPRegister);

	// Same thing for the MMU registers
	MMU_SetBase(processTable[PID].initialPhysicalAddress);
	MMU_SetLimit(processTable[PID].processSize);

	// Inicio V1-ej13(e)
	Processor_SetAccumulator(processTable[PID].copyOfAccumulatorRegister);
	Processor_SetRegisterA(processTable[PID].copyOfARegister); // CAMBIO HECHO EN LA V2
	Processor_SetRegisterB(processTable[PID].copyOfBRegister); // CAMBIO HECHO EN LA V2
	// Fin V1-ej13(e)
}


// Function invoked when the executing process leaves the CPU 
void OperatingSystem_PreemptRunningProcess() {

	// Save in the process' PCB essential values stored in hardware registers and the system stack
	OperatingSystem_SaveContext(executingProcessID);
	// Change the process' state
	OperatingSystem_MoveToTheREADYState(executingProcessID);
	// The processor is not assigned until the OS selects another process
	executingProcessID=NOPROCESS;
}


// Save in the process' PCB essential values stored in hardware registers and the system stack
void OperatingSystem_SaveContext(int PID) {
	
	// Load PSW saved for interrupt manager
	processTable[PID].copyOfPSWRegister=Processor_PopFromSystemStack();
	
	// Load PC saved for interrupt manager
	processTable[PID].copyOfPCRegister=Processor_PopFromSystemStack();
	
	// Save RegisterSP 
	processTable[PID].copyOfSPRegister=Processor_GetRegisterSP();

	// Inicio V1-ej13(e)
	processTable[PID].copyOfAccumulatorRegister=Processor_GetAccumulator(); // almacenamos una copia del registro acumulador
	processTable[PID].copyOfARegister=Processor_GetRegisterA(); // almacenamos una copia del registro A // CAMBIO HECHO EN LA V2
	processTable[PID].copyOfBRegister=Processor_GetRegisterB(); // almacenamos una copia del registro B // CAMBIO HECHO EN LA V2
	// Fin V1-ej13(e)
}


// Exception management routine
void OperatingSystem_HandleException() {
  
	// Show message "Process [executingProcessID] has generated an exception and is terminating\n"
	ComputerSystem_DebugMessage(TIMED_MESSAGE,71,INTERRUPT,executingProcessID,programList[processTable[executingProcessID].programListIndex]->executableName);
	
	OperatingSystem_TerminateExecutingProcess();

	// Inicio V2-ej3(a)
	OperatingSystem_PrintStatus(); // Llamamos a la funcion
	// Fin V2-ej3(a)
}

// All tasks regarding the removal of the executing process
void OperatingSystem_TerminateExecutingProcess() {

	// Inicio V1-ej10(b)
	int anteriorEstado = processTable[executingProcessID].state; // guardamos el anterior estado del proceso
	processTable[executingProcessID].state=EXIT;
	int indiceEnLaListaDeProgramas = processTable[executingProcessID].programListIndex; // obtenemos el indice del proceso en la lista de programas
	ComputerSystem_DebugMessage(TIMED_MESSAGE,110,SYSPROC,executingProcessID,programList[indiceEnLaListaDeProgramas]->executableName,statesNames[anteriorEstado],statesNames[processTable[executingProcessID].state]); // mostramos por pantalla el mensaje que indica que el proceso pasa de un estado a otro 
	// Fin V1-ej10(b)
	
	if (executingProcessID==sipID) {
		// finishing sipID, change PC to address of OS HALT instruction
		Processor_SetSSP(MAINMEMORYSIZE-1);
		Processor_PushInSystemStack(OS_address_base+1);
		Processor_PushInSystemStack(Processor_GetPSW());
		executingProcessID=NOPROCESS;
		ComputerSystem_DebugMessage(TIMED_MESSAGE,99,SHUTDOWN,"The system will shut down now...\n");
		return; // Don't dispatch any process
	}

	Processor_SetSSP(Processor_GetSSP()+2); // unstack PC and PSW stacked

	if (programList[processTable[executingProcessID].programListIndex]->type==USERPROGRAM) 
		// One more user process that has terminated
		numberOfNotTerminatedUserProcesses--;
	
	if (numberOfNotTerminatedUserProcesses==0) {
		// Simulation must finish, telling sipID to finish
		OperatingSystem_ReadyToShutdown();
	}
	// Select the next process to execute (sipID if no more user processes)
	int selectedProcess=OperatingSystem_ShortTermScheduler();

	// Assign the processor to that process
	OperatingSystem_Dispatch(selectedProcess);
}

// System call management routine
void OperatingSystem_HandleSystemCall() {

	// Inicio V1-ej12
	// Declaramos las variables locales a usar en esta parte del metodo por temas de compilacion
	int PIDQueAbandona;
	int queueIDQueAbandona;
	int prioridadQueAbandona;
	// Fin V1-ej12

	// Inicio V2-ej5(f)
	// Declaramos las variables locales a usar en esta parte del metodo por temas de compilacion
	int PIDABloquear;
	// Fin V2-ej5(f)

	int systemCallID;

	// Register A contains the identifier of the issued system call
	systemCallID=Processor_GetRegisterC();
	
	switch (systemCallID) {
		case SYSCALL_PRINTEXECPID:
			// Show message: "Process [executingProcessID] has the processor assigned\n"
			ComputerSystem_DebugMessage(TIMED_MESSAGE,72,SYSPROC,executingProcessID,programList[processTable[executingProcessID].programListIndex]->executableName,Processor_GetRegisterA(),Processor_GetRegisterB());
			break;

		case SYSCALL_END:
			// Show message: "Process [executingProcessID] has requested to terminate\n"
			ComputerSystem_DebugMessage(TIMED_MESSAGE,73,SYSPROC,executingProcessID,programList[processTable[executingProcessID].programListIndex]->executableName);
			OperatingSystem_TerminateExecutingProcess();
			// Inicio V2-ej3(a)
			OperatingSystem_PrintStatus(); // Llamamos a la funcion
			// Fin V2-ej3(a)
			break;

		// Inicio V1-ej12
		case SYSCALL_YIELD:
			/* cederá voluntariamente el control del procesador al proceso READY con prioridad idéntica a la suya y que figure como proceso más prioritario en la cola LISTOS 
			que le corresponde al proceso que invoca la llamada a sistema */

			// Primero obtengo el queueID y la prioridad del proceso actual en ejecucion (executingProcessID)
			PIDQueAbandona = executingProcessID; // guardamos el PID del proceso que abandona voluntatiamente la CPU para poder acceder a su PCB e imprimir por pantalla el mensaje 116 o 117 correctamente
			queueIDQueAbandona = processTable[PIDQueAbandona].queueID;
			prioridadQueAbandona = processTable[PIDQueAbandona].priority;

			int PIDObtenido = Heap_getFirst(readyToRunQueue[queueIDQueAbandona],numberOfReadyToRunProcesses[queueIDQueAbandona]); // nos retorna el PID del primer proceso
			// Compruebo si hay algun proceso en la cola queueID
			if(PIDObtenido != NOPROCESS) // el proceso retornado no es NOPROCESS...
			{
				// compruebo si las prioridades de ambos procesos coinciden (el obtenido y el que abandona)
				if(processTable[PIDObtenido].priority == prioridadQueAbandona) // si coinciden...
				{
					// imprimo el mensaje 116
					ComputerSystem_DebugMessage(TIMED_MESSAGE,116,SHORTTERMSCHEDULE,PIDQueAbandona,programList[processTable[PIDQueAbandona].programListIndex]->executableName,PIDObtenido,programList[processTable[PIDObtenido].programListIndex]->executableName);
					// suspendo temporalmente el proceso actual (el que abandona)
					OperatingSystem_PreemptRunningProcess();
					// selecciono el proceso con mayor prioridad (que sera el obtenido anteriormente) con OperatingSystem_ShortTermScheduler()
					int PIDDelRelevador = OperatingSystem_ShortTermScheduler();
					// por ultimo, usando el Dispatch() le asigno a la CPU al proceso relevador (es el mismo que el obtenido anteriormente)
					OperatingSystem_Dispatch(PIDDelRelevador);

					// Inicio V2-ej3(a)
					OperatingSystem_PrintStatus(); // Llamamos a la funcion
					// Fin V2-ej3(a)
				} else // si no coinciden...
				{
					// imprimimos el mensaje 117
					ComputerSystem_DebugMessage(TIMED_MESSAGE,117,SHORTTERMSCHEDULE,PIDQueAbandona,programList[processTable[PIDQueAbandona].programListIndex]->executableName);
				}
			} else // si no hay procesos en la cola...
			{
				// imprimimos el mensaje 117
				ComputerSystem_DebugMessage(TIMED_MESSAGE,117,SHORTTERMSCHEDULE,PIDQueAbandona,programList[processTable[PIDQueAbandona].programListIndex]->executableName);
			}
			break;
		// Fin V1-ej12

		// Inicio V2-ej5(f)
		case SYSCALL_SLEEP:
			/*  Bloqueará al proceso en ejecución (es decir, se tendrá que mover al estado BLOCKED) y lo insertará por orden creciente del campo
			whenToWakeUp en la sleepingProcessesQueue.*/

			PIDABloquear = executingProcessID;
			OperatingSystem_MoveToTheBlockedState(PIDABloquear);
			
			int PIDDelRelevador = OperatingSystem_ShortTermScheduler();
			OperatingSystem_Dispatch(PIDDelRelevador);

			// Inicio V2-ej5(e)
			OperatingSystem_PrintStatus();
			// Fin V2-ej5(e)

			break;
		// Fin V2-ej5(f)
	}
}
	
//	Implement interrupt logic calling appropriate interrupt handle
void OperatingSystem_InterruptLogic(int entryPoint){
	switch (entryPoint){
		case SYSCALL_BIT: // SYSCALL_BIT=2
			OperatingSystem_HandleSystemCall();
			break;
		case EXCEPTION_BIT: // EXCEPTION_BIT=6
			OperatingSystem_HandleException();
			break;
		// Inicio V2-ej1(c)
		case CLOCKINT_BIT:
			OperatingSystem_HandleClockInterrupt();
			break;
		// Fin V2-ej1(c)
	}

}

// Inicio V1-ej9(a)
void OperatingSystem_PrintReadyToRunQueue()
{
	ComputerSystem_DebugMessage(TIMED_MESSAGE,106,SHORTTERMSCHEDULE);

	// Inicio V1-ej11(b)
	// muestra por pantalla el contenido de la cola de procesos del usuario LISTOS
	for(int i = 0; i < NUMBEROFQUEUES; i++)
	{
		ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,113,SHORTTERMSCHEDULE,queueNames[i]); // mostramos por pantalla el nombre de la cola de listos
		
		if(numberOfReadyToRunProcesses[i] == 0)
		{
			ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,109,SHORTTERMSCHEDULE); // el salto de linea
		} else
		{
			int aux = 1; // para contabilizar los procesos del usuario listos que llevamos impresos
			for(int j = 0; j < numberOfReadyToRunProcesses[i] /* el numero de procesos listos de la cola i */; j++)
			{
				int pid = readyToRunQueue[i][j].info; // el PID funciona como el indice de la entrada PCB en la tabla de procesos
				if(numberOfReadyToRunProcesses[i] == 1 || aux == numberOfReadyToRunProcesses[i] /* para este caso tratamos la ultima impresion y hacemos break en ambos casos */)
				{
					ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,114,SHORTTERMSCHEDULE,pid,processTable[pid].priority);
					ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,109,SHORTTERMSCHEDULE); // el salto de linea
					break;
				}
				ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,115,SHORTTERMSCHEDULE,pid,processTable[pid].priority);
				aux++;
			
			}
		}
	}
	// Fin V1-ej11(b)
}
// Fin V1-ej9(a)

// Inicio V2-ej1(b)
void OperatingSystem_HandleClockInterrupt()
{
	// Inicio V2-ej6(a)

	// Inicio V2-ej1(e)
	numberOfClockInterrupts++;
	ComputerSystem_DebugMessage(TIMED_MESSAGE,120,INTERRUPT,numberOfClockInterrupts);
	// Fin V2-ej1(e)

	// Inicio V2-ej6(b)
	int contadorDesbloqueos = 0; // llevamos la cuenta del numero de procesos desbloqueados al tratar la interrupcion
	// Fin V2-ej6(b)

	while(numberOfSleepingProcesses > 0 && processTable[Heap_getFirst(sleepingProcessesQueue, numberOfSleepingProcesses)].whenToWakeUp == numberOfClockInterrupts)
	{
		int PIDProceso = Heap_getFirst(sleepingProcessesQueue, numberOfSleepingProcesses);

		// el proceso se desbloqueara pasando su estado a READY y siendo eliminado de la sleepingProcessesQueue
		contadorDesbloqueos = OperatingSystem_ExtractFromSleepingQueue(PIDProceso);
	}
	// Fin V2-ej6(a)

	// Inicio V2-ej6(b)
	if(contadorDesbloqueos) // si el contador de desbloqueos es mayor que 0...
	{
		OperatingSystem_PrintStatus(); // CASO: se desperto al menos un proceso ; mostramos el nuevo estado del sistema mediante la llamada a la funcion escrita

		// Inicio V2-ej6(c)
		// Tenemos que comprobar si el proceso en ejecucion sigue siendo el mas prioritario de todos los procesos listos para ejecucion
		int PIDProcesoEnEjecucion = executingProcessID;
		
		// El proceso mas prioritario de una cola de listos es el primero en este (RECORDEMOS QUE LA COLA DE USER ES MAS PRIORITARIA QUE LA COLA DE DAEMONS)
		for(int i = 0; i < NUMBEROFQUEUES; i++) // recorremos las colas de listos
		{
			int PIDProcesoMasPrioritario = Heap_getFirst(readyToRunQueue[i], numberOfReadyToRunProcesses[i]);
			if(PIDProcesoMasPrioritario != NOPROCESS
			&& processTable[PIDProcesoMasPrioritario].priority < processTable[PIDProcesoEnEjecucion].priority /* si el proceso mas prioritario obtenido es de una prooridad mayor que la del proceso en ejecucion... */
			&& processTable[PIDProcesoMasPrioritario].queueID == processTable[PIDProcesoEnEjecucion].queueID /* y son de la misma cola... */)
			{
				// sustituimos el proceso en ejecucion por el proceso mas prioritario
				OperatingSystem_PreemptRunningProcess(); // echamos de la CPU al proceso que estaba en ejecucion (se guarda su contexto dentro de la funcion llamada)
				OperatingSystem_Dispatch(Heap_poll(readyToRunQueue[i], QUEUE_PRIORITY, &numberOfReadyToRunProcesses[i])); // asignamos a la CPU el proceso mas prioritario

				ComputerSystem_DebugMessage(TIMED_MESSAGE,121,SHORTTERMSCHEDULE,PIDProcesoEnEjecucion,programList[processTable[PIDProcesoEnEjecucion].programListIndex]->executableName,executingProcessID,programList[processTable[executingProcessID].programListIndex]->executableName); //  mostramos el mensaje 121

				// Inicio V2-ej6(d)
				OperatingSystem_PrintStatus(); // CASO: se cambio el proceso en ejecucion
				// Fin V2-ej6(d)
				break; // para que no se sigan procesando las colas de listos (ya hemos encontrado al proceso mas prioritario sustituto)
			}
		}
		// Fin V2-ej6(c)
	}
	// Fin V2-ej6(b)

	// Inicio V2-ej1(e)
	//numberOfClockInterrupts++;
	//ComputerSystem_DebugMessage(TIMED_MESSAGE,120,INTERRUPT,numberOfClockInterrupts);
	// Fin V2-ej1(e)
}
// Fin V2-ej1(b)

// Inicio V2-ej5(f)
void OperatingSystem_MoveToTheBlockedState(int PidProcesoInsertar)
{
	// Calculamos el retardo del proceso a insertar en la cola de bloqueados
	int retardo = 0;
	if(Processor_GetRegisterD() > 0)
	{
		retardo = Processor_GetRegisterD();
	} else
	{
		retardo = abs(Processor_GetAccumulator());
	}

	processTable[PidProcesoInsertar].whenToWakeUp = retardo + numberOfClockInterrupts + 1; // asignamos el tiempo en el que el proceso despertara
	
	// inserta el proceso cuyo PID es pasado por parametro en la cola de procesos bloqueados
	if (Heap_add(PidProcesoInsertar, sleepingProcessesQueue, QUEUE_WAKEUP, &numberOfSleepingProcesses) >=0) // si se puede insertar el proceso bloqueado en la cola de procesos bloqueados...
	{
		// Inicio V1-ej10(b) (HECHO EN LA V2)
		int anteriorEstado = processTable[PidProcesoInsertar].state;
		processTable[PidProcesoInsertar].state = BLOCKED; // asignamos el estado bloqueado
		ComputerSystem_DebugMessage(TIMED_MESSAGE,110,SYSPROC,PidProcesoInsertar,programList[processTable[PidProcesoInsertar].programListIndex]->executableName,statesNames[anteriorEstado],statesNames[processTable[PidProcesoInsertar].state]); // mostramos que el proceso ha cambiado de estado
		// Fin V1-ej10(b) (HECHO EN LA V2)


		OperatingSystem_SaveContext(PidProcesoInsertar); // salvamos el contexto del proceso insertado en la cola de bloqueados porque va a ser retirado de su ejecucion (mediante un dispatch)
	}
}
// Fin V2-ej5(f)

// Inicio V2-ej6(a)
int OperatingSystem_ExtractFromSleepingQueue(int PID)
{
	// Inicio V2-ej6(b)
	int contadorDesbloqueos = 0;
	// Fin V2-ej6(b)

	// el proceso pasara al estado ready y sera eliminado de la cola de procesos dormidos
	int PIDProcesoSeleccionado = Heap_poll(sleepingProcessesQueue, QUEUE_WAKEUP, &numberOfSleepingProcesses); // eliminamos el proceso de la cola de procesos dormidos (la cola esta ordenada de forma ascendente en funcion del valor del
	// campo whenToWakeUp, es decir, el proceso que le va a tocar ser eliminado de la cola de procesos bloqueados va a ser el primero ya que va a ser el primero con menos whenToWakeUp restante)
	if(PIDProcesoSeleccionado != NOPROCESS)
	{
		OperatingSystem_MoveToTheREADYState(PID); // pasamos el proceso a la cola de procesos listos para ejecucion (el proceso en la funcion llamada pasara a tenr el estado READY)

		// Inicio V2-ej6(b)
		contadorDesbloqueos++; // actualizamos el contador de desbloqueos indicando que el proceso ha sido desbloqueado
		// Fin V2-ej6(b)
	}

	return contadorDesbloqueos;
}
// Fin V2-ej6(a)
