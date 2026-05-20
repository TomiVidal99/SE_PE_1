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
Es decir, un sistema que sea capaz de medir valor de resistencia o capacidad de un determinado elemento (DUT, device under test) y enviar dicha información por protocolo UART para finalmente mostrarlo (en nuestro caso) en la pantalla de una PC, todo sin necesidad de ajustar manualmente el alcance de medición. \

El sistema es configurable a dos modos de disparo:\
- Continuo: Realiza una medición cada 100ms y actualiza constantemente la información en pantalla.\
- Único: Realiza solo una medición y la muestra en pantalla.\

Y por otro lado, es también configurable el parámetro a medir:\

- Resistencia.\
- Capacidad.\

]

#slide(title: "El problema", outlined: true)[
  Dichas configuraciones se llevan a cabo desde el menú de inicio que se muestra y se maneja mediante comandos por UART desde la PC.
  Una vez realizada la configuración, el inicio de medición se activa mediante la pulsación del único boton.
  Para alternar en pantalla entre el menú de configuración y el menú que muestra el valor medido, se puede presionar el mismo botón.
]

// A simple slide
#slide[
  - Medir #stress("Capacidad") a partir
  - Medir #stress("Resistencia")
  - Medir #stress("Interfaz")
  // - Diferentes #stress("modos de operación")
  // - Sample link: #link("typst.app").
    // - Link styling using `link-style`: `"color"`, `"underline"`, `"both"`
  // - Font selection using `font: "Fira Sans"`, `size: 21pt`.

  // #framed[This text has been written using `#framed(text)`. The background color of the box is customisable.]

  // #framed(title: "Frame with title")[This text has been written using `#framed(title:"Frame with title")[text]`.]

  // #framed(title: "Medir capacidad")[Constante de tiempo]
  // #framed(title: "Medir resistencia")[Compara con resistencias]
  // #framed(title: "Interfaz de usuario")[Botones y UART]
]

// Focus slide
#focus-slide[
  // This is an auto-resized _focus slide_.
  Solución  \
  Hardware
]

#slide(title: "Conexión", outlined: true)[
  #set align(center)
  #image("esquematico.svg", height: 14cm)
]

#slide(title: "Solución hardware", outlined: true)[
La tarea completa se lleva a cabo con los siguientes materiales:

- Microcontrolador STM32F103C8T6
- 1 botón normalmente abierto
- 3 Resistencias de:
	- 330ohm
	- 10kohm
	- 1Mohm
- Terminal para colocar el DUT
]

#focus-slide[
  // This is an auto-resized _focus slide_.
  Solución  \
  Software
]

#slide(title: "Solución software", outlined: true)[
  #set align(center)
  // #image("DIAGRAMAS/FSM.png", height: 14cm)
]

#slide(title: "Solución software", outlined: true)[
  - Se hizo una #stress("Máquina de estados finitos (FSM)") para resolver el problema:
  - Las señales de la #stress("FSM") son:
    - *TICK100US* (timer de 100us)
    - *TICK1MS* (timer de 1ms)
    - *TICK100MS* (timer de 100MS)
      - Estas 
    - *COMANDO* (Cuando el usuario ingresa text por _UART_)
    - *BTN_MENU* (Cuando el usuario presiona el botón)
]

#slide(title: "Solución software", outlined: true)[
  Para los distintos tipos de medición se necesita conocer la tensión que cae en el DUT. Se conecta para ello el ADC1 en sus terminales (como muestra el esquemático).\
  El método de muestreo elegido es mediante el cálculo de un promedio entre N muestras a la mayor frecuencia permitida: esto significa que apenas el ADC termina una conversión, se inicia la siguiente.\
  El valor default para N es 32 pero es modificable por código (no por comando).\
  De acá en adelante, siempre que se mencione la tensión medida $V_"med"$ , nos referimos a la tensión medida promedio de las N muestras.\
]

#slide(title: "Solución", outlined: true)[
]

#let bib = bibliography("bibliography.bib")
#bibliography-slide(bib)

