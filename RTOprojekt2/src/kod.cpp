#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include<freertos/queue.h>
#include "freertos/task.h"


int pinCzerwona1=4;
int pinCzerwona2=16;
int pinZolta=23;
int pinZielona=18;
int pinCzujnik=14;
int pinBrzeczyk=13;

int pinNiebieska=33;

LiquidCrystal_I2C lcd(0x27, 16, 2);

QueueHandle_t kolejka;
SemaphoreHandle_t sygnal;
TaskHandle_t taskhandle;
TaskHandle_t taskhandle2;


int trigPin=26;
int echoPin=27;

typedef struct{
  int odleglosc;
  bool czyRuch;
} Dane; 


void TaskWykrycieRuchu(void *a){
    for(;;){
        int czujnik=digitalRead(pinCzujnik);
        if(czujnik==HIGH){
            xSemaphoreGive(sygnal);
            Serial.println("wykryto ruch");
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void TaskMierzenieOdleglosci(void *a){
    long czasLotuDzwieku;
    int odlegloscCm;
    Dane dane;
    for(;;){
        if(xSemaphoreTake(sygnal,portMAX_DELAY)==pdTRUE){
            Serial.println("dostalem sygnal");
            digitalWrite(trigPin, LOW);
            delayMicroseconds(2);

            digitalWrite(trigPin, HIGH);
            delayMicroseconds(10);
            digitalWrite(trigPin, LOW);

        
            czasLotuDzwieku = pulseIn(echoPin, HIGH, 50000);
            if(czasLotuDzwieku==0){
                odlegloscCm=-1; //blad
            }
            else{
                odlegloscCm=(int)(czasLotuDzwieku/58);
            }
            dane.odleglosc=odlegloscCm;
            xQueueSend(kolejka,&dane,portMAX_DELAY);
            // vTaskDelay(pdMS_TO_TICKS(60));
            vTaskDelay(pdMS_TO_TICKS(300));
        }
    }

}

void TaskWyswietlanie(void *a){
    Dane odebraneDane;

    for(;;){
        if(xQueueReceive(kolejka,&odebraneDane,pdMS_TO_TICKS(1000))){
            lcd.clear();
            if(odebraneDane.odleglosc==-1){
                lcd.setCursor(0,0);
                lcd.print("zly odczyt");
                xTaskNotify(taskhandle, -1, eSetValueWithOverwrite);

            }
            else if(odebraneDane.odleglosc<8){
                lcd.setCursor(0,0);
                lcd.print("odleglosc: ");
                lcd.print(odebraneDane.odleglosc);
                lcd.print(" cm");
                xTaskNotify(taskhandle, 1, eSetValueWithOverwrite);
                
            }
            else if(odebraneDane.odleglosc<13){
                lcd.clear();
                lcd.setCursor(0,0);
                lcd.print("odleglosc: ");
                lcd.print(odebraneDane.odleglosc);
                lcd.print(" cm");
                xTaskNotify(taskhandle, 2, eSetValueWithOverwrite);
                
            }
            else{
                xTaskNotify(taskhandle, 3, eSetValueWithOverwrite);
                lcd.clear();
                lcd.setCursor(0,0);
                lcd.print("odleglosc: ");
                lcd.print(odebraneDane.odleglosc);
                lcd.print(" cm");
            }
            
        }
        
        
    }
}


void TaskLampki(void *a){
    uint32_t aktualnyStan = 3; 
    uint32_t ostatniStan = 0;  
    uint32_t odebranaWartosc;

    for(;;){
        if(xTaskNotifyWait(0, 0, &odebranaWartosc, 0)==pdTRUE){
            aktualnyStan = odebranaWartosc;
        }

        if(aktualnyStan!=ostatniStan){
            
            digitalWrite(pinCzerwona1,LOW);
            digitalWrite(pinCzerwona2,LOW);
            digitalWrite(pinZolta,LOW);
            digitalWrite(pinZielona,LOW);
            digitalWrite(pinNiebieska,LOW);

            if(aktualnyStan==1){ 
                digitalWrite(pinCzerwona1,HIGH);
                digitalWrite(pinCzerwona2,HIGH);
                tone(pinBrzeczyk, 1000); 
            }
            else if(aktualnyStan==-1){
                digitalWrite(pinNiebieska,HIGH);
                noTone(pinBrzeczyk);
                digitalWrite(pinBrzeczyk, LOW);
            }
            else if(aktualnyStan==2){
                digitalWrite(pinZolta,HIGH);
                noTone(pinBrzeczyk); 
            }
            else{
                digitalWrite(pinZielona,HIGH);
                noTone(pinBrzeczyk); 
            }

            ostatniStan = aktualnyStan;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}




void setup(){

  Serial.begin(9600);
  
  pinMode(pinCzujnik,INPUT_PULLDOWN);
  pinMode(pinCzerwona1,OUTPUT);
  pinMode(pinCzerwona2,OUTPUT);
  pinMode(pinZolta,OUTPUT);
  pinMode(pinZielona,OUTPUT);
  pinMode(pinNiebieska,OUTPUT);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(pinBrzeczyk, OUTPUT);
  digitalWrite(pinBrzeczyk, LOW);
    
  
  digitalWrite(trigPin, LOW);

  lcd.init();       
  lcd.backlight();  

  sygnal = xSemaphoreCreateBinary();
  kolejka = xQueueCreate(5, sizeof(Dane));

  xTaskCreate(TaskWykrycieRuchu,"TWR",2048,NULL,1,NULL);
  xTaskCreate(TaskMierzenieOdleglosci,"TMO",2048,NULL,1,NULL);
  xTaskCreate(TaskWyswietlanie,"TW",2048,NULL,1,NULL);
  xTaskCreate(TaskLampki,"TL",2048,NULL,1,&taskhandle);


  
}

void loop(){

  
}

