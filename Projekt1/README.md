Projekt FreeRTOS ESP32
======================

Projekt demonstrujący użycie systemu czasu rzeczywistego FreeRTOS w środowisku Arduino. Program zarządza dwoma czujnikami oraz dwiema diodami za pomocą niezależnych zadań, kolejek i semaforów.

Wykorzystane biblioteki
-----------------------
- Arduino.h - główna biblioteka środowiska
- FreeRTOS.h - rdzeń systemu czasu rzeczywistego
- task.h - zarządzanie zadaniami
- queue.h - obsługa kolejek przesyłania danych
- semphr.h - obsługa semaforów synchronizacyjnych

Zmienne globalne i struktury danych
-----------------------------------
- Piny - stałe przypisujące piny GPIO mikrokontrolera do urządzeń (fotorezystor, czujnik temperatury, dwie lampki).
- myTaskHandle - uchwyt pozwalający na bezpośrednie komunikowanie się z zadaniem obsługującym lampki.
- signalSem - semafor binarny informujący logikę o pojawieniu się nowych danych.
- kolejka1 - bufor, w którym czujniki umieszczają odczytane struktury.
- Struktura Pomiary - definiuje format przesyłanych danych. Składa się z losowego znacznika czasu, wartości pomiaru oraz identyfikatora określającego źródło danych.

Opis zadań (Tasks)
------------------

TaskLampka1:
- Działa w nieskończonej pętli.
- Oczekuje w uśpieniu na bezpośrednie powiadomienie systemowe z innych zadań.
- Po otrzymaniu wartości mniejszej od 2, zmienia stan pinu pierwszej lampki.
- Po otrzymaniu wartości 2 lub większej, zmienia stan pinu drugiej lampki.
- Usypia się na 100 ms po wykonaniu operacji.

TaskFotoRezystor:
- Działa w nieskończonej pętli z interwałem 1000 ms.
- Odczytuje stan z analogowego wejścia fotorezystora.
- Przypisuje do struktury danych odczytaną wartość oraz identyfikator 1.
- Przesyła strukturę do kolejki.
- Wypisuje dane na port szeregowy.
- Podnosi semafor, aby powiadomić zadanie logiczne o nowym pomiarze.

TaskCzujnikTemp:
- Działa w nieskończonej pętli z interwałem 500 ms.
- Odczytuje napięcie z wejścia analogowego i przelicza je na stopnie Celsjusza.
- Przypisuje do struktury danych wynik oraz identyfikator 2.
- Przesyła strukturę do kolejki.
- Wypisuje dane na port szeregowy.
- Podnosi semafor, podobnie jak zadanie fotorezystora.

TaskLogic:
- Główne zadanie decyzyjne systemu.
- Oczekuje w zawieszeniu na podniesienie semafora przez którykolwiek z czujników.
- Po wybudzeniu pobiera z kolejki strukturę z pomiarem.
- Sprawdza identyfikator czujnika:
  - Dla fotorezystora (ID 1): jeśli wartość < 1000, wysyła sygnał 0 do TaskLampka1, w przeciwnym razie wysyła 1.
  - Dla czujnika temperatury (ID 2): jeśli wartość < 14, wysyła sygnał 2 do TaskLampka1, w przeciwnym razie wysyła 3.

Konfiguracja główna
-------------------

Setup:
- Uruchamia komunikację szeregową (baud rate 9600).
- Konfiguruje tryby pinów wejścia i wyjścia.
- Alokuje pamięć i tworzy semafor binarny.
- Tworzy kolejkę zdolną pomieścić 5 struktur typu Pomiary.
- Rejestruje cztery powyższe zadania w systemie FreeRTOS z jednakowym priorytetem i stosem 2048 bajtów.

Loop:
- Pętla pozostaje pusta. Cała logika obsługiwana jest asynchronicznie przez scheduler FreeRTOS za pomocą zadań.
