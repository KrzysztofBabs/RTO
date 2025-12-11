#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"

const int lampkaPin=32;
const int przyciskPin=33;


TimerHandle_t timer;

bool pierwszeWylaczenie=false;



void vBlinkCallback(TimerHandle_t xTimer){
    (void)xTimer;
    digitalWrite(lampkaPin,!digitalRead(lampkaPin));

}

//LOW to wcisniety przycisk
//HIGH to NIEwcisniety przycisk


void taskLampka(void *a){
    static int lastButton=HIGH;

    for(;;){
    int aktualny=digitalRead(przyciskPin);

    if(aktualny==LOW && lastButton==HIGH){
        if(xTimerIsTimerActive(timer)){
            xTimerStop(timer,0);
            digitalWrite(lampkaPin,LOW);

            if(pierwszeWylaczenie==false){
                pierwszeWylaczenie=true;
            }

            
        
    }
        else{
            if(pierwszeWylaczenie==true){
                xTimerChangePeriod(timer,pdMS_TO_TICKS(6000),0);

            }

            xTimerStart(timer,0);
            
        }

    }
    lastButton=aktualny;
    vTaskDelay(pdMS_TO_TICKS(100));

}

}


void setup(){
    Serial.begin(9600);

    pinMode(lampkaPin,OUTPUT);
    pinMode(przyciskPin,INPUT_PULLUP);

    timer=xTimerCreate("t",pdMS_TO_TICKS(3000),pdTRUE,nullptr,vBlinkCallback);
    xTaskCreate(taskLampka, "TL", 2048, NULL, 1, NULL);




}


void loop(){

}