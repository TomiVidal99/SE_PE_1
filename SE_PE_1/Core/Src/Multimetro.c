/*
 * Multimetro.c
 *
 *  Created on: 7 may 2026
 *      Author: Tomás Vidal e Ignacio Chantiri
 */

/**
 * TODO: cambiar los pines y puertos a algo más modificable por el usuario
 */

#define DEBOUNCER_BTN_TIMOUT_MS (50)

typedef enum {
	SIN_PRESIONAR,
	PRESIONADO
} EstadoBoton;

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
	MENU_INFO,
	MENU_MODO,
	MENU_PARAM,
	MOSTRAR_MED,
	C_DESCARGA,
	C_CARGA,
	R330,
	R10K,
	R1M,
} FSM_State;

typedef enum {
	TICK_100US,
	TICK_1MS,
	TICK_100MS,
	BTN_MENU,
	UART_COMMAND
} FSM_Signals;

FSM_State FSM_General(FSM_State state, FSM_Signals signal);

Configuracion_t config = {
	.parametro = RESISTENCIA,
	.modo = CONTINUO,
	.comando = OPCION_1,
	.unidad = OHMS,
};

#define MAX_LENGTH_BUFFER_MUESTRAS (32)

typedef struct {
	uint16_t buffer[MAX_LENGTH_BUFFER_MUESTRAS];
	uint32_t index;
} Buffer_t;

Buffer_t buffer_muestras = {
		.buffer = 0,
		.index = 0,
};

#define ESTADO_INICIAL (0)
FSM_State estado_actual = {ESTADO_INICIAL};
uint32_t contador_timer_1ms = 0;
uint32_t contador_timer_10ms = 0;

void Multimetro_activar(void) {

	// Corroboro que el usuario tenga configurado el clock a 72MHz
	if (HAL_RCC_GetSysClockFreq() != 72000000)
	{
	    Error_Handler();
	}

	// Se hacen los pines PA5, PA6 y PA7 OUTPUT
	/*Configure GPIO pin : BTN_MENU_Pin */
	GPIO_InitStruct.Pin = BTN_MENU_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(BTN_MENU_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : GPIO330R_Pin */
	GPIO_InitStruct.Pin = GPIO330R_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIO330R_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : GPIO10K_Pin GPIO1M_Pin */
	GPIO_InitStruct.Pin = GPIO10K_Pin|GPIO1M_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	// se activa Timer 1 para interrupción cada 1ms
    __HAL_RCC_TIM1_CLK_ENABLE();

    htim1.Instance = TIM1;
    htim1.Init.Prescaler = 7200 - 1;      // 72MHz / 7200 = 10kHz
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = 10 - 1;           // 10 cuentas = 1ms
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_NVIC_SetPriority(TIM1_UP_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM1_UP_IRQn);


	// activar interrupción botón
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();

    GPIO_InitStruct.Pin  = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;

    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(EXTI0_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);


    // Se inicializa el USART1
	/* PA9 -> TX */
	GPIO_InitStruct.Pin   = GPIO_PIN_9;
	GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* PA10 -> RX */
	GPIO_InitStruct.Pin  = GPIO_PIN_10;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	huart1.Instance          = USART1;
	huart1.Init.BaudRate     = 115200;
	huart1.Init.WordLength   = UART_WORDLENGTH_8B;
	huart1.Init.StopBits     = UART_STOPBITS_1;
	huart1.Init.Parity       = UART_PARITY_NONE;
	huart1.Init.Mode         = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;

	HAL_UART_Init(&huart1);

	HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(USART1_IRQn);

	/* Start first interrupt reception */
	HAL_UART_Receive_IT(&huart1, &rx_byte, 1);

	// TODO: configurar interrupción ADC

}

// Callback del timer 100us
void HAL_TIM_EXTI_Callback(void) {
	estado_actual = FSM_General(estado_actual, TICK_100US);
	contador_timer_1ms++;
	if (contador_timer_1ms >= 10) {
		contador_timer_1ms = 0;
		estado_actual = FSM_General(estado_actual, TICK_1MS);
	}
	contador_timer_10ms++;
	if (contador_timer_10ms >= 100) {
		contador_timer_10ms = 0;
		estado_actual = FSM_General(estado_actual, TICK_100MS);
	}
}

uint32_t btn_debouncer = 0;
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == GPIO_PIN_0 && (HAL_GetTick() - btn_debouncer) >= DEBOUNCER_BTN_TIMOUT_MS){
	  btn_menu = PRESIONADO; //Enciendo flag de que se presiono el boton menu
	  btn_debouncer = HAL_GetTick();
  }
}

/* HAL callback override */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
    	// TODO
        HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
    }
}

// TODO
uint32_t GetAverage() {
	uint32_t acc;
	for (int i = 0; i < MAX_LENGTH_BUFFER_MUESTRAS; i++) {
		acc += buffer[i];
	}
	return acc / MAX_LENGTH_BUFFER_MUESTRAS;
}

// TODO
void CALLBACK_UART(void){

//	if (index < )
	buffer[index++] = muestra;

//typedef struct {
//	uint16_t buffer[32];
//	uint32_t index;
//} Buffer_t;
//
//Buffer_t buffer_muestras = {
//		.buffer = 0,
//		.index = 0,
//};
}

FSM_State FSM_General(FSM_State state, FSM_Signals signal) {
	switch (state) {
		case MENU_INFO:
			if (signal == UART_COMMAND) {
//				UART_mostrar_menu(menu, handle_uart
				return MENU_PARAM;
			} else if (signal == TICK_1MS) {
				return MENU_INFO;
			} else if (signal == BTN_MENU) {
				return MENU_INFO;
			}
			break;
		case MENU_MODO:
			if (signal == UART_COMMAND) {
				return MENU_INFO;
			} else if (signal == TICK_1MS) {
				return MENU_INFO;
			} else if (signal == BTN_MENU) {
				return MENU_INFO;
			}
			break;
		case MENU_PARAM:
			if (signal == UART_COMMAND) {
				return MENU_INFO;
			} else if (signal == TICK_1MS) {
				return MENU_INFO;
			} else if (signal == BTN_MENU) {
				return MENU_INFO;
			}
			break;
		case MOSTRAR_MED:
			if (signal == UART_COMMAND) {
				return MENU_INFO;
			} else if (signal == TICK_1MS) {
				return MENU_INFO;
			} else if (signal == BTN_MENU) {
				return MENU_INFO;
			}
			break;
		case CAP_DESCARGA:
			if (signal == UART_COMMAND) {
				return MENU_INFO;
			} else if (signal == TICK_1MS) {
				return MENU_INFO;
			} else if (signal == BTN_MENU) {
				return MENU_INFO;
			}
			break;
		case CAP_CARGA:
			if (signal == UART_COMMAND) {
				return MENU_INFO;
			} else if (signal == TICK_1MS) {
				return MENU_INFO;
			} else if (signal == BTN_MENU) {
				return MENU_INFO;
			}
			break;
		case R330:
			if (signal == UART_COMMAND) {
				return MENU_INFO;
			} else if (signal == TICK_1MS) {
				return MENU_INFO;
			} else if (signal == BTN_MENU) {
				return MENU_INFO;
			}
			break;
		case R10K:
			if (signal == UART_COMMAND) {
				return MENU_INFO;
			} else if (signal == TICK_1MS) {
				return MENU_INFO;
			} else if (signal == BTN_MENU) {
				return MENU_INFO;
			}
			break;
		case R1M:
			if (signal == UART_COMMAND) {
				return MENU_INFO;
			} else if (signal == TICK_1MS) {
				return MENU_INFO;
			} else if (signal == BTN_MENU) {
				return MENU_INFO;
			}
			break;
	}
}
