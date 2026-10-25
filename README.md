# StacjaMeteo_Garni_5_INT_v2

Projekt odbiornika stacji meteo Garni z wyświetlaczem 2,7"

 BresserWeatherSensor wedlug DarKo (SQ3HTZ)

BresserWeatherSensor wedlug DarKo (SQ3HTZ)

Odczyt stacji meteo Bresser lub Garni
   Projekt wykonany dla płytek:
    - Heltec v2 - wybrać w PlatformIO: "env:heltec_wifi_lora_32_V2"
    - Heltec v3 - wybrać w PlatformIO: "env:heltec_wifi_lora_32_V3"
    - ESP-32 2432s028R (popularny CYD) - wybrać w PlatformIO: "env:jczn_2432s028r"

  Ostatnia modyfikacja: 2026.10.25

Przy pierwszym uruchomieniu urządzenie ustawia Wi-Fi:
   WIFI_SSID     = "WeatherStation";
   WIFI_PASS     = "meteo123";
 Jeśli wcześniej był wgrany i skonfigurowany program to próbuje on połączyć się do zapisanej sieci Wi_Fi
   jeśli nie udaje się to przez 60 sekund to wystawia AccesPoint Wi-Fi jak powyżej
 W Arduino IDE aby całkowicie skasować ustawienia to w ustawieniach płytki należy wybrać: "Erase All Flash Before Sketch Upload: Enable"
 Muszę jeszcze przetestować tą funkcję w PlatformIO!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  Należy w preferencjach dodać dodatkowe biblioteki (adresy URL) płytek:
      https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
  Ustawić w narzedziach płytkę: "Heltec WiFi LoRa32(v2)"

## Projekt jest kopią projektu
  https://github.com/ArturVSS/APRS-WX-station-Garni-2055/tree/main
 oraz brał inspiracje i fragmenty kodu z projektów:
  https://github.com/matthias-bs/BresserWeatherSensorReceiver
  https://github.com/tzapu/WiFiManager/blob/master/examples/Parameters/SPIFFS/AutoConnectWithFSParameters/AutoConnectWithFSParameters.ino
  https://github.com/espressif/arduino-esp32
  https://github.com/espressif/arduino-esp32/tree/master/libraries/WebServer/examples

## Obecnie działa i był testowany na płytce Heltec v2 z ekranem LCD oraz Heltec v3 z ekranem LCD
  Jego funkcje:
  - wyświetla dane na wyświetlaczu
  - łączy się z siecią Wi-Fi
  - wyświetla dane na stronie WWW (gdy połączy się z siecią Wi-FI) (na adresie IP który otrzyma z routera DHCP)
  - konfiguracje zapisuje w pliku konfiguracyjnym

  Funkcje zaimplementowane, ale do sprawdzenia:
  - MQTT


Projekt był utworzony w ArduinoIDE, ale został przekonwertowany do PlatformIO co pozwoliło na określenie konkretnych bibliotek i ich wersji.
 Do wgrania należy mieć doinstalowane odpowiednie płytki (Boards), czyli w konfiguracji (Preferences) powinna być dodana linia:
 Additional boards manager URLs = https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
  (nie wiem czy nie będą także potrzebne: https://adafruit.github.io/arduino-board-index/package_adafruit_index.json,https://arduino.esp8266.com/stable/package_esp8266com_index.json )
 Projekt działą poprawnie na płytce ESP32 CYD (ESP32 z wyświetlaczem 2,7").
  Dlatego trzeba wybracć płytkę: "ESP32-2432S028R CYD"
 Do wgrania programu potrzebne jest zmodyfikowanie ustawień pamięci wykorzystywanej przez ESP32 (Partition Scheme:),
  standardowo jest "Default 4MB witch spiffs (1.2MB APP/1.5 SPIFFS)" (oraz 1.2 MB na OTA)
  natomiast ten projekt potrzebuje więcej miejsca na aplikację
  (czcionki są wkompilowane w program żeby nie trzeba było osobno wgrywać ich na osobną partycję co by komplikowało proces wgrywania programu do ESP)
  dlatego należy wybrać Partition Scheme: "Huge APP (3MB No OTA/1MB SPIFFS)"
  (tracimy przestrzeń na funkcję OTA-która nie jest planowana, ale mamy więcej miejsca na program i ładne czcionki)

Przy pierwszym uruchomieniu (po pierwszym wgrywaniu programu do ESP) po uruchomieniu załącza się WiFiManager
  - czyli Punk dostępowy - to znaczy sieć Wi-Fi o nazwie "WeatherStation" oraz z hasłem: "meteo123" do której należy
  się zalogować i skonfigurować przynajmniej parametry lokalnej sieci Wi-Fi.
 Jak się już raz ustawi Wi-Fi lub inne parametry, to zostają one na partycji ESP32 który standardowo nie jest wymazywany
  podczas wgrywania nowych wersji programu z ArduinoIDE.

 Jeśli nie było w ESP32 skonfigurowane żadne Wi-Fi to ESP przechodzi w trybi AP na 5 minut,
  po tym czasie wyłącza całkowiecie Wi-Fi i przechodzi do wykonywania normalnego programu.
 Jeśli jest skonfigurowane Wi-Fi, ale ESP32 nie jest w jej zasięgu to przez 1 minute próbuje się połączyć z zapisaną Wi-Fi,
  jeśli w tym czasie nie połączy się to przechodzi w tryb AP - gdzie można ustawić nową Wi-Fi, ale w tym trybie jest tylko przez 5 minut,
  po czym wyłącza się i wyłącza Wi-Fi i przechodzi do pozostałej części programu (odbieranie danych z stacji meteo i czujnika BME).

 Jeśli ESP nie odbierze chociaż 1 pakietu z stacji Garni/Bresser to nie wyświetli żadnych parametrów, nawet jeśli ma lokalne dane z czujnika BME
  (ta funkcja jest do poprawienia)
  ------------------------------


## ToDo:
  - poprawić całą konfigurację żeby była brana i zapisywana z pliku
  - nie działa BME
  - zrobić config po boot gdy nie będzie pliku konfiguracyjnego
  - w panelu konfiguracji zrobić przycisk wyczyść konfigurację (przywróć ustawienia fabryczne) - funkcja która skasuje plik konfiguracyjny
  - jeśli ktos bedzie chcaił to dodanie funkcji dla APRS
  - sprawdzić czy na Heltec v2 nadal działa poprawnie
  - ustawianie w interfejsie ID stacji meteo (i odczyt danych tylko z tej stacji) - przydatne jeśli w okolicy jest więcej stacji meteo
  - możliwość ręcznego ustawiania adresu IP w interfejsie (jak chcemy żeby mieć do niego dostęp zawsze pod tym samym adresem IP)


## UWAGI:
1.
 Dwie biblioteki:
 - WeatherSensor (BresserWeatherSensor)
 - Adafruit_Sensor
 uzywaja tego samego parametru: "SENSOR_TYPE_CO2" co powodowalo konflikt
 dlatego biblioteki: Adafruit_Sensor oraz Adafruit_BME280 znajduja sie w folderze z programem i maja zmodyfikowany ten parametr

## Podlaczenie dla ESP32 CYD - BME280:
BME280    - ESP32_CYD
VCC       - +3.3V
GND       - GND
SDA       - IO17
CLK       - IO4

Podlaczenie dla ESP32 CYD - C1101:
C1101      - ESP32_CYD
VCC        - +3.3V
GND        - GND
MOSI       - IO23
SCLK (SCK) - IO18
MISO       - IO19
GDO2 (IRQ) - xxx
GDO0 (CE)  - IO21
CSN (CNS)  - IO27
-----------------------------------------------------------------------------------------------

BresserWeatherSensorBasic.ino

Example for BresserWeatherSensorReceiver -
Using getMessage() for non-blocking reception of a single data message.

 The data may be incomplete, because certain sensors need two messages to
transmit a complete data set.
 Which sensor data is received in case of multiple sensors are in range
 depends on the timing of transmitter and receiver.

 https://github.com/matthias-bs/BresserWeatherSensorReceiver


 created: 05/2022


 MIT License

 Copyright (c) 2022 Matthias Prinke

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.