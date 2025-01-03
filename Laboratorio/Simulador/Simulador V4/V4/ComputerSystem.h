#ifndef COMPUTERSYSTEM_H
#define COMPUTERSYSTEM_H

#include "Simulator.h"
#include "ComputerSystemBase.h"
// Inicio V3-ej1(b)
#define ARRIVALQUEUE
// Fin V3-ej1(b)

// Functions prototypes
void ComputerSystem_PowerOn(int argc, char *argv[], int);
void ComputerSystem_PowerOff();

// This "extern" declarations enables other source code files to gain access
// to the variables "programList", etc.
extern PROGRAMS_DATA *programList[];
extern char STUDENT_MESSAGES_FILE[];

#endif
