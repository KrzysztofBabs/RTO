#include <Arduino.h>

SemaphoreHandle_t signalSem;


void TaskProducer(void *a){
    for(;;){
        
        vTaskDelay(1000);
        Serial.println("daje sygnal do roboty");
        xSemaphoreGive(signalSem);
        vTaskDelay(3000);
    }
    

}
void TaskConsumer(void *a){
    for(;;){
        if(xSemaphoreTake(signalSem, portMAX_DELAY)==pdTRUE){
            Serial.println("zapalam lampke");
            digitalWrite(21,HIGH);
            vTaskDelay(2000);
            digitalWrite(21,LOW);
            // vTaskDelay(10);
            Serial.println("Lampka zgasla");
            
        }

    }
}


void setup(){
    Serial.begin(9600);
    pinMode(21,OUTPUT);

    signalSem=xSemaphoreCreateBinary();

    xTaskCreate(TaskProducer,"TP",2048,NULL,1,NULL);
    xTaskCreate(TaskConsumer,"TC",2048,NULL,1,NULL);

}



void loop(){}