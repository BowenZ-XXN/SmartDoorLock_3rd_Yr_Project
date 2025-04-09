/*
 * menu.c
 *
 *  Created on: Dec 4, 2024
 *      Author: 16107
 */

#include "ssd1306.h"
#include "ssd1306_tests.h"
#include "ssd1306_fonts.h"
#include "menu.h"
#include "as608.h"
#include <string.h>
#include <stdlib.h>
#include "usart.h"


extern uint8_t keyPressed;
extern char DoorPin[];
extern uint8_t BT_Flag;
extern uint8_t facial_flag;

int initial_menu(void)
{
    static uint32_t delayStartTime = 0; // Stores the start time for non-blocking delay
    static int delayInProgress = 0; // Indicates if a non-blocking delay is active

    if (!delayInProgress) { // Only update screen when not in a delay period
        ssd1306_Fill(Black);
        ssd1306_SetCursor(11, 10);
        ssd1306_WriteString("Smart Door Lock", Font_7x10, White);
        ssd1306_SetCursor(0, 30);
        ssd1306_WriteString("Press C to Continue", Font_6x8, White);
        ssd1306_SetCursor(0, 40);
        ssd1306_WriteString("Press A to Settings", Font_6x8, White);
        ssd1306_UpdateScreen();
    }

    if (delayInProgress) {
        // **Non-blocking delay: Wait for 2 seconds**
        if (HAL_GetTick() - delayStartTime >= 2000) {
            delayInProgress = 0; // End delay
            BT_Flag = 0; // Reset Bluetooth unlock flag
            return 0; // Return to the main menu
        }
        return 0; // Stay in the current menu during the delay period
    }

    if (keyPressed == 'C') { // 'C' key pressed
        keyPressed = 0;
        return 1; // Switch to the next menu
    } else if (keyPressed == 'A') { // 'A' key pressed
        keyPressed = 0;
        return 2; // Switch to settings menu
    } else if (BT_Flag == 1) { // Bluetooth unlock triggered
        ssd1306_Fill(Black);
        ssd1306_SetCursor(0, 0);
        ssd1306_WriteString("Unlock!", Font_7x10, White);
        ssd1306_UpdateScreen();

        delayStartTime = HAL_GetTick(); // Record start time
        delayInProgress = 1; // Activate non-blocking delay
    }

    return 0; // Stay in the initial menu
}


int menu_pin(void)
{
    static char userInput[5] = {0};  // Stores user input PIN (max 4 digits)
    static uint8_t InputIndex = 0;   // Current PIN input length
    static uint32_t delayStartTime = 0;  // Records non-blocking delay start time
    static int delayInProgress = 0;  // Indicates if a non-blocking delay is active
    static int nextState = 1;  // Stores the next state to return after delay

    if (!delayInProgress) { // Only update the screen when no delay is active
        ssd1306_Fill(Black);
        ssd1306_SetCursor(0, 0);
        ssd1306_WriteString("Enter Door Pin", Font_7x10, White);
        ssd1306_SetCursor(0, 40);
        ssd1306_WriteString("Press B to Go back", Font_6x8, White);
        ssd1306_SetCursor(0, 20);
        ssd1306_WriteString(userInput, Font_7x10, White);
        ssd1306_UpdateScreen();
    }

    if (delayInProgress) {
        // **Non-blocking delay: Wait for 0.5s or 2s**
        if (HAL_GetTick() - delayStartTime >= 2000) {
            delayInProgress = 0; // End non-blocking delay
            InputIndex = 0; // Reset PIN input
            memset(userInput, 0, sizeof(userInput)); // Clear input
            return nextState;  // Return predefined state (11=success, 1=failure)
        }
        return 1;  // Stay in the current menu during delay
    }

    if (keyPressed >= '0' && keyPressed <= '9') { // Numeric input detection
        if (InputIndex < 4) { // Limit input to 4 digits
            userInput[InputIndex++] = keyPressed;
            userInput[InputIndex] = '\0';
        }
        keyPressed = 0; // Clear key press status
    } else if (keyPressed == 'C') { // Confirm input
        keyPressed = 0;
        if (InputIndex == 4) { // Only verify PIN if 4 digits are entered
            if (strcmp(userInput, DoorPin) == 0) { // Correct PIN
                ssd1306_Fill(Black);
                ssd1306_SetCursor(0, 40);
                ssd1306_WriteString("PIN Correct!", Font_7x10, White);
                ssd1306_UpdateScreen();

                delayStartTime = HAL_GetTick(); // Record start time
                delayInProgress = 1; // Activate non-blocking delay
                nextState = 11; // Return to the main menu after 2s
            } else { // Incorrect PIN
                ssd1306_Fill(Black);
                ssd1306_SetCursor(0, 40);
                ssd1306_WriteString("Fail to Unlock!", Font_7x10, White);
                ssd1306_UpdateScreen();

                delayStartTime = HAL_GetTick();
                delayInProgress = 1;
                nextState = 1; // Retry after 2s
            }
        } else { // PIN is not complete
            ssd1306_Fill(Black);
            ssd1306_SetCursor(0, 40);
            ssd1306_WriteString("Incomplete PIN!", Font_7x10, White);
            ssd1306_UpdateScreen();

            delayStartTime = HAL_GetTick();
            delayInProgress = 1;
            nextState = 1; // Retry after 2s
        }
    } else if (keyPressed == 'B') { // Return to the main menu
        keyPressed = 0;
        InputIndex = 0;
        memset(userInput, 0, sizeof(userInput));
        return 0;
    } else if (keyPressed == 'D') { // Backspace function
        keyPressed = 0;
        if (InputIndex > 0) {
            InputIndex--;
            userInput[InputIndex] = '\0';
        }
    }

    return 1; // Stay in the current menu
}


int unlock_method(void)
{
    ssd1306_Fill(Black);
	ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("1.Fingerprint", Font_7x10, White);
	ssd1306_SetCursor(0, 20);
    ssd1306_WriteString("2.Facial", Font_7x10, White);
	ssd1306_SetCursor(0, 40);
    ssd1306_WriteString("3.Gesture", Font_7x10, White);
    ssd1306_UpdateScreen();
    if (keyPressed == '1') {
    	keyPressed = 0;
        return 12; // Switch to the next menu
    }else if (keyPressed == '2'){
    	keyPressed =0;
    	return 13;
    }else if (keyPressed == 'B'){
    	keyPressed = 0;
    	return 0;
    }
    return 11;
}

int menu_fingerprint_check(void)
{
    ssd1306_Fill(Black);
    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("Place Finger", Font_7x10, White); // 提示用户放置手指
    ssd1306_UpdateScreen();


    int fingerprint_result = verify_FR();//刷指纹测试
    if (fingerprint_result == 1) { // 验证成功

        return 0; // 返回主菜单

    } else { // 验证失败

        return 11; // return to the menu to choose verify method

    }
}

int menu_facial_check(void)
{
    static uint32_t delayStartTime = 0; // Stores the start time for non-blocking delay
    static int delayInProgress = 0; // Indicates if a non-blocking delay is active

    if (!delayInProgress) { // Only update screen when not in a delay period
        ssd1306_Fill(Black);
        ssd1306_SetCursor(0, 0);
        ssd1306_WriteString("Watch camera", Font_7x10, White); // Prompt user to face the camera
        ssd1306_UpdateScreen();
    }

    if (delayInProgress) {
        // **Non-blocking delay: Wait for 2 seconds**
        if (HAL_GetTick() - delayStartTime >= 2000) {
            delayInProgress = 0; // End delay
            return 0; // Return to the main menu
        }
        return 13; // Stay in the facial recognition menu during delay
    }

    if (facial_flag == 1) { // If facial recognition is successful
        facial_flag = 0; // Reset flag
        ssd1306_Fill(Black);
        ssd1306_SetCursor(0, 0);
        ssd1306_WriteString("Unlock!", Font_7x10, White); // Display unlock message
        ssd1306_UpdateScreen();

        delayStartTime = HAL_GetTick(); // Record start time
        delayInProgress = 1; // Activate non-blocking delay
    }

    else if (keyPressed == 'B')
    {
        	keyPressed = 0;
        	return 11;
    }

    return 13; // Stay in the current menu
}

int menu_settings_check(void)
{
    static char userPin[7] = {0}; // Stores the user-entered PIN (max 6 digits)
    static uint8_t pinIndex = 0;  // Current PIN input length
    static uint32_t delayStartTime = 0;  // Stores the start time for non-blocking delay
    static int delayInProgress = 0;  // Indicates if a non-blocking delay is active
    static int nextState = 2;  // Stores the next state after delay
    const char systemPin[] = "970329"; // Predefined system PIN

    if (!delayInProgress) { // Update screen only if not in a delay period
        ssd1306_Fill(Black);
        ssd1306_SetCursor(0, 0);
        ssd1306_WriteString("Enter System PIN", Font_7x10, White);
        ssd1306_SetCursor(0, 15);
        ssd1306_WriteString("to Access Settings", Font_7x10, White);
        ssd1306_SetCursor(0, 30);
        ssd1306_WriteString(userPin, Font_7x10, White);
        ssd1306_UpdateScreen();
    }

    if (delayInProgress) {
        // **Non-blocking delay: Wait for 1 second**
        if (HAL_GetTick() - delayStartTime >= 1000) {
            delayInProgress = 0; // End non-blocking delay
            pinIndex = 0; // Reset PIN input
            memset(userPin, 0, sizeof(userPin)); // Clear input
            return nextState;  // Return to the next menu (3 = success, 2 = failure)
        }
        return 2;  // Stay in the current menu during the delay period
    }

    if (keyPressed >= '0' && keyPressed <= '9') { // Detect numeric input
        if (pinIndex < 6) { // Ensure PIN does not exceed 6 digits
            userPin[pinIndex++] = keyPressed; // Store the entered digit
            userPin[pinIndex] = '\0'; // Ensure null termination
        }
        keyPressed = 0; // Clear key press status
    } else if (keyPressed == 'C') { // Confirm input
        keyPressed = 0;
        if (pinIndex == 6) { // Ensure PIN has the correct length
            if (strcmp(userPin, systemPin) == 0) { // Validate PIN
                ssd1306_Fill(Black);
                ssd1306_SetCursor(0, 40);
                ssd1306_WriteString("PIN Correct!", Font_7x10, White);
                ssd1306_UpdateScreen();

                delayStartTime = HAL_GetTick(); // Record start time
                delayInProgress = 1; // Activate non-blocking delay
                nextState = 3; // Move to the settings menu after 1 second
            } else { // Incorrect PIN
                ssd1306_Fill(Black);
                ssd1306_SetCursor(0, 40);
                ssd1306_WriteString("PIN Incorrect!", Font_7x10, White);
                ssd1306_UpdateScreen();

                delayStartTime = HAL_GetTick();
                delayInProgress = 1;
                nextState = 2; // Retry after 1 second
            }
        } else { // PIN not complete
            ssd1306_Fill(Black);
            ssd1306_SetCursor(0, 40);
            ssd1306_WriteString("Incomplete PIN!", Font_7x10, White);
            ssd1306_UpdateScreen();

            delayStartTime = HAL_GetTick();
            delayInProgress = 1;
            nextState = 2; // Retry after 1 second
        }
    } else if (keyPressed == 'B') { // Return to main menu
        keyPressed = 0;
        pinIndex = 0;
        memset(userPin, 0, sizeof(userPin));
        return 0; // Return to main menu
    } else if (keyPressed == 'D') { // Backspace function
        keyPressed = 0;
        if (pinIndex > 0) {
            pinIndex--;
            userPin[pinIndex] = '\0';
        }
    }

    return 2; // Stay in the current menu
}


int menu_settings(void)
{
    ssd1306_Fill(Black);
	ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("1.Edit Door PIN", Font_6x8, White);
	ssd1306_SetCursor(0, 20);
    ssd1306_WriteString("2.Edit Fingerprint", Font_6x8, White);
	ssd1306_SetCursor(0, 40);
    ssd1306_WriteString("3.Edit Facial", Font_6x8, White);
    ssd1306_UpdateScreen();
    if (keyPressed == '1') {
    	keyPressed = 0;
        return 31; // Switch to the next menu
    }else if (keyPressed == 'B'){
    	keyPressed = 0;
    	return 0;
    }else if (keyPressed == '2'){
    	keyPressed = 0;
    	return 32;
    }else if (keyPressed == '3'){
    	keyPressed = 0;
    	return 35;
    }
    return 3;
}

int set_doorPIN(void)
{
    static char userInput[5] = {0}; // Stores the new PIN set by the user (max 4 digits)
    static uint8_t InputIndex = 0;  // Current PIN input length
    static uint32_t delayStartTime = 0;  // Stores the start time for non-blocking delay
    static int delayInProgress = 0;  // Indicates if a non-blocking delay is active
    static int nextState = 31;  // Stores the next state after delay

    if (!delayInProgress) { // Only update screen when no delay is active
        ssd1306_Fill(Black);
        ssd1306_SetCursor(0, 0);
        ssd1306_WriteString("Enter New Door PIN", Font_6x8, White);
        ssd1306_SetCursor(0, 20);
        ssd1306_WriteString(userInput, Font_6x8, White);
        ssd1306_UpdateScreen();
    }

    if (delayInProgress) {
        // **Non-blocking delay: Wait for 2 seconds**
        if (HAL_GetTick() - delayStartTime >= 2000) {
            delayInProgress = 0; // End non-blocking delay
            InputIndex = 0; // Reset PIN input
            memset(userInput, 0, sizeof(userInput)); // Clear input
            return nextState;  // Return to the main menu
        }
        return 31;  // Stay in the current menu during the delay
    }

    if (keyPressed >= '0' && keyPressed <= '9') { // Detect numeric input
        if (InputIndex < 4) { // Ensure PIN does not exceed 4 digits
            userInput[InputIndex++] = keyPressed; // Store the entered digit
            userInput[InputIndex] = '\0'; // Ensure null termination
        }
        keyPressed = 0; // Clear key press status
    } else if (keyPressed == 'C') { // Confirm input
        keyPressed = 0;
        if (InputIndex == 4) { // Only save PIN if 4 digits are entered
            strcpy(DoorPin, userInput);
            ssd1306_Fill(Black);
            ssd1306_SetCursor(0, 0);
            ssd1306_WriteString("PIN SET OK", Font_7x10, White);
            ssd1306_UpdateScreen();

            delayStartTime = HAL_GetTick(); // Record start time
            delayInProgress = 1; // Activate non-blocking delay
            nextState = 0; // Move to the main menu after 2 seconds
        } else { // Incomplete PIN
            ssd1306_Fill(Black);
            ssd1306_SetCursor(0, 40);
            ssd1306_WriteString("Incomplete PIN!", Font_7x10, White);
            ssd1306_UpdateScreen();

            delayStartTime = HAL_GetTick();
            delayInProgress = 1;
            nextState = 31; // Retry after 2 seconds
        }
    } else if (keyPressed == 'B') { // Return to the main menu
        keyPressed = 0;
        InputIndex = 0;
        memset(userInput, 0, sizeof(userInput));
        return 3; // Return to the main menu
    }

    return 31; // Stay in the current menu
}

int menu_modify_FR(void)
{
    ssd1306_Fill(Black);
	ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("1.Add Fingerprint", Font_6x8, White);
	ssd1306_SetCursor(0, 20);
    ssd1306_WriteString("2.Delete Fingerprint", Font_6x8, White);
    ssd1306_UpdateScreen();
    if (keyPressed == 'B'){
    	keyPressed = 0;
    	return 3;
    }
    else if (keyPressed == '1'){
    	keyPressed = 0;
    	return 33;
    }
    else if (keyPressed == '2') {
        keyPressed = 0;
        return 34;
    }
    return 32;
}

int menu_delete_FR(void) {
    static char userInput[2] = {0}; // 用户输入的 ID，只允许 1 位数字

    ssd1306_Fill(Black);
    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("Enter ID to Delete", Font_6x8, White);
    ssd1306_SetCursor(0, 20);
    ssd1306_WriteString(userInput, Font_6x8, White);
    ssd1306_UpdateScreen();

    if (keyPressed >= '0' && keyPressed <= '9') {
        userInput[0] = keyPressed; // Overwrite previous input
        userInput[1] = '\0'; // Ensure null-termination
        keyPressed = 0;
    }
    else if (keyPressed == 'C' && userInput[0] != '\0') { // Confirm delete
        keyPressed = 0;
        uint16_t id = userInput[0] - '0'; // Convert char to int
        Del_FR(id);
        userInput[0] = '\0'; // Clear input
        return 32; // Return to fingerprint management menu
    }
    else if (keyPressed == 'B') { // 取消
        keyPressed = 0;
        memset(userInput, 0, sizeof(userInput));
        return 32; // 返回到指纹管理菜单
    }
    return 34; // 继续当前界面
}

uint8_t ID_NUM = 0;
int menu_record_FR(void) {
    static char userInput[2] = {0}; // 只允许输入 1 位数字
    static uint8_t step = 0; // 0: 输入 ID, 1: 录入指纹

    ssd1306_Fill(Black);

    // Step 0: 让用户输入要存储的指纹 ID
    if (step == 0) {
        ssd1306_SetCursor(0, 0);
        ssd1306_WriteString("Enter Finger ID", Font_6x8, White);
        ssd1306_SetCursor(0, 20);
        ssd1306_WriteString(userInput, Font_6x8, White);
        ssd1306_UpdateScreen();

        if (keyPressed >= '1' && keyPressed <= '9') {
            userInput[0] = keyPressed; // 覆盖用户输入
            userInput[1] = '\0'; // 确保字符串结束
            keyPressed = 0;
        }
        else if (keyPressed == 'C' && userInput[0] != '\0') { // 确认
            keyPressed = 0;
            ID_NUM = userInput[0] - '0'; // 转换为整数 (1-9)
            Record_FR(); // 调用录入指纹函数
            step = 0; // 复位步骤
            memset(userInput, 0, sizeof(userInput));
            return 32; // 返回指纹管理菜单
        }
        else if (keyPressed == 'B') { // 取消
            keyPressed = 0;
            memset(userInput, 0, sizeof(userInput));
            step = 0;
            return 32; // 返回指纹管理菜单
        }
        return 33; // 继续等待用户输入 ID
    }
    return 33;
}

uint8_t data[1];
int menu_modify_Facial(void){
    ssd1306_Fill(Black);
	ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("1.Add Face", Font_6x8, White);
	ssd1306_SetCursor(0, 20);
    ssd1306_WriteString("2.Delete All Faces", Font_6x8, White);
    ssd1306_UpdateScreen();
    if (keyPressed == 'B'){
    	keyPressed = 0;
    	return 3;
    }
    else if (keyPressed == '1'){
    	keyPressed = 0;
        uint8_t data[] = {'A'};  // Create a buffer with the character 'A'
    	HAL_UART_Transmit(&huart1, data ,1,100);
    	return 35;
    }
    else if (keyPressed == '2') {
        keyPressed = 0;
        uint8_t data[] = {'D'};  // Create a buffer with the character 'D'
    	HAL_UART_Transmit(&huart1,data,1,100);
        return 35; // 进入删除指纹菜单
    }
    return 35;
}
