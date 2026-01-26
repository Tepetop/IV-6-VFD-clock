# IV-6-VFD-clock
Simple clock based on STMF103C8T6 with IV-6 VFD lamps and DS3231 as time source.


# Projekt: Zegar VFD na lampach IV-6 (V1)

## 1. Założenia ogólne

* Wyświetlanie czasu w formacie **HH:MM**
* 4 lampy VFD **IV-6** (2 godziny, 2 minuty)
* Multipleks **1/4**, odświeżanie ≥ 200 Hz
* Obudowa jest **częścią V1**
* Projekt zamykalny – brak funkcji dodatkowych w V1

---

## 2. Sprzęt (Hardware)

### 2.1 Jednostka sterująca

* MCU: **STM32F103C8T6**
* Programowanie: SWD
* Zasilanie logiki: **3.3 V**

### 2.2 RTC

* Układ: **DS3231**
* Komunikacja: I²C
* Przerwanie: **1 Hz**
* RTC jest jedynym źródłem czasu (MCU nie liczy czasu samodzielnie)

### 2.3 Wyświetlacz VFD

* Lampy: **IV-6**
* Sterowanie:

  * Segmenty: wspólne dla wszystkich lamp
  * Siatki (anody): osobne dla każdej lampy
* Tryb pracy: multipleks

### 2.4 Sterowanie segmentów

* Ekspander I/O na **I²C**
* Wyjścia typu **open-collector / open-drain**
* Segmenty pracują jako obciążenie sinkowane

### 2.5 Sterowanie anod (siatek)

* Rejestr przesuwny (np. **74HC595**)
* Klucze: **N-MOSFET**
* Rezystor bramki + pull-down
* Praca z napięciem 25–30 V

### 2.6 Zasilanie

#### Wysokie napięcie VFD

* Transformator AC
* Prostownik + filtr
* Docelowe napięcie: **~30 V DC**
* Jeśli trafo 12 V → przetwornica boost

#### Zasilanie logiki

* AC → prostownik → filtr → **LDO 3.3 V**

#### Filament VFD

* Zasilanie **AC ~1.1–1.3 V**
* Brak zasilania DC w V1 (redukcja gradientu jasności)

---

## 3. Firmware (Software)

### 3.1 Przerwania

* **TIMx (~1 kHz)**

  * obsługa multipleksu
  * przełączanie aktywnej lampy
* **EXTI (1 Hz z DS3231)**

  * flaga odświeżenia czasu

### 3.2 Pętla główna

* Odczyt czasu z RTC
* Obsługa przycisków
* Tryb ustawiania czasu
* Aktualizacja bufora wyświetlacza

### 3.3 Tryb ustawiania czasu

* Przyciski:

  * `SET`
  * `+`
  * `–`
* Wejście/wyjście: `SET`
* Krótkie `SET`: przełączanie **godziny → minuty**
* `+ / –`: zmiana wartości
* Podczas ustawiania **czas nie biegnie**
* Po wyjściu: zapis do DS3231

---

## 4. Etapy realizacji

### Etap 1 – MCU

* Zasilanie 3.3 V
* Programowanie SWD
* Test GPIO / LED
* Skan I²C

### Etap 2 – RTC

* Podłączenie DS3231
* Obsługa przerwania 1 Hz
* Odczyt czasu

### Etap 3 – Jedna lampa

* Jedna siatka
* Segmenty na stałe
* Test napięć HV

### Etap 4 – Pełny multipleks

* 4 lampy
* Timer multipleksu
* Stabilne wyświetlanie HH:MM

### Etap 5 – Przyciski

* Debounce
* Tryb SET
* Zapis czasu

### Etap 6 – Obudowa

* Montaż lamp
* Mocowanie PCB
* Wyprowadzenie przycisków
* Wentylacja
* Ochrona przed HV

---

## 5. Czego NIE robimy w V1

* Sekundy
* Alarm
* Regulacja jasności
* Animacje
* Komunikacja bezprzewodowa

---

## 6. Roadmapa (V2 / V3 – przyszłość)

* Dwie lampy sekund
* PWM jasności
* Alarm / buzzer
* Auto-dimming nocny
* BLE / WiFi
