#include "uart_shell.h"



int initializeUart(u16 id) {
	XUartPs_Config *config;
	int status;
	u32 mask;

	/*
	 * TODO:
	 * 		 6. log.h (?)
	 * 		 7. memory write via V2F
	 */


	config = XUartPs_LookupConfig(id);

	if(config == NULL)
		return XST_FAILURE;


	status = XUartPs_CfgInitialize(&uart, config, config->BaseAddress);

	if(status != XST_SUCCESS)
		return XST_FAILURE;


	status = XUartPs_SelfTest(&uart);
	if(status != XST_SUCCESS)
		return XST_FAILURE;

	/**
	XUartPs_SetHandler(uart, uartHandler, uart);

	//list of interrupts
	mask =  XUARTPS_IXR_TOUT | XUARTPS_IXR_PARITY | XUARTPS_IXR_FRAMING |
			XUARTPS_IXR_OVER | XUARTPS_IXR_TXEMPTY | XUARTPS_IXR_RXFULL |
			XUARTPS_IXR_RXOVR;

	XUartPs_SetInterruptMask(uart, mask);
	**/

	XUartPs_SetOperMode(&uart, XUARTPS_OPER_MODE_LOCAL_LOOP);
	XUartPs_SetRecvTimeout(&uart, 8);

	//verify success

	//verify interrupt

	XUartPs_SetOperMode(&uart, XUARTPS_OPER_MODE_NORMAL);



	return XST_SUCCESS;
}

void receiveCommand() {
	// SHELL COMPLETE gj boy
	//

	/**
	 * It seems that the two things needed for writing directly
	 * to memory include the V2F drivers and the
	 * die control functions. This includes
	 * InitDieReqQueue() and InitDieStatusTable()
	 * which allow us to communicate with the die
	 * directly via the Tiger controller
	 * Then, once you do that you may be able to
	 * accesss everything directly without the
	 * LRU buffer or the ftl map table. Make sure
	 * everything in the die requests are also done by
	 * you to ensure ememory is valid. This also
	 * implies that you would have to create your
	 * own filesystem (maybe) if you wanna
	 * save stuff.
	 */
	int totalReceived = 0;
	int received;
	int i;

	xil_printf("\r\nWelcome to Cosmos Plus shell \r\n");
	xil_printf("\r\nType anything... \r\n");

	xil_printf("CosmosPlus>");
	while(1) {

		received = XUartPs_Recv(&uart, &receiveBuffer[totalReceived], BUFFER_SIZE-totalReceived);
		if(received != 0) {

			switch(receiveBuffer[received+totalReceived-1]) {
			case ENTER:
				//xil_printf("\r\nenter received \r\n");
				xil_printf("\r\n");
				totalReceived = 0;
				break;
			case BACKSPACE:
				//xil_printf("\r\nbackspace received \r\n");
				while(receiveBuffer[totalReceived] == 0) {
					totalReceived--;
				}
				totalReceived--;
				break;
			default:
				totalReceived += received;
			}
			xil_printf("\rCosmosPlus>");
			for(i=0; i < totalReceived; i++) {
				xil_printf("%c", receiveBuffer[i]);
			}
		}
	}
}

int uartTest() {
	int index, send_count;
	int loop;
	int received_count;

	XUartPs_SetOperMode(&uart, XUARTPS_OPER_MODE_LOCAL_LOOP);


	for (index = 0; index < BUFFER_SIZE; index++) {
			sendBuffer[index] = 'A' + index;
			receiveBuffer[index] = 0;
	}
	send_count = XUartPs_Send(&uart, sendBuffer, BUFFER_SIZE);
	if(send_count != BUFFER_SIZE) {
		return XST_FAILURE;
	}

	// finish sending ocmmand
    loop=0;
	while(XUartPs_IsSending(&uart)) {
		loop++;
	}

	received_count = 0;
	while(received_count < BUFFER_SIZE) {
		received_count += XUartPs_Recv(&uart, &receiveBuffer[received_count], BUFFER_SIZE - received_count);
	}


	for(index = 0; index < BUFFER_SIZE; index++) {
		if(sendBuffer[index] != receiveBuffer[index]) {
			xil_printf("Error detected in self-test.");
			return XST_FAILURE;
		}
	}
	XUartPs_SetOperMode(&uart, XUARTPS_OPER_MODE_NORMAL);


	return XST_SUCCESS;
}

//unused
void uartHandler( void *callbackRef, u32 event, unsigned int eventData) {
	switch(event) {
		case XUARTPS_EVENT_SENT_DATA:
			break;
		case XUARTPS_EVENT_RECV_DATA:
			break;
		case XUARTPS_EVENT_RECV_TOUT:
#ifdef LOG_UART
			xil_printf("\r\n UART timeout");
#endif
			break;
		case XUARTPS_EVENT_RECV_ERROR:
#ifdef LOG_UART
			xil_printf("\r\n UART error on receive");
#endif
			break;
		default:
			xil_printf("\r\n unhandled uart event");
	}
}

