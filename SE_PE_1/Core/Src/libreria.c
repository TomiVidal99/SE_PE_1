		/*
 * libreria.c
 *
 *  Created on: 16 abr 2026
 *      Authors: Tomás Vidal & Ignacio Chantiri
 */

#include "libreria.h"

#define ADC_N_MUESTRAS 32

extern volatile Configuracion_t config;
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
	}
}

Comando_t UART_leer_comando(UART_HandleTypeDef *handle_uart){

	// Es raro que se lea byte a byte, sería mejor tener un buffer de 64 bytes
	// por ejemplo, y luego procesarlo caracter a caracter.
	// la lógica de procesamiento puede ser la misma, pero cambiaría el
	// tamaño del buffer de entrada.

	/*
	Esta funcion lee byte a byte por UART. Lo hago asi para poder manejar
	mas facil errores de inpts del ususario

	El comando resultante queda modificado (se paso por referencia)
	*/

	char comando[4];//Buffer de 4 bytes
	char com; // Buffer de 1 byte
	uint8_t i = 0;

	while (i < sizeof(comando) - 1)
	{
		HAL_UART_Receive(handle_uart, (uint8_t*)&com, 1, HAL_MAX_DELAY);

		if (com == '\r' || com == '\n') //si detecto enter dejo de leer
			break;

		comando[i++] = com;
	}

	comando[i] = '\0';  // cierro string

	//Retorno comando correspondiente
	if (i == 1 && (comando[0] == '1')) return OPCION_1;

	else if (i == 1 && (comando[0] == '2')) return OPCION_2;

	else return ANY;

}

// creo que esto está mal, set_configuracion no debería llamar a comando, sino
// recibirlo por parámetro, hay que acotar la funcionalidad
void set_configuracion(UART_HandleTypeDef *handle_uart, Configurables_t configurable){

	/*
	Modifica la estructura "config", seteando el modo y el parametro leyendo comando por uart.
	Configurable: indica si se quiere configurar modo o parmetro
	*/

	Comando_t comando = UART_leer_comando(handle_uart);
//	uint32_t comando = buffer_comando[0] - '0'; //Dejo el comando como uint

	switch (configurable){

		case MODO:

			if (comando == OPCION_1) {
				config->modo = UNICO;
			}
			else if (comando == OPCION_2) {
				config->modo = CONTINUO;
			}

		break;

		case PARAMETRO:

			if (comando == OPCION_1) {
				config->parametro = RESISTENCIA;
			}
			else if (comando == OPCION_2) {
				config->parametro = CAPACITANCIA;
			}
		break;
	}

}

void FSM_general(Estado_t estado, Event_t evento, UART_HandleTypeDef *handle_uart) {

	switch(estado) {

	case MENU_INFO:

		UART_mostrar_menu(menu_info, handle_uart);

		switch(evento) {
			case NUEVO_COMANDO:
				// y acá no se perdería el modo al que va?
				// tendríamos hacer que vaya a MENU_MODO
				// pero sabiendo si es resistencia o capacidad no?

				// yo haría que la configuración se actualice acá,
				// set_configuracion(handle_uart, MODO, &config);
				estado = MENU_MODO;
				break;

			case BOTON_MENU:
				break;

			default:
				break;
			}

	case MENU_MODO:

		UART_mostrar_menu(menu_modo, handle_uart);

		switch(evento) {
			case NUEVO_COMANDO:
				set_configuracion(handle_uart, MODO, &config);
				break;
			case BOTON_MENU:
				break;
			default:
				break;
			}
		break;

		case MENU_PARAM:
			switch(evento) {
			case NUEVO_COMANDO:
				break;
			case TICK_1MS:
				break;
			case TICK_100MS:
				break;
			case BOTON_MENU:
				break;
			}
			break;
		case MOSTRAR_MEDICION:
			switch(evento) {
			case NUEVO_COMANDO:
				break;
			case TICK_1MS:
				break;
			case TICK_100MS:
				break;
			case BOTON_MENU:
				break;
			}
		break;
		case MEDIR:
			switch(evento) {
			case NUEVO_COMANDO:
				break;
			case TICK_1MS:
				break;
			case TICK_100MS:
				break;
			case BOTON_MENU:
				break;
			}
		break;

			default:
			break;
	}
}
