/*
 * definitions.h
 *
 *  Created on: 16 abr 2026
 *      Author: Tomás Vidal
 */

#ifndef INC_DEFINITIONS_H_
#define INC_DEFINITIONS_H_

typedef enum {
	UNICO,
	CONTINUO,
	RAFAGA, // POSIBLE MEJORA
} Modo_t;

typedef enum {
	RESISTENCIA,
	CAPACITANCIA,
	TENSION, // POSIBLE MEJORA
} Parametro_t;

typedef struct {
	Parametro_t parametro;
	Modo_t modo;
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

#endif /* INC_DEFINITIONS_H_ */
