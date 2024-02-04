/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdlib.h>
#include <time.h>
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define NUM_HEARTS 3
#define NUM_BOXES 2
#define NUM_BULLETS 5
#define NUM_OBSTACLE 6
#define LCD_WIDTH 38
#define LCD_HEIGHT 2
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart1;

PCD_HandleTypeDef hpcd_USB_FS;

/* USER CODE BEGIN PV */
int screen = 0; //0=main 1=play 2=about 3=setting 4=health 5=bullets
int R_Health = 0;
int L_Health = 0;
int R_bullets = 0;
int L_bullets = 0;
int vfx = 1; //1 if on 0 if off
typedef unsigned char byte;
byte bullet[8] = { 0x08, 0x1C, 0x0B, 0x07, 0x0E, 0x1C, 0x08, 0x00 };
byte heart[8] = { 0x00, 0x0A, 0x1F, 0x1F, 0x1F, 0x0E, 0x04, 0x00 };
byte MisteryBox[8] = { 0x1F, 0x11, 0x15, 0x1D, 0x1B, 0x1B, 0x1F, 0x1B };
byte wall[8] = { 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F };
byte obstacle[8] = { 0x1F, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x1F };
byte leftFox[8] = { 0x00, 0x00, 0x06, 0x1A, 0x0F, 0x1A, 0x06, 0x00 };
byte rightFox[8] = { 0x00, 0x00, 0x0C, 0x0B, 0x1E, 0x0B, 0x0C, 0x00 };
byte topFox[8] = { 0x00, 0x00, 0x00, 0x0A, 0x0E, 0x15, 0x1F, 0x04 };
byte bottomFox[8] = { 0x00, 0x04, 0x1F, 0x15, 0x0E, 0x0A, 0x00, 0x00 };

typedef struct {
	int x;
	int y;
	char c;
} misteryBoxOBJ;
typedef struct {
	int x;
	int y;
	char c;
} heartOBJ;
typedef struct {
	int x;
	int y;
	char c;
} bulletOBJ;
typedef struct {
	int x;
	int y;
	char c;
} obstacleOBJ;
typedef struct {
	int x;
	int y;
	char c;
} Player;

Player playerR;
Player playerL;
byte *foxStates[4] = { leftFox, rightFox, topFox, bottomFox };
int currentStateR = 0;
int currentStateL = 0;
heartOBJ hearts[NUM_HEARTS];
misteryBoxOBJ boxes[NUM_BOXES];
bulletOBJ bullets[NUM_BULLETS];
obstacleOBJ obstacles[NUM_OBSTACLE];
char lcd[LCD_HEIGHT][LCD_WIDTH];

GPIO_TypeDef *const Row_ports[] = { GPIOB, GPIOB, GPIOB, GPIOB };
const uint16_t Row_pins[] =
		{ GPIO_PIN_12, GPIO_PIN_13, GPIO_PIN_14, GPIO_PIN_15 };
// RIGHT WIRES: Column1 PB12, Column2 PB13, Column3 PB14, Column4 PB15
GPIO_TypeDef *const Column_ports[] = { GPIOD, GPIOD, GPIOD, GPIOD };
const uint16_t Column_pins[] =
		{ GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3 };
// LEFT WIRES : D0 , D1,D2,D3
volatile uint32_t last_gpio_exti;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {

	if (last_gpio_exti + 200 > HAL_GetTick()) // Simple button debouncing
			{
		return;
	}
	last_gpio_exti = HAL_GetTick();

	int8_t row_number = -1;
	int8_t column_number = -1;

	if (GPIO_Pin == GPIO_PIN_0) {
		// blue_button_pressed = 1;
		// return;
	}

	for (uint8_t row = 0; row < 4; row++) // Loop through Rows
			{
		if (GPIO_Pin == Row_pins[row]) {
			row_number = row;
		}
	}

	HAL_GPIO_WritePin(Column_ports[0], Column_pins[0], 0);
	HAL_GPIO_WritePin(Column_ports[1], Column_pins[1], 0);
	HAL_GPIO_WritePin(Column_ports[2], Column_pins[2], 0);
	HAL_GPIO_WritePin(Column_ports[3], Column_pins[3], 0);

	for (uint8_t col = 0; col < 4; col++) // Loop through Columns
			{
		HAL_GPIO_WritePin(Column_ports[col], Column_pins[col], 1);
		if (HAL_GPIO_ReadPin(Row_ports[row_number], Row_pins[row_number])) {

			column_number = col;
		}
		HAL_GPIO_WritePin(Column_ports[col], Column_pins[col], 0);
	}

	HAL_GPIO_WritePin(Column_ports[0], Column_pins[0], 1);
	HAL_GPIO_WritePin(Column_ports[1], Column_pins[1], 1);
	HAL_GPIO_WritePin(Column_ports[2], Column_pins[2], 1);
	HAL_GPIO_WritePin(Column_ports[3], Column_pins[3], 1);

	if (row_number == -1 || column_number == -1) {
		return; // Reject invalid scan
	}
	//   C0   C1   C2   C3
	// +----+----+----+----+
	// | 1  | 2  | 3  | 4  |  R0
	// +----+----+----+----+
	// | 5  | 6  | 7  | 8  |  R1
	// +----+----+----+----+
	// | 9  | 10 | 11 | 12 |  R2
	// +----+----+----+----+
	// | 13 | 14 | 15 | 16 |  R3
	// +----+----+----+----+
	//1 - 6 - 9 for the Right Player
	//4 -7 -12 for the left player
	const uint8_t button_number = row_number * 4 + column_number + 1;
	switch (button_number) {
	case 1:
		//TO DECREASE IN SETTING
		if (screen == 4) {
			R_Health = (R_Health + 10 - 1) % 10;
			HealthSetting();
		} else if (screen == 5) {
			R_bullets = (R_bullets + 10 - 1) % 10;
			bulletsSetting();
		}
		if (screen == 0) {
			screen = 1;
			refreshAll();
			initializeObjects();
			displayObjects();
		}

		/* code */
		break;
	case 2:
		HAL_UART_Transmit(&huart1, "2", 1,
		HAL_MAX_DELAY);
		if (screen == 0) {
			screen = 2;
			setCursor(20, 0);
			print("                    ");
			setCursor(0, 1);
			print("                      ");
			showAbout();
		}
		/* code */
		break;
	case 3:
		HAL_UART_Transmit(&huart1, "3", 1,
		HAL_MAX_DELAY);
		if (screen == 0) {
			screen = 3;
			setCursor(20, 0);
			print("                    ");
			setCursor(0, 1);
			print("                      ");
			showSetting();
		}
		/* code */
		break;
	case 4:
		HAL_UART_Transmit(&huart1, "4", 1,
		HAL_MAX_DELAY);

		//TO DECREASE IN SETTING
		if (screen == 4) {
			L_Health = (L_Health + 10 - 1) % 10;
			HealthSetting();
		} else if (screen == 5) {
			L_bullets = (L_bullets + 10 - 1) % 10;
			bulletsSetting();
		}

		break;
	case 5:
		HAL_UART_Transmit(&huart1, "5", 1,
		HAL_MAX_DELAY);
		/* code */
		break;
	case 6:
		HAL_UART_Transmit(&huart1, "6", 1,
		HAL_MAX_DELAY);
		changeFoxStateR(0);
		break;
	case 7:
		HAL_UART_Transmit(&huart1, "7", 1,
		HAL_MAX_DELAY);
		changeFoxStateR(1);
		/* code */
		break;
	case 8:
		//back btn
		refreshAll();
		if (screen == 4 | screen == 5) {
			screen = 3;
			showSetting();
		} else {
			screen = 0;
			showMain();
		}

		HAL_UART_Transmit(&huart1, "B", 1,
		HAL_MAX_DELAY);
		/* code */
		break;
	case 9:

		if (screen == 4) {
			R_Health = (R_Health + 1) % 10;
			HealthSetting();
		} else if (screen == 5) {
			R_bullets = (R_bullets + 1) % 10;
			bulletsSetting();
		}
		//right player
		if (screen == 1) {
			movement(&playerR, currentStateR);
		}
		break;
	case 10:
		/* code */
		break;
	case 11:
		/* code */
		break;
	case 12:
		if (screen == 4) {
			L_Health = (L_Health + 1) % 10;
			HealthSetting();
		} else if (screen == 5) {
			L_bullets = (L_bullets + 1) % 10;
			bulletsSetting();
		}
		if (screen == 1) {
			movement(&playerL, currentStateL);
		}
		/* code */
		break;
	case 13:
		/* code */
		if (screen = 3) {
			setCursor(20, 1);
			print("                   ");
			HealthSetting();
			screen = 4;
		}
		break;
	case 14:
		/* code */
		if (screen == 3) {
			screen = 5;
			setCursor(20, 1);
			print("                   ");
			bulletsSetting();
		}
		break;
	case 15:
		if (screen == 3) {
			vfx = (vfx + 1) % 2;
		}
		if (screen == 4 | screen == 5) {
			screen = 3;
			showSetting();
		}
		/* code */
		break;
	case 16:
		/* code */
		break;

	default:
		break;
	}
}

void refreshAll(void) {
	setCursor(0, 0);
	print("                   ");
	setCursor(0, 1);
	print("                   ");
	setCursor(20, 0);
	print("                   ");
	setCursor(20, 1);
	print("                   ");
}
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);
static void MX_USB_PCD_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_RTC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void changeFoxStateR(int Player) {
	byte *currentFoxR = foxStates[currentStateR];
	byte *currentFoxL = foxStates[currentStateL];

	if (!Player) {
		createChar(7, currentFoxR);
		currentStateR = (currentStateR + 1) % 4;
	} else {
		createChar(8, currentFoxL);
		currentStateL = (currentStateL + 1) % 4;
	}
}
void increase(int isHealth, Player *player) {
	if (isHealth) {
		HAL_UART_Transmit(&huart1, "HART", 4,
		HAL_MAX_DELAY);
		if (player->c == 7) {
			R_Health++;
			if (R_Health > 9) {
				R_Health = 9;
			}
		} else {
			L_Health++;
			if (L_Health > 9) {
				L_Health = 9;
			}
		}
	} else {
		HAL_UART_Transmit(&huart1, "BULT", 4,
		HAL_MAX_DELAY);
		if (player->c == 7) {
			R_bullets++;
			if (R_bullets > 9) {
				R_bullets = 9;
			}
		} else {
			L_bullets++;
			if (L_bullets > 9) {
				L_bullets = 9;
			}
		}
	}
}
void movement(Player *player, int currentState) {
//	char buffer[50]; // Buffer to hold the string
//
//	sprintf(buffer, "Player X: %d, Player Y: %d", player->x, player->y); // Convert integers to string
//
//	HAL_UART_Transmit(&huart1, (uint8_t*) buffer, strlen(buffer),
//	HAL_MAX_DELAY); // Transmit string over UART
	int oldX = player->x;
	int oldY = player->y;
	switch (currentState) {
	case 0:	//top
		if (player->x == 1 && lcd[0][player->y] != 4
				&& lcd[0][player->y] != 3) {
			//catching heart and bullet
			if (lcd[0][player->y] == 1) {
				increase(1, player);
			} else if (lcd[0][player->y] == 5) {
				increase(0, player);
			}
			player->x = 0;
		} else if (player->x == 0 && player->y >= 20) {
			if (lcd[1][(player->y) - 20] != 4
					&& lcd[1][(player->y) - 20] != 3) {
				//catching heart and bullet
				if (lcd[1][(player->y) - 20] == 1) {
					increase(1, player);
				} else if (lcd[1][(player->y) - 20] == 5) {
					increase(0, player);
				}
				player->x = 1;
				player->y = (player->y) - 20;
			}
		} else {
			HAL_UART_Transmit(&huart1, "CANT TOP", 8,
			HAL_MAX_DELAY);
		}
		break;
	case 1:	//right
		if (player->y >= 0 && player->y < 19) {
			if (lcd[player->x][(player->y) + 1] != 3
					&& lcd[player->x][(player->y) + 1] != 4) {
				//catching heart and bullet
				if (lcd[player->x][(player->y) + 1] == 1) {
					increase(1, player);
				} else if (lcd[player->x][(player->y) + 1] == 5) {
					increase(0, player);
				}
				player->y = (player->y) + 1;
			}
		} else if (player->y < 39 && player->y >= 20) {
			if (lcd[player->x][(player->y) + 1] != 3
					&& lcd[player->x][(player->y) + 1] != 4) {
				//catching heart and bullet
				if (lcd[player->x][(player->y) + 1] == 1) {
					increase(1, player);
				} else if (lcd[player->x][(player->y) + 1] == 5) {
					increase(0, player);
				}
				player->y = (player->y) + 1;
			}
		} else {
			HAL_UART_Transmit(&huart1, "CANT RIGHt", 10,
			HAL_MAX_DELAY);
		}
		break;

	case 2:	//left
		if (player->y > 0 && player->y <= 19) {
			if (lcd[player->x][(player->y) - 1] != 3
					&& lcd[player->x][(player->y) - 1] != 4) {
				//catching heart and bullet
				if (lcd[player->x][(player->y) - 1] == 1) {
					increase(1, player);
				} else if (lcd[player->x][(player->y) - 1] == 5) {
					increase(0, player);
				}

				player->y = (player->y) - 1;
			}
		} else if (player->y <= 39 && player->y > 20) {
			if (lcd[player->x][(player->y) - 1] != 3
					&& lcd[player->x][(player->y) - 1] != 4) {

				//catching heart and bullet
				if (lcd[player->x][(player->y) - 1] == 1) {
					increase(1, player);
				} else if (lcd[player->x][(player->y) - 1] == 5) {
					increase(0, player);
				}

				player->y = (player->y) - 1;
			}
		} else {
			HAL_UART_Transmit(&huart1, "CANT LEFT", 9,
			HAL_MAX_DELAY);
		}
		break;
		break;
	case 3:	//bot
		if (player->x == 0 && lcd[1][player->y] != 4
				&& lcd[1][player->y] != 3) {
			//catching heart and bullet
			if (lcd[1][player->y] == 1) {
				increase(1, player);
			} else if (lcd[1][player->y] == 5) {
				increase(0, player);
			}

			player->x = 1;
		} else if (player->x == 1 && player->y < 20) {
			if (lcd[0][(player->y) + 20] != 4) {
				//catching heart and bullet
				if (lcd[0][(player->y) + 20]==1) {
					increase(1, player);
				} else if (lcd[0][(player->y) + 20]==5) {
					increase(0, player);
				}
				player->x = 0;
				player->y = (player->y) + 20;
			}
		} else {
			HAL_UART_Transmit(&huart1, "CANT BOT", 8,
			HAL_MAX_DELAY);
		}
		break;
	}

	lcd[player->x][player->y] = player->c;

	// Clear the old position on the LCD
	setCursor(oldY, oldX);
	write(' ');

	// Write the player's character at the new position on the LCD
	setCursor(player->y, player->x);
	write(player->c);

}

void initializeObjects() {
	playerR.x = 1;
	playerR.y = 19;
	playerR.c = 7;
	playerL.c = 8;
	playerL.x = 1;
	playerL.y = 0;
	for (int i = 0; i < LCD_HEIGHT; i++) {
		for (int j = 0; j < LCD_WIDTH; j++) {
			lcd[i][j] = ' ';  // Empty cell
		}
	}

	//hearts
	for (int i = 0; i < NUM_HEARTS; i++) {
		hearts[i].x = 3 + rand() % 28;  // Random number between 3 and 30
		hearts[i].y = rand() % 2;       // Random number between 0 and 1
		hearts[i].c = 1;
	}

	//  mistery boxes
	for (int i = 0; i < NUM_BOXES; i++) {
		boxes[i].x = 3 + rand() % 28;  // Random number between 3 and 30
		boxes[i].y = rand() % 2;       // Random number between 0 and 1
		boxes[i].c = 2;
	}

	//bullets
	for (int i = 0; i < NUM_BULLETS; i++) {
		bullets[i].x = 3 + rand() % 28;  // Random number between 3 and 30
		bullets[i].y = rand() % 2;       // Random number between 0 and 1
		bullets[i].c = 5;
	}
	//obstacles
	for (int i = 0; i < NUM_OBSTACLE; i++) {
		obstacles[i].x = 3 + rand() % 28;  // Random number between 3 and 30
		obstacles[i].y = rand() % 2;       // Random number between 0 and 1
		obstacles[i].c = 4;
	}
	for (int i = 0; i < NUM_HEARTS; i++) {
		lcd[hearts[i].y][hearts[i].x] = hearts[i].c;
	}
	for (int i = 0; i < NUM_BULLETS; i++) {
		lcd[bullets[i].y][bullets[i].x] = bullets[i].c;
	}
	for (int i = 0; i < NUM_BOXES; i++) {
		lcd[boxes[i].y][boxes[i].x] = boxes[i].c;
	}
	for (int i = 0; i < NUM_OBSTACLE; i++) {
		lcd[obstacles[i].y][obstacles[i].x] = obstacles[i].c;
	}
	//WALLS
	lcd[1][1] = 3;
	lcd[0][22] = 3;
	lcd[1][18] = 3;
	lcd[0][37] = 3;


	//[0][38] [0][39] [1][38] [1][39]  are dead
	//Wolves
	lcd[playerR.x][playerR.y] = playerR.c;
	lcd[playerL.x][playerL.y] = playerL.c;
	//lcd[1][0]=8;
	//lcd[1][19]=7;
}

void displayObjects() {
	// Display the objects based on the screen array
	for (int i = 0; i < LCD_HEIGHT; i++) {
		for (int j = 0; j < LCD_WIDTH; j++) {
			setCursor(j, i);
			write(lcd[i][j]);
		}
	}
}

//MENU PART
void showMain(void) {
	setCursor(0, 0);
	print("1- Play");
	setCursor(0, 1);
	print("2- About");
	write(1);
	setCursor(20, 0);
	print("3- Setting");
}

void showAbout(void) {
	setCursor(0, 0);
	print("Kimiya & Atiye");
}

void showSetting(void) {
	setCursor(0, 0);
	print("Right vs Left");
	setCursor(0, 1);
	print("0-Bullets");
	setCursor(20, 0);
	print("*-Health");
}

void bulletsSetting(void) {
	setCursor(0, 1);
	char RBStr[20];
	sprintf(RBStr, "Right Bullets:%d", R_bullets);
	print(RBStr);
	setCursor(20, 0);

	char LBStr[20];
	sprintf(LBStr, "Left Bullets:%d", L_bullets);
	print(LBStr);
	setCursor(20, 1);

	print("#-Confirm");
}

void HealthSetting(void) {
	setCursor(0, 1);
	char RBStr[20];
	sprintf(RBStr, "Right Health:%d", R_Health);
	print(RBStr);
	setCursor(20, 0);

	char LBStr[20];
	sprintf(LBStr, "Left Health:%d", L_Health);
	print(LBStr);
	setCursor(20, 1);

	print("#-Confirm");
}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_I2C1_Init();
	MX_SPI1_Init();
	MX_USB_PCD_Init();
	MX_USART1_UART_Init();
	MX_RTC_Init();
	/* USER CODE BEGIN 2 */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);
	HAL_UART_Transmit(&huart1, "yo", 2,
	HAL_MAX_DELAY);
	srand(time(NULL));  // Initialize the random number generator

	LiquidCrystal(GPIOD, GPIO_PIN_8, GPIO_PIN_9, GPIO_PIN_10, GPIO_PIN_11,
	GPIO_PIN_12, GPIO_PIN_13, GPIO_PIN_14);

	byte *foxStates[4] = { leftFox, rightFox, topFox, bottomFox };

	createChar(2, MisteryBox);
	createChar(3, wall);
	createChar(4, obstacle);
	createChar(1, heart);
	createChar(5, bullet);
	RTC_TimeTypeDef mytime;
	RTC_DateTypeDef mydate;
	mydate.Year = 19;
	mydate.Month = 6;
	mydate.Date = 5;

	HAL_RTC_SetDate(&hrtc, &mydate, RTC_FORMAT_BIN);
	char timeStr[20];
	char dateStr[20];
	showMain();
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		if (screen == 2) {
			HAL_RTC_GetTime(&hrtc, &mytime, RTC_FORMAT_BIN);
			HAL_RTC_GetDate(&hrtc, &mydate, RTC_FORMAT_BIN);
			setCursor(20, 0);
			sprintf(timeStr, "%2d:%2d:%2d", mytime.Hours, mytime.Minutes,
					mytime.Seconds);

			print(timeStr);
			setCursor(20, 1);
			sprintf(dateStr, "%2d//%2d//%2d", mydate.Year, mydate.Month,
					mydate.Date);

			print(dateStr);

		}

		if (screen == 3) {
			setCursor(20, 1);
			if (vfx) {
				print("vfx on-# to turn off");
			} else {
				print("vfx off-# to turn on");
			}
		}

	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
	RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI
			| RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
	RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.LSIState = RCC_LSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) {
		Error_Handler();
	}
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB
			| RCC_PERIPHCLK_USART1 | RCC_PERIPHCLK_I2C1 | RCC_PERIPHCLK_RTC;
	PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
	PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
	PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
	PeriphClkInit.USBClockSelection = RCC_USBCLKSOURCE_PLL;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init(void) {

	/* USER CODE BEGIN I2C1_Init 0 */

	/* USER CODE END I2C1_Init 0 */

	/* USER CODE BEGIN I2C1_Init 1 */

	/* USER CODE END I2C1_Init 1 */
	hi2c1.Instance = I2C1;
	hi2c1.Init.Timing = 0x2000090E;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
		Error_Handler();
	}

	/** Configure Analogue filter
	 */
	if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE)
			!= HAL_OK) {
		Error_Handler();
	}

	/** Configure Digital filter
	 */
	if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN I2C1_Init 2 */

	/* USER CODE END I2C1_Init 2 */

}

/**
 * @brief RTC Initialization Function
 * @param None
 * @retval None
 */
static void MX_RTC_Init(void) {

	/* USER CODE BEGIN RTC_Init 0 */

	/* USER CODE END RTC_Init 0 */

	RTC_TimeTypeDef sTime = { 0 };
	RTC_DateTypeDef sDate = { 0 };

	/* USER CODE BEGIN RTC_Init 1 */

	/* USER CODE END RTC_Init 1 */

	/** Initialize RTC Only
	 */
	hrtc.Instance = RTC;
	hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
	hrtc.Init.AsynchPrediv = 39;
	hrtc.Init.SynchPrediv = 999;
	hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
	hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
	hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
	if (HAL_RTC_Init(&hrtc) != HAL_OK) {
		Error_Handler();
	}

	/* USER CODE BEGIN Check_RTC_BKUP */

	/* USER CODE END Check_RTC_BKUP */

	/** Initialize RTC and set the Time and Date
	 */
	sTime.Hours = 0x0;
	sTime.Minutes = 0x0;
	sTime.Seconds = 0x0;
	sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sTime.StoreOperation = RTC_STOREOPERATION_RESET;
	if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK) {
		Error_Handler();
	}
	sDate.WeekDay = RTC_WEEKDAY_MONDAY;
	sDate.Month = RTC_MONTH_JANUARY;
	sDate.Date = 0x1;
	sDate.Year = 0x0;

	if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN RTC_Init 2 */

	/* USER CODE END RTC_Init 2 */

}

/**
 * @brief SPI1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_SPI1_Init(void) {

	/* USER CODE BEGIN SPI1_Init 0 */

	/* USER CODE END SPI1_Init 0 */

	/* USER CODE BEGIN SPI1_Init 1 */

	/* USER CODE END SPI1_Init 1 */
	/* SPI1 parameter configuration*/
	hspi1.Instance = SPI1;
	hspi1.Init.Mode = SPI_MODE_MASTER;
	hspi1.Init.Direction = SPI_DIRECTION_2LINES;
	hspi1.Init.DataSize = SPI_DATASIZE_4BIT;
	hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi1.Init.NSS = SPI_NSS_SOFT;
	hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
	hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi1.Init.CRCPolynomial = 7;
	hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
	hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
	if (HAL_SPI_Init(&hspi1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN SPI1_Init 2 */

	/* USER CODE END SPI1_Init 2 */

}

/**
 * @brief USART1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART1_UART_Init(void) {

	/* USER CODE BEGIN USART1_Init 0 */

	/* USER CODE END USART1_Init 0 */

	/* USER CODE BEGIN USART1_Init 1 */

	/* USER CODE END USART1_Init 1 */
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 9600;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART1_Init 2 */

	/* USER CODE END USART1_Init 2 */

}

/**
 * @brief USB Initialization Function
 * @param None
 * @retval None
 */
static void MX_USB_PCD_Init(void) {

	/* USER CODE BEGIN USB_Init 0 */

	/* USER CODE END USB_Init 0 */

	/* USER CODE BEGIN USB_Init 1 */

	/* USER CODE END USB_Init 1 */
	hpcd_USB_FS.Instance = USB;
	hpcd_USB_FS.Init.dev_endpoints = 8;
	hpcd_USB_FS.Init.speed = PCD_SPEED_FULL;
	hpcd_USB_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
	hpcd_USB_FS.Init.low_power_enable = DISABLE;
	hpcd_USB_FS.Init.battery_charging_enable = DISABLE;
	if (HAL_PCD_Init(&hpcd_USB_FS) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USB_Init 2 */

	/* USER CODE END USB_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOE,
			CS_I2C_SPI_Pin | LD4_Pin | LD3_Pin | LD5_Pin | LD7_Pin | LD9_Pin
					| LD10_Pin | LD8_Pin | LD6_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOD,
			GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12
					| GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_0 | GPIO_PIN_1
					| GPIO_PIN_2 | GPIO_PIN_3, GPIO_PIN_RESET);

	/*Configure GPIO pins : DRDY_Pin MEMS_INT3_Pin MEMS_INT4_Pin MEMS_INT1_Pin
	 MEMS_INT2_Pin */
	GPIO_InitStruct.Pin = DRDY_Pin | MEMS_INT3_Pin | MEMS_INT4_Pin
			| MEMS_INT1_Pin | MEMS_INT2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pins : CS_I2C_SPI_Pin LD4_Pin LD3_Pin LD5_Pin
	 LD7_Pin LD9_Pin LD10_Pin LD8_Pin
	 LD6_Pin */
	GPIO_InitStruct.Pin = CS_I2C_SPI_Pin | LD4_Pin | LD3_Pin | LD5_Pin | LD7_Pin
			| LD9_Pin | LD10_Pin | LD8_Pin | LD6_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pin : B1_Pin */
	GPIO_InitStruct.Pin = B1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : PB12 PB13 PB14 PB15 */
	GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pins : PD8 PD9 PD10 PD11
	 PD12 PD13 PD14 PD0
	 PD1 PD2 PD3 */
	GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11
			| GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_0 | GPIO_PIN_1
			| GPIO_PIN_2 | GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
