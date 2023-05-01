# Proyecto-1-BD2-

## Sequential File
Esta técnica de organización de archivos consiste en almacenar
registros de manera ordenada en base a una llave especifica en 
un archivo principal. Asimismo, los nuevos registros se insertarán 
en un archivo auxiliar manteniendo el orden de la llave mediante
punteros para finalmente unirlos.

### Load

Se lee el archivo csv, donde cada linea representa un registro y este 
registro es encapsulado por la estructura fixedRecord que tiene por 
atributos extras a nextFile y nextPositon que representan los 
punteros, para luego escribirlos en el archivo dataFile.

### Insert

Esta funcion permite agregar registros manteniendo el orden de acuerdo a 
la llave, se tienen dos casos. El primero es cuando el archivo auxFile no ha 
llegado a su limite y se inserta al final de este y se actualizan los 
punteros. El segundo es cuando ya no hay espacio en auxFile y se necesita 
realizar una union de archivos, con los punteros actualizados el archivo
resultante tambien estará ordenado, luego el último registro se inserta al 
final del auxFile como en el primer caso.


### Search

### Remove

## Extendible Hash





