/*
 * Multimetro.h
 *
 *  Created on: 7 may 2026
 *      Author: Tomás Vidal e Ignacio Chantiri
 */

#ifndef SRC_MULTIMETRO_H_
#define SRC_MULTIMETRO_H_

/**
 * IMPORTANTE: se espera que el clock esté configurado a 72MHz
 */

/**
 * EJEMPLO DE USO:
 *
 * #include "Multimetro.h"
 *
 * int main(void) {
 * 	HAL_Init();
 *  SystemClock_Config();
 *
 *  Multimetro_activar();
 *
 * }
 *
 */

/**
 * Este es el entry point de la librería.
 * Sólo con ejecutar una vez esta función
 * la librería está activa.
 *
 * IMPORTANTE:
 * Se incian internamente: Timer 1 y el ADC 1.
 *
 * Se usan los sigueintes pines:
 *
 * A0: BOTÓN MENU
 * A1: ADC
 * A5: RESISTOR 330 Ohms
 * A6: RESISTOR 10K Ohms
 * A7: RESISTOR 1M Ohms
 *
 * PB8: SEÑAL DEBUG
 *
 * A9: USART1 TX
 * A10: USART1 RX
 */
void Multimetro_activar(void);

#endif /* SRC_MULTIMETRO_H_ */
