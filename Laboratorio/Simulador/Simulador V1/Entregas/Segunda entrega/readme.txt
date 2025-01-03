He hecho hasta el ejercicio 9 sin tener éste último sin terminar.

-Respuesta al ejercicio 6:

Porque con tamaño 65 estamos superando el tamaño maximo de seccion de memoria definido por la constante MAINMEMORYSECTIONSIZE (en OperatingSystem.c) con un valor de 60.
Luego, el programa prog-V1-E6 no se carga y solo se ejecuta el SIP dando lugar a un bucle infinito.

Cuando cambiamos el valor de 65 a 50 el tamaño que estamos definiendo para el programa entra en el limite de tamaño de seccion de memoria antes mencionado
dando lugar a una correcta carga del programa y que no se produzca el bucle infinito con el SIP.
