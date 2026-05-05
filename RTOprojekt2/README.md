# Projekt FreeRTOS ESP32 - System wykrywania ruchu i pomiaru odległości

Projekt demonstrujący użycie systemu czasu rzeczywistego FreeRTOS w środowisku Arduino. Program integruje czujnik ruchu, ultradźwiękowy czujnik odległości, wyświetlacz LCD I2C oraz system powiadomień świetlnych i dźwiękowych.

## Wykorzystane biblioteki
- Arduino.h - główna biblioteka środowiska
- Wire.h - obsługa magistrali I2C
- LiquidCrystal_I2C.h - obsługa wyświetlacza LCD po I2C
- queue.h - obsługa kolejek przesyłania danych we FreeRTOS
- task.h - zarządzanie zadaniami oraz systemem powiadomień we FreeRTOS

## Zmienne globalne i struktury danych
- Piny - stałe i zmienne przypisujące piny GPIO (diody: 2 czerwone, żółta, zielona, niebieska; czujnik ruchu PIR, brzęczyk, piny Trig i Echo dla HC-SR04).
- lcd - obiekt wyświetlacza na adresie I2C 0x27 o rozmiarze 16x2.
- taskhandle - uchwyt pozwalający na bezpośrednie przesyłanie powiadomień do zadania sterującego oświetleniem.
- sygnal - semafor binarny informujący o wykryciu ruchu.
- kolejka - bufor służący do przesyłania wyników pomiaru odległości.
- Struktura Dane - definiuje format przesyłanych informacji. Zawiera zmierzoną odległość w centymetrach (lub kod błędu) oraz flagę logiczną o wykrytym ruchu.

## Opis zadań (Tasks)

### TaskWykrycieRuchu
- Działa w nieskończonej pętli z interwałem 200 ms.
- Odczytuje stan cyfrowy z pinu czujnika ruchu.
- Jeżeli stan jest wysoki (wykryto ruch), zadanie podnosi semafor (sygnal) oraz wypisuje informację na port szeregowy.

### TaskMierzenieOdleglosci
- Działa w nieskończonej pętli i oczekuje na podniesienie semafora przez zadanie wykrywające ruch.
- Po wybudzeniu wysyła impuls wyzwalający (10 us) na pin Trig czujnika ultradźwiękowego.
- Oczekuje na odpowiedź na pinie Echo i mierzy czas trwania impulsu.
- Oblicza odległość w centymetrach (czas / 58). Jeżeli odczyt jest błędny, ustawia wartość na -1.
- Pakuje wynik do struktury i wysyła ją do kolejki.
- Usypia się na 300 ms.

### TaskWyswietlanie
- Pobiera odczyty z kolejki danych (maksymalny czas oczekiwania to 1000 ms).
- Na podstawie otrzymanej odległości czyści i aktualizuje treść na wyświetlaczu LCD.
- Podejmuje decyzje o wysłaniu powiadomienia do zadania TaskLampki:
  - Wartość -1 (błąd): wysyła sygnał -1.
  - Odległość < 8 cm: wysyła sygnał 1.
  - Odległość < 13 cm: wysyła sygnał 2.
  - W pozostałych przypadkach: wysyła sygnał 3.

### TaskLampki
- Działa w nieskończonej pętli z interwałem 100 ms.
- Sprawdza, czy w systemie pojawiło się nowe powiadomienie (bez blokowania zadania).
- Jeśli odebrany stan różni się od poprzedniego, resetuje (wyłącza) wszystkie diody.
- Następnie włącza odpowiednie peryferia zgodnie z otrzymanym kodem:
  - Sygnał 1: Włącza dwie czerwone diody oraz generuje dźwięk z brzęczyka (1000 Hz).
  - Sygnał -1: Włącza diodę niebieską i wycisza brzęczyk.
  - Sygnał 2: Włącza diodę żółtą i wycisza brzęczyk.
  - Sygnał 3: Włącza diodę zieloną i wycisza brzęczyk.
- Zapisuje aktualny stan, aby uniknąć ponownego konfigurowania pinów, dopóki stan się nie zmieni.

## Konfiguracja główna

### Setup
- Uruchamia komunikację szeregową (baud rate 9600).
- Konfiguruje tryby pinów wejścia (w tym pull-down dla czujnika) i wyjścia.
- Inicjalizuje wyświetlacz LCD oraz włącza jego podświetlenie.
- Tworzy semafor binarny.
- Tworzy kolejkę zdolną pomieścić 5 struktur typu Dane.
- Rejestruje cztery zadania systemowe z odpowiednimi uchwytami, stosem 2048 bajtów i priorytetem 1.

### Loop
- Pętla pozostaje pusta. System działa asynchronicznie, opierając się wyłącznie na przydzielaniu czasu procesora przez scheduler systemu FreeRTOS.
