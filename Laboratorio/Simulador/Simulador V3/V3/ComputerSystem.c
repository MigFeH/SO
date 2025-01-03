#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "ComputerSystem.h"
#include "OperatingSystem.h"
#include "ComputerSystemBase.h"
#include "Processor.h"
#include "Messages.h"
#include "Asserts.h"
#include "Wrappers.h"
// Inicio V3-ej1(a)
#include "Heap.h" // para poder crear el moticulo de arrival
// Fin V3-ej1(a)

extern int COLOURED;
extern char *debugLevel;

// Inicio V3-ej1(a)
heapItem *arrivalTimeQueue;
int numberOfProgramsInArrivalTimeQueue=0;
// Fin V3-ej1(a)

// Functions prototypes
void ComputerSystem_PrintProgramList(); // V1-ej1

// Powers on of the Computer System.
void ComputerSystem_PowerOn(int argc, char *argv[], int paramIndex) {
	unsigned int i;
	// To remember the simulator sections to be message-debugged
	for (i=0; i< strlen(debugLevel);i++) {
	  if (isupper(debugLevel[i])){
		COLOURED = 1;
		debugLevel[i]=tolower(debugLevel[i]);
	  }
	}

	// Load debug messages
	int nm=0;
	nm=Messages_Load_Messages(nm,TEACHER_MESSAGES_FILE);
	if (nm<0) {
		ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,64,SHUTDOWN,TEACHER_MESSAGES_FILE);
		exit(2);
	}
	nm=Messages_Load_Messages(nm,STUDENT_MESSAGES_FILE);

	// Prepare if necesary the assert system
	Asserts_LoadAsserts();

	ComputerSystem_DebugMessage(TIMED_MESSAGE, 99, POWERON, "STARTING simulation\n");
	// Obtain a list of programs in the command line
	int programsFromFilesBaseIndex = ComputerSystem_ObtainProgramList(argc, argv, paramIndex);

	// Inicio V1-ej2
	ComputerSystem_PrintProgramList();
	// Fin V1-ej2

	// Inicio V3-ej1(a)
	arrivalTimeQueue = Heap_create(PROCESSTABLEMAXSIZE); /* creamos el montículo de la misma forma que el resto de los montículos del sistema, con el
	mismo tamaño que la lista de programas, antes de inicializar el sistema operativo. */
	// Fin V3-ej1(a)

	// Request the OS to do the initial set of tasks. The last one will be
	// the processor allocation to the process with the highest priority
	OperatingSystem_Initialize(programsFromFilesBaseIndex);
	
	// Tell the processor to begin its instruction cycle 
	Processor_InstructionCycleLoop();
	
}

// Powers off the CS (the C program ends)
void ComputerSystem_PowerOff() {
	// Show message in red colour: "END of the simulation\n" 
	ComputerSystem_DebugMessage(TIMED_MESSAGE,99,SHUTDOWN,"END of the simulation\n"); 
	exit(0);
}

/////////////////////////////////////////////////////////
//  New functions below this line  //////////////////////

// Inicio V1-ej1
void ComputerSystem_PrintProgramList()
{
	ComputerSystem_DebugMessage(TIMED_MESSAGE,101,INIT); // mostramos la linea "[0] User program list:"
	int i = 1; // porque en la posicion 0 esta el SIP
	for(; i < PROGRAMSMAXNUMBER && programList[i] != NULL; i++) // recorremos los programas que hay en programList
	{
		ComputerSystem_DebugMessage(NO_TIMED_MESSAGE,102,INIT,programList[i]->executableName,programList[i]->arrivalTime); // mostramos la linea "<tab>Program [ejemplo1] with arrival time [0]"
	}
}
// Fin V1-ej1
