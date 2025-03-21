#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAMANIOMAXIMO 20

#define NUMERO_DE_ALUMNOS 3

struct persona {
  char nombre[TAMANIOMAXIMO];
  int edad;
  float ingresos;
};

float ingresosMedios(struct persona []);
int alumnoMasJoven(struct persona []);

void main (int argc, char *argv[]) {
  
  struct persona alumnos[NUMERO_DE_ALUMNOS]={{"Juan", 23, 777.11}, {"Luis",19,111.1}, {"Pedro",56,9327.2}};
  
  printf("Los ingresos medios de los alumnnos son %f\n", ingresosMedios(alumnos));
  printf("Posicion del alumno mas joven en el array de alumnos: %d\n", alumnoMasJoven(alumnos));
  exit(1);
}

int alumnoMasJoven(struct persona gente[])
{
  char name[TAMANIOMAXIMO];
  int index = -1;
  int edadMasAlta = 0;

  for(int i = 0; i < NUMERO_DE_ALUMNOS; i++)
  {
    if(gente[i].edad > edadMasAlta)
    {
      edadMasAlta = gente[i].edad;
    }
  }

  int edadMasBaja = edadMasAlta;

  for(int i = 0; i < NUMERO_DE_ALUMNOS; i++)
  {
    if(gente[i].edad < edadMasBaja)
    {
      edadMasBaja = gente[i].edad;
      strcpy(name, gente[i].nombre);
      index++;
    }
  }
  printf("Nombre del alumno mas joven: %s\n", name);
  return index;
}

float ingresosMedios(struct persona gente[]) {
  
  int i;
  float acumulado=0;
    
  for (i=0;i<NUMERO_DE_ALUMNOS;i++)
    acumulado += gente[i].ingresos;
  
  return acumulado/NUMERO_DE_ALUMNOS;
}
  

// 1. Compilar con "gcc 09ArrayDeEstructuras.c"
// 2. Ejecutar con "./a.out"
// 3. Añade una segunda función que muestre el nombre del alumno más joven y devuelva como resultado la posición de dicho alumno en el array de alumnos

