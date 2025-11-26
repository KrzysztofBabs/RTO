#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

const int pinFotoR=34;
const int pinLampka1=15;
const int pinLampka2=5;
const int pinCzujnikTemp=26;

TaskHandle_t myTaskHandle = NULL;
SemaphoreHandle_t signalSem;
QueueHandle_t kolejka1;

typedef struct{
  uint32_t timestamp;
  int wartosc;
  int id;
} Pomiary;

void TaskLampka1(void *a){
    uint32_t value;
    for(;;){
        xTaskNotifyWait(0, 0, &value, portMAX_DELAY);

        if(value<2){
            digitalWrite(pinLampka1, value);       
        }
        else{
            digitalWrite(pinLampka2, value-2); 
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void TaskFotoRezystor(void *a){
  for(;;){
    Pomiary pomiar;
    pomiar.wartosc = analogRead(pinFotoR);
    pomiar.id = 1;
    pomiar.timestamp = random(0,50);

    if(xQueueSend(kolejka1, &pomiar, pdMS_TO_TICKS(500)) == pdPASS){
      Serial.print(pomiar.id);
      Serial.print(" ");
      Serial.print(pomiar.wartosc);
      Serial.print(" o czasie: ");
      Serial.println(pomiar.timestamp);

      xSemaphoreGive(signalSem); 
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void TaskCzujnikTemp(void *a){
  for(;;){
     Pomiary pomiar;
     int value = analogRead(pinCzujnikTemp);
     float voltage = value * 3.3 / 4095.0;  
     float tempC = voltage * 100.0; 
     pomiar.wartosc = tempC;
     pomiar.id = 2;
     pomiar.timestamp = random(0,50);

     if(xQueueSend(kolejka1, &pomiar, pdMS_TO_TICKS(500)) == pdPASS){
        Serial.print(pomiar.id);
        Serial.print(" ");
        Serial.print(pomiar.wartosc);
        Serial.print(" o czasie: ");
        Serial.println(pomiar.timestamp);

        xSemaphoreGive(signalSem); 
     }
     vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void TaskLogic(void *a){
  Pomiary pomiar;
  uint32_t wartoscOtrzymana;
  for(;;){
    if(xSemaphoreTake(signalSem, portMAX_DELAY) == pdTRUE){
      Serial.println("dostałem sygnał");

      if(xQueueReceive(kolejka1, &pomiar, pdMS_TO_TICKS(portMAX_DELAY)) == pdPASS){
        if(pomiar.id == 1){
          Serial.println("zapalam lampke czerwona");
          if(pomiar.wartosc<1000){
            xTaskNotify(myTaskHandle, 0, eSetValueWithoutOverwrite);
          }
          else{
            xTaskNotify(myTaskHandle, 1, eSetValueWithoutOverwrite);
          }
        }
        if(pomiar.id == 2){
          Serial.println("zapalam lampke zielona");
          if(pomiar.wartosc<14){
            xTaskNotify(myTaskHandle, 2, eSetValueWithoutOverwrite);
          } 
          else{
            xTaskNotify(myTaskHandle, 3, eSetValueWithoutOverwrite);
          }
        }
      }
    }
  }
}

void setup(){
  Serial.begin(9600);
  pinMode(pinLampka1, OUTPUT);
  pinMode(pinLampka2, OUTPUT);
  pinMode(pinFotoR, INPUT);
  pinMode(pinCzujnikTemp, INPUT);

  signalSem = xSemaphoreCreateBinary();
  kolejka1 = xQueueCreate(5, sizeof(Pomiary));

  xTaskCreate(TaskLampka1, "TL1", 2048, NULL, 1, &myTaskHandle);
  xTaskCreate(TaskFotoRezystor, "TFR", 2048, NULL, 1, NULL);
  xTaskCreate(TaskCzujnikTemp, "TCT", 2048, NULL, 1, NULL);
  xTaskCreate(TaskLogic, "TL", 2048, NULL, 1, NULL);
}

void loop(){
  
}
