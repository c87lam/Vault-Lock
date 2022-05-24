#include <stdbool.h> // booleans, i.e. true and false
#include <stdio.h>   // sprintf() function
#include <stdlib.h>  // srand() and random() functions

#include "ece198.h"

int main(void){
    bool previousClk = false;  // needed by ReadEncoder() to store the previous state of the CLK pin
    int count = 0;  // this gets incremented or decremented as we rotate the encoder
    int password[3] = {1, 2, 3}; //The password of the vault
    int userPassword[3] = {-1, -1, -1}; 
    int index = 0;

    pinInitialize();

    //Blue LED lights up
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 1 & 0x01);

    //Rotary encoder increments
    while (true){
        int delta = ReadEncoder(GPIOB, GPIO_PIN_5, GPIOB, GPIO_PIN_4, &previousClk);  // update the count by -1, 0 or +1
        if (delta != 0) {
            if (!previousClk) {
                count += delta;
            }
            char buff[100];
            sprintf(buff, "%d   \r", count); 
            SerialPuts(buff); 
            Display7Segment(count);
        }

        //limit range of segment display
        if(count < 0){
            count = 9;
        } else if (count > 9){
            count = 0; 
        } 

        //button pressed
        if(!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10)){
            while(!HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10)) {} //once button escapes low state, the rest of the if statement can run
            userPassword[index] = count; //add userPassword to the count of the segement display
            index++; 
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 1 & 0x01); //turn on Blue LED 
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 0 & 0x02); //turn off green LED
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, 0 & 0x04); //turn off red LED

            //Reset index if button is pressed at 0
            if(count == 0){
                index = 0;
            }
        }

        char buff[100];
        sprintf(buff, "%d - %d\r", index, count); 
        SerialPuts(buff); 

        //If the player guesses the password, green led lights up to indicate success
        if((index == 3 && userPassword[0]) == password[0] && userPassword[1] == password[1] && userPassword[2] == password[2]){
            correct();
            index = 0;
        //If the player doesn't guess the password
        } else if (index == 3) {
            incorrect();
            index = 0;
        }
    }

    return 0;
}

// This function is called by the HAL once every millisecond
void SysTick_Handler(void)
{
    HAL_IncTick(); // tell HAL that a new tick has happened
    // we can do other things in here too if we need to, but be careful
}

//Function to initialize all pins
void pinInitialize(void){
    HAL_Init(); // initialize the Hardware Abstraction Layer

    //Enable Ports
    __HAL_RCC_GPIOA_CLK_ENABLE(); // enable port A (for the on-board LED, for example)
    __HAL_RCC_GPIOB_CLK_ENABLE(); // enable port B (for the rotary encoder inputs, for example)
    __HAL_RCC_GPIOC_CLK_ENABLE(); // enable port C (for the on-board blue pushbutton, for example)

    // initialize the pins to be input, output, alternate function, etc...
    InitializePin(GPIOA, GPIO_PIN_5, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, 0);  // on-board LED

    //Initialize coloured LED pins
    InitializePin(GPIOA, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, 0);  // initialize color LED output pins

    //Initialize Rotary Encoder Pins
    InitializePin(GPIOB, GPIO_PIN_5, GPIO_MODE_INPUT, GPIO_PULLUP, 0);   // initialize CLK pin
    InitializePin(GPIOB, GPIO_PIN_4, GPIO_MODE_INPUT, GPIO_PULLUP, 0);   // initialize DT pin
    InitializePin(GPIOB, GPIO_PIN_10, GPIO_MODE_INPUT, GPIO_PULLUP, 0);  // initialize SW pin

    //Initialize 7 Segment Display
    Initialize7Segment(); 

    SerialSetup(9600);
}

//Function for when the player correctly guesses the password
void correct(void){
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 0 & 0x01); //turn off Blue LED
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, 0 & 0x04); //turn off red LED
    
    //blink green LED 4 times
    for(int i = 0; i <= 4; i++){
        HAL_Delay(500); //delay half a second
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 2 & 0x02); //turn on green LED 
        HAL_Delay(500); //delay half a second
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 0 & 0x02); //turn off green LED
    }
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 2 & 0x02); //turn on green LED
}

//Function for when the player incorrectly guesses the password
void incorrect(void){
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 0 & 0x01); //turn off Blue LED 
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 0 & 0x02); //turn off green LED
    
    //blink red LED 4 times
    for(int i = 0; i <= 4; i++){
        HAL_Delay(500); //delay half a second
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, 4 & 0x04); //turn on red LED 
        HAL_Delay(500); //delay half a second
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, 0 & 0x04); //turn off red LED
    }
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, 1 & 0x01); //turn on blue LED
}
