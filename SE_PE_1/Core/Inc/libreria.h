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

uint32_t ADC_muestrear(ADC_HandleTypeDef *handle_adc);
void UART_mostrar_menu(Menu_t menu, UART_HandleTypeDef *handle_uart);
Error_t UART_leer_comando(UART_HandleTypeDef *handle_uart, char *comando);

#endif
