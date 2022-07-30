#include "doomgeneric.h"
#include "doomkeys.h"
#include "main.h"
#include "doomtype.h"

#define num_buttons 5
#define deadzone (32767 * 0.065)

#define CalSamples 50

int AD_RV = 0;
int AD_RH = 0;

int AD_RV_CENT = 0;
int AD_RH_CENT = 0;

int AD_RV_MP = 2048;
int AD_RV_MM = -2048;

int AD_RH_MP = 2048;
int AD_RH_MM = -2048;

boolean AD_calibrated = false;
//extern void D_PostEvent (event_t* ev);

int map_values(int x, int in_min, int in_max, int out_min, int out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

typedef struct Button {
	GPIO_TypeDef* GPIOx;
	uint16_t GPIO_Pin;
	boolean pressed;
	boolean old_pressed;
} button;

button buttons[num_buttons];

uint8_t key_index;

void DG_Init()
{
	key_index = 0;

	buttons[0].GPIOx = RTN_BTN_GPIO_Port;
	buttons[0].GPIO_Pin = RTN_BTN_Pin;
	buttons[0].pressed = false;
	buttons[0].old_pressed  = false;

	buttons[1].GPIOx = SYS_BTN_GPIO_Port;
	buttons[1].GPIO_Pin = SYS_BTN_Pin;
	buttons[1].pressed = false;
	buttons[1].old_pressed  = false;

	buttons[2].GPIOx = TLHL_BTN_GPIO_Port;
	buttons[2].GPIO_Pin = TLHL_BTN_Pin;
	buttons[2].pressed = false;
	buttons[2].old_pressed  = false;

	buttons[3].GPIOx = TLHR_BTN_GPIO_Port;
	buttons[3].GPIO_Pin = TLHR_BTN_Pin;
	buttons[3].pressed = false;
	buttons[3].old_pressed  = false;

	buttons[4].GPIOx = MDL_BTN_GPIO_Port;
	buttons[4].GPIO_Pin = MDL_BTN_Pin;
	buttons[4].pressed = false;
	buttons[4].old_pressed  = false;
}

void button_update_loop() {
	//Power Off If PWR Pressed
	if (!HAL_GPIO_ReadPin(PWR_BTN_GPIO_Port, PWR_BTN_Pin)) {
		HAL_GPIO_WritePin(PWR_ON_GPIO_Port, PWR_ON_Pin, GPIO_PIN_RESET);
	}

	extern ADC_HandleTypeDef hadc1;
	extern ADC_HandleTypeDef hadc2;

	HAL_ADC_Start(&hadc1);
	HAL_ADC_Start(&hadc2);

	HAL_ADC_PollForConversion(&hadc1, 1);
	HAL_ADC_PollForConversion(&hadc2, 1);
	int ADC_RV_RAW = HAL_ADC_GetValue(&hadc1) - AD_RV_CENT;
	int ADC_RH_RAW = HAL_ADC_GetValue(&hadc2) - AD_RV_CENT;

	//Calibrate
	if (!AD_calibrated) {
		int ADC_RV_RAW_CAL = 0;
		int ADC_RH_RAW_CAL = 0;
		for (int i = 0; i < CalSamples; i++) {
			HAL_ADC_Start(&hadc1);
			HAL_ADC_Start(&hadc2);
			HAL_ADC_PollForConversion(&hadc1, 2);
			HAL_ADC_PollForConversion(&hadc2, 2);
			ADC_RV_RAW_CAL += HAL_ADC_GetValue(&hadc1);
			ADC_RH_RAW_CAL += HAL_ADC_GetValue(&hadc2);
		}
		AD_RV_CENT = ADC_RV_RAW_CAL/CalSamples;
		AD_RH_CENT = ADC_RH_RAW_CAL/CalSamples;
		AD_calibrated = true;
		return;
	}

	AD_RV = map_values(ADC_RV_RAW, AD_RV_MM, AD_RV_MP, -32767, 32767);
	AD_RH = map_values(ADC_RH_RAW, AD_RH_MM, AD_RH_MP, -32767, 32767);

	if (AD_RV < deadzone && AD_RV > -deadzone) {
		AD_RV = 0;
	}

	if (AD_RH < deadzone && AD_RH > -deadzone) {
		AD_RH = 0;
	}

	//char buffer[20];
	//sprintf(buffer, "%d, %d\n", AD_RV, AD_RH);
	//CDC_Transmit_FS(buffer, sizeof(buffer));


	if (!HAL_GPIO_ReadPin(TEL_BTN_GPIO_Port, TEL_BTN_Pin)) {
		if (ADC_RV_RAW < 500 && ADC_RV_RAW > -500) {
			AD_calibrated = false;
			return;
		}

		if (ADC_RV_RAW > 0) {
			AD_RV_MP = ADC_RV_RAW;
		} else {
			AD_RV_MM = ADC_RV_RAW;
		}

		if (ADC_RH_RAW > 0) {
			AD_RH_MP = ADC_RH_RAW;
		} else {
			AD_RH_MM = ADC_RH_RAW;
		}

	}
}

void DG_DrawFrame()
{
}

void DG_SleepMs(uint32_t ms)
{
	HAL_Delay(ms);
}

uint32_t DG_GetTicksMs()
{
	return HAL_GetTick();
}


int DG_GetKey(int* pressed, unsigned char* key)
{
	extern boolean	menuactive;
	
	switch(key_index)
	{
	case 0:
	{
		*key = KEY_ESCAPE;
		break;
	}
	case 1:
	{
		if (menuactive) {
			*key = KEY_ENTER;
		} else {
			*key = KEY_FIRE;
		}
		break;
	}
	case 2:
	{
		*key = KEYP_MINUS;
		break;
	}
	case 3:
	{
		*key = KEYP_PLUS;
		break;
	}
	case 4:
	{
		*key = KEY_USE;
		break;
	}
	default:
	{
		key_index = 0;
		return 0;
	}
	}

	//*pressed = buttons[key_index].state == button_pressed;
	buttons[key_index].pressed = !HAL_GPIO_ReadPin(buttons[key_index].GPIOx, buttons[key_index].GPIO_Pin);
	*pressed = buttons[key_index].pressed && (buttons[key_index].old_pressed != buttons[key_index].pressed);

	buttons[key_index].old_pressed = buttons[key_index].pressed;

	key_index++;
	return 1;
}

void DG_SetWindowTitle(const char * title)
{

}
