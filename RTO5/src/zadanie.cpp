#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"


QueueHandle_t kolejka;
int pinCzujnik=15;
int lampkaZielona=22;
int lampkaCzerwona=23;


typedef struct{
    uint32_t timestamp;
    int wartoscZCzujnika;
} Pomiary;


void SignalError(){
        for(int i=0;i<2;i++){
            digitalWrite(lampkaCzerwona,HIGH);
            vTaskDelay(1000);
            digitalWrite(lampkaCzerwona,LOW);
            vTaskDelay(500);
        }

}

void SignalDone(){
    for(int i=0;i<1;i++){
            digitalWrite(lampkaZielona,HIGH);
            vTaskDelay(1000);
            digitalWrite(lampkaZielona,LOW);
            vTaskDelay(200);
        }

}
void SignalError3(){
        for(int i=0;i<3;i++){
            digitalWrite(lampkaCzerwona,HIGH);
            vTaskDelay(300);
            digitalWrite(lampkaCzerwona,LOW);
            vTaskDelay(300);
        }

}


void TaskProducer(void *a){
    Pomiary wartosc;
    vTaskDelay(50);
    
    for(;;){
        int raw=analogRead(pinCzujnik);
        

        if(raw<500){
            raw=0;
            Serial.println("brak sygnalu na czujniku");
            SignalError();
            vTaskDelay(pdMS_TO_TICKS(1000)); 
            continue;
        }


        

        else{
            
            float voltage = raw * 3.3 / 4095.0;  
            float tempC = voltage * 100.0;     // LM35: 10 mV/°C
            wartosc.wartoscZCzujnika = tempC;
            wartosc.timestamp = millis();
            
            if(xQueueSend(kolejka,&wartosc,500)!=pdPASS){
                Serial.println("timeout - kolejka jest pelna ");
                SignalError3();
            }
            else{
                Serial.print("wyslano: ");
                Serial.println(wartosc.wartoscZCzujnika);
                SignalDone();
            }
            
        }
        vTaskDelay(1000);
        
    }
    // vTaskDelay(2000);
    
}

void TaskConsumer(void *a){
    Pomiary odbior;
    odbior.timestamp=millis();
    int timeout=3000;

    
    for(;;){
        if(xQueueReceive(kolejka,&odbior,timeout)==pdPASS){
            
                Serial.print("Consumer odczyt: ");
                Serial.print(odbior.wartoscZCzujnika);
                Serial.print(" czas: ");
                Serial.println(odbior.timestamp);
                SignalDone();


               
            }

        else{
            Serial.println("brak danych w kolejce");
            SignalError3();

        }
        vTaskDelay(3000);

    }
}




void setup(){
    Serial.begin(9600);
    delay(2000);
    // pinMode(5,OUTPUT);
    pinMode(lampkaZielona,OUTPUT);   
    pinMode(lampkaCzerwona,OUTPUT); 
    pinMode(pinCzujnik, INPUT);
    kolejka=xQueueCreate(5,sizeof(Pomiary));


    if(kolejka!=NULL){
        xTaskCreate(TaskProducer,"TP",2048, NULL,1,NULL);
        xTaskCreate(TaskConsumer,"TC",2048, NULL,1,NULL);
    }
    else{
        Serial.print("nie udalo sie utworzyc kolejki");
    }
    

}





void loop(){}
