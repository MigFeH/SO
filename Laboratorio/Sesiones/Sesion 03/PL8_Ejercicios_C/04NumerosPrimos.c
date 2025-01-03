#include <stdio.h>
#include <stdlib.h>

void main (int argc, char *argv[]) {

  int numerosPrimos[]={1,2,3,5,7,11,13,17,19,23,29};
  int posicion;
  int tam = sizeof(numerosPrimos)/sizeof(int);
  int suma = 0;

  if (argc==1) // solo hay la llamada
    printf("La lista contiene los %d primeros números primos\n",tam);
  else {
    posicion=atoi(argv[1]);
    if(posicion >= 0 && posicion < tam)
    {
      printf("El número primo que ocupa la posición %d es %d\n",posicion, numerosPrimos[posicion]);
      // calculo la suma de los elementos a excepcion de esa posicion
      for(int i = 0; i < tam; i++)
      {
        if(i != posicion)
        {
          suma += numerosPrimos[i];
        }
      }
      printf("La sima de los números primos, quitando el de la posicion %d es: %d\n", posicion, suma);
    } else
    {
      printf("La posición indicada (pos=%d) está fuera del rango válido [%d,%d].\n", posicion, 0, tam-1);
    }
  }
  exit(1);
}


// 1. Compilar con "gcc 04NumerosPrimos.c"
// 2. Ejecutar con "./a.out"
// 3. Obtener la ayuda sobre atoi: "man atoi"
// 4. Modificar el programa para que calcule y muestre la suma de todos los números primos de la lista excepto el indicado en línea de comandos