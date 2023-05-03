# Proyecto-1-BD2-

## Sequential File

Esta técnica de organización de archivos consiste en almacenar
registros de manera ordenada en base a una llave específica en 
un archivo principal. Asimismo, los nuevos registros se insertarán 
en un archivo auxiliar manteniendo el orden de la llave mediante
punteros para finalmente unirlos.

### Load

Se lee el archivo csv con la libreria rapidcsv, donde cada linea representa
un registro de longitud fija y este registro es encapsulado por la estructura fixedRecord 
que tiene por atributos extras a nextFile y nextPositon que representan los 
punteros, para luego escribirlos en el archivo dataFile, que tiene un header
que apunta al registro con el menor valor de la llave.

### Insert

Esta función permite agregar registros manteniendo el orden de acuerdo a 
la llave, se tienen dos casos. El primero es cuando el archivo auxFile no ha 
llegado a su límite, se inserta al final de este y se actualizan los 
punteros manteniendo el orden. El segundo es cuando ya no hay espacio en auxFile y se necesita 
realizar una unión de archivos, con los punteros actualizados el archivo
resultante también estará ordenado, luego el último registro se inserta al 
final del auxFile como en el primer caso.

`Complejidad: O(log n + k)` 

### Search

Para esta función se ha implementado tanto la búsqueda única y la búsqueda 
por rango de los registros. Primero se verifica que exista la llave, luego
se procede a hacer la búsqueda binaria en el archivo dataFile y la búsqueda
secuencial en el archivo auxFile. Esto es mas sencillo porque utilizamos registros
con longitud fija.

`Complejidad: O(log n + k)`

### Remove

## Extendible Hash

Esta técnica de organización de archivos es un tipo de sistema de hash que trata 
un hash como una cadena de bits y utiliza un trie para la búsqueda de cubos. 
Debido a la naturaleza jerárquica del sistema, la re-hashing es una operación 
incremental (realizada un cubo a la vez, según sea necesario), lo que significa 
que las aplicaciones sensibles al tiempo se ven menos afectadas por el crecimiento 
de la tabla que por los rehashes completos estándar.


### Load

Se lee el archivo csv con la librería rapidcsv y luego se llama a
función insert para que cada registro se ubique dentro de su bucket
correspondiente en HashFile, tener en cuenta que cada bucket esta escrito
en la posición lógica de HashFile según su id. Asimismo, se crea
otro archivo llamado indexFile para saber más rápido cual es el tamaño
del sufijo a ser tomado para saber la id del bucket. 

### Insert

En esta función primero se raliza la función hash que en nuestro caso es módulo 
y se verifica en el indexFile de acuerdo a su valor cual es el tamaño del sufijo
a tomar para la id del Bucket, luego se encuentra el Bucket en el archivo
hashFile, a partir de esto tenemos 2 casos. El primero es cuando hay espacio en el
Bucket por lo que insertamos el registro en la posicion siguiente al ultimo registro.
El segundo caso sucede cuando el Bucket esta lleno donde tenemos que dividir nuestros 
registros y reorganizarlos de acuerdo a los nuevos hash de cadena de bits que se 
les agrega 1 o 0 al inicio de la cadena. Si ya no es posible hacer esta division 
porque llegamos al limite de nuestro globalDepth entonces procedemos a hacer 
encadenamiento de Buckets, cada Bucket agregado en el encadenamiento ira al final 
de todo el archivo HashFile.

`Complejidad: O(4 + 2 * (# de encadenamientos) + globalSize*2)`

### Search

### Remove





