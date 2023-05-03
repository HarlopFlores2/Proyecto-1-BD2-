# Proyecto-1-BD2-

## Introducción

### Objetivo del proyecto
El objetivo principal de este proyecto es implementar y comparar dos técnicas de organización de archivos en memoria secundaria para almacenar y gestionar datos de manera eficiente. Las técnicas de organización de archivos seleccionadas son el Sequential File y Extendible Hashing. Para cada una de estas técnicas, se implementarán las principales operaciones de manipulación de datos, incluyendo búsqueda, búsqueda por rango, inserción y eliminación.

### Descripción del dominio de datos a usar
El proyecto utiliza un conjunto de datos llamado "Retail Data Set", que se encuentra disponible en [Kaggle](https://www.kaggle.com/datasets/shedai/retail-data-set).  Este conjunto de datos contiene información detallada sobre las transacciones de venta al por menor, principalmente de agencias de ventas, revendedores y sucursales de una empresa.

Debido a preocupaciones de privacidad, los identificadores de cliente, SKU y documentID han sido procesados con [LabelEncoder](https://scikit-learn.org/stable/modules/generated/sklearn.preprocessing.LabelEncoder.html), lo que significa que cada cliente y producto tiene un ID único en el archivo de datos. Es importante tener en cuenta que los datos abarcan más de tres años, y los precios de los productos pueden haber aumentado con el tiempo.

Los campos en el conjunto de datos incluyen:

- **DocumentID:** ID de la transacción. Una transacción puede contener múltiples registros para el mismo cliente en la misma fecha con múltiples productos (SKU). El DocumentID puede ser útil para combinar transacciones y detectar los artículos vendidos juntos.
- **Date:** Fecha de la transacción/venta. Está en formato de fecha y hora.
- **SKU:** Código del producto. El código único para cada artículo vendido.
- **Price:** Precio de venta de la transacción. El precio del producto para el cliente en la fecha dada.
- **Discount:** Monto del descuento para la transacción.
- **Customer:** ID único del cliente para cada cliente. En el conjunto de datos, el cliente puede ser un revendedor o una sucursal de la empresa.
- **Quantity:** Cantidad de artículos vendidos en la transacción.

El uso de este conjunto de datos permitirá analizar el rendimiento y la eficacia de las técnicas de organización de archivos implementadas en un entorno práctico y realista.

### Resultados que se esperan obtener
Al finalizar este proyecto, se espera obtener una implementación funcional y eficiente de las técnicas de organización de archivos Sequential File y Extendible Hashing. Estas implementaciones permitirán realizar comparaciones de rendimiento y eficiencia entre las dos técnicas, utilizando métricas como el tiempo de búsqueda, inserción y eliminación, así como el uso del espacio en disco.




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





