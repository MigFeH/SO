// V1
#include "Clock.h"
#include "Processor.h"
// Inicio V2-ej1(d)
#include "ComputerSystemBase.h"
// Fin V2-ej1(d)

int tics=0;

void Clock_Update() {
	tics++;
	// Inicio V2-ej1(d)
	if(tics % intervalBetweenInterrupts == 0)
	{
		Processor_RaiseInterrupt(CLOCKINT_BIT);
	}
	// Fin V2-ej1(d)
}


int Clock_GetTime() {

	return tics;
}
