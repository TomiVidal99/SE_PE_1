#ifndef LIBRERIA_H
#define LIBRERIA_H

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include "main.h"

#define DEFAULT_MODO UNICO
#define DEFAULT_PARAMETRO RESISTENCIA
#define DEFUALT_COMANDO OPCION_1

typedef enum {
	INVALIDO,
	OPCION_1,
	OPCION_2,
}Comando_t;

typedef enum {
	UNICO = 1,
	CONTINUO,
	RAFAGA, // POSIBLE MEJORA
} Modo_t;

typedef enum {
	RESISTENCIA = 1,
	CAPACITANCIA,
	TENSION, // POSIBLE MEJORA
} Parametro_t;

typedef enum {
	MODO,
	PARAMETRO,
} Configurables_t;

typedef struct {
	Parametro_t parametro;
	Modo_t modo;
	Comando_t comando;
} Configuracion_t;

typedef enum {
	MENU_INFO,
	MENU_MODO,
	MENU_PARAM,
	MOSTRAR_MEDICION,
	MEDIR,
} Estado_t;

typedef enum {
	NUEVO_COMANDO,
	TICK_1MS,
	TICK_100MS,
	BOTON_MENU,
} Event_t;

typedef enum {
	ERROR_OK,
	ERROR_NULL,
	ERROR_INVALIDO,
}Error_t;

typedef enum {
	menu_parametro,
	menu_modo,
	menu_info,
	menu_medicion,
}Menu_t;

void UART_mostrar_menu(Menu_t menu, UART_HandleTypeDef *handle_uart);
Comando_t UART_leer_comando(UART_HandleTypeDef *handle_uart);
uint32_t ADC_muestrear(ADC_HandleTypeDef *handle_adc);
void set_configuracion(Configurables_t configurable, Comando_t comando);
void medir_c();
void medir_r(ADC_HandleTypeDef *handle_adc);
Estado_t FSM_general(Estado_t estado, Event_t evento, UART_HandleTypeDef *handle_uart);
void uart_leer_comando_it();

#endif
