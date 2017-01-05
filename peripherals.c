#include "peripherals.h"
#include "modem.h"


#include <stdio.h>
#include <services\int\adi_int.h>
#include <drivers\uart\adi_uart.h>
#include <services\pwr\adi_pwr.h>
#include "system/adi_initialize.h"
#include <string.h>
#include <stdlib.h>
/* AD1854 driver includes */
#include <drivers/dac/ad1854/adi_ad1854.h>
/* AD1871 driver includes */
#include <drivers/adc/ad1871/adi_ad1871.h>



#define BAUD_RATE           9600u

/* Enable macro to display debug information */
#define ENABLE_DEBUG_INFO

/* AD1854 Device instance number */
#define AD1854_DEV_NUM          (0u)

/* AD1871 Device instance number */
#define AD1871_DEV_NUM          (0u)

/* SPORT Device number allocated to AD1854 */
#define AD1854_SPORT_DEV_NUM    (0u)

/* SPORT Device number allocated to AD1871 */
#define AD1871_SPORT_DEV_NUM    (0u)

/* Buffer size */
//#define	BUFFER_SIZE				(65536u)

/* Application time out value */
#define	TIME_OUT_VAL			(0xFFFFFFFu)

/* GPIO port/pin connected to AD1854 DAC reset pin */
#define	AD1854_RESET_PORT		ADI_GPIO_PORT_F
#define	AD1854_RESET_PIN		ADI_GPIO_PIN_12

/* GPIO port/pin connected to AD1871 ADC reset pin */
#define	AD1871_RESET_PORT		ADI_GPIO_PORT_F
#define	AD1871_RESET_PIN		ADI_GPIO_PIN_12

/**** Processor specific Macros ****/

/* External input clock frequency in Hz */
#define 	PROC_CLOCK_IN       		25000000
/* Maximum core clock frequency in Hz */
#define 	PROC_MAX_CORE_CLOCK 		600000000
/* Maximum system clock frequency in Hz */
#define 	PROC_MAX_SYS_CLOCK  		133333333
/* Minimum VCO clock frequency in Hz */
#define 	PROC_MIN_VCO_CLOCK  		25000000
/* Required core clock frequency in Hz */
#define 	PROC_REQ_CORE_CLOCK 		400000000
/* Required system clock frequency in Hz */
#define 	PROC_REQ_SYS_CLOCK  		100000000



#define BUFFER_SIZE_CONV (NUM_SAMPLES*4*2)

/* UART Device Number connected to RS232 socket on the BF537 EZ-KIT Lite */
#define UART_DEVICE_NUM     0u

	/* IF (Debug info enabled) */
#if defined(ENABLE_DEBUG_INFO)
#define DEBUG_MSG1(message)     printf(message)
#define DEBUG_MSG2(message, result) \
		do { \
			printf(message); \
			if(result) \
			{ \
				printf(", Error Code: 0x%08X", result); \
				printf("\n"); \
			} \
		} while (0)
	/* ELSE (Debug info disabled) */
#else

#define DEBUG_MSG1(message)
#define DEBUG_MSG2(message, result)

#endif

/* Audio data buffers */
#pragma align 4
segment ("sdram0") uint8_t RxAudioBuf1[BUFFER_SIZE_CONV];
segment ("sdram0") uint8_t RxAudioBuf2[BUFFER_SIZE_CONV];
segment ("sdram0") uint8_t TxAudioBuf1[BUFFER_SIZE_CONV];
segment ("sdram0") uint8_t TxAudioBuf2[BUFFER_SIZE_CONV];

/* Rx and Tx buffers */
char BufferTx1[BUFFER_SIZE];
char BufferTx2[BUFFER_SIZE];
char BufferRx1[BUFFER_SIZE];
char BufferRx2[BUFFER_SIZE];

/* IF (Callback mode) */
#if defined (ENABLE_CALLBACK)
/* Callback from AD1854 DAC driver */
static void Ad1854DacCallback(
    void        *AppHandle,
    uint32_t    Event,
    void        *pArg);

/* Callback from AD1871 ADC driver */
static void Ad1871AdcCallback(
    void        *AppHandle,
    uint32_t    Event,
    void        *pArg);

#endif /* ENABLE_CALLBACK */

/*=============  C O D E  =============*/

/* IF (Callback mode) */
#if defined (ENABLE_CALLBACK)

/*
 *  Callback from AD1854 DAC driver
 *
 * Parameters
 *  - [in]  AppHandle   Callback parameter supplied by application
 *  - [in]  Event       Callback event
 *  - [in]  pArg        Callback argument
 *
 * Returns  None
 *
 */
static void Ad1854DacCallback(
    void        *AppHandle,
    uint32_t    Event,
    void        *pArg)
{
    /* CASEOF (Event) */
    switch (Event)
    {
    	/* CASE (Buffer Processed) */
        case (ADI_AD1854_EVENT_BUFFER_PROCESSED):
            /* Update processed DAC buffer address */
            pDacBuf = pArg;
            break;
    }
}

/*
 *  Callback from AD1871 ADC driver
 *
 * Parameters
 *  - [in]  AppHandle   Callback parameter supplied by application
 *  - [in]  Event       Callback event
 *  - [in]  pArg        Callback argument
 *
 * Returns  None
 *
 */
static void Ad1871AdcCallback(
    void        *AppHandle,
    uint32_t    Event,
    void        *pArg)
{
    /* CASEOF (Event) */
    switch (Event)
    {
    	/* CASE (Buffer Processed) */
        case (ADI_AD1871_EVENT_BUFFER_PROCESSED):
            /* Update processed ADC buffer address */
            pAdcBuf = pArg;
            break;
    }
}

#endif /* ENABLE_CALLBACK */

/* Opens and configures AD1871 ADC driver */
static uint32_t InitAd1871Adc(void);
/* Opens and configures AD1854 ADC driver */
static uint32_t InitAd1854Dac(void);

/* Handle to the AD1871 device instance */
static ADI_AD1871_HANDLE    hAd1871Adc;
/* Handle to the AD1854 device instance */
static ADI_AD1871_HANDLE    hAd1854Dac;

/* Memory required to handle an AD1871 device instance */
static uint8_t Ad1871AdcMemory[ADI_AD1871_MEMORY_SIZE];
/* Memory required to handle an AD1854 device instance */
static uint8_t Ad1854DacMemory[ADI_AD1854_MEMORY_SIZE];

/* Pointer to processed audio buffers */
static void		*pAdcBuf, *pDacBuf;

void *UBufferBidali,*UBufferJaso;
bool enviar;

bool bIsBufAvailable;

/* UART Handle */
static ADI_UART_HANDLE hDevice;

static ADI_UART_RESULT respuestaTx;
static ADI_UART_RESULT respuestaRx;

uint32_t	Result;

//Variables conversores
fract32 *ptr_fr32;

#define CARACTERES_PRUEBA 8


section ("sdram0") fract32 send_through_dac_right[BUFFER_SIZE_CONV/4];
section ("sdram0") fract32 send_through_dac_left[BUFFER_SIZE_CONV/4];
section ("sdram0") fract32 received_from_adc_right[NUM_SAMPLES_TX];
section ("sdram0") fract32 received_from_adc_left[NUM_SAMPLES_TX];

section ("sdram0") int indice_guardado = 0;

//Variables UART
section ("sdram0") unsigned char trama_entrada_mod[CARACTERES_PRUEBA + FRAME];// = {"actualizar"};
section ("sdram0") unsigned char trama_salida_demod[FRAME];
section ("sdram0") unsigned char send_through_uart[BUFFER_SIZE];
section ("sdram0") unsigned char receive_from_uart[BUFFER_SIZE];
bool packetReceivedUART;
bool packetReceivingADC;



/* Memory required for operating UART in dma mode */
static uint8_t gUARTMemory[ADI_UART_BIDIR_DMA_MEMORY_SIZE];

static bool bError;

static void CheckResult(ADI_UART_RESULT result) {
	if (result != ADI_UART_SUCCESS) {
		printf("UART failure\n");
		bError = true;
	}


}

void initializate_peripherals(){
	 unsigned int i;

	 packetReceivedUART = false;

	    ADI_UART_RESULT eResult;

	    /* Initialize managed drivers and/or services */
	    	adi_initComponents();

		/* Filter initializations */
		/*for (i = 0; i < NUM_COEFFS+2; i++) delay1[i] = 0; */       /* initialize state array       */
		/*for (i = 0; i < NUM_COEFFS+2; i++) delay2[i] = 0; */       /* initialize state array       */

		//fir_init(state1, filter_fr32, delay1, NUM_COEFFS, 0);
		//fir_init(state2, filter_fr32, delay2, NUM_COEFFS, 0);


		/* Initialize buffer pointers */
		pAdcBuf = NULL;
		pDacBuf = NULL;

		/* Clear audio input and output buffers */
		memset(RxAudioBuf1, 0, BUFFER_SIZE_CONV);
		memset(RxAudioBuf2, 0, BUFFER_SIZE_CONV);
		memset(TxAudioBuf1, 0, BUFFER_SIZE_CONV);
		memset(TxAudioBuf2, 0, BUFFER_SIZE_CONV);

		/* Initialize power service */
		Result = (uint32_t) adi_pwr_Init (PROC_CLOCK_IN, PROC_MAX_CORE_CLOCK, PROC_MAX_SYS_CLOCK, PROC_MIN_VCO_CLOCK);

		/* IF (Failure) */
		if (Result)
		{
			DEBUG_MSG1 ("Failed to initialize Power service\n");
		}

		/* IF (Success) */
		if (Result == 0)
		{
			/* Set the required core clock and system clock */
			Result = (uint32_t) adi_pwr_SetFreq(PROC_REQ_CORE_CLOCK, PROC_REQ_SYS_CLOCK);

			/* IF (Failure) */
			if (Result)
			{
				DEBUG_MSG1 ("Failed to initialize Power service\n");
			}
		}

		//UART initialization
			/* Open UART driver */
			eResult = adi_uart_Open(UART_DEVICE_NUM, ADI_UART_DIR_BIDIRECTION,
					gUARTMemory, ADI_UART_BIDIR_DMA_MEMORY_SIZE, &hDevice);
			CheckResult(eResult);

			/* Set UART Baud Rate */
			eResult = adi_uart_SetBaudRate(hDevice, BAUD_RATE);
			CheckResult(eResult);

			/* Configure  UART device with NO-PARITY, ONE STOP BIT and 8bit word length. */
			eResult = adi_uart_SetConfiguration(hDevice, ADI_UART_NO_PARITY,
					ADI_UART_ONE_STOPBIT, ADI_UART_WORDLEN_8BITS);
			CheckResult(eResult);

			/* Enable the DMA associated with UART if UART is expeced to work with DMA mode */
			eResult = adi_uart_EnableDMAMode(hDevice, true);
			CheckResult(eResult);

			printf("Setup terminal on PC as described in Readme file. \n\n");
			printf("Type characters in the terminal program and notice the characters being echoed.\n\n");
			printf("Press the return key to stop the program.\n");

			adi_uart_SubmitTxBuffer (hDevice, BufferTx1, BUFFER_SIZE);
			adi_uart_SubmitRxBuffer (hDevice, BufferTx2, BUFFER_SIZE);
			adi_uart_SubmitTxBuffer (hDevice, BufferRx1, BUFFER_SIZE);
			adi_uart_SubmitRxBuffer (hDevice, BufferRx2, BUFFER_SIZE);

			eResult= adi_uart_EnableTx(hDevice,true);
			eResult= adi_uart_EnableRx(hDevice,true);

#ifdef NO_ADC
		    /* IF (Success) */
		    if (Result == 0)
		    {
		    	/* Open and configure AD1871 ADC device instance */
		    	Result = InitAd1871Adc ();
		    }
#endif
		    /* IF (Success) */
		    if (Result == 0)
		    {
		        /* Open and configure AD1854 DAC device instance */
		        Result = InitAd1854Dac ();
		    }


		    // IF (Success)
			if (Result == 0)
			{
				// Enable AD1854 DAC dataflow
				Result = adi_ad1854_Enable (hAd1854Dac, true);

				// IF (Failure)
				if(Result)
				{
					DEBUG_MSG2("Failed to enable AD1854 DAC dataflow", Result);
				}
			}
#ifdef NO_ADC
			// IF (Success)
			if (Result == 0)
			{
				// Enable AD1871 ADC dataflow
				Result = adi_ad1871_Enable (hAd1871Adc, true);

				// IF (Failure)
				if(Result)
				{
					DEBUG_MSG2("Failed to enable AD1871 ADC dataflow", Result);
				}
			}
#endif
}

void sendThroughUART(){

	enviar = 0;

	// Se bloquea hasta que haya buffer (en condiciones normales siempre debería haber)
	while(!enviar)
	{
		respuestaTx=adi_uart_IsTxBufferAvailable (hDevice, &enviar);
	}

	// Cpgemos puntero a buffer de tx libre
	respuestaTx = adi_uart_GetTxBuffer (hDevice, &UBufferBidali);

	// Copiamos lo que quermoes enviar
	memcpy(UBufferBidali, symbols, BUFFER_SIZE);

	//Enviamos el buffer
	respuestaTx = adi_uart_SubmitTxBuffer (hDevice, UBufferBidali, BUFFER_SIZE); //envia
}

void receiveFromUART(){


	/* Read a character */
	respuestaRx=adi_uart_IsRxBufferAvailable (hDevice, &enviar);

	if(enviar){
		respuestaRx = adi_uart_GetRxBuffer (hDevice, &UBufferJaso);//recibe

		// Aquí lo copio al buffer de tx y lo envio
		memcpy(receive_from_uart,UBufferJaso, BUFFER_SIZE);	//void *s1, const void *s2, size_t n
		//sendThroughUART();

		// Acordarse de volver a enviar el buffer
		respuestaRx=adi_uart_SubmitRxBuffer (hDevice, UBufferJaso, BUFFER_SIZE);
		packetReceivedUART = true;
	}
	//

}

void getADCinput(){

	int cont = 0;
	int indice;

	bool guardar = false;

	float valor = 0;

	/* IF (Valid ADC buffer available) */
		if ((pAdcBuf != NULL)){

			ptr_fr32 = (fract32 *) pAdcBuf;

			for(int indice_conversor=0 ; indice_conversor<NUM_SAMPLES ; indice_conversor++){

				if(received_from_adc_left[indice_conversor] < -1073741823){
					packetReceivingADC = true;
				}

				if(packetReceivingADC){
					received_from_adc_right[indice_guardado]=(ptr_fr32[indice_conversor*2])<<8;
					received_from_adc_left[indice_guardado]=(ptr_fr32[indice_conversor*2+1])<<8;//syncronization signal
					if(indice_guardado++ >= NUM_SAMPLES_TX){
						packetReceivingADC = false;
						packetReceivedADC = true;
						indice_guardado = 0;
					}
				}
			}

			/* Re-submit ADC buffer */
			Result = adi_ad1871_SubmitRxBuffer(hAd1871Adc, pAdcBuf, BUFFER_SIZE_CONV);

			/* IF (Failure) */
			if(Result)
			{
				DEBUG_MSG2("Failed to submit buffer to AD1871", Result);
				//break;
			}



			/* Clear the buffer pointer */
			pAdcBuf = NULL;
		}


}


void checkADCinput(){

	/* Query AD1871 for processed buffer status */
		Result = (uint32_t) adi_ad1871_IsRxBufAvailable (hAd1871Adc, &bIsBufAvailable);

		/* IF (Failure) */
		if (Result)
		{
			DEBUG_MSG2("Failed to query AD1871 for processed buffer status", Result);
			//break;
		}

		/* IF (AD1871 Buffer available) */
		if (bIsBufAvailable)
		{
			/* Get AD1871 processed buffer address */
			Result = (uint32_t) adi_ad1871_GetRxBuffer (hAd1871Adc, &pAdcBuf);

			/* IF (Failure) */
			if (Result)
			{
				DEBUG_MSG2("Failed to get AD1871 processed buffer address", Result);
				//break;
			}
		}

}


void sendThroughDAC(){

	/* IF (Valid DAC buffer available) */
		if (pDacBuf != NULL){

			/* Re-submit DAC buffer */
			Result = adi_ad1854_SubmitTxBuffer(hAd1854Dac, pDacBuf, BUFFER_SIZE_CONV);

			/* IF (Failure) */
			if(Result)
			{
				DEBUG_MSG2("Failed to submit buffer to AD1854", Result);
				//break;
			}

			/* Clear the buffer pointer */
			pDacBuf = NULL;

		}

}


void checkDACinput(){

	/* Query AD1854 for processed buffer status */
	Result = (uint32_t) adi_ad1854_IsTxBufAvailable (hAd1854Dac, &bIsBufAvailable);

		/* IF (Failure) */
		if (Result)
		{
			DEBUG_MSG2("Failed to query AD1854 for processed buffer status", Result);
			//break;
		}

		/* IF (AD1854 Buffer available) */
		if (bIsBufAvailable)
		{
			/* Get AD1854 processed buffer address */
			Result = (uint32_t) adi_ad1854_GetTxBuffer (hAd1854Dac, &pDacBuf);

			/* IF (Failure) */
			if (Result)
			{
				DEBUG_MSG2("Failed to get AD1854 processed buffer address", Result);
				//break;
			}

			ptr_fr32 = (fract32 *) pDacBuf;

			for(int index_sample=0 ; index_sample<NUM_SAMPLES; index_sample++){

				if(index_sample < NUM_SAMPLES_TX && packetReceivedUART){
					ptr_fr32[index_sample*2]	= modulated_signal[index_sample]>>8;
					ptr_fr32[index_sample*2+1]	= modulated_synchronization[index_sample]>>8;
				}else{
					ptr_fr32[index_sample*2]	= 0;
					ptr_fr32[index_sample*2+1]	= 0;
				}
			}

			if(packetReceivedUART){
				packetReceivedUART = false;
			}


		}

}

/*
 * Opens and initializes AD1854 DAC device instance.
 *
 * Parameters
 *  None
 *
 * Returns
 *  0 if success, other values for error
 *
 */
static uint32_t InitAd1854Dac (void)
{
    /* Return code */
    ADI_AD1854_RESULT   eResult;

    /* Open AD1854 instance */
    eResult = adi_ad1854_Open (AD1854_DEV_NUM,
                               &Ad1854DacMemory,
                               ADI_AD1854_MEMORY_SIZE,
                               &hAd1854Dac);

    /* IF (Failed) */
    if (eResult != ADI_AD1854_SUCCESS)
    {
        DEBUG_MSG2("Failed to open AD1854 DAC instance", eResult);
        return ((uint32_t) eResult);
    }

    /* Reset AD1854 */
    eResult = adi_ad1854_HwReset (hAd1854Dac, AD1854_RESET_PORT, AD1854_RESET_PIN);

    /* IF (Failed) */
    if (eResult != ADI_AD1854_SUCCESS)
    {
    	DEBUG_MSG2("Failed to reset AD1854 DAC instance", eResult);
        return ((uint32_t) eResult);
    }

    /* Set SPORT device number, External clock source (SPORT as Slave) */
    eResult = adi_ad1854_SetSportDevice (hAd1854Dac, AD1854_SPORT_DEV_NUM, true);

    /* IF (Failed) */
    if (eResult != ADI_AD1854_SUCCESS)
    {
        DEBUG_MSG2("Failed to set AD1854 DAC SPORT device instance", eResult);
        return ((uint32_t) eResult);
    }

/* IF (Callback mode) */
#if defined (ENABLE_CALLBACK)

    /* Set AD1854 callback function */
    eResult = adi_ad1854_SetCallback (hAd1854Dac, Ad1854DacCallback, NULL);

    /* IF (Failed) */
    if (eResult != ADI_AD1854_SUCCESS)
    {
    	DEBUG_MSG2("Failed to set AD1854 Callback function", eResult);
        return ((uint32_t) eResult);
    }

#endif /* ENABLE_CALLBACK */

    /* Submit Audio buffer 1 to AD1854 DAC */
    eResult = adi_ad1854_SubmitTxBuffer (hAd1854Dac, &TxAudioBuf1, BUFFER_SIZE_CONV);

    /* IF (Failed) */
    if (eResult != ADI_AD1854_SUCCESS)
    {
        DEBUG_MSG2("Failed to submit Audio buffer 1 to AD1854 DAC", eResult);
        return ((uint32_t) eResult);
    }

    /* Submit Audio buffer 2 to AD1854 DAC */
    eResult = adi_ad1854_SubmitTxBuffer (hAd1854Dac, &TxAudioBuf2, BUFFER_SIZE_CONV);

    /* IF (Failed) */
    if (eResult != ADI_AD1854_SUCCESS)
    {
        DEBUG_MSG2("Failed to submit Audio buffer 2 to AD1854 DAC", eResult);
    }

    /* Return */
    return ((uint32_t) eResult);
}

/*
 * Opens and initializes AD1871 ADC device instance.
 *
 * Parameters
 *  None
 *
 * Returns
 *  0 if success, other values for error
 *
 */
static uint32_t InitAd1871Adc (void)
{
    /* Return code */
    ADI_AD1871_RESULT   eResult;

    /* Open AD1871 instance */
    eResult = adi_ad1871_Open (AD1871_DEV_NUM,
                               &Ad1871AdcMemory,
                               ADI_AD1871_MEMORY_SIZE,
                               &hAd1871Adc);

    /* IF (Failed) */
    if (eResult != ADI_AD1871_SUCCESS)
    {
        DEBUG_MSG2("Failed to open AD1871 ADC instance", eResult);
        return ((uint32_t) eResult);
    }

    /* Reset AD1871 */
    eResult = adi_ad1871_HwReset (hAd1871Adc, AD1871_RESET_PORT, AD1871_RESET_PIN);

    /* IF (Failed) */
    if (eResult != ADI_AD1871_SUCCESS)
    {
    	DEBUG_MSG2("Failed to reset AD1871 ADC instance", eResult);
        return ((uint32_t) eResult);
    }

    /* Set SPORT device number, AD1871 as Master (SPORT as Slave) */
    eResult = adi_ad1871_SetSportDevice (hAd1871Adc, AD1871_SPORT_DEV_NUM, true);

    /* IF (Failed) */
    if (eResult != ADI_AD1871_SUCCESS)
    {
        DEBUG_MSG2("Failed to set AD1871 ADC SPORT device instance", eResult);
        return ((uint32_t) eResult);
    }

/* IF (Callback mode) */
#if defined (ENABLE_CALLBACK)

    /* Set AD1871 callback function */
    eResult = adi_ad1871_SetCallback (hAd1871Adc, Ad1871AdcCallback, NULL);

    /* IF (Failed) */
    if (eResult != ADI_AD1871_SUCCESS)
    {
    	DEBUG_MSG2("Failed to set AD1871 Callback function", eResult);
        return ((uint32_t) eResult);
    }

#endif /* ENABLE_CALLBACK */

    /* Submit Audio buffer 1 to AD1871 ADC */
    eResult = adi_ad1871_SubmitRxBuffer (hAd1871Adc, &RxAudioBuf1, BUFFER_SIZE);

    /* IF (Failed) */
    if (eResult != ADI_AD1871_SUCCESS)
    {
        DEBUG_MSG2("Failed to submit Audio buffer 1 to AD1871 ADC", eResult);
        return ((uint32_t) eResult);
    }

    /* Submit Audio buffer 2 to AD1871 ADC */
    eResult = adi_ad1871_SubmitRxBuffer (hAd1871Adc, &RxAudioBuf2, BUFFER_SIZE);

    /* IF (Failed) */
    if (eResult != ADI_AD1871_SUCCESS)
    {
        DEBUG_MSG2("Failed to submit Audio buffer 2 to AD1871 ADC", eResult);
    }

    /* Return */
    return ((uint32_t) eResult);
}



