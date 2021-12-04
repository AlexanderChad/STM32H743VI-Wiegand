#ifndef IB_RFID_H_
#define IB_RFID_H_

#include "main.h"

void rfid_init(uint16_t D0, uint16_t D1, uint32_t pollPeriodMs,
		void (*callback)(uint32_t RfidCardData));
void rfid_EXTI_pinDx(uint16_t GPIO_Pin);
void rfid_loop();

#endif /* IB_RFID_H_ */
