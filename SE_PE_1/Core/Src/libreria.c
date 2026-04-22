		/*
 * libreria.c
 *
 *  Created on: 16 abr 2026
 *      Authors: Tomás Vidal & Ignacio Chantiri
 */

#include "libreria.h"

#define ADC_N_MUESTRAS 32

extern volatile Configuracion_t config;
extern volatile uint32_t r_medida;
extern volatile uint32_t c_medida;
/*
void ADC_calibrar(ADC_HandleTypeDef *handle_adc){

	// Dejo esta funcion comentada por si la hacemos mas adelante
	// pero creo que mide bastante bien asi q por ahora no
	ADC_ChannelConfTypeDef sConfig = {0};
	sConfig.Channel = ADC_CHANNEL_7;
	HAL_ADC_ConfigChannel(handle_adc, &sConfig);
	sConfig.Channel = ADC_CHANNEL_1;

}
*/

void medir_c(){
	/*

	 */
	// Descarga



}

void set_resistencia(uint16_t pin){


	/*
	Toma como input el define del pin, y lo setea en alto
	mientras que setea los otros dos pines en alta Z.
	*/

	  GPIO_InitTypeDef GPIO_InitStruct = {0}; //Esto habria que ver si hace falta llamarlo siempre, capaz
	  	  	  	  	  	  	  	  	  	  	 	 //podemos evitarnos tambien inicializar gpio initstruct cada vez

	  switch (pin){

	  case GPIO330R_Pin:
          //330r como salida en alto
		  GPIO_InitStruct.Pin = GPIO330R_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		  HAL_GPIO_WritePin(GPIOA, GPIO330R_Pin, GPIO_PIN_SET); //High

		  GPIO_InitStruct.Pin = GPIO10K_Pin|GPIO1M_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_INPUT; // Z
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	  case GPIO10K_Pin:
		  // GPIO10K en alto
		  GPIO_InitStruct.Pin = GPIO10K_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		  HAL_GPIO_WritePin(GPIOA, GPIO10K_Pin, GPIO_PIN_SET); //High

		  // Y las demas en Z
		  GPIO_InitStruct.Pin = GPIO330R_Pin|GPIO1M_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_INPUT; // Z
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	  case GPIO1M_Pin:
		  //GPIO1M en alto
		  GPIO_InitStruct.Pin = GPIO1M_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		  HAL_GPIO_WritePin(GPIOA, GPIO1M_Pin, GPIO_PIN_SET); //High

		  // Y las demas en Z
		  GPIO_InitStruct.Pin = GPIO10K_Pin|GPIO330R_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_INPUT; // Z
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);



	  }
}

void medir_r(ADC_HandleTypeDef *handle_adc){
	/*

	Esta funcion primero genera la secuencia de autorango y luego
	almacena la medida del parametro del DUT en la variable global
	r_medida o c_medida segun corresponda
	// MEJORA: hacer que la parte de autorango solo se ejcute solo la primera vez si se esta haciendo disparo continuio
	// Btw al final no la hice con maquina de estados era mucha paja seguir declarando mas enums

	*/

	  //Cambio la configuracion de los pines

	  set_resistencia(GPIO330R_Pin);
	  uint32_t muestra = ADC_muestrear(handle_adc);
	  if (muestra < (0.95*3300)) {
		  r_medida = muestra;
		  return;
	  }

	  set_resistencia(GPIO10K_Pin);
	  muestra = ADC_muestrear(handle_adc);
	  if (muestra < (0.95*3300)) {
		  r_medida = muestra;
		  return;
	  }

	  set_resistencia(GPIO1M_Pin);
	  r_medida = ADC_muestrear(handle_adc);

	  return;
}

uint32_t ADC_muestrear(ADC_HandleTypeDef *handle_adc){

	//Realiza una lectura de ADC_N_MUESTRAS muestras y devuelve el promedio
	uint32_t acc = 0;

	for (uint32_t i=0; i<ADC_N_MUESTRAS; i++) {

		HAL_ADC_Start(handle_adc);
		HAL_ADC_PollForConversion(handle_adc, 1000);
		acc += HAL_ADC_GetValue(handle_adc);

	}

	//Convierto a mV, calculo promedio y retorno
	return (acc * 3300) / (4095 * ADC_N_MUESTRAS);
}

//UART
void UART_mostrar_menu(Menu_t menu, UART_HandleTypeDef *handle_uart){

	/*
	Toma como parametro el tipo Menu_t y lo imprime en pantalla a traves de la uart especificada
	Existen 4 menus distitnos:
		-menu_info: muestra informacion acerca de la configuracion (modo y parametro a medir)
		-menu_modo: muestra el menu de seleccion de modo (unico y continuo)
		-menu_parametro: muestra el menu de seleccion de parametro (resistencia y capacitancia)
		-menu_medicion: muestra el menu con los datos de la ultima medicion
	*/

	char clear[] = "\033[2J\033[H"; // Clear
	HAL_UART_Transmit(handle_uart, (uint8_t*)clear, sizeof(clear)-1, HAL_MAX_DELAY);
	switch (menu){

		case menu_info:
			// Esto dsp lo midifico para que muestre modo y parametro dinamicamente
			char buffer_uart_info[] =
			"\r\n"
			"====================================\r\n"
			"             MENU INFO              \r\n"
			"====================================\r\n"
			"\r\n"
			"Estado actual:\r\n"
			"  • Modo      : blabla\r\n"
			"  • Parametro : blabla\r\n"
			"\r\n"
			"------------------------------------\r\n"
			"Seleccione una opcion:\r\n"
			"\r\n"
			"  [1] Cambiar modo\r\n"
			"  [2] Cambiar parametro\r\n"
			"\r\n"
			"------------------------------------\r\n"
			"> ";
			HAL_UART_Transmit(handle_uart, (uint8_t*)buffer_uart_info, strlen(buffer_uart_info), HAL_MAX_DELAY);

		break;

		case menu_modo:

			char buffer_uart_modo[] =
			"====================================\r\n"
			"           MENU MODO                \r\n"
			"====================================\r\n"
			"Seleccione el modo:\r\n"
			"\r\n"
			"  [1] Disparo unico\r\n"
			"  [2] Disparo continuo\r\n"
			"\r\n"
			"------------------------------------\r\n"
			"> ";
			HAL_UART_Transmit(handle_uart, (uint8_t*)buffer_uart_modo, strlen(buffer_uart_modo), HAL_MAX_DELAY);

		break;

		case menu_parametro:

			char buffer_uart_parametro[] =
			"====================================\r\n"
			"         MENU PARAMETRO             \r\n"
			"====================================\r\n"
			"Seleccione el parametro a medir:\r\n"
			"\r\n"
			"  [1] Resistencia\r\n"
			"  [2] Capacitancia\r\n"
			"\r\n"
			"------------------------------------\r\n"
			"> ";
			HAL_UART_Transmit(handle_uart, (uint8_t*)buffer_uart_parametro, strlen(buffer_uart_parametro), HAL_MAX_DELAY);

		break;

		case menu_medicion:

			//Defino el valor que se va a mostrar (resistencia o capacitancia)

			char buffer_uart_medicion[200];

			const char *parametro_string;
			const char *modo_string;
			const char *unidad;
			int valor;

			if (config.modo == UNICO) {
			    modo_string = "DISPARO UNICO";
			} else {
			    modo_string = "DISPARO CONTINUO";
			}

			if (config.parametro == RESISTENCIA) {
			    parametro_string = "RESISTENCIA";
			    valor = r_medida;
			    unidad = "Ω";

			} else {
			    parametro_string = "CAPACITANCIA";
			    valor = c_medida;
			    unidad = "F";
			}

			// Armo el mensaje
			snprintf(buffer_uart_medicion, sizeof(buffer_uart_medicion),
			    "====================================\r\n"
			    "        MEDICION %s        \r\n"
			    "            %s        \r\n"
			    "====================================\r\n"
			    "\r\n"
			    "Valor: %d %s\r\n"
			    ,
			    parametro_string,
				modo_string,
			    valor,
				unidad
			);

			// enviar por UART
			HAL_UART_Transmit(handle_uart, (uint8_t*)buffer_uart_medicion, strlen(buffer_uart_medicion), HAL_MAX_DELAY);

		break;
	}
}

//Comando_t UART_leer_comando(UART_HandleTypeDef *handle_uart){
//
//	// Es raro que se lea byte a byte, sería mejor tener un buffer de 64 bytes
//	// por ejemplo, y luego procesarlo caracter a caracter.
//	// la lógica de procesamiento puede ser la misma, pero cambiaría el
//	// tamaño del buffer de entrada.
//	//RESPUESTA: pasa que para que quiero leer 64 bytes si el usuario pondria como mucho 1 o 2?
//	// es mas, le puse 4 bytes (3 efectivos contando el \0) para que si se detecta que puso sin
//	// querer algo de mas, (por ej escribe 12 en vez de 1) no lo tome como valido
//	//Otra cosa: leo byte a byte porque me permite contar de a uno cuantos caracteres puso (variable i)y manejar validez con eso
//	/*
//	Esta funcion lee byte a byte por UART. Lo hago asi para poder manejar
//	mas facil validez de inpts del ususario
//
//	Retorna una variable de tipo Comando_t que puede ser OPCION_1 u OPCION_2, o INVALIDO.
//	*/
//
//	char comando[4];//Buffer de 4 bytes
//	char byte; // Buffer de 1 byte
//	uint8_t i = 0;
//
//	while (i < sizeof(comando) - 1)
//	{
//		HAL_UART_Receive(handle_uart, (uint8_t*)&byte, 1, HAL_MAX_DELAY);
//
//		if (byte == '\r' || byte == '\n') //si detecto enter dejo de leer
//			break;
//
//		comando[i++] = byte;
//	}
//
//	comando[i] = '\0';  // cierro string
//
//	//Aviso con evento que hay un nuevo comando
//
//
//	//Retorno comando correspondiente
//	if (i == 1 && (comando[0] == '1')) return OPCION_1;
//
//	else if (i == 1 && (comando[0] == '2')) return OPCION_2;
//
//	else return INVALIDO;
//
//}
void set_configuracion(Configurables_t configurable, Comando_t comando){

	/*
	Modifica la estructura global "config" segun comando y configurable
	Comando: indica si usuario eligio opcion 1 o 2
	Configurable: indica si se quiere configurar modo o parmetro
	*/

	switch (configurable){

		case MODO:

			if (comando == OPCION_1) {
				config.modo = UNICO;
			}
			else if (comando == OPCION_2) {
				config.modo = CONTINUO;
			}

		break;

		case PARAMETRO:

			if (comando == OPCION_1) {
				config.parametro = RESISTENCIA;
			}
			else if (comando == OPCION_2) {
				config.parametro = CAPACITANCIA;
			}
		break;
	}

}

Estado_t FSM_general(Estado_t estado, Event_t evento, UART_HandleTypeDef *handle_uart) {

	switch(estado) {

	case MENU_INFO:

		switch(evento) {
			case NUEVO_COMANDO:

				if (config.comando == OPCION_1){
					UART_mostrar_menu(menu_modo, handle_uart);
					return MENU_MODO;
				}
				else if (config.comando == OPCION_2){
					UART_mostrar_menu(menu_parametro, handle_uart);
					return MENU_PARAM;
				}

			default:
				return estado;
			}

	case MENU_MODO:

		switch(evento) {

			case NUEVO_COMANDO:

				set_configuracion(MODO, config.comando);

				UART_mostrar_menu(menu_info, handle_uart);

				return MENU_INFO;


			case BOTON_MENU:
				// en la maquina de estados falta definir que pasa si tocamos el boton durante un menu
				return estado;

			default:
				return estado;
			}


		case MENU_PARAM:

			switch(evento) {
			case NUEVO_COMANDO:

				set_configuracion(PARAMETRO, config.comando);

				UART_mostrar_menu(menu_info, handle_uart);

				return MENU_INFO;

			case BOTON_MENU:
				// en la maquina de estados falta definir que pasa si tocamos el boton durante un menu
				return estado;
			default:
				return estado;
			}

		case MOSTRAR_MEDICION:

		UART_mostrar_menu(menu_medicion, handle_uart);

			switch(evento) {
				case TICK_100MS:
					if (config.modo == CONTINUO) {
						return MEDIR;
					}

					break;
				default:
					return estado;
			}



		case MEDIR: //para este estado capaz ni tengamos q ponerle switch interno, solo sale de aca con el evento TICK1ms. No lo borre por si queremos agregarle respuesta al boton

			switch(evento) {

				case TICK_1MS:

//					if (config.parametro == RESISTENCIA)
//						//medir_r(hadc1); para usar medir r y medir c hay q pasarle handle_adc, pero no esta pasada a esta funcion, hay q corregir eso antes de descomentar
//					else if (config.parametro == CAPACITANCIA) //el checkeo de condicion es redundante pero se entiende mejor, depsues lo podemos borrar
//						//medir_c(hadc1);

					return MOSTRAR_MEDICION;

				case BOTON_MENU:
					return estado;

				default:
					return estado;

			}


			default:
				return estado;
	}
}
