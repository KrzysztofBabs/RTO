#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

const int lampkaPin=32;
const int przyciskPin=33;


TimerHandle_t timer;
TaskHandle_t taskhandle;





void vDebounceCallback(TimerHandle_t xTimer) {
    xTaskNotifyGive(taskhandle);
}

// void vBlinkCallback(TimerHandle_t xTimer){
//     (void)xTimer;
//     digitalWrite(lampkaPin,!digitalRead(lampkaPin));

//     }

// }

void taskLampka(void *a){
    for(;;){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        digitalWrite(lampkaPin, !digitalRead(lampkaPin));
    }
}




void taskPrzycisk(void *a){
    static int lastButton=HIGH;

    for(;;){
    int aktualny=digitalRead(przyciskPin);
    if(aktualny==LOW  && lastButton==HIGH){
        xTimerStart(timer,0);
    }
            
        
    lastButton=aktualny;
    vTaskDelay(pdMS_TO_TICKS(10));
}

}


void setup(){
    Serial.begin(9600);

    pinMode(lampkaPin,OUTPUT);
    pinMode(przyciskPin,INPUT_PULLUP);

    timer=xTimerCreate("t",pdMS_TO_TICKS(3000),pdFALSE,nullptr,vDebounceCallback);
    // xTaskCreate(taskLampka, "TL", 2048, NULL, 1, NULL);
    xTaskCreate(taskLampka, "TL", 2048, NULL, 1, &taskhandle);
    xTaskCreate(taskPrzycisk, "TP", 2048, NULL, 1, NULL);
}


void loop(){

}