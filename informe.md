un informe
en forma de presentación (power point o pdf) donde debe figurar:

​ya está:
-Una introducción explicando el problema planteado y cualquier detalle técnico que haya sido necesario investigar para implementar la solución.
-Una explicación de lo implementado.
-El diagrama de la máquina de estados
falta:
-La explicación de la implementación en código de la máquina de estados
-La explicación de la librería desarrollada para manejar el periférico.

# 0) Descripción del proyecto

El proyecto corresponde a un medidor de componentes R-C con autorango. 
Es decir, un sistema que sea capaz de medir valor de resistencia o capacidad de un determinado elemento (DUT, device under test) y enviar dicha información por protocolo UART para finalmente mostrarlo (en nuestro caso) en la pantalla de una PC, todo sin necesidad de ajustar manualmente el alcance de medición.

El sistema es configurable a dos modos de disparo:
-Continuo: Realiza una medición cada 100ms y actualiza constantemente la información en pantalla.
-Único: Realiza solo una medición y la muestra en pantalla.

Y por otro lado, es también configurable el parámetro a medir:

-Resistencia.
-Capacidad.

Dichas configuraciones se llevan a cabo desde el menú de inicio que se muestra y se maneja mediante comandos por UART desde la PC.
Una vez realizada la configuración, el inicio de medición se activa mediante la pulsación del único boton. 
Para alternar en pantalla entre el menú de configuración y el menú que muestra el valor medido, se puede presionar el mismo botón.

# 1) Detalles técnicos de la implementación

La tarea completa se lleva a cabo con los siguientes materiales:

-Microcontrolador STM32f103c8t6
-1 botón normalmente abierto
-3 Resistencias de:
	-330ohm
	-10kohm
	-1Mohm
-Terminal para colocar el DUT
### 1.1) Esquemático

![[esquematico.svg]]


### 1.2) Muestreo

Para los distintos tipos de medición se necesita conocer la tensión que cae en el DUT. Se conecta para ello el ADC1 en sus terminales (como muestra el esquemático).
El método de muestreo elegido es mediante el cálculo de un promedio entre N muestras a la mayor frecuencia permitida: esto significa que apenas el ADC termina una conversión, se inicia la siguiente.
El valor default para N es 32 pero es modificable por código (no por comando).
De acá en adelante, siempre que se mencione la tensión medida $V_{med}$ , nos referimos a la tensión medida promedio de las N muestras.

### 1.3) Comunicación por UART

La configuración se realiza con comandos por UART1. Rellenar esto

### 1.3) Medición de resistencia con autorango

La función de autorango es necesaria para intentar minimizar el error de medición de resistencia.
Como se vió anteriormente, el valor de resistencia se obtiene a partir del divisor resistivo, resultando:

$$
R_{DUT} = R_{ref} \cdot \frac{V_{med}}{V_{cc} - V_{med}}
$$

Graficando la función anterior con valores normalizados (Rref = 1, Vcc = 1) y dejando Vmed como variable x:

![[Pasted image 20260506134426.png]]

Donde se observa que por ser una relación no lineal, los valores de Vmed cercanos Vcc (x cercano a 1) provocan grandes cambios en el cálculo de R_DUT (y), por lo que el resultado de la medición será más sensible mientras más cerca esté la tensión medida de la de referencia. **Esto sucede si Rmed es mucho más grande que Rref, para el divisor resistivo en nuestro caso.**

El objetivo de diseño entonces se basa en esa premisa: **Intentar que el valor de resistencia a medir no sea demasiado grande respecto de la resistencia de referencia,** y se logra modificando Rref entre tres opciones: 330, 10k y 1M, según se requiera.

La elección de la resistencia de referencia comienza en 330ohm. Si el valor de tensión medido está dentro del 5% del valor Vcc (3.27V), se switchea a la siguiente Rref, la de 10k. Se repite la misma lógica para ver si se necesita cambiar a la última resistencia. En caso de que la última resistencia no sea suficiente para asegurar un error bajo de medición, se avisa con un mensaje de fuera de escala.

### 1.4) Descarga, carga y medida del capacitor

Previo a la medición, se debe realizar un **proceso de descarga** para asegurar el estado incial del capacitor. Esto se logra Conectando el DUT directamente a 0V a través de la resisntencia 1Mohm. **El dispositivo está listo para ser medido una vez que se obtiene con el ADC una tensión menor al 2% de Vcc (0.065V).**

Luego se procede a la carga, que se basa en la medición de la constante de tiempo $\tau$ de un circuito R-C, donde R es una resistencia R_ref conocida fija, de 1Mohm.

 Encendiendo el GPIO correspondiente a R_ref = 1M y el resto en alta impedancia, se forma entonces el circuito y comienza la carga del DUT.


$$
C = \frac{\tau}{R_{ref}}
$$
Para obtener el valor de tau, se cuentan muestras cada 1ms repetidamente hasta medir con el ADC una tensión igual al 63% del valor final de carga (0.63Vcc = 2.06V) .
El valor de capacidad en nano faradios será simplemente la cantidad de muestras, puesto que se asume que la R_ref es de exactamente 1M y también que el intervalo de muestreo es de precisamente 1ms.

Existe un timeout para el caso en que nunca se alcance el valor de carga anterior.

# 2) Maquina de estados

![[FSM.png|1290]]

El diagrama anterior corresponde a la única máquina de estados implementada en el sistema.

Los parámetros iniciales (que no se muestran en la imagen) son:

-config.modo = UNICO
-config.parametro = RESISTENCIA
-comando = OPCION_1

(Los tres son valores que se definen según DEFAULT_MODO, DEFAULT_PARAMETRO y DEFAULT_COMANDO)

### 2.1) Idea general

El estado de inicio es MENU_INFO. La máquina está "dividida" (no estrictamente) en dos sectores: a la derecha, el display de los menús de configuración, a la izquierda, el proceso de medición y su muestra.
**La idea general es moverse de un lado a otro mediante la pulsación del botón, permitiendo ir y venir entre el menú de configuración y el menú/proceso de medición.**

### 2.2) Menúes de configuración

Partiendo desde el menú info, que indica la configuración actual (modo y parámetro), **es posible moverse entre los distintos submenús de configuración solamente con comandos por UART que se indican en pantalla.**
Luego de la configuración de modo o de parámetro, siempre se vuelve al menú inicial (menú info). 

### 2.3) Proceso de medición y menú medición
Cuando se llega al estado medir por medio de el botón, se procesa una primer medición. En caso de que el modo sea único, se procede a mostrar el menú medición con el valor obtenido.
En caso de que el modo sea continuo, se comienza un bucle infinito medir-mostrar cada 100ms, hasta que se presione el botón una vez más para salir y volver al menú info.


# 3) Código

El código se desarrolla en C y se hace uso principalmente de las siguientes librerías:

-
-
-
-

La HAL del microcontrolador es también utilizada principalmente para el manejo de los periféricos (UART, ADC), al SysTick, a los GPIO, y para sus configuraciones por MX.

### 3.1) Implementación de la FSM en código
### 3.2) Librería desarrollada para el proyecto