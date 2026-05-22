#import "@preview/typslides:1.3.3": *

// Project configuration
#show: typslides.with(
  ratio: "16-9",
  theme: "bluey",
  font: "Fira Sans",
  font-size: 20pt,
  link-style: "color",
  show-progress: true,
)

// The front slide is the first slide of your presentation
#front-slide(
  title: "Problema Especial",
  subtitle: [Sistemas Embebidos],
  authors: "Vidal Tomás, Chantiri Ignacio",
  info: [
    #link("https://github.com/TomiVidal99/SE_PE_1/")
    #set align(center)
    #image("logo.png", height: 5cm)
  ],
)

// Custom outline
// #table-of-contents()

// Title slides create new sections
#title-slide[
  El problema
]

#slide(title: "El problema", outlined: true)[
  El proyecto corresponde a un medidor de componentes R-C con autorango. 
  Es decir, un sistema que sea capaz de medir valor de resistencia o capacidad de un DUT (_device under test_), y enviar dicha información por protocolo UART, para finalmente mostrarlo (en nuestro caso) en la pantalla de una PC, todo sin necesidad de ajustar manualmente el alcance de medición. \

  El sistema es configurable a dos modos de disparo:\
- Continuo: Realiza una medición cada 100ms y actualiza constantemente la información en pantalla.\
- Único: Realiza solo una medición y la muestra en pantalla.\

Y por otro lado, es también configurable el parámetro a medir:\

- Resistencia.\
- Capacidad.\

]

#slide(title: "El problema", outlined: true)[
  Dichas configuraciones se llevan a cabo desde el menú de inicio que se muestra y se maneja mediante comandos por UART desde la PC. \
  Una vez realizada la configuración, el inicio de medición se activa mediante la pulsación del único boton. \
  Para alternar en pantalla entre el menú de configuración y el menú que muestra el valor medido, se puede presionar el mismo botón.
]

// Focus slide
#focus-slide[
  // This is an auto-resized _focus slide_.
  Solución  \
  Hardware
]

#slide(title: "Solución hardware", outlined: true)[
  La tarea completa se lleva a cabo con los siguientes materiales:

- Microcontrolador STM32F103C8T6
- 1 botón normalmente abierto
- 3 Resistencias de:
- $330#math.Omega$
- $10K#math.Omega$
- $1M#math.Omega$
- Terminal para colocar el DUT
]

#slide(title: "Conexión", outlined: true)[
  #set align(center)
  // #image("esquematico.svg", height: 14cm)
  #box(image("esquematico.svg", width: 20cm, height: 24cm), height: 14cm, width: 15cm, clip: true, inset: (
    bottom: -2.8cm,
    left: -3cm,
  ))
]

#slide(title: "Solución hardware", outlined: true)[
  La función de autorango es necesaria para intentar minimizar el error de medición de resistencia.
  Según el circuito propuesto, el valor de resistencia se obtiene a partir del divisor resistivo:

  $R_"DUT" #math.approx  R_"ref"  V_"med"/(V_"cc" - V_"med")$

  Graficando la función anterior con valores normalizados (Rref = 1, Vcc = 1) y dejando Vmed como variable x:

]

#slide(title: "Solución hardware", outlined: true)[
  #set align(center)
  // #image("grafico_vm.png", height: 10cm)
  #box(image("grafico_vm.png", height: 10cm), height: 14cm, clip: true, inset: (
    bottom: 0cm,
    left: 2cm,
  ))
]

#slide(title: "Solución hardware", outlined: true)[

  Donde se observa que por ser una relación no lineal, los valores de Vmed cercanos Vcc (x cercano a 1) provocan grandes cambios en el cálculo de R_DUT (y), por lo que el resultado de la medición será más sensible mientras más cerca esté la tensión medida de la de referencia. *Esto sucede si $R_"med"$ es mucho más grande que $R_"ref"$, para el divisor resistivo en nuestro caso.*

]
#slide(title: "Solución hardware", outlined: true)[
  El objetivo de diseño entonces se basa en esa premisa: *Intentar que el valor de resistencia a medir no sea demasiado grande respecto de la resistencia de referencia,* y se logra modificando $R_"ref"$ entre tres opciones: 330, 10k y 1M, según se requiera.

  La elección de la resistencia de referencia comienza en 330ohm. Si el valor de tensión medido está dentro del 5% del valor Vcc (3.27V), se switchea a la siguiente $R_"ref"$, la de 10k. Se repite la misma lógica para ver si se necesita cambiar a la última resistencia. En caso de que la última resistencia no sea suficiente para asegurar un error bajo de medición, se avisa con un mensaje de fuera de escala.

]

#slide(title: "Solución software", outlined: true)[
  #set text(size: 30pt)
  #stress("Descarga, carga y medida del capacitor")

  #set text(size: 20pt)
  Previo a la medición, se debe realizar un *proceso de descarga* para asegurar el estado incial del capacitor. Esto se logra Conectando el *DUT* directamente a 0V a través de la resisntencia 1M$#math.Omega$. _El dispositivo está listo para ser medido una vez que se obtiene con el ADC una tensión menor al 2% de Vcc (0.065V)._

  Luego se procede a la carga, que se basa en la medición de la constante de tiempo $#math.tau$ de un circuito R-C, donde R es una resistencia $R_"ref"$ conocida fija, de 1M$#math.Omega$.

  Encendiendo el GPIO correspondiente a $R_"ref"$ = 1M$#math.Omega$ y el resto en alta impedancia, se forma entonces el circuito y comienza la carga del *DUT*.

  #set align(center)
  #set text(size: 30pt)
  $C = #math.tau/R_"ref"$

  #set text(size: 20pt)
  #set align(left)

  Para obtener el valor de *$#math.tau$*, se cuentan muestras cada 1ms repetidamente hasta medir con el ADC una tensión igual al 63% del valor final de carga (0.63Vcc = 2.06V) .
  El valor de capacidad en nano faradios será simplemente la cantidad de muestras, puesto que se asume que la $R_"ref"$ es de exactamente 1M$#math.Omega$ y también que el intervalo de muestreo es de precisamente 1ms.

  _Existe un timeout para el caso en que nunca se alcance el valor de carga anterior._  (*fuera de escala*)
]

#focus-slide[
  // This is an auto-resized _focus slide_.
  Solución  \
  Software
]

#slide(title: "Solución software", outlined: true)[
  #set align(center)
  #image("DIAGRAMAS/FSM.png", height: 14cm)

]

#slide(title: "Solución software", outlined: true)[
- Se hizo una #stress("Máquina de estados finitos (FSM)") para resolver el problema:
- Las señales de la #stress("FSM") son:
- *TICK100US*: timer de 100us (generado por *TIM1*)
- *TICK1MS*: timer de 1ms (generado por contador a partir de *TIM1*)
- *TICK100MS*: timer de 100ms (generado por contador a partir de *TIM1*)
- *COMANDO*: Cuando el usuario ingresa un caracter por _UART_ (*UART modo IT*)
- *BTN_MENU*: Cuando el usuario presiona el botón (*BOTÓN CON EXTI*)
]

#slide(title: "Solución software", outlined: true)[
  Funcionamiento general de la #stress("FSM"):
- Los estados: *R330_FSM*, *R10K_FSM*, *R1M_FSM* y *R_MOSTRAR_FSM*, se encargan de resolver la parte de medición de #stress("resistencia").
- Los estados: *C_DESCARGAR_FSM*, *C_CARGA_FSM* y *C_MOSTRAR_FSM*, se encargan de la parte de #stress("capacidad").
- Los estados: *MENU_INFO_FSM*, *MENU_MODO_FSM* y *MENU_PARAM_FSM*, se encargan de la parte del #stress("menu UART").
]

#slide(title: "Solución software", outlined: true)[
  #grid(
    columns: (2fr, 1fr),
    gutter: 1cm,
    [
      #set align(center)
      #set text(size: 30pt)
      *Funcionamiento general* \
      #set align(left)
      #set text(size: 20pt)
      Para los distintos tipos de medición se necesita conocer la tensión que cae en el DUT. Para ello, se conecta el ADC1 en sus terminales (como muestra el esquemático).\
      El método de muestreo elegido es mediante el cálculo de un promedio entre N muestras a la mayor frecuencia permitida: esto significa que apenas el ADC termina una conversión, se inicia la siguiente.\
      El valor default para N es 32 pero es modificable por código (no por comando).\
      De acá en adelante, siempre que se mencione la tensión medida $V_"med"$ , nos referimos a la tensión medida promedio de las N muestras.\
    ],
    [
      #box(image("esquematico.svg", width: 20cm, height: 29cm), height: 10cm, width: 10cm, clip: true, inset: (bottom: 4cm, left: -3cm))
    ]
  )
]

#slide(title: "Solución software", outlined: true)[
  #set text(size: 30pt)
  #stress("Libraría de usuario") \
  #set text(size: 20pt)

  _Multimetro.c_ contiene 2 funciones principales:
- *Multimetro_activar()* \
- *Multimetro_procesar()* \

]

#slide(title: "Solución software", outlined: true)[
  #image("funciones_lib.png")
]

#slide(title: "Solución software", outlined: true)[
  #stress("Multimetro_activar()")
- HAL_TIM_Base_Start_IT(): Comienza a correr *TIM1*.
- HAL_UART_Receive_IT(): Activa interrupciones del *UART*.
- UART_Mostrar_Menu(MENU_INFO): Muestra el menu inicial.
]

#slide(title: "Solución software", outlined: true)[
  #stress("Multimetro_procesar()")
- Actúa como el _"background"_ de nuestra librería.
- Allí se capturan los flags de entrada y se llama a la #stress("FSM") con el evento correspondiente:
- *flag_nuevo_comando*: Cada vez que llega una interrupción por *UART*.
- *btn_menu*: Cada vez que se presiona el *botón*.
- *TICKs*: Cada vez que se el *TIM1* dispara una interrupción cada _100us_, se procesan los contadores.
]

#slide(title: "Solución software", outlined: true)[

  #set text(size: 30pt)
  #stress("Menúes de configuración")
  #set text(size: 20pt)

  Partiendo desde el menú info, que indica la configuración actual (modo y parámetro), *es posible moverse entre los distintos submenús de configuración solamente con comandos por UART que se indican en pantalla.*
  Luego de la configuración de modo o de parámetro, siempre se vuelve al menú inicial (menú info). 

]

#slide(title: "Solución software", outlined: true)[

  #set text(size: 30pt)
  #stress("Proceso de medición y menú medición")
  #set text(size: 20pt)

  Cuando se llega al estado medir por medio de el botón, se procesa una primer medición. En caso de que el modo sea único, se procede a mostrar el menú medición con el valor obtenido.
  En caso de que el modo sea continuo, se comienza un bucle infinito medir-mostrar cada 100ms, hasta que se presione el botón una vez más para salir y volver al menú info.

]

#slide(title: "Solución software", outlined: true)[

  #set text(size: 30pt)
  #stress("Posibles mejoras")
  #set text(size: 20pt)

  - Accessos rápidos con teclas especiales: 
    - 'r': Modo *resistencia*.
    - 'c': Modo *capacidad*.
    - 'u': Modo *único*.
    - 'p': Modo *periódico*.

  - Inicializar periféricos (baremetal)
  - Medir tensión
  - Agregar un modo de auto escala para capacidad

]

#slide(title: "Solución software", outlined: true)[

  #set align(center)
  #set text(size: 30pt)
  #stress("FIN")
  #set text(size: 20pt)

]


#let bib = bibliography("bibliography.bib")
// #bibliography-slide(bib)

