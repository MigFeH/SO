#include "MMU.h"
#include "Buses.h"
#include "Processor.h"
#include "Simulator.h"

// The base register
int registerBase_MMU;

// The limit register
int registerLimit_MMU;

// The MAR register
int registerMAR_MMU;

// The CTRL register
int registerCTRL_MMU;

void MMU_SetCTRL (int ctrl) {
	registerCTRL_MMU=ctrl&0x3;
	switch (registerCTRL_MMU) {
  	case CTRLREAD:
			if (Processor_PSW_BitState(EXECUTION_MODE_BIT)){ // Protected mode
				// Inicio V4-ej1(c)
				//if (registerMAR_MMU < MAINMEMORYSIZE){
				if (registerMAR_MMU < MAINMEMORYSIZE && registerMAR_MMU >= 0){
				// Fin V4-ej1(c)
					// Send to the main memory HW the physical address to write in
					Buses_write_AddressBus_From_To(MMU, MAINMEMORY);
					// Tell the main memory HW to read
					// registerCTRL_MMU is CTRLREAD 
					Buses_write_ControlBus_From_To(MMU,MAINMEMORY);
					// Success
			  		registerCTRL_MMU |= CTRL_SUCCESS;
				}
				else {
					// Fail
					registerCTRL_MMU |= CTRL_FAIL;
					// Inicio V4-ej1(c)
					Processor_RaiseException(INVALIDADDRESS);
					// Fin V4-ej1(c)
				}
			}
			else // Non-Protected mode
				// Inicio V4-ej1(c)
				//if (registerMAR_MMU<registerLimit_MMU) {
				if (registerMAR_MMU<registerLimit_MMU && registerMAR_MMU >= 0) { 
				// Fin V4-ej1(c)
					// Physical address = logical address + base register
					registerMAR_MMU+=registerBase_MMU;
					// Send to the main memory HW the physical address to write in
					Buses_write_AddressBus_From_To(MMU,MAINMEMORY);
					// Tell the main memory HW to read
					// registerCTRL_MMU is CTRLREAD 
					Buses_write_ControlBus_From_To(MMU,MAINMEMORY);
					// Success
			  	registerCTRL_MMU |= CTRL_SUCCESS;
				}
				else {
					// Fail
					registerCTRL_MMU |= CTRL_FAIL;
					// Inicio V4-ej1(c)
					Processor_RaiseException(INVALIDADDRESS);
					// Fin V4-ej1(c)
				}
			break;
  	case CTRLWRITE:
			if (Processor_PSW_BitState(EXECUTION_MODE_BIT)) // Protected mode
				// Inicio V4-ej1(c)
				//if (registerMAR_MMU < MAINMEMORYSIZE) {
				if (registerMAR_MMU < MAINMEMORYSIZE && registerMAR_MMU >= 0) {
				// Fin V4-ej1(c)
					// Send to the main memory HW the physical address to write in
					Buses_write_AddressBus_From_To(MMU, MAINMEMORY);
					// Tell the main memory HW to read
					// registerCTRL_MMU is CTRLWRITE 
					Buses_write_ControlBus_From_To(MMU,MAINMEMORY);
					// Success
			  	registerCTRL_MMU |= CTRL_SUCCESS;
				}
				else {
					// Fail
					registerCTRL_MMU |= CTRL_FAIL;
					// Inicio V4-ej1(c)
					Processor_RaiseException(INVALIDADDRESS);
					// Fin V4-ej1(c)
				}
			else   // Non-Protected mode
				// Inicio V4-ej1(c)
				//if (registerMAR_MMU<registerLimit_MMU) {
				if (registerMAR_MMU<registerLimit_MMU && registerMAR_MMU >= 0) {
				// Fin V4-ej1(c)
					// Physical address = logical address + base register
					registerMAR_MMU+=registerBase_MMU;
					// Send to the main memory HW the physical address to read from
					Buses_write_AddressBus_From_To(MMU, MAINMEMORY);
					// Tell the main memory HW to write
					// registerCTRL_MMU is CTRLWRITE 
					Buses_write_ControlBus_From_To(MMU,MAINMEMORY);
					// Success
			  	registerCTRL_MMU |= CTRL_SUCCESS;
				}
				else {
					// Fail
					registerCTRL_MMU |= CTRL_FAIL;
					// Inicio V4-ej1(c)
					Processor_RaiseException(INVALIDADDRESS);
					// Fin V4-ej1(c)
				}
  			break;
  		default:
				registerCTRL_MMU |= CTRL_FAIL;
				break;
  	}
  	// registerCTRL_MMU return value was CTRL_SUCCESS or CTRL_FAIL
  	Buses_write_ControlBus_From_To(MMU,CPU);
}

// Getter for registerCTRL_MMU
int MMU_GetCTRL () {
  return registerCTRL_MMU;
}

// Setter for registerMAR_MMU
void MMU_SetMAR (int newMAR) {
  registerMAR_MMU = newMAR;
}

// Getter for registerMAR_MMU
int MMU_GetMAR () {
  return registerMAR_MMU;
}

// Setter for registerBase_MMU
void MMU_SetBase (int newBase) {
  registerBase_MMU = newBase;
}

// Getter for registerBase_MMU
int MMU_GetBase () {
  return registerBase_MMU;
}

// Setter for registerLimit_MMU
void MMU_SetLimit (int newLimit) {
  registerLimit_MMU = newLimit;
}

// Getter for registerLimit_MMU
int MMU_GetLimit () {
  return registerLimit_MMU;
}
