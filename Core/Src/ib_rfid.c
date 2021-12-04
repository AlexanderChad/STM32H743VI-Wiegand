#include "ib_rfid.h"

uint32_t cardTempHigh = 0;
uint32_t cardTemp = 0;
uint32_t lastWiegand = 0;
int16_t bitCount = 0;
int16_t W_D0;
int16_t W_D1;
uint8_t enable_interrupt = 1;
void (*in_main_callback)(uint32_t RfidCardData);
uint32_t PPMs;
uint32_t lastLoop = 0;

void rfid_init(uint16_t D0, uint16_t D1, uint32_t pollPeriodMs,
		void (*callback)(uint32_t RfidCardData)) {
	W_D0 = D0;
	W_D1 = D1;
	in_main_callback = callback;
	PPMs = pollPeriodMs;
}

void ReadD0() {
	bitCount++;			// Increament bit count for Interrupt connected to D0
	if (bitCount > 31)			// If bit count more than 31, process high bits
			{
		cardTempHigh |= ((0x80000000 & cardTemp) >> 31);//	shift value to high bits
		cardTempHigh <<= 1;
		cardTemp <<= 1;
	} else {
		cardTemp <<= 1;	// D0 represent binary 0, so just left shift card data
	}
	lastWiegand = HAL_GetTick();	// Keep track of last wiegand bit received
}

void ReadD1() {
	bitCount++;				// Increment bit count for Interrupt connected to D1
	if (bitCount > 31)			// If bit count more than 31, process high bits
			{
		cardTempHigh |= ((0x80000000 & cardTemp) >> 31);// shift value to high bits
		cardTempHigh <<= 1;
		cardTemp |= 1;
		cardTemp <<= 1;
	} else {
		cardTemp |= 1;	// D1 represent binary 1, so OR card data with 1 then
		cardTemp <<= 1;		// left shift card data
	}
	lastWiegand = HAL_GetTick();	// Keep track of last wiegand bit received
}

void CleanReadBuff() {
	bitCount = 0;
	cardTemp = 0;
	cardTempHigh = 0;
}

void rfid_EXTI_pinDx(uint16_t GPIO_Pin) {
	if (enable_interrupt) {
		if (GPIO_Pin == W_D0) {
			ReadD0();
		} else if (GPIO_Pin == W_D1) {
			ReadD1();
		}
	}
}

uint32_t GetCardId(volatile uint32_t *codehigh, volatile uint32_t *codelow,
		uint8_t bitlength) {
	*codehigh = *codehigh & 0x03;	// only need the 2 LSB of the codehigh
	*codehigh <<= 30;							// shift 2 LSB to MSB
	*codelow >>= 1;
	return *codehigh | *codelow;
}

void rfid_loop() {
	uint32_t time_l = HAL_GetTick();
	if ((time_l - lastLoop) > PPMs) {
		if (bitCount == 34) {
			enable_interrupt = 0;
			cardTemp >>= 1;
			cardTempHigh >>= 1;
			uint32_t cardIDraw = GetCardId(&cardTempHigh, &cardTemp, bitCount);
			CleanReadBuff();
			enable_interrupt = 1;
			in_main_callback(
					((cardIDraw >> 24) & 0xFF) | ((cardIDraw & 0xFF0000) >> 8)
							| (cardIDraw << 24) | ((cardIDraw & 0xFF00) << 8));
			lastLoop = time_l;
		} else if ((HAL_GetTick() - lastWiegand) > 25) { //если данных нет больше 25 мс считаем, что их уже и не будет
			CleanReadBuff();
		}
	}
}

