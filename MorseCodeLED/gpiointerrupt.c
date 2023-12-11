/*
 * Copyright (c) 2015-2020, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== gpiointerrupt.c ========
 */
#include <stdint.h>
#include <stddef.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/Timer.h>

/* Driver configuration */
#include "ti_drivers_config.h"

//Prototypes
void WaitInMS(uint32_t ms);
void SOSPattern();
void OKPattern();
void RedOn();
void RedOff();
void GreenOn();
void GreenOff();

//GLOBALS
volatile char messageIndex = 0;
volatile char newMessage = 0; //0 - 'SOS'  1 - 'OK'
char allowTransition = 1;
char currentMessage = 0; //SOS = 0  OK = 1

//MESSAGES
char* messagePattern = ". . .:- - -:. . .";

//Space each char out for wait.
// : denotes end of letter
const char* SOS_PATTERN = ". . .:- - -:. . .";
const char* OK_PATTERN = "- - -:- . -";

//Timing (all values are multiplied by 100 when used, so adjust accordingly) (to allow for reduction to char type)
uint32_t Timer_tickPeriod; //We will set this to the period of the timer
static const char DOT_DUR = 5; //ms
static const char DASH_DUR = 15; //ms
static const char END_CHAR_DELAY = 5; //ms
static const char BTWN_CHAR_DELAY = 15; //ms
static const char END_WORD_DELAY = 35; //ms

//Used to know how many times we have cycled the timer
char loopCount = 0;

//LED Methods
void RedOn(){
    GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON);
}

void RedOff(){
    GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_OFF);
}

void GreenOn(){
    GPIO_write(CONFIG_GPIO_LED_1, CONFIG_GPIO_LED_ON);
}

void GreenOff(){
    GPIO_write(CONFIG_GPIO_LED_1, CONFIG_GPIO_LED_OFF);
}

//SOS Message, handling via the characters of the message rather than a traditional state machine.
//Increment the index per run, until we hit the null character at the end of the String, then loop back around.
void SOSPattern(){

    //State machine
    switch(messagePattern[messageIndex]){
    case '.':
        //Disallow transition, message has begun
        allowTransition = 0;

        // DOT - red LED on for 500ms
        RedOn();
        GreenOff();

        //Check if we have looped enough, if so, increment our message counter, reset the loop
        if(loopCount > ((DOT_DUR * 100) / Timer_tickPeriod)){
            //Increment our index
            messageIndex++;
            //Reset the counter
            loopCount = 0;
        }

        break;
    case '-':
        //Disallow transition, message has begun
        allowTransition = 0;

        //DASH - Green LED on for 1500ms
        RedOff();
        GreenOn();

        //Check if we have looped enough, if so, increment our message counter, reset the loop
        if(loopCount > ((DASH_DUR * 100) / Timer_tickPeriod)){
            //Increment our index
            messageIndex++;
            //Reset the counter
            loopCount = 0;
        }

        break;
    case ' ':
        //Disallow transition, message has begun
        allowTransition = 0;

        //End of char, turn off both LEDs for 500ms
        RedOff();
        GreenOff();

        //Check if we have looped enough, if so, increment our message counter, reset the loop
        if(loopCount > ((END_CHAR_DELAY * 100) / Timer_tickPeriod)){
            //Increment our index
            messageIndex++;
            //Reset the counter
            loopCount = 0;
        }

        break;
    case ':':
        //Disallow transition, message has begun
        allowTransition = 0;

        //Between letters, both LEDS off
        RedOff();
        GreenOff();

        //Check if we have looped enough, if so, increment our message counter, reset the loop
        if(loopCount > ((BTWN_CHAR_DELAY * 100) / Timer_tickPeriod)){
            //Increment our index
            messageIndex++;
            //Reset the counter
            loopCount = 0;
        }

        break;
    case '\0':
        //Allow transition, message has ended
        allowTransition = 1;

        //End of message,turn off all leds
        RedOff();
        GreenOff();

        //Check if we have looped enough, if so, increment our message counter, reset the loop
        if(loopCount > ((END_WORD_DELAY * 100) / Timer_tickPeriod)){
            //Reset the counter
            loopCount = 0;

            //End of Message, reset the index
            messageIndex = 0;
        }
        break;
    default:
        //Reset the index/loop count back to beginning, something happened
        allowTransition = 1;
        messageIndex = 0;
        loopCount = 0;
        break;
    }
}

//Callback

//The timer callback, this will be called every time the timer ticks over
void TimerCallback(Timer_Handle myHandle, int_fast16_t status)
{
    //Increment our loopCount
    loopCount++;

    //Check if current message differs from the toggle
    if(currentMessage != newMessage){

        //Can we transition?
        if(allowTransition){
            //Change currentMessage to messageToggle
            currentMessage = newMessage;

            //Set the message
            switch(currentMessage){
            case 0:
                messagePattern = SOS_PATTERN;
                messageIndex = 0;
                loopCount = 0;
                break;
            case 1:
                messagePattern = OK_PATTERN;
                messageIndex = 0;
                loopCount = 0;
                break;
            default:
                //RESET
                currentMessage = 0;
                newMessage = 0;
                loopCount = 0;
                messageIndex = 0;
                messagePattern = SOS_PATTERN;
                break;
            }
        }
    }


SOSPattern();


}

//Button Callback, will call any time the button is pressed
void ButtonCallback(uint_least8_t index){
    //Just toggles between SOS and OK
    newMessage = (newMessage == 0) ? 1 : 0;
}

//Init stuff...

//Initialize the timer
void initTimer(void)
{
 Timer_Handle timer0;
 Timer_Params params;

Timer_init();
Timer_Params_init(&params);

params.period = 500000;
params.periodUnits = Timer_PERIOD_US;
params.timerMode = Timer_CONTINUOUS_CALLBACK;
params.timerCallback = TimerCallback;

Timer_tickPeriod = params.period / 1000; //convert to MS

 timer0 = Timer_open(CONFIG_TIMER_0, &params);
 if (timer0 == NULL) {
 /* Failed to initialized timer */
 while (1) {}
 }
 if (Timer_start(timer0) == Timer_STATUS_ERROR) {
 /* Failed to start timer */
 while (1) {}
 }
}

//=============================================================
//BEGIN PROCESSING
//=============================================================

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    /* Call driver init functions */
    GPIO_init();

    /* Configure the LED and button pins */
    GPIO_setConfig(CONFIG_GPIO_LED_0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_LED_1, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_BUTTON_0, GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_FALLING);

    /* Turn on user LED */
    GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON);

    /* Install Button callback, adding to both buttons */
    GPIO_setCallback(CONFIG_GPIO_BUTTON_0, ButtonCallback);
    GPIO_setCallback(CONFIG_GPIO_BUTTON_1, ButtonCallback);

    /* Enable interrupts */
    GPIO_enableInt(CONFIG_GPIO_BUTTON_0);

    //Initialize the timer
    initTimer();

    return (NULL);
}
