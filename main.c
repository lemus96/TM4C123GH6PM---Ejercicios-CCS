#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"

uint32_t estado = 0;
uint32_t carga_init = 40000000;

int main(void)
{
	SysCtlClockSet ( SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN );
	SysCtlPeripheralEnable ( SYSCTL_PERIPH_GPIOF );
	GPIOPinTypeGPIOOutput ( GPIO_PORTF_BASE , GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 );

	GPIO_PORTF_LOCK_R =GPIO_LOCK_KEY;
	GPIO_PORTF_CR_R=0x0F ;

	GPIOPinTypeGPIOInput (GPIO_PORTF_BASE , GPIO_PIN_4 | GPIO_PIN_0 );
	GPIOPadConfigSet ( GPIO_PORTF_BASE , GPIO_PIN_4 | GPIO_PIN_0 , GPIO_STRENGTH_2MA , GPIO_PIN_TYPE_STD_WPU );

	GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_4 | GPIO_PIN_0, GPIO_FALLING_EDGE);
	GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_0 | GPIO_INT_PIN_4);
	IntEnable(INT_GPIOF);
	IntPrioritySet(INT_GPIOF, 0);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
	TimerLoadSet(TIMER0_BASE, TIMER_A, 40000000-1);

	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	IntEnable(INT_TIMER0A);
	IntPrioritySet(INT_TIMER0A, 1);

	IntMasterEnable();	//Habilitar las interrupciones globales del microprocesador

	TimerEnable(TIMER0_BASE, TIMER_A);

	while(1)
	{
		GPIOPinWrite ( GPIO_PORTF_BASE , GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 , estado );
	}

}

void Cambio_Estado(void){

	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT); //Limpia la bandera de interrupción del Timer

	if (estado==0){
		estado=2;
	}

	else{
		estado=0;
	}
}

void Cambio_Frecuencia(void){

	uint32_t boton = GPIOIntStatus(GPIO_PORTF_BASE, true);	//Máscara de bits de los puertos con interrupciones habilitadas
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


