#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"

uint32_t estado = 0;			//Indica si existe salida digital o no
uint32_t carga_init = 40000000;		//Carga para el timer

int main(void)
{
	SysCtlClockSet ( SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN );	//Reloj a 20MHz
	SysCtlPeripheralEnable ( SYSCTL_PERIPH_GPIOF );							//Habilitar señal de reloj al puerto F
	GPIOPinTypeGPIOOutput ( GPIO_PORTF_BASE , GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 );		//Declarar pines del puerto F como salidas

	GPIO_PORTF_LOCK_R =GPIO_LOCK_KEY;
	GPIO_PORTF_CR_R=0x0F ;			//Desbloquear pin PF0

	GPIOPinTypeGPIOInput (GPIO_PORTF_BASE , GPIO_PIN_4 | GPIO_PIN_0 );		//Declarar pines del puerto F como entradas digitales
	GPIOPadConfigSet ( GPIO_PORTF_BASE , GPIO_PIN_4 | GPIO_PIN_0 , GPIO_STRENGTH_2MA , GPIO_PIN_TYPE_STD_WPU );	//Configuración entrada Pull-Up

	GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_4 | GPIO_PIN_0, GPIO_FALLING_EDGE);	//Configurar el trigger de la interrupción para flancos de bajada
	GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_0 | GPIO_INT_PIN_4);		//Habilitar las interrupciones en el periférico 
	IntEnable(INT_GPIOF);								//Habilitar interrupciones del puerto F en el NVIC
	IntPrioritySet(INT_GPIOF, 0);							//Fijar la prioridad de la interrupción como la más alta (0)

	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);		//Habilitar señal de reloj al TIMER0
	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);	//Configuración del timer como periódico
	TimerLoadSet(TIMER0_BASE, TIMER_A, 40000000-1);		//Fijar la carga del timer 0 subtimer A 

	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);	//Habilitación de interrupciones en el periférico (Interrupción cada fin de conteo)
	IntEnable(INT_TIMER0A);					//Habilitar las interrupciones en el NVIC
	IntPrioritySet(INT_TIMER0A, 1);				//Fijar la prioridad de la interrupción como la segunda más alta (1)

	IntMasterEnable();	//Habilitar las interrupciones globales del microprocesador

	TimerEnable(TIMER0_BASE, TIMER_A);	//Habilitar el timer para que comience a operar

	while(1)
	{
		GPIOPinWrite ( GPIO_PORTF_BASE , GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 , estado );	//Imprime la salida en el LED RGB
	}

}

void Cambio_Estado(void){

	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT); //Limpia la bandera de interrupción del Timer

	//Conmutar el LED dependiendo de su estado
	if (estado==0){
		estado=2;
	}

	else{
		estado=0;
	}
}

void Cambio_Frecuencia(void){

	uint32_t boton = GPIOIntStatus(GPIO_PORTF_BASE, true);	//Máscara de bits de los pines con interrupciones habilitadas
	GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_4 | GPIO_PIN_0); //Limpia la bandera de interrupción del puerto F

	switch (boton)
	{
		case 1:
			carga_init-=(SysCtlClockGet()/20);
			TimerLoadSet(TIMER0_BASE, TIMER_A, carga_init-1); //Aumento de frecuencia (+10Hz)
			break;

		case 16:
			carga_init+=(SysCtlClockGet()/20);
			TimerLoadSet(TIMER0_BASE, TIMER_A, carga_init-1); //Disminucion de frecuencia (-10Hz)
			break;

		default:
			break;
	}

}


