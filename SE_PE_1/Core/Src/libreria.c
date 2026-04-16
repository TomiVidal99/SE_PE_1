/*
 * libreria.c
 *
 *  Created on: 16 abr 2026
 *      Authors: Tomás Vidal & Ignacio Chantiri
 */


#include "definitions.h"


Estado_t FSM_general(Estado_t estado, Event_t evento) {
	switch(estado) {
	case MENU_INFO:
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
		case MENU_MODO:
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
	}
}
