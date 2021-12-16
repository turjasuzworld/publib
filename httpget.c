/*
 * Copyright (c) 2015-2017, Texas Instruments Incorporated
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

#include "ti_drivers_config.h"
#include <unistd.h>
/*
 *  ======== httpget.c ========
 *  HTTP Client GET example application
 */
#include <ti/drivers/ADC.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/Watchdog.h>
/* BSD support */
#include "string.h"
#include <ti/display/Display.h>
#include <ti/net/http/httpclient.h>
#include "semaphore.h"
#include "global_vars.h"

#define TIMEOUT_MS      15000


#define HOSTNAME              "your server domain"
char REQUEST_URI[]  =         "your request uri";
char SET_REQUEST_URI[] =      "your set uri";
char UPDATE_URI_temp[100]   ;
const char UPDATE_URI_hum[10]   =           "&hum=";
#define USER_AGENT            "HTTPClient (ARM; TI-RTOS)"
#define HTTP_MIN_RECV         (256)

uint16_t adcValue0;
uint32_t adcValue0MicroVolt;
volatile uint_fast8_t gpio_read_byte_arr[100],gpio_read_byte,gpio_send_byte, count1, count2;
volatile uint_fast64_t count4, count3;
volatile uint_fast8_t Read_Val[9]={0,0,0,0,0,0,0,0,0};
volatile uint8_t getCnt = 0;
volatile float temp_measured = 0;
volatile unsigned char* str_reply = NULL;
volatile int MotorSts = 0;
extern Display_Handle display;
extern sem_t ipEventSyncObj;
extern void printError(char *errString,
                       int code);


/*
 *  ======== watchdogCallback ========
 */
void watchdogCallback(uintptr_t watchdogHandle)
{
    /*
     * If the Watchdog Non-Maskable Interrupt (NMI) is called,
     * loop until the device resets. Some devices will invoke
     * this callback upon watchdog expiration while others will
     * reset. See the device specific watchdog driver documentation
     * for your device.
     */
    while (1) {}
}

void    set_seq_1wire(void)
{
    GPIO_setConfig(CONFIG_GPIO_0, GPIO_CFG_OUT_STD| GPIO_CFG_OUT_HIGH);
}
int    reset_seq_1wire(void)
{
    int count = 0;
    GPIO_setConfig(CONFIG_GPIO_0, GPIO_CFG_OUT_STD| GPIO_CFG_OUT_HIGH);
    GPIO_write(CONFIG_GPIO_0, 0);
    usleep(500);
    GPIO_write(CONFIG_GPIO_0, 1);
//    usleep(70);
    GPIO_setConfig(CONFIG_GPIO_0, GPIO_CFG_IN_PU);
    //usleep(20);
    /*
     * Wait while confirmation low
     * input comes from slave
     */
    do {
        gpio_read_byte = GPIO_read(CONFIG_GPIO_0);
        count++;
    } while (gpio_read_byte != 0);

//    /*
//     * Wait till the low pull down
//     * confirmation phase passes by
//     * 60-240 uS
//     */
//    count = 0;
//    do {
//        gpio_read_byte = GPIO_read(CONFIG_GPIO_0);
//        count++;
//    } while (gpio_read_byte == 0);

    usleep(300);
    return count;
}
void    send_1wire_data(unsigned char data)
{

    volatile unsigned char gpio_send_byte_ = data;
    int count1_;
    uint_fast64_t del_ctr;
    for (count1_ = 0; count1_ < 8; ++count1_)
    {
        //usleep(2); // recovery time
        for (del_ctr = 0; del_ctr < 3; ++del_ctr);//2 = 1.4uS
        GPIO_setConfig(CONFIG_GPIO_0, GPIO_CFG_OUT_STD| GPIO_CFG_OUT_LOW);
        GPIO_write(CONFIG_GPIO_0, 0);
        if(gpio_send_byte_ & 1)
        {
            //send 1
            //usleep(6);
            for (del_ctr = 0; del_ctr < 9; ++del_ctr);//2 = 1.4uS
            GPIO_write(CONFIG_GPIO_0, 1);
            //usleep(64);
            for (del_ctr = 0; del_ctr < 91; ++del_ctr);//2 = 1.4uS
        }
        else
        {
            //send 0
            //usleep(64);
            for (del_ctr = 0; del_ctr < 91; ++del_ctr);//2 = 1.4uS
            GPIO_write(CONFIG_GPIO_0, 1);
            //usleep(6);
            for (del_ctr = 0; del_ctr < 9; ++del_ctr);//2 = 1.4uS
        }

        gpio_send_byte_ >>= 1;
    }


}

uint_fast8_t    read_1wire_data(void)
{
    volatile uint_fast8_t read_byte = 0;
    int count1_;
    uint_fast64_t del_ctr;
    for (count1_ = 0; count1_ < 8; ++count1_)
    {
        read_byte >>= 1;
        //usleep(2); // recovery time
        for (del_ctr = 0; del_ctr < 3; ++del_ctr);//2 = 1.4uS
        GPIO_setConfig(CONFIG_GPIO_0, GPIO_CFG_OUT_STD| GPIO_CFG_OUT_HIGH);
        GPIO_write(CONFIG_GPIO_0, 0);
        //usleep(5);
        for (del_ctr = 0; del_ctr < 7; ++del_ctr);//2 = 1.4uS
        GPIO_write(CONFIG_GPIO_0, 1);
        GPIO_setConfig(CONFIG_GPIO_0, GPIO_CFG_IN_PU);
        //usleep(6);
        for (del_ctr = 0; del_ctr < 9; ++del_ctr);//2 = 1.4uS
        if(GPIO_read(CONFIG_GPIO_0)) read_byte |= 0x80;
        //usleep(60);
        for (del_ctr = 0; del_ctr < 86; ++del_ctr);//2 = 1.4uS
    }
    return read_byte;
}
/*
 *  ======== httpTask ========
 *  Makes a HTTP GET request
 */
void* httpTask(void* pvParameters)
{
    bool moreDataFlag = false;
    char data[HTTP_MIN_RECV];
    memset(data, 0, sizeof(data));
    int16_t ret = 0;
    int16_t len = 0;




    Display_printf(display, 0, 0, "Sending a HTTP GET request to '%s'\n",
                   HOSTNAME);

    Watchdog_Handle watchdogHandle;
    Watchdog_Params params_w;
    uint32_t        reloadValue;
    /* Open a Watchdog driver instance */
    Watchdog_Params_init(&params_w);
    params_w.callbackFxn = (Watchdog_Callback) watchdogCallback;
    params_w.debugStallMode = Watchdog_DEBUG_STALL_ON;
    params_w.resetMode = Watchdog_RESET_ON;

    watchdogHandle = Watchdog_open(CONFIG_WATCHDOG_0, &params_w);
    if (watchdogHandle == NULL) {
        /* Error opening Watchdog */
        while (1) {}
    }

    /*
     * The watchdog reload value is initialized during the
     * Watchdog_open() call. The reload value can also be
     * set dynamically during runtime.
     *
     * Converts TIMEOUT_MS to watchdog clock ticks.
     * This API is not applicable for all devices.
     * See the device specific watchdog driver documentation
     * for your device.
     */
    reloadValue = Watchdog_convertMsToTicks(watchdogHandle, TIMEOUT_MS);

    /*
     * A value of zero (0) indicates the converted value exceeds 32 bits
     * OR that the API is not applicable for this specific device.
     */
    if (reloadValue != 0) {
        Watchdog_setReload(watchdogHandle, reloadValue);
    }



    ADC_Handle   adc;
    ADC_Params   params;
    int_fast16_t res;
    HTTPClient_Handle httpClientHandle;
    int16_t statusCode;
    int units = 0;
    char* t = NULL;


        ADC_Params_init(&params);

        adc = ADC_open(CONFIG_ADC_0, &params);
        if (adc == NULL) {
            Display_printf(display, 0, 0, "Error initializing CONFIG_ADC_0\n");
            while (1);
        }

        while(1)
        {



                // 1 wire
                count3 = 0;

                count3 = reset_seq_1wire();

                send_1wire_data(0xcc);
                send_1wire_data(0x44);
                set_seq_1wire();

                count3 = reset_seq_1wire();
                send_1wire_data(0xcc);
                send_1wire_data(0xbe);

                for (count2 = 0; count2 < 9; ++count2)
                {
                    Read_Val[count2] =  read_1wire_data();
                    Display_printf(display, 0, 0, "Read_Val[%d] : %d\n", count2,Read_Val[count2]);
                }

                temp_measured = Read_Val[0] + Read_Val[1]*256;
                temp_measured = temp_measured*0.0625;
                temp_measured*= 100.00;



            Watchdog_clear(watchdogHandle);
            sleep(10);

    }


    return(0);
}
