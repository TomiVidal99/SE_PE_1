#ifndef LIBRERIA_H
#define LIBRERIA_H

#include <stdint.h>
#include <string.h>
#include "definitions.h"
#include "main.h"

typedef enum {
	menu_parametro,
	menu_modo,
	menu_info,

}Menu_t;

typedef enum {
	INVALIDO,
	OPCION_1,
	OPCION_2,

}Comando_t;

void UART_mostrar_menu(Menu_t menu, UART_HandleTypeDef *handle_uart);
Comando_t UART_leer_comando(UART_HandleTypeDef *handle_uart);
uint32_t ADC_muestrear(ADC_HandleTypeDef *handle_adc);
void set_configuracion(Configurables_t configurable, Comando_t comando);
void medir_c();
void medir_r();

#endif
