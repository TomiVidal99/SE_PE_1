/*
 * Multimetro.c
 *
 *  Created on: 7 may 2026
 *      Author: Tomás Vidal e Ignacio Chantiri
 */

/**
 * TODO: cambiar los pines y puertos a algo más modificable por el usuario
 */

#include "main.h"
#include "stdio.h"
#include "string.h"

//Macros
#define CALCULAR_CAPACIDAD(cuentas) ((cuentas * 1000) / VALOR_RESISTOR_1M_KOHMS)

#define DEFAULT_MODO UNICO
#define DEFAULT_PARAMETRO RESISTENCIA
#define DEFUALT_COMANDO OPCION_1

#define VCC_MV (3270)
#define VALOR_RESISTOR_330_OHMS (331)
#define VALOR_RESISTOR_10K_OHMS (9920)
#define VALOR_RESISTOR_1M_KOHMS (999) // TODO: como manejamos esta proporcion?
#define VALOR_RESISTOR_1M_OHMS (VALOR_RESISTOR_1M_KOHMS * 1000) // TODO: como manejamos esta proporcion?

#define VCC_AL_95_PORCIENTO (0.95*VCC_MV)
#define VCC_AL_63_PORCIENTO (0.63*VCC_MV)
#define VCC_AL_2_PORCIENTO (0.02*VCC_MV)

#define DEBOUNCER_BTN_TIMEOUT_MS (200)

#define MAX_CUENTAS_DESCARGA (10 * 1000 * 1000) // 10 segundos?
#define MAX_CUENTAS_CARGA (10 * 1000) // 10 segundos?

#define FALLO_MAX_TIEMPO_CARGA (-1)
#define FALLO_MAX_TIEMPO_DESCARGA (-2)
#define FALLO_R_FUERA_DE_ESCALA (-3)

typedef enum {
	SIN_PRESIONAR,
	PRESIONADO
} EstadoBoton;

typedef enum {
	MENU_PARAMETRO,
	MENU_MODO,
	MENU_INFO,
	MENU_R,
	MENU_C,
} Menu_t;

EstadoBoton btn_menu = SIN_PRESIONAR;
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

typedef enum {
	OHMS,
	KILO_OHMS,
	MEGA_OHMS,
	MICRO_FARADIOS,
	NANO_FARADIOS,
	PICO_FARADIOS
} Unidad_t;

typedef struct {
	Parametro_t parametro;
	Modo_t modo;
	Comando_t comando;
	Unidad_t unidad;
} Configuracion_t;

typedef enum {
	FSM_MENU_INFO,
	FSM_MENU_MODO,
	FSM_MENU_PARAM,
	FSM_C_CARGA,
	FSM_C_DESCARGA,
	FSM_MOSTRAR_C,
	FSM_MOSTRAR_R,
	FSM_R330,
	FSM_R10K,
	FSM_R1M,
} FSM_State;

typedef enum {
	TICK_100US,
	TICK_1MS,
	TICK_100MS,
	BTN_MENU,
	UART_COMMAND,
} FSM_Signals;

typedef enum {
	RESISTOR_330,
	RESISTOR_10K,
	RESISTOR_1M,
} OutputResistor_Type;

FSM_State FSM_General(FSM_State state, FSM_Signals evento);

// Imprime por UART el menu pasado como parametro
void UART_mostrar_menu(Menu_t menu);

//
//Comando_t UART_leer_comando(UART_HandleTypeDef *handle_uart);
uint32_t ADC_muestrear(uint32_t cantidad_muestras);
//void set_configuracion(Configurables_t configurable, Comando_t comando);
//void medir_c(ADC_HandleTypeDef *handle_adc);
//void medir_r(ADC_HandleTypeDef *handle_adc);
//void uart_leer_comando_it();
int procesar_comando(void);
void set_resistencia(OutputResistor_Type resistorType);

void configurar_carga();
void configurar_descarga();
void configurar_resistencia330();

const char* ObtenerUnidadEnString();

extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim1;
extern UART_HandleTypeDef huart1;

volatile uint32_t ultima_muestra=0;

volatile Configuracion_t config = {
	.parametro = DEFAULT_PARAMETRO,
	.modo = DEFAULT_MODO,
	.comando = OPCION_1,
	.unidad = OHMS,
};

char comando_buffer;

volatile FSM_State estado_actual = FSM_MENU_INFO;
volatile uint32_t flag_nuevo_comando = 0;
volatile int32_t r_medida = 0;
volatile int32_t c_medida = 0;
volatile uint32_t btn_debouncer = 0;
volatile uint32_t primer_medicion = 0;

volatile uint32_t contador_timer_1ms = 0;
volatile uint32_t contador_timer_100ms = 0;
volatile int32_t contador_capacitor = 0;

volatile uint32_t procesar_activo = 0;

// TODO: la capacidad tiene que borrar la pantalla o mostrar
// las muestras una tras otra??
// TODO: manejar los ERRORES de MAX conteo del cap
void UART_mostrar_menu(Menu_t menu){
	/*
	Toma como parametro el tipo Menu_t y lo imprime en pantalla a traves de la uart especificada
	Existen 4 menus distitnos:
		-menu_info: muestra informacion acerca de la configuracion (modo y parametro a medir)
		-menu_modo: muestra el menu de seleccion de modo (unico y continuo)
		-menu_parametro: muestra el menu de seleccion de parametro (resistencia y capacitancia)
	*/

	switch (menu){

		static char clear[] = "\033[2J\033[H"; // Clear

		case MENU_INFO:

			HAL_UART_Transmit(&huart1, (uint8_t*)clear, sizeof(clear)-1, HAL_MAX_DELAY);

			char buffer_uart_info[360];

			snprintf(buffer_uart_info, sizeof(buffer_uart_info),
					"\r\n"
					"====================================\r\n"
					"             MENU INFO              \r\n"
					"====================================\r\n"
					"\r\n"
					"Estado actual:\r\n"
					"  • Modo      : %s\r\n"
					"  • Parametro : %s\r\n"
					"\r\n"
					"------------------------------------\r\n"
					"Seleccione una opcion:\r\n"
					"\r\n"
					"  [1] Cambiar modo\r\n"
					"  [2] Cambiar parametro\r\n"
					"\r\n"
					"------------------------------------\r\n"
					"> ",
					config.modo == CONTINUO ? "CONTINUO" : "UNICO",
					config.parametro == RESISTENCIA ? "RESISTENCIA" : "CAPACITANCIA"
			);

			// enviar por UART
			HAL_UART_Transmit(&huart1, (uint8_t*)buffer_uart_info, strlen(buffer_uart_info), HAL_MAX_DELAY);

		break;

		case MENU_MODO:

			HAL_UART_Transmit(&huart1, (uint8_t*)clear, sizeof(clear)-1, HAL_MAX_DELAY);

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
			HAL_UART_Transmit(&huart1, (uint8_t*)buffer_uart_modo, strlen(buffer_uart_modo), HAL_MAX_DELAY);

		break;

		case MENU_PARAMETRO:

			HAL_UART_Transmit(&huart1, (uint8_t*)clear, sizeof(clear)-1, HAL_MAX_DELAY);

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
			HAL_UART_Transmit(&huart1, (uint8_t*)buffer_uart_parametro, strlen(buffer_uart_parametro), HAL_MAX_DELAY);

		break;

		case MENU_R:

			char buffer_uart_medicion_resistencia[200];

			if(  ((config.modo == CONTINUO) && (primer_medicion)) || (config.modo == UNICO)  ){

				HAL_UART_Transmit(&huart1, (uint8_t*)clear, sizeof(clear)-1, HAL_MAX_DELAY);

				primer_medicion = 0;
				snprintf(buffer_uart_medicion_resistencia, sizeof(buffer_uart_medicion_resistencia),
					"====================================\r\n"
					"        MEDICION RESISTENCIA        \r\n"
					"           %s        \r\n"
					"====================================\r\n"
					"\r\n"
					,
					config.modo == UNICO ? "DISPARO UNICO" : "DISPARO CONTINUO"
				);

			HAL_UART_Transmit(&huart1, (uint8_t*)buffer_uart_medicion_resistencia, strlen(buffer_uart_medicion_resistencia), HAL_MAX_DELAY);

			}

			// Verifico valor fuera de escala
			if (r_medida == FALLO_R_FUERA_DE_ESCALA){
				snprintf(buffer_uart_medicion_resistencia, sizeof(buffer_uart_medicion_resistencia),
							"Valor: FUERA DE ESCALA\r\n"
				);
			}else{
				snprintf(buffer_uart_medicion_resistencia,
						sizeof(buffer_uart_medicion_resistencia),
						"Valor: %ld %s\r\n",
						r_medida,
						ObtenerUnidadEnString()
				);
			}

			// Envio por uart

			HAL_UART_Transmit(&huart1, (uint8_t*)buffer_uart_medicion_resistencia, strlen(buffer_uart_medicion_resistencia), HAL_MAX_DELAY);

			break;

		case MENU_C:

			char buffer_uart_medicion_capacidad[200];

			if(  ((config.modo == CONTINUO) && (primer_medicion)) || (config.modo == UNICO)  ){

				HAL_UART_Transmit(&huart1, (uint8_t*)clear, sizeof(clear)-1, HAL_MAX_DELAY);

				primer_medicion = 0;
				snprintf(buffer_uart_medicion_capacidad, sizeof(buffer_uart_medicion_capacidad),
					"====================================\r\n"
					"        MEDICION CAPACITANCIA        \r\n"
					"            %s        \r\n"
					"====================================\r\n"
					"\r\n"
					,
					config.modo == UNICO ? "DISPARO UNICO" : "DISPARO CONTINUO"
				);

				HAL_UART_Transmit(&huart1, (uint8_t*)buffer_uart_medicion_capacidad, strlen(buffer_uart_medicion_capacidad), HAL_MAX_DELAY);

			}

			// Verifico si hubo medicion fuera de escala
			if ((c_medida == FALLO_MAX_TIEMPO_CARGA) || (c_medida == FALLO_MAX_TIEMPO_DESCARGA)){
				snprintf(buffer_uart_medicion_capacidad, sizeof(buffer_uart_medicion_capacidad),
							"Valor: FUERA DE ESCALA\r\n"
					);
			}else{
				snprintf(buffer_uart_medicion_capacidad,
						sizeof(buffer_uart_medicion_capacidad),
						"Valor: %ld %s\r\n",
						c_medida,
						ObtenerUnidadEnString()
				);
			}

			// enviar por UART/
			HAL_UART_Transmit(&huart1, (uint8_t*)buffer_uart_medicion_capacidad, strlen(buffer_uart_medicion_capacidad), HAL_MAX_DELAY);

			break;

		}

	}


void Multimetro_activar(void) {

	HAL_TIM_Base_Start_IT(&htim1);

	HAL_UART_Receive_IT(&huart1,(uint8_t*) &comando_buffer,1);

	// Corroboro que el usuario tenga configurado el clock a 72MHz
	if (HAL_RCC_GetSysClockFreq() != 72000000)
	{
	    Error_Handler();
	}

	// Imprimir primer menu

	UART_mostrar_menu(MENU_INFO);

	// TODO: habría que hacer que se configuren
	// todos los periféricos acá

//
//	// Se hacen los pines PA5, PA6 y PA7 OUTPUT
//	/*Configure GPIO pin : BTN_MENU_Pin */
//	GPIO_InitStruct.Pin = BTN_MENU_Pin;
//	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
//	GPIO_InitStruct.Pull = GPIO_PULLUP;
//	HAL_GPIO_Init(BTN_MENU_GPIO_Port, &GPIO_InitStruct);
//
//	/*Configure GPIO pin : GPIO330R_Pin */
//	GPIO_InitStruct.Pin = GPIO330R_Pin;
//	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//	GPIO_InitStruct.Pull = GPIO_NOPULL;
//	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//	HAL_GPIO_Init(GPIO330R_GPIO_Port, &GPIO_InitStruct);
//
//	/*Configure GPIO pins : GPIO10K_Pin GPIO1M_Pin */
//	GPIO_InitStruct.Pin = GPIO10K_Pin|GPIO1M_Pin;
//	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
//	GPIO_InitStruct.Pull = GPIO_NOPULL;
//	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
//
//	// se activa Timer 1 para interrupción cada 1ms
//    __HAL_RCC_TIM1_CLK_ENABLE();
//
//    htim1.Instance = TIM1;
//    htim1.Init.Prescaler = 7200 - 1;      // 72MHz / 7200 = 10kHz
//    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
//    htim1.Init.Period = 10 - 1;           // 10 cuentas = 1ms
//    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
//    htim1.Init.RepetitionCounter = 0;
//    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
//
//    if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
//    {
//        Error_Handler();
//    }
//
//    HAL_NVIC_SetPriority(TIM1_UP_IRQn, 0, 0);
//    HAL_NVIC_EnableIRQ(TIM1_UP_IRQn);
//
//
//	// activar interrupción botón
//    GPIO_InitTypeDef GPIO_InitStruct = {0};
//
//    __HAL_RCC_GPIOA_CLK_ENABLE();
//    __HAL_RCC_AFIO_CLK_ENABLE();
//
//    GPIO_InitStruct.Pin  = GPIO_PIN_0;
//    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
//    GPIO_InitStruct.Pull = GPIO_PULLUP;
//
//    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
//
//    HAL_NVIC_SetPriority(EXTI0_IRQn, 1, 0);
//    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
//
//
//    // Se inicializa el USART1
//	/* PA9 -> TX */
//	GPIO_InitStruct.Pin   = GPIO_PIN_9;
//	GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
//	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
//	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
//
//	/* PA10 -> RX */
//	GPIO_InitStruct.Pin  = GPIO_PIN_10;
//	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
//	GPIO_InitStruct.Pull = GPIO_NOPULL;
//	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
//
//	huart1.Instance          = USART1;
//	huart1.Init.BaudRate     = 115200;
//	huart1.Init.WordLength   = UART_WORDLENGTH_8B;
//	huart1.Init.StopBits     = UART_STOPBITS_1;
//	huart1.Init.Parity       = UART_PARITY_NONE;
//	huart1.Init.Mode         = UART_MODE_TX_RX;
//	huart1.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
//	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
//
//	HAL_UART_Init(&huart1);
//
//	HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);
//	HAL_NVIC_EnableIRQ(USART1_IRQn);
//
//	/* Start first interrupt reception */
//	HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
//
//	// TODO: configurar interrupción ADC

}

// Callback del timer 100u
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	procesar_activo = 1;
	contador_timer_1ms++;
	contador_timer_100ms++;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == GPIO_PIN_0 && (HAL_GetTick() - btn_debouncer) >= DEBOUNCER_BTN_TIMEOUT_MS){
	  btn_menu = PRESIONADO; //Enciendo flag de que se presiono el boton menu
	  btn_debouncer = HAL_GetTick();
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	flag_nuevo_comando = 1;
}

int procesar_comando(){

		if (estado_actual == FSM_MENU_MODO){

			if (config.comando == OPCION_1) {
				config.modo = UNICO;
				return 1;
			}
			else if (config.comando == OPCION_2) {
				config.modo = CONTINUO;
				return 1;
			}

		}

		if (estado_actual == FSM_MENU_PARAM){

			if (config.comando == OPCION_1) {
				config.parametro = RESISTENCIA;
				return 1;
			}
			else if (config.comando == OPCION_2){
				config.parametro = CAPACITANCIA;
				return 1;
			}

		}

		return 0;

}

FSM_State FSM_General(FSM_State state, FSM_Signals evento) {
	switch (state) {
	case FSM_MENU_INFO:

		if (evento == UART_COMMAND) {

			if (config.comando == OPCION_1) {
				UART_mostrar_menu(MENU_MODO);
				return FSM_MENU_MODO;
			} else if (config.comando == OPCION_2) {
				UART_mostrar_menu(MENU_PARAMETRO);
				return FSM_MENU_PARAM;
			}
			return FSM_MENU_INFO;

		}

		if (evento == BTN_MENU) {
			primer_medicion = 1;

			if (config.parametro == RESISTENCIA) {
				set_resistencia(RESISTOR_330);
				HAL_ADCEx_Calibration_Start(&hadc1);
				return FSM_R330;
			}

			if (config.parametro == CAPACITANCIA) {
				configurar_descarga();
				return FSM_C_DESCARGA;
			}
			return FSM_MENU_INFO;
		}

		break;

	case FSM_MENU_MODO:

		if (evento == UART_COMMAND) {

			int ok = procesar_comando();

			if (!ok) return FSM_MENU_MODO;

			UART_mostrar_menu(MENU_INFO);
			return FSM_MENU_INFO;

		}

		break;

	case FSM_MENU_PARAM:

		if (evento == UART_COMMAND) {

			int ok = procesar_comando();

			if (!ok) return FSM_MENU_PARAM;

			UART_mostrar_menu(MENU_INFO);
			return FSM_MENU_INFO;
		}

		break;

	case FSM_C_DESCARGA:

		if (evento == TICK_100US) {

				int i = ADC_muestrear(2);
			    if (i >= VCC_AL_2_PORCIENTO) {

			        contador_capacitor++;

			        if (contador_capacitor >= MAX_CUENTAS_DESCARGA) {
			            c_medida = FALLO_MAX_TIEMPO_DESCARGA;
			            UART_mostrar_menu(MENU_C);
			            contador_timer_100ms = 0;
			            return FSM_MOSTRAR_C;
			        }

			        return FSM_C_DESCARGA;
			    }

			    // ya descargo
			    configurar_carga();
			    return FSM_C_CARGA;

		}

		if (evento == BTN_MENU) {
			contador_capacitor = 0;
			UART_mostrar_menu(MENU_INFO);
			return FSM_MENU_INFO;
		}

		configurar_descarga();
		return FSM_C_DESCARGA;

	case FSM_C_CARGA:
		if (evento == TICK_1MS) {

			contador_capacitor++;

			if ((ADC_muestrear(1) >= VCC_AL_63_PORCIENTO) && contador_capacitor < MAX_CUENTAS_CARGA) {

				config.unidad = NANO_FARADIOS;
//				c_medida = CALCULAR_CAPACIDAD(contador_capacitor);
				c_medida = contador_capacitor;
				UART_mostrar_menu(MENU_C);
				HAL_GPIO_WritePin(GPIO1M_GPIO_Port, GPIO1M_Pin, GPIO_PIN_RESET);
			    contador_timer_100ms = 0;
				return FSM_MOSTRAR_C;

			} else if(contador_capacitor <= MAX_CUENTAS_CARGA){
				return FSM_C_CARGA;

			} else {
				contador_capacitor = 0;
				c_medida = FALLO_MAX_TIEMPO_CARGA;

				UART_mostrar_menu(MENU_C);
				contador_timer_100ms = 0;
				return FSM_MOSTRAR_C;

			}
		}

		 if (evento == BTN_MENU) {
			contador_capacitor = 0;
			UART_mostrar_menu(MENU_INFO);
			return FSM_MENU_INFO;
		}

		break;

	case FSM_MOSTRAR_C:

		if (evento == TICK_100MS && config.modo == CONTINUO) {
			configurar_descarga();
			return FSM_C_DESCARGA;
		} else if (evento == BTN_MENU) {
			UART_mostrar_menu(MENU_INFO);
			return FSM_MENU_INFO;
		}
		break;

	case FSM_R330:
		if (evento == TICK_100US) {
			ultima_muestra = ADC_muestrear(32);
			if (ultima_muestra > VCC_AL_95_PORCIENTO) {
				set_resistencia(RESISTOR_10K);
				return FSM_R10K;
			}

			r_medida = (VALOR_RESISTOR_330_OHMS * ultima_muestra) / (VCC_MV - ultima_muestra);
			config.unidad = OHMS;
			UART_mostrar_menu(MENU_R);
			contador_timer_100ms = 0;
			return FSM_MOSTRAR_R;

		} else if (evento == BTN_MENU) {
			UART_mostrar_menu(MENU_INFO);
			return FSM_MENU_INFO;
		}
		break;

	case FSM_R10K:
		if (evento == TICK_100US) {
			ultima_muestra = ADC_muestrear(32);
			if (ultima_muestra > VCC_AL_95_PORCIENTO) {
				set_resistencia(RESISTOR_1M);
				return FSM_R1M;
			}

			r_medida = (VALOR_RESISTOR_10K_OHMS * ultima_muestra) / (VCC_MV - ultima_muestra);
			config.unidad = OHMS;
			UART_mostrar_menu(MENU_R);

			contador_timer_100ms = 0;
			return FSM_MOSTRAR_R;

		} else if (evento == BTN_MENU) {
			UART_mostrar_menu(MENU_INFO);
			return FSM_MENU_INFO;
		}
		break;

	case FSM_R1M:
		if (evento == TICK_100US) {
			ultima_muestra = ADC_muestrear(32);
			if (ultima_muestra < VCC_AL_95_PORCIENTO) {
				r_medida = ( (VALOR_RESISTOR_1M_OHMS * ultima_muestra) / (VCC_MV - ultima_muestra) ) / 1000;
				config.unidad = KILO_OHMS;
			} else {
				r_medida = FALLO_R_FUERA_DE_ESCALA;
			}

			UART_mostrar_menu(MENU_R);

			contador_timer_100ms = 0;
			return FSM_MOSTRAR_R;

		} else if (evento == BTN_MENU) {
			UART_mostrar_menu(MENU_INFO);
			return FSM_MENU_INFO;
		}
		break;

	case FSM_MOSTRAR_R:
		if (evento == TICK_100MS && config.modo == CONTINUO) {
			set_resistencia(RESISTOR_330);
			return FSM_R330;
		} else if (evento == BTN_MENU) {
			UART_mostrar_menu(MENU_INFO);
			return FSM_MENU_INFO;
		}
		return FSM_MOSTRAR_R;
	}
	return state;
}

void Multimetro_procesar() {

	btn_debouncer++;
	if (flag_nuevo_comando) { //Atajo el flag del interrupt de la uart.
		switch (comando_buffer) {
			case '1':
				config.comando = OPCION_1;
				break;
			case '2':
				config.comando = OPCION_2;
				break;
			default:
				config.comando = INVALIDO;
				break;
		}
		estado_actual = FSM_General(estado_actual, UART_COMMAND);
		HAL_UART_Receive_IT(&huart1,(uint8_t*) &comando_buffer,1);
		flag_nuevo_comando = 0;
	}

	if (btn_menu == PRESIONADO){ //Atajo el flag del interrupt del boton
		estado_actual = FSM_General(estado_actual, BTN_MENU);
		btn_menu = SIN_PRESIONAR;
	}

	if (!procesar_activo) return;
	procesar_activo=0;

	estado_actual = FSM_General(estado_actual, TICK_100US);

	if (contador_timer_1ms >= 10) {
		contador_timer_1ms = 0;
		estado_actual = FSM_General(estado_actual, TICK_1MS);
	}
	if (contador_timer_100ms >= 1000) {
		contador_timer_100ms = 0;
		estado_actual = FSM_General(estado_actual, TICK_100MS);
	}

}

void configurar_carga() {
	contador_capacitor = 0;

	GPIO_InitTypeDef GPIO_InitStruct = {0};

	// Bajo los 3 pines
	HAL_GPIO_WritePin(GPIO330R_GPIO_Port, GPIO330R_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIO10K_GPIO_Port, GPIO10K_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIO1M_GPIO_Port, GPIO1M_Pin, GPIO_PIN_RESET);

	//1M como salida en bajo (no pull up ni pull down)
	GPIO_InitStruct.Pin = GPIO1M_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIO1M_GPIO_Port, &GPIO_InitStruct);

	// El resto (10k y 330ohm) en alta impedancia
	GPIO_InitStruct.Pin = GPIO330R_Pin|GPIO10K_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT; // Z
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	//Reinicio timer
	__HAL_TIM_SET_COUNTER(&htim1, 0);
	HAL_TIM_Base_Start_IT(&htim1);
	contador_timer_1ms = 0;
	// 1M en alto
	HAL_GPIO_WritePin(GPIO1M_GPIO_Port, GPIO1M_Pin, GPIO_PIN_SET);

//	c_medida = contador_de_muestras * (1000 / VALOR_RESISTOR_1M_KOHMS);

//	HAL_GPIO_WritePin(GPIO1M_GPIO_Port, GPIO1M_Pin, GPIO_PIN_RESET);
}

void configurar_descarga() {
	contador_capacitor = 0;
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	// TODO: preguntar orden de WritePin
	HAL_GPIO_WritePin(GPIO330R_GPIO_Port, GPIO330R_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIO10K_GPIO_Port, GPIO10K_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIO1M_GPIO_Port, GPIO1M_Pin, GPIO_PIN_RESET);

	GPIO_InitStruct.Pin = GPIO330R_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIO330R_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO10K_Pin|GPIO1M_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT; // Z
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void configurar_resistencia330() {
	set_resistencia(RESISTOR_330);
}

uint32_t ADC_muestrear(uint32_t cantidad_muestras) {

	//Realiza una lectura de ADC_N_MUESTRAS muestras y devuelve el promedio
	uint32_t acc = 0;

	for (uint32_t i=0; i<cantidad_muestras; i++) {

		HAL_ADC_Start(&hadc1);
		HAL_ADC_PollForConversion(&hadc1, 1000);
		acc += HAL_ADC_GetValue(&hadc1);

	}

	//Convierto a mV, calculo promedio y retorno
	return (acc * 3300) / (4095 * cantidad_muestras);
}

void set_resistencia(OutputResistor_Type resistorType){


	/*
	Toma como input el define del pin, y lo setea en alto
	mientras que setea los otros dos pines en alta Z.
	*/

	  GPIO_InitTypeDef GPIO_InitStruct = {0}; //Esto habria que ver si hace falta llamarlo siempre, capaz
	  	  	  	  	  	  	  	  	  	  	 	 //podemos evitarnos tambien inicializar gpio initstruct cada vez

	  switch (resistorType){

	  case RESISTOR_330:
          //330r como salida en alto
		  GPIO_InitStruct.Pin = GPIO330R_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		  HAL_GPIO_Init(GPIO330R_GPIO_Port, &GPIO_InitStruct);
		  HAL_GPIO_WritePin(GPIO330R_GPIO_Port, GPIO330R_Pin, GPIO_PIN_SET);

		  GPIO_InitStruct.Pin = GPIO10K_Pin|GPIO1M_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_INPUT; // Z
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		  // Hay que tener cuidado que justo GPIO10K y GPIO1M
		  // son del puerto GPIOA, pero si no fuera así
		  // hay que inicializarlos separados
		  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		  break;

	  case RESISTOR_10K:
		  // GPIO10K en alto
		  GPIO_InitStruct.Pin = GPIO10K_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		  HAL_GPIO_Init(GPIO10K_GPIO_Port, &GPIO_InitStruct);
		  HAL_GPIO_WritePin(GPIO10K_GPIO_Port, GPIO10K_Pin, GPIO_PIN_SET); //High

		  // Y las demas en Z
		  GPIO_InitStruct.Pin = GPIO330R_Pin|GPIO1M_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_INPUT; // Z
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		  // Hay que tener cuidado que justo GPIO10K y GPIO1M
		  // son del puerto GPIOA, pero si no fuera así
		  // hay que inicializarlos separados
		  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		  break;

	  case RESISTOR_1M:
		  //GPIO1M en alto
		  GPIO_InitStruct.Pin = GPIO1M_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		  HAL_GPIO_Init(GPIO1M_GPIO_Port, &GPIO_InitStruct);
		  HAL_GPIO_WritePin(GPIO1M_GPIO_Port, GPIO1M_Pin, GPIO_PIN_SET); //High

		  // Y las demas en Z
		  GPIO_InitStruct.Pin = GPIO10K_Pin|GPIO330R_Pin;
		  GPIO_InitStruct.Mode = GPIO_MODE_INPUT; // Z
		  GPIO_InitStruct.Pull = GPIO_NOPULL;
		  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		  break;

	  default:
		  // TODO habría que tener un fallback para el error
		  break;

	  }
}

const char* ObtenerUnidadEnString() {
	switch (config.unidad) {
	case OHMS:
		return "Ω";
	case KILO_OHMS:
		return "KΩ";
	case MEGA_OHMS:
		return "MΩ";
	case MICRO_FARADIOS:
		return "uF";
	case NANO_FARADIOS:
		return "nF";
	case PICO_FARADIOS:
		return "pF";
	}
	return "UNIDAD INVALIDA";
}
