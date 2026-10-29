#include <Arduino.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
//  BresserWeatherSensor wedlug DarKo (SQ3HTZ)
//
// Odczyt stacji meteo Bresser lub Garni
//   Projekt wykonany dla płytek:
//    - Heltec v2 - wybrać w PlatformIO: "env:heltec_wifi_lora_32_V2"
//    - Heltec v3 - wybrać w PlatformIO: "env:heltec_wifi_lora_32_V3"
//    - ESP-32 2432s028R (popularny CYD) - wybrać w PlatformIO: "env:jczn_2432s028r"
//
//  Ostatnia modyfikacja: 2026.10.25
//
// Przy pierwszym uruchomieniu urządzenie ustawia Wi-Fi:
//   WIFI_SSID     = "WeatherStation";
//   WIFI_PASS     = "meteo123";
// Jeśli wcześniej był wgrany i skonfigurowany program to próbuje on połączyć się do zapisanej sieci Wi_Fi
//   jeśli nie udaje się to przez 60 sekund to wystawia AccesPoint Wi-Fi jak powyżej
// W Arduino IDE aby całkowicie skasować ustawienia to w ustawieniach płytki należy wybrać: "Erase All Flash Before Sketch Upload: Enable"
// Muszę jeszcze przetestować tą funkcję w PlatformIO!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//
//  Należy w preferencjach dodać dodatkowe biblioteki (adresy URL) płytek:
//      https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
//  Ustawić w narzedziach płytkę: "Heltec WiFi LoRa32(v2)"
// 
// Projekt jest kopią projektu
//  https://github.com/ArturVSS/APRS-WX-station-Garni-2055/tree/main
// oraz brał inspiracje i fragmenty kodu z projektów:
//  https://github.com/matthias-bs/BresserWeatherSensorReceiver
//  https://github.com/tzapu/WiFiManager/blob/master/examples/Parameters/SPIFFS/AutoConnectWithFSParameters/AutoConnectWithFSParameters.ino
//  https://github.com/espressif/arduino-esp32
//  https://github.com/espressif/arduino-esp32/tree/master/libraries/WebServer/examples
//
//
//
// Obecnie działa i był testowany na płytce Heltec v2 z ekranem LCD oraz Heltec v3 z ekranem LCD
//  Jego funkcje:
//  - wyświetla dane na wyświetlaczu
//  - łączy się z siecią Wi-Fi
//  - wyświetla dane na stronie WWW (gdy połączy się z siecią Wi-FI) (na adresie IP który otrzyma z routera DHCP)
//  - konfiguracje zapisuje w pliku konfiguracyjnym
//
//  Funkcje zaimplementowane, ale do sprawdzenia:
//  - MQTT
//
//
//
// Projekt był utworzony w ArduinoIDE, ale został przekonwertowany do PlatformIO co pozwoliło na określenie konkretnych bibliotek i ich wersji.
// Do wgrania należy mieć doinstalowane odpowiednie płytki (Boards), czyli w konfiguracji (Preferences) powinna być dodana linia:
// Additional boards manager URLs = https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json 
//  (nie wiem czy nie będą także potrzebne: https://adafruit.github.io/arduino-board-index/package_adafruit_index.json,https://arduino.esp8266.com/stable/package_esp8266com_index.json )
// Projekt działą poprawnie na płytce ESP32 CYD (ESP32 z wyświetlaczem 2,7").
//  Dlatego trzeba wybracć płytkę: "ESP32-2432S028R CYD"
// Do wgrania programu potrzebne jest zmodyfikowanie ustawień pamięci wykorzystywanej przez ESP32 (Partition Scheme:),
//  standardowo jest "Default 4MB witch spiffs (1.2MB APP/1.5 SPIFFS)" (oraz 1.2 MB na OTA)
//  natomiast ten projekt potrzebuje więcej miejsca na aplikację
//  (czcionki są wkompilowane w program żeby nie trzeba było osobno wgrywać ich na osobną partycję co by komplikowało proces wgrywania programu do ESP) 
//  dlatego należy wybrać Partition Scheme: "Huge APP (3MB No OTA/1MB SPIFFS)"
//  (tracimy przestrzeń na funkcję OTA-która nie jest planowana, ale mamy więcej miejsca na program i ładne czcionki)
//
// Przy pierwszym uruchomieniu (po pierwszym wgrywaniu programu do ESP) po uruchomieniu załącza się WiFiManager
//  - czyli Punk dostępowy - to znaczy sieć Wi-Fi o nazwie "WeatherStation" oraz z hasłem: "meteo123" do której należy
//  się zalogować i skonfigurować przynajmniej parametry lokalnej sieci Wi-Fi.
// Jak się już raz ustawi Wi-Fi lub inne parametry, to zostają one na partycji ESP32 który standardowo nie jest wymazywany
//  podczas wgrywania nowych wersji programu z ArduinoIDE.
//
// Jeśli nie było w ESP32 skonfigurowane żadne Wi-Fi to ESP przechodzi w trybi AP na 5 minut,
//  po tym czasie wyłącza całkowiecie Wi-Fi i przechodzi do wykonywania normalnego programu. 
// Jeśli jest skonfigurowane Wi-Fi, ale ESP32 nie jest w jej zasięgu to przez 1 minute próbuje się połączyć z zapisaną Wi-Fi,
//  jeśli w tym czasie nie połączy się to przechodzi w tryb AP - gdzie można ustawić nową Wi-Fi, ale w tym trybie jest tylko przez 5 minut,
//  po czym wyłącza się i wyłącza Wi-Fi i przechodzi do pozostałej części programu (odbieranie danych z stacji meteo i czujnika BME).
//
// Jeśli ESP nie odbierze chociaż 1 pakietu z stacji Garni/Bresser to nie wyświetli żadnych parametrów, nawet jeśli ma lokalne dane z czujnika BME
//  (ta funkcja jest do poprawienia)
//   ------------------------------
//
//
//  ToDo:
//  - poprawić całą konfigurację żeby była brana i zapisywana z pliku
//  - nie działa BME
//  - zrobić config po boot gdy nie będzie pliku konfiguracyjnego
//  - w panelu konfiguracji zrobić przycisk wyczyść konfigurację (przywróć ustawienia fabryczne) - funkcja która skasuje plik konfiguracyjny
//  - jeśli ktos bedzie chcaił to dodanie funkcji dla APRS
//  - sprawdzić czy na Heltec v2 nadal działa poprawnie
//
//  Funkcje do zrobienia (ToDo - w planach)
//  - ustawianie w interfejsie ID stacji meteo (i odczyt danych tylko z tej stacji) - przydatne jeśli w okolicy jest więcej stacji meteo
//  - możliwość ręcznego ustawiania adresu IP w interfejsie (jak chcemy żeby mieć do niego dostęp zawsze pod tym samym adresem IP)
//
//
// UWAGI:
// 1.
// Dwie biblioteki:
// - WeatherSensor (BresserWeatherSensor)
// - Adafruit_Sensor
// uzywaja tego samego parametru: "SENSOR_TYPE_CO2" co powodowalo konflikt
// dlatego biblioteki: Adafruit_Sensor oraz Adafruit_BME280 znajduja sie w folderze z programem i maja zmodyfikowany ten parametr
//
// Podlaczenie dla ESP32 CYD - BME280:
// BME280    - ESP32_CYD
// VCC       - +3.3V
// GND       - GND
// SDA       - IO17
// CLK       - IO4
//
//
// Podlaczenie dla ESP32 CYD - C1101:
// C1101      - ESP32_CYD
// VCC        - +3.3V
// GND        - GND
// MOSI       - IO23
// SCLK (SCK) - IO18
// MISO       - IO19
// GDO2 (IRQ) - xxx
// GDO0 (CE)  - IO21
// CSN (CNS)  - IO27
//
//-----------------------------------------------------------------------------------------------
//
// BresserWeatherSensorBasic.ino
//
// Example for BresserWeatherSensorReceiver - 
// Using getMessage() for non-blocking reception of a single data message.
//
// The data may be incomplete, because certain sensors need two messages to
// transmit a complete data set.
// Which sensor data is received in case of multiple sensors are in range
// depends on the timing of transmitter and receiver.  
//
// https://github.com/matthias-bs/BresserWeatherSensorReceiver
//
//
// created: 05/2022
//
//
// MIT License
//
// Copyright (c) 2022 Matthias Prinke
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// History:
//
// 20220523 Created from https://github.com/matthias-bs/Bresser5in1-CC1101
// 20220524 Moved code to class WeatherSensor
// 20220810 Changed to modified WeatherSensor class; fixed Soil Moisture Sensor Handling
// 20220815 Changed to modified WeatherSensor class; added support of multiple sensors
// 20221227 Replaced DEBUG_PRINT/DEBUG_PRINTLN by Arduino logging functions
// 20230624 Added Bresser Lightning Sensor decoder
// 20230804 Added Bresser Water Leakage Sensor decoder
// 20231023 Modified detection of Lightning Sensor
// 20231025 Added Bresser Air Quality (Particulate Matter) Sensor decoder
// 20240209 Added Leakage, Air Quality (HCHO/VOC) and CO2 Sensors
// 20240213 Added PM1.0 to Air Quality (Particulate Matter) Sensor decoder
// 20240716 Fixed output of invalid battery state with 6-in-1 decoder
// 20250127 Added Globe Thermometer Temperature (8-in-1 Weather Sensor)
//
// ToDo: 
// - 
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <Arduino.h>

#include <WeatherSensorCfg.h>
#include <WeatherSensor.h>
#include <WeatherUtils.h>
#include <RainGauge.h>
#include <Lightning.h>
#include <InitBoard.h>
//#include "src/mqtt_comm.h"


// Library Defines - Need to be defined before library import
#define FORMAT_LITTLEFS_IF_FAILED true
#define ESP_DRD_USE_LITTLEFS true
#define DOUBLERESETDETECTOR_DEBUG true

//Wi-Fi
#include <WiFi.h>
//#include <NetworkClient.h>
#include <WebServer.h>
//#include <ESPmDNS.h>
#include <LittleFS.h>
#include <PubSubClient.h>

// LCD
#if defined(HARDWARE_HELTEC_V2) || defined(HARDWARE_HELTEC_V3) || defined(ARDUINO_LILYGO_T3S3_SX1262) || defined(ARDUINO_LILYGO_T3S3_SX1276) || defined(ARDUINO_LILYGO_T3S3_LR1121)
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#elif defined(HARDWARE_CYD)
#include <TFT_eSPI.h>
#include <Adafruit_GFX.h>

// Dołączenie wkompilowanych czcionek (muszą znajdować się w folderze ze szkicem)
#include "FreeSans14pt7b.h" 
#include "FreeSans28pt7b.h"
#endif

//BME
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include <WiFiManager.h>
#include <ESP_DoubleResetDetector.h>
#include <ArduinoJson.h>


// Stop reception when data of all (max_sensors) is complete
#define RX_FLAGS (DATA_COMPLETE | DATA_ALL_SLOTS)

#define MAX_SENSORS 1
#define RX_TIMEOUT 180000 // sensor receive timeout [ms]
#define UPDATE_DELAY 30   // Delay between updates [s]

//Definicja pinow OLED i BME280
#if defined(ARDUINO_TTGO_LoRa32_V1) || \
    defined(ARDUINO_TTGO_LoRa32_V2) || \
    defined(ARDUINO_TTGO_LoRa32_v21new)
#define OLED_RESET -1
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 32    // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#elif defined(ARDUINO_LILYGO_T3S3_SX1262) || \
    defined(ARDUINO_LILYGO_T3S3_SX1276) ||   \
    defined(ARDUINO_LILYGO_T3S3_LR1121)
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#elif defined(HARDWARE_HELTEC_V2)
#define SCREEN_WIDTH 128    // Szerokość wyświetlacza w pikselach
#define SCREEN_HEIGHT 64    // Wysokość wyświetlacza w pikselach
#define OLED_RESET 16       // Pin resetowania wyświetlacza (bardzo ważny w V2!)
#define SCREEN_ADDRESS 0x3C // Standardowy adres I2C dla tego ekranu
#define I2C_SDA 12
#define I2C_SCL 13
#define OLED_SDA        4
#define OLED_SCL        15
#elif defined(HARDWARE_HELTEC_V3)
#define SCREEN_WIDTH 128    // Szerokość wyświetlacza w pikselach
#define SCREEN_HEIGHT 64    // Wysokość wyświetlacza w pikselach
#define OLED_RESET 21       // Pin resetowania wyświetlacza (bardzo ważny w V2!)
#define SCREEN_ADDRESS 0x3C // Standardowy adres I2C dla tego ekranu
#define I2C_SDA 38
#define I2C_SCL 39
#define VEXT_PIN        36
#define OLED_RST_PIN    21
#define OLED_SDA        17
#define OLED_SCL        18
#elif defined(HARDWARE_CYD)

//#define BME_SCK 1
//#define BME_MISO 3
//#define BME_MOSI 17
//#define BME_CS 13
#define I2C_SDA 17
#define I2C_SCL 4
// --- Paleta kolorów (Nowoczesny Dark Mode) ---
#define BG_COLOR       0x0841  // Bardzo ciemny szary/granatowy
#define CARD_COLOR     0x18C3  // Jaśniejszy szary dla ramek/sekcji
#define TEXT_MAIN      0xFFFF  // Biały dla głównych wartości
#define TEXT_SUB       0x9CF3  // Jasnoszary dla jednostek
#define ACCENT_BLUE    0x34BF  // Niebieski dla danych zewnętrznych
#define ACCENT_GREEN   0x4610  // Zielony dla danych domowych
#define ACCENT_YELLOW  0xF6A0  // Żółty dla słońca/czasu

// #define TFT_WIDTH  240 // ST7789 240 x 240 and 240 x 320
// #define TFT_HEIGHT 320 // ST7789 240 x 320

/*
#define ILI9341_2_DRIVER     // Alternative ILI9341 driver, see https://github.com/Bodmer/TFT_eSPI/issues/1172
#define TFT_WIDTH  240 // ST7789 240 x 240 and 240 x 320
#define TFT_HEIGHT 320 // ST7789 240 x 320
#define TFT_BL   21            // LED back-light control pin
#define TFT_BACKLIGHT_ON HIGH  // Level to turn ON back-light (HIGH or LOW)
// The hardware SPI can be mapped to any pins
#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15  // Chip select control pin
#define TFT_DC    2  // Data Command control pin
//#define TFT_RST   4  // Reset pin (could connect to RST pin)
#define TFT_RST  -1  // Set TFT_RST to -1 if display RESET is connected to ESP32 board RST
#define TOUCH_CS 33     // Chip select pin (T_CS) of touch screen
// Define the fonts that are to be used here
#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
#define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:-.
#define LOAD_FONT8  // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
//#define LOAD_FONT8N // Font 8. Alternative to Font 8 above, slightly narrower, so 3 digits fit a 160 pixel TFT
#define LOAD_GFXFF  // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts
#define SPI_FREQUENCY  27000000
#define SPI_READ_FREQUENCY  20000000  // Optional reduced SPI frequency for reading TFT
#define SPI_TOUCH_FREQUENCY  2500000  // The XPT2046 requires a lower SPI clock rate of 2.5MHz so we define that here
#define USE_HSPI_PORT
*/
#else
#pragma message("Board not supported yet!")
#define BME_SCK 1
#define BME_MISO 3
#define BME_MOSI 17
#define BME_CS 13
#endif

//Definicja pinow BME280

#define SEALEVELPRESSURE_HPA (1013.25)  //ciśnienie poziomu moża

// ==== UWAGA - jak masz wiele nadajników to pamiętaj ustawić odpowiednie ID w pliku "src/WeatherSensorCfg.h" w polu:
//   #define SENSOR_IDS_INC { 0xF805 }

#if defined(HARDWARE_HELTEC_V2) || defined(HARDWARE_HELTEC_V3) || defined(ARDUINO_LILYGO_T3S3_SX1262) || defined(ARDUINO_LILYGO_T3S3_SX1276) || defined(ARDUINO_LILYGO_T3S3_LR1121)
// 16x16 thermometer icon
const unsigned char thermo_icon[] PROGMEM = {
  0x06,0x00, 0x09,0x00, 0x09,0x00, 0x09,0x00,
  0x09,0x00, 0x0F,0x00, 0x0F,0x00, 0x0F,0x00,
  0x1F,0x80, 0x3F,0xC0, 0x3F,0xC0, 0x1F,0x80,
  0x1F,0x80, 0x3F,0xC0, 0x1F,0x80, 0x00,0x00
};
#endif

// mark parameters not used in example
#define UNUSED __attribute__((unused))

// TRACE output simplified, can be deactivated here
#define TRACE(...) Serial.printf(__VA_ARGS__)

// name of the server. You reach it using http://webserver
#define HOSTNAME "StacjaMeteoGarni"

// local time zone definition (Berlin)
#define TIMEZONE "CET-1CEST,M3.5.0,M10.5.0/3"

//WWW
WebServer server(80);

//MQTT
WiFiClient       espClient;
PubSubClient     mqttClient(espClient);

// Heltec LCD / LILYGO LCD
#if defined(HARDWARE_HELTEC_V2) || defined(HARDWARE_HELTEC_V3) || defined(ARDUINO_LILYGO_T3S3_SX1262) || defined(ARDUINO_LILYGO_T3S3_SX1276) || defined(ARDUINO_LILYGO_T3S3_LR1121)
TwoWire twi = TwoWire(1);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &twi, OLED_RESET);
#elif defined(HARDWARE_CYD)
TFT_eSPI tft = TFT_eSPI();
#endif

// Bresser - 868 MHz Reciver (SX1276 / CC1101 / inne obsługiwane przez bibliotękę BresserWeatherSensor)
WeatherSensor ws;


Adafruit_BME280 bme;
bool bme_available = false;

// ==========================================================================
#define wxSerial Serial1
// czas co jaki jest wysyłana ramka na UART
#define UART_INTERVAL_MS 5000UL

// czas co jakie są odświerzane wartosci na wyswietlaczu
#define LCD_INTERVAL_MS 10000UL

// ==========================================================================
// --- KONFIGURACJA APRS I WIFI
String WIFI_SSID     = "WeatherStation";
String WIFI_PASS     = "meteo123";
String APRS_CALLSIGN = "SP3XXX-10";
String APRS_PASSCODE = "1234567";
double SITE_LAT      = 52.4085;
double SITE_LON      = 16.9341;
float  SITE_ALT      = 84.0;
int    UTC_OFFSET    = 1;       // przesunięcie względem UTC w godzinach, np. +1 = CET, +2 = CEST
// ==========================================================================

// ==========================================================================
// --- KONFIGURACJA MQTT (parametry ustawić po połączeniu się z Wi-Fi przez panel konfiguracji)
String   MQTT_HOST    = "192.168.1.2";
uint16_t MQTT_PORT    = 1883;
String   MQTT_USER    = "user";
String   MQTT_PASS    = "password";
String   MQTT_TOPIC   = "wx/StacjaMeteo";
bool     MQTT_ENABLED = false;
// ==========================================================================

#define SLEEP_INTERVAL 300000 // sleep interval [ms]

static const char* APRS_HOST = "rotate.aprs2.net";
static const uint16_t APRS_PORT = 14580;
unsigned long REPORT_INTERVAL_MS = 15UL * 60UL * 1000UL;

WiFiManager wifiManager;

//------------------------- parametry pogody--------------------
float wind_speed_sum = 0.0;
int   wind_sample_count = 0;
float wind_gust_max_period = 0.0;

unsigned long last_uart_send = 0;

struct WeatherData {
  float   temp_c = NAN;
  uint8_t humidity = 0;
  float   wind_dir = NAN;
  float   wind_speed = NAN;
  float   wind_gust = NAN;
  float   rain_total_mm = NAN;
  float   uv_index = NAN;
  float   light_klx = NAN;
  int     radio_rssi = -100;
  bool    battery_ok = true;
  bool    valid_data = false;
  float   bme_temp = NAN;
  float   bme_humidity = NAN;
  float   bme_pressure = NAN;
  float   bme_qnh = NAN;
  unsigned long last_update = 0;
} current_wx;

struct RainHistory { float total_mm; bool valid; };
RainHistory rain_buffer[4];
uint8_t rain_idx = 0;
unsigned long last_report_time = 0;

unsigned long last_LCD_refresh = 0;  //zmienna do obliczania kiedy ma następny raz odświerzyć ekran

#ifndef SECRETS
const char ssid[] = "WiFiSSID";
const char pass[] = "WiFiPassword";

#define APPEND_CHIP_ID

// define your default values here, if there are different values in config.json, they are overwritten.
char mqtt_host[55];
char mqtt_port[6] = "1883";
char mqtt_user[21] = "";
char mqtt_pass[21] = "";

#ifdef CHECK_CA_ROOT
static const char digicert[] PROGMEM = R"EOF(
    -----BEGIN CERTIFICATE-----
    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    -----END CERTIFICATE-----
    )EOF";
#endif

#ifdef CHECK_PUB_KEY
// Extracted by: openssl x509 -pubkey -noout -in fullchain.pem
static const char pubkey[] PROGMEM = R"KEY(
    -----BEGIN PUBLIC KEY-----
    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
    xxxxxxxx
    -----END PUBLIC KEY-----
    )KEY";
#endif
#endif

DoubleResetDetector *drd;
String Hostname = String(HOSTNAME);

// flag for saving data
bool shouldSaveConfig = false;

// flag for forcing WiFiManager re-config
bool forceConfig = false;


/*!
 * \brief Callback notifying us of the need to save config
 */
void saveConfigCallback()
{
    Serial.println("Should save config");
    shouldSaveConfig = true;
}


// --- ZAPIS/ODCZYT KONFIGURACJI -------------------------------------------
void saveConfig() {
  File f = LittleFS.open("/config.json", "w");

  if (!f) {
    Serial.println(F("Błąd otwarcia pliku do zapisu!"));
    return;
  }

  WiFi.begin(WIFI_SSID.c_str(), WIFI_PASS.c_str());
  Serial.println("Nowe dane WiFi zostały zapisane w NVS.");

  JsonDocument doc; // Rezerwacja miejsca w pamięci
  //StaticJsonDocument<1024> doc; // Rezerwacja miejsca w pamięci

  doc["aprs_call"] = APRS_CALLSIGN;
  doc["aprs_pass"] = APRS_PASSCODE;
  doc["lat"] = SITE_LAT;
  doc["lon"] = SITE_LON;
  doc["alt"] = SITE_ALT;
  doc["utc"] = UTC_OFFSET;
  doc["interval"] = REPORT_INTERVAL_MS / 60000; // Zapisujemy w minutach
  doc["mqtt_host"] = MQTT_HOST;
  doc["mqtt_port"] = MQTT_PORT;
  doc["mqtt_user"] = MQTT_USER;
  doc["mqtt_pass"] = MQTT_PASS;
  doc["mqtt_topic"] = MQTT_TOPIC;
  doc["mqtt_en"] = MQTT_ENABLED;

  if (serializeJson(doc, f) == 0) {
    Serial.println(F("Błąd zapisu JSON!"));
  }
  f.close();
  Serial.println(F("[CFG] Zapisano w formacie JSON"));

}

void loadConfig() {
  if (LittleFS.exists("/config.json")) {
    File f = LittleFS.open("/config.json", "r");
    if (f) {
      JsonDocument doc;
//      StaticJsonDocument<1024> doc;
      DeserializationError error = deserializeJson(doc, f);

      if (!error) {
        // Używamy operatora || aby zachować wartości domyślne w razie braku klucza
        APRS_CALLSIGN = doc["aprs_call"] | APRS_CALLSIGN;
        APRS_PASSCODE = doc["aprs_pass"] | APRS_PASSCODE;
        SITE_LAT      = doc["lat"] | SITE_LAT;
        SITE_LON      = doc["lon"] | SITE_LON;
        SITE_ALT      = doc["alt"] | SITE_ALT;
        UTC_OFFSET    = doc["utc"] | UTC_OFFSET;
        REPORT_INTERVAL_MS = (doc["interval"] | (REPORT_INTERVAL_MS/60000)) * 60000UL;
        MQTT_HOST     = doc["mqtt_host"] | MQTT_HOST;
        MQTT_PORT     = doc["mqtt_port"] | MQTT_PORT;
        MQTT_USER     = doc["mqtt_user"] | MQTT_USER;
        MQTT_PASS     = doc["mqtt_pass"] | MQTT_PASS;
        MQTT_TOPIC    = doc["mqtt_topic"] | MQTT_TOPIC;
        MQTT_ENABLED  = doc["mqtt_en"] | MQTT_ENABLED;
        
        Serial.println(F("[CFG] Wczytano JSON"));
      }      

      f.close();
    }
  } else {
    MQTT_TOPIC = "wx/" + APRS_CALLSIGN;
  }
}


// --- PRZELICZENIE QNH ----------------------------------------------------
float calc_qnh(float pressure_hpa, float temp_c, float altitude_m) {
  if (isnan(pressure_hpa) || isnan(temp_c) || altitude_m <= 0.0) return pressure_hpa;
  float t_kelvin = temp_c + 273.15;
  return pressure_hpa * pow(
    1.0 - (0.0065 * altitude_m) / (t_kelvin + 0.0065 * altitude_m), -5.257);
}


// --- CZAS LOKALNY z uwzględnieniem UTC_OFFSET ----------------------------
// Zwraca time_t przesunięty o UTC_OFFSET godzin
time_t local_time() {
  return time(nullptr) + (time_t)(UTC_OFFSET * 3600);
}

String get_time_hhmm() {
  time_t t = local_time();
  if (t < 100000) return "NTP";
  struct tm* tm_info = gmtime(&t);   // gmtime bo już ręcznie dodaliśmy offset
  char buf[10];
  snprintf(buf, sizeof(buf), "%02d:%02d", tm_info->tm_hour, tm_info->tm_min);
  return String(buf);
}

String get_date_ddmmyyyy() {
  time_t t = local_time();
  if (t < 100000) return "NTP";
  struct tm* tm_info = gmtime(&t);
  char buf[14];
  snprintf(buf, sizeof(buf), "%02d-%02d-%04d",
           tm_info->tm_mday, tm_info->tm_mon + 1, tm_info->tm_year + 1900);
  return String(buf);
}

// Czytelny label strefy czasowej, np. "UTC+1" / "UTC-5" / "UTC"
String get_tz_label() {
  if (UTC_OFFSET == 0) return "UTC";
  if (UTC_OFFSET > 0)  return "UTC+" + String(UTC_OFFSET);
  return "UTC" + String(UTC_OFFSET);   // String(-5) = "-5"
}


// --- UART WX -------------------------------------------------------------
// Format: WX,HH:MM,DD-MM-YYYY,temp,hum,wdir,wspd,wgst,rain,uv,lux,rssi,bat,btemp,bhum,bpress,qnh
String buildUartString() {
  String s = "WX,";
  s += get_time_hhmm();                                                                 s += ",";
  s += get_date_ddmmyyyy();                                                             s += ",";
  s += !isnan(current_wx.temp_c)        ? String(current_wx.temp_c, 1)        : "NAN"; s += ",";
  s += current_wx.humidity > 0          ? String(current_wx.humidity)          : "NAN"; s += ",";
  s += !isnan(current_wx.wind_dir)      ? String(current_wx.wind_dir, 1)       : "NAN"; s += ",";
  s += !isnan(current_wx.wind_speed)    ? String(current_wx.wind_speed, 1)     : "NAN"; s += ",";
  s += !isnan(current_wx.wind_gust)     ? String(current_wx.wind_gust, 1)      : "NAN"; s += ",";
  s += !isnan(current_wx.rain_total_mm) ? String(current_wx.rain_total_mm, 1)  : "NAN"; s += ",";
  s += !isnan(current_wx.uv_index)      ? String(current_wx.uv_index, 1)       : "NAN"; s += ",";
  s += !isnan(current_wx.light_klx)     ? String(current_wx.light_klx, 1)      : "NAN"; s += ",";
  s += String(current_wx.radio_rssi);                                                    s += ",";
  s += current_wx.battery_ok            ? "1"                                   : "0";  s += ",";
  s += !isnan(current_wx.bme_temp)      ? String(current_wx.bme_temp, 1)       : "NAN"; s += ",";
  s += !isnan(current_wx.bme_humidity)  ? String(current_wx.bme_humidity, 1)   : "NAN"; s += ",";
  s += !isnan(current_wx.bme_pressure)  ? String(current_wx.bme_pressure, 1)   : "NAN"; s += ",";
  s += !isnan(current_wx.bme_qnh)       ? String(current_wx.bme_qnh, 1)        : "NAN";
  return s;
}

void sendUartWX() {
  wxSerial.println(buildUartString());
}


// --- KONWERSJE -----------------------------------------------------------
int c_to_f(float c) { return (int)lround(c * 1.8 + 32); }
int ms_to_mph(float ms) { return (int)lround(ms * 2.23694); }
int mm_to_hin(float mm) { return (int)lround(mm * 3.93701); }

String format_lat(double lat) {
  char b[20]; char h = (lat >= 0) ? 'N' : 'S'; lat = fabs(lat);
  int d = (int)lat; double m = (lat - d) * 60.0;
  snprintf(b, sizeof(b), "%02d%05.2f%c", d, m, h);
  return String(b);
}

String format_lon(double lon) {
  char b[20]; char h = (lon >= 0) ? 'E' : 'W'; lon = fabs(lon);
  int d = (int)lon; double m = (lon - d) * 60.0;
  snprintf(b, sizeof(b), "%03d%05.2f%c", d, m, h);
  return String(b);
}

String p3(int v) {
  char b[5]; snprintf(b, sizeof(b), "%03d", (v<0?0:(v>999?999:v))); return String(b);
}

// APRS timestamp zawsze UTC (bez offsetu!)
String get_timestamp_utc() {
  time_t now = time(nullptr);
  struct tm* t = gmtime(&now);
  char buff[10];
  snprintf(buff, sizeof(buff), "%02d%02d%02dz", t->tm_mday, t->tm_hour, t->tm_min);
  return String(buff);
}


// --- MQTT ----------------------------------------------------------------
void mqttReconnect() {
  if (!MQTT_ENABLED) return;
  if (!WiFi.isConnected()) return;
  if (mqttClient.connected()) return;
  Serial.print(F("[MQTT] Laczenie... "));

#if defined(HARDWARE_HELTEC_V2) || (HARDWARE_HELTEC_V3) || defined(ARDUINO_LILYGO_T3S3_SX1262) || defined(ARDUINO_LILYGO_T3S3_SX1276) || defined(ARDUINO_LILYGO_T3S3_LR1121) || defined(HARDWARE_CYD)
    String clientId = "WX-" + APRS_CALLSIGN + "-" + String(ESP.getEfuseMac(), HEX);
#elif defined(ESP8266)
    String clientId = "WX-" + APRS_CALLSIGN + "-" + String(ESP.getChipId(), HEX);
#endif
  bool ok = MQTT_USER.length() > 0
    ? mqttClient.connect(clientId.c_str(), MQTT_USER.c_str(), MQTT_PASS.c_str())
    : mqttClient.connect(clientId.c_str());
  Serial.println(ok ? F("OK") : F("FAIL"));
  if (!ok) { Serial.print(F("rc=")); Serial.println(mqttClient.state()); }
}

void mqttPublishWX() {
  if (!MQTT_ENABLED) return;
  if (!mqttClient.connected()) mqttReconnect();
  if (!mqttClient.connected()) return;

  String p = "{";
  p += "\"call\":\""  + APRS_CALLSIGN + "\",";
  p += "\"time\":\""  + get_time_hhmm() + "\",";
  p += "\"date\":\""  + get_date_ddmmyyyy() + "\",";
  p += "\"tz\":\""    + get_tz_label() + "\",";
  p += "\"lat\":"     + String(SITE_LAT, 6) + ",";
  p += "\"lon\":"     + String(SITE_LON, 6) + ",";
  p += "\"alt\":"     + String(SITE_ALT, 1) + ",";
  p += "\"temp\":"    + (isnan(current_wx.temp_c)        ? String("null") : String(current_wx.temp_c, 1))        + ",";
  p += "\"hum\":"     + (current_wx.humidity > 0         ? String(current_wx.humidity) : String("null"))         + ",";
  p += "\"wdir\":"    + (isnan(current_wx.wind_dir)      ? String("null") : String(current_wx.wind_dir, 1))      + ",";
  p += "\"wspd\":"    + (isnan(current_wx.wind_speed)    ? String("null") : String(current_wx.wind_speed, 1))    + ",";
  p += "\"wgst\":"    + (isnan(current_wx.wind_gust)     ? String("null") : String(current_wx.wind_gust, 1))     + ",";
  p += "\"rain\":"    + (isnan(current_wx.rain_total_mm) ? String("null") : String(current_wx.rain_total_mm, 1)) + ",";
  p += "\"uv\":"      + (isnan(current_wx.uv_index)      ? String("null") : String(current_wx.uv_index, 1))      + ",";
  p += "\"light\":"   + (isnan(current_wx.light_klx)     ? String("null") : String(current_wx.light_klx, 1))     + ",";
  p += "\"rssi\":"    + String(current_wx.radio_rssi) + ",";
  p += "\"bat\":"     + String(current_wx.battery_ok ? "true" : "false") + ",";
  p += "\"btemp\":"   + (isnan(current_wx.bme_temp)      ? String("null") : String(current_wx.bme_temp, 1))      + ",";
  p += "\"bhum\":"    + (isnan(current_wx.bme_humidity)  ? String("null") : String(current_wx.bme_humidity, 1))  + ",";
  p += "\"bpress\":"  + (isnan(current_wx.bme_pressure)  ? String("null") : String(current_wx.bme_pressure, 1))  + ",";
  p += "\"qnh\":"     + (isnan(current_wx.bme_qnh)       ? String("null") : String(current_wx.bme_qnh, 1))       + ",";
  p += "\"unix\":"    + String(time(nullptr)) + ",";
  p += "\"uptime\":"  + String(millis() / 1000);
  p += "}";

  bool ok = mqttClient.publish(MQTT_TOPIC.c_str(), p.c_str(), true);
  Serial.print(F("[MQTT] ")); Serial.print(MQTT_TOPIC);
  Serial.println(ok ? F(" -> OK") : F(" -> FAIL"));
}


// --- APRS ----------------------------------------------------------------
void send_aprs() {
  Serial.println(F("\n[APRS] Wysylanie..."));
  float avg_w = (wind_sample_count > 0) ? (wind_speed_sum / wind_sample_count) : 0.0;

  int r1h = 0;
  if (!isnan(current_wx.rain_total_mm)) rain_buffer[rain_idx] = {current_wx.rain_total_mm, true};
  uint8_t oid = (rain_idx + 1) % 4;
  if (rain_buffer[rain_idx].valid && rain_buffer[oid].valid) {
    float d = rain_buffer[rain_idx].total_mm - rain_buffer[oid].total_mm;
    r1h = mm_to_hin(d < 0 ? 0 : d);
  }
  rain_idx = (rain_idx + 1) % 4;

  int baro = 0;
  if (bme_available) {
    float p = bme.readPressure();
    if (!isnan(p) && p > 80000.0) {
      current_wx.bme_pressure = p / 100.0;
      baro = (int)(p / 10.0);
    }
  }

  int lum_wm2 = 0;
  if (!isnan(current_wx.light_klx)) lum_wm2 = (int)(current_wx.light_klx * 7.9);

  // APRS zawsze UTC!
  String body = "@" + get_timestamp_utc() + format_lat(SITE_LAT) + "/" + format_lon(SITE_LON) + "_";
  int wd = isnan(current_wx.wind_dir) ? 0 : (int)current_wx.wind_dir;
  body += p3(wd) + "/" + p3(ms_to_mph(avg_w));
  body += "g" + p3(ms_to_mph(wind_gust_max_period));
  body += "t" + p3(c_to_f(current_wx.temp_c));
  if (r1h > 0) body += "r" + p3(r1h);
  if (current_wx.humidity > 0) {
    char hb[5]; snprintf(hb, sizeof(hb), "h%02d", (int)current_wx.humidity); body += hb;
  }
  if (baro > 0) {
    char bb[10]; snprintf(bb, sizeof(bb), "b%05d", baro); body += bb;
  }
  if (lum_wm2 > 0) {
    lum_wm2 = min(lum_wm2, 999);
    char lb[6]; snprintf(lb, sizeof(lb), "L%03d", lum_wm2); body += lb;
  }

  String comment = " GarniWX";
  comment += " Sig:" + String(current_wx.radio_rssi) + "dBm";
  if (!isnan(current_wx.uv_index)) comment += " UV:" + String(current_wx.uv_index, 1);
  comment += current_wx.battery_ok ? " Bat:OK" : " Bat:LOW";

  String packet = APRS_CALLSIGN + ">APRS,TCPIP*:" + body + comment;

  WiFiClient cl;
  if (cl.connect(APRS_HOST, APRS_PORT)) {
    cl.printf("user %s pass %s vers RodosBME 1.9\n",
              APRS_CALLSIGN.c_str(), APRS_PASSCODE.c_str());
    delay(200);
    cl.println(packet);
    delay(500);
    cl.stop();
    Serial.println("-> " + packet);
    wind_speed_sum = 0; wind_sample_count = 0; wind_gust_max_period = 0;
  } else {
    Serial.println(F("[APRS] Blad polaczenia!"));
  }
}


//---wyglad strony WWW-------------------------------------------------
void handleRoot() {
  String html = F("<!DOCTYPE html><html><head><meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<title>Stacja Pogodowa APRS</title>"
    "<style>"
    "body{font-family:Arial,sans-serif;margin:0;padding:20px;background:#000;color:#fff}"
    ".container{display:flex;gap:20px;max-width:1400px;margin:0 auto;flex-wrap:wrap}"
    ".column{flex:1;min-width:300px;background:#1a1a1a;padding:20px;border-radius:8px;"
    "box-shadow:0 2px 8px rgba(255,255,255,0.1);border:1px solid #333}"
    "h1{color:#00aaff;margin-top:0;font-size:24px;text-shadow:0 0 10px rgba(0,170,255,0.5)}"
    "h2{color:#00aaff;border-bottom:2px solid #00aaff;padding-bottom:8px;font-size:18px}"
    ".data-row{display:flex;justify-content:space-between;padding:8px 0;border-bottom:1px solid #333}"
    ".label{font-weight:bold;color:#aaa}"
    ".value{color:#fff}"
    ".value-highlight{color:#00ffcc;font-weight:bold;text-shadow:0 0 6px rgba(0,255,200,0.5)}"
    ".status-ok{color:#0f0;font-weight:bold;text-shadow:0 0 5px rgba(0,255,0,0.5)}"
    ".status-warn{color:#ff0;font-weight:bold;text-shadow:0 0 5px rgba(255,255,0,0.5)}"
    "input,select{width:100%;padding:8px;margin:5px 0 15px 0;border:1px solid #444;"
    "border-radius:4px;box-sizing:border-box;background:#222;color:#fff}"
    "label{font-weight:bold;color:#aaa;display:block}"
    "button{background:#00aaff;color:#000;padding:12px 24px;border:none;border-radius:4px;"
    "cursor:pointer;font-size:16px;width:100%;font-weight:bold}"
    "button:hover{background:#0088cc;box-shadow:0 0 15px rgba(0,170,255,0.6)}"
    ".info{background:#2a2a2a;padding:10px;border-radius:4px;margin:10px 0;"
    "font-size:14px;border:1px solid #444}"
    "a{color:#00aaff;text-decoration:none;font-weight:bold}"
    "a:hover{color:#0088cc;text-decoration:underline}"
    ".aprs-link{background:#2a2a2a;padding:12px;border-radius:4px;margin:0 0 15px 0;"
    "text-align:center;border:1px solid #00aaff}"
    ".section-title{margin-top:20px}"
    ".uart-box{background:#111;border:1px solid #00aaff;border-radius:4px;padding:10px;"
    "font-family:monospace;font-size:12px;color:#00ffcc;margin-top:10px;word-break:break-all}"
    ".pin-table{width:100%;border-collapse:collapse;font-size:13px;margin-top:8px}"
    ".pin-table td{padding:4px 8px;border:1px solid #333}"
    ".pin-table tr:nth-child(even){background:#222}"
    ".pin-ok{color:#0f0}"
    ".tz-badge{display:inline-block;background:#003355;border:1px solid #00aaff;"
    "border-radius:4px;padding:2px 8px;font-size:13px;color:#00aaff;margin-left:8px}"
    "</style>"
    "<script>"
    "setInterval(function(){fetch('/data').then(r=>r.json()).then(d=>{"
    "document.getElementById('temp').innerText=d.temp;"
    "document.getElementById('hum').innerText=d.hum;"
    "document.getElementById('wdir').innerText=d.wdir;"
    "document.getElementById('wspd').innerText=d.wspd;"
    "document.getElementById('wgst').innerText=d.wgst;"
    "document.getElementById('rain').innerText=d.rain;"
    "document.getElementById('uv').innerText=d.uv;"
    "document.getElementById('lux').innerText=d.lux;"
    "document.getElementById('rssi').innerText=d.rssi;"
    "document.getElementById('bat').innerText=d.bat;"
    "document.getElementById('bat').className=d.bat=='OK'?'value status-ok':'value status-warn';"
    "document.getElementById('btemp').innerText=d.btemp;"
    "document.getElementById('bhum').innerText=d.bhum;"
    "document.getElementById('bpress').innerText=d.bpress;"
    "document.getElementById('bqnh').innerText=d.bqnh;"
    "document.getElementById('wxtime').innerText=d.wxtime;"
    "document.getElementById('wxdate').innerText=d.wxdate;"
    "document.getElementById('wxtzbadge').innerText=d.tz;"
    "document.getElementById('upd').innerText=d.upd;"
    "document.getElementById('uart_preview').innerText=d.uart;"
    "});},2000);"
    "</script>"
    "</head><body>"
    "<h1>🌤️ Stacja Pogodowa APRS - ");
  html += APRS_CALLSIGN;
  html += F("</h1><div class='container'><div class='column'>");

  html += F("<div class='aprs-link'>📍 <a href='https://aprs.fi/");
  html += APRS_CALLSIGN;
  html += F("' target='_blank'>Zobacz stację na APRS.fi</a></div>");

  // Czas lokalny z badgem strefy
  html += F("<div class='data-row'><span class='label'>⏰ Czas lokalny:</span>"
            "<span class='value-highlight' id='wxtime'>");
  html += get_time_hhmm();
  html += F("</span><span class='tz-badge' id='wxtzbadge'>");
  html += get_tz_label();
  html += F("</span></div>");

  html += F("<div class='data-row'><span class='label'>📅 Data:</span>"
            "<span class='value' id='wxdate'>");
  html += get_date_ddmmyyyy();
  html += F("</span></div>");

  html += F("<h2>📡 Dane z Radia (Bresser 7-in-1)</h2>");

  html += F("<div class='data-row'><span class='label'>Temperatura:</span>"
            "<span class='value' id='temp'>");
  html += !isnan(current_wx.temp_c) ? String(current_wx.temp_c, 1) + " °C" : "---";
  html += F("</span></div>");

  html += F("<div class='data-row'><span class='label'>Wilgotność:</span>"
            "<span class='value' id='hum'>");
  html += current_wx.humidity > 0 ? String(current_wx.humidity) + " %" : "---";
  html += F("</span></div>");

  html += F("<div class='data-row'><span class='label'>Kierunek wiatru:</span>"
            "<span class='value' id='wdir'>");
  html += !isnan(current_wx.wind_dir) ? String((int)current_wx.wind_dir) + " °" : "---";
  html += F("</span></div>");

  html += F("<div class='data-row'><span class='label'>Prędkość wiatru:</span>"
            "<span class='value' id='wspd'>");
  html += !isnan(current_wx.wind_speed) ? String(current_wx.wind_speed, 1) + " m/s" : "---";
  html += F("</span></div>");

  html += F("<div class='data-row'><span class='label'>Porywy wiatru:</span>"
            "<span class='value' id='wgst'>");
  html += !isnan(current_wx.wind_gust) ? String(current_wx.wind_gust, 1) + " m/s" : "---";
  html += F("</span></div>");

  html += F("<div class='data-row'><span class='label'>Opady (suma):</span>"
            "<span class='value' id='rain'>");
  html += !isnan(current_wx.rain_total_mm) ? String(current_wx.rain_total_mm, 1) + " mm" : "---";
  html += F("</span></div>");

  html += F("<div class='data-row'><span class='label'>Indeks UV:</span>"
            "<span class='value' id='uv'>");
  html += !isnan(current_wx.uv_index) ? String(current_wx.uv_index, 1) : "---";
  html += F("</span></div>");

  html += F("<div class='data-row'><span class='label'>Światło:</span>"
            "<span class='value' id='lux'>");
  html += !isnan(current_wx.light_klx) ? String(current_wx.light_klx, 1) + " klx" : "---";
  html += F("</span></div>");

  html += F("<div class='data-row'><span class='label'>Siła sygnału CC1101:</span>"
            "<span class='value' id='rssi'>");
  html += String(current_wx.radio_rssi) + " dBm";
  html += F("</span></div>");

  html += F("<div class='data-row'><span class='label'>Bateria stacji:</span>"
            "<span class='value' id='bat' class='");
  html += current_wx.battery_ok ? F("status-ok'>OK") : F("status-warn'>LOW");
  html += F("</span></div>");

  html += F("<h2 class='section-title'>🌡️ Dane z BME280 (GPIO0/GPIO3)</h2>");

  html += F("<div class='data-row'><span class='label'>Temperatura:</span>"
            "<span class='value' id='btemp'>");
  html += !isnan(current_wx.bme_temp) ? String(current_wx.bme_temp, 1) + " °C" : "---";
  html += F("</span></div>");

  html += F("<div class='data-row'><span class='label'>Wilgotność:</span>"
            "<span class='value' id='bhum'>");
  html += !isnan(current_wx.bme_humidity) ? String(current_wx.bme_humidity, 1) + " %" : "---";
  html += F("</span></div>");

  html += F("<div class='data-row'><span class='label'>Ciśnienie absolutne:</span>"
            "<span class='value' id='bpress'>");
  html += !isnan(current_wx.bme_pressure) ? String(current_wx.bme_pressure, 1) + " hPa" : "---";
  html += F("</span></div>");

  html += F("<div class='data-row'><span class='label'>Ciśnienie QNH (MSL):</span>"
            "<span class='value-highlight' id='bqnh'>");
  html += !isnan(current_wx.bme_qnh) ? String(current_wx.bme_qnh, 1) + " hPa" : "---";
  html += F("</span></div>");

  html += F("<div class='info'>Ostatnia aktualizacja: <span id='upd'>");
  html += current_wx.last_update > 0
    ? String((millis() - current_wx.last_update) / 1000) + " s temu" : "brak danych";
  html += F("</span></div>");

  html += F("<h2 class='section-title'>🔌 UART TX (GPIO2/D4, 9600, co 5s)</h2>"
            "<div class='uart-box' id='uart_preview'>oczekiwanie na dane...</div>"
            "</div>");

  // ---- PRAWA KOLUMNA ----
  html += F("<div class='column'><h2>⚙️ Konfiguracja Stacji</h2>"
    "<form action='/save' method='POST'>"
    "<label>Nazwa WiFi (SSID):</label><input name='ssid' value='");
  html += WiFi.SSID();
  html += F("'><label>Hasło WiFi:</label><input name='pass' type='password' value='");
  html += WiFi.psk();
  html += F("'><label>Znak wywoławczy APRS:</label><input name='call' value='");
  html += APRS_CALLSIGN;
  html += F("'><label>Passcode APRS:</label><input name='passcode' value='");
  html += APRS_PASSCODE;
  html += F("'><label>Szerokość geograficzna:</label>"
    "<input name='lat' type='number' step='0.0001' value='");
  html += String(SITE_LAT, 6);
  html += F("'><label>Długość geograficzna:</label>"
    "<input name='lon' type='number' step='0.0001' value='");
  html += String(SITE_LON, 6);
  html += F("'><label>Wysokość stacji n.p.m. [m]:</label>"
    "<input name='alt' type='number' step='0.1' value='");
  html += String(SITE_ALT, 1);
  html += F("'><label>Interwał raportów APRS (min):</label>"
    "<input name='interval' type='number' min='1' value='");
  html += String(REPORT_INTERVAL_MS / 60000);

  // ---- STREFA CZASOWA ----
  html += F("'><h2 class='section-title'>🕐 Strefa czasowa (UART)</h2>"
    "<label>Przesunięcie względem UTC (godziny, -12 do +14):</label>"
    "<select name='utc_offset'>");
  for (int i = -12; i <= 14; i++) {
    html += "<option value='" + String(i) + "'";
    if (i == UTC_OFFSET) html += " selected";
    if (i == 0)          html += ">UTC";
    else if (i > 0)      html += ">UTC+" + String(i);
    else                 html += ">UTC" + String(i);
    html += "</option>";
  }
  html += F("</select>"
    "<div class='info' style='margin-top:-10px;margin-bottom:10px;font-size:13px'>"
    "⚠️ Czas APRS zawsze wysyłany jako UTC (standard APRS).<br>"
    "Strefa czasowa dotyczy tylko ramek UART i wyświetlania na stronie.</div>");

  // ---- MQTT ----
  html += F("<h2 class='section-title'>📡 MQTT 3.1.1</h2>"
    "<label>MQTT włączony:</label><select name='mqtt_en'>");
  html += MQTT_ENABLED
    ? F("<option value='1' selected>Tak</option><option value='0'>Nie</option>")
    : F("<option value='1'>Tak</option><option value='0' selected>Nie</option>");
  html += F("</select>"
    "<label>MQTT Host:</label><input name='mqtt_host' value='");
  html += MQTT_HOST;
  html += F("'><label>MQTT Port:</label>"
    "<input name='mqtt_port' type='number' min='1' max='65535' value='");
  html += String(MQTT_PORT);
  html += F("'><label>MQTT User:</label><input name='mqtt_user' value='");
  html += MQTT_USER;
  html += F("'><label>MQTT Hasło:</label>"
    "<input name='mqtt_pass' type='password' value='");
  html += MQTT_PASS;
  html += F("'><label>MQTT Topic (JSON):</label><input name='mqtt_topic' value='");
  html += MQTT_TOPIC;
  html += F("'><button type='submit'>💾 Zapisz Ustawienia</button></form>");

  html += F("<h2>⚙️ Reset Stacji</h2> <form action='/resetconfig' method='POST'>");
  html += F("<button type='submit'>💾 Przywróć urządzenie do ustawień fabrycznych</button></form>");

  html += F("<div class='info' style='margin-top:15px'>"
    "IP: "); html += WiFi.localIP().toString();
  html += F("<br>APRS: "); html += APRS_HOST;
  html += F("<br>MQTT: "); html += MQTT_ENABLED ? "WLACZONY" : "WYLACZONY";
  html += F("<br>Strefa: "); html += get_tz_label();
  html += F("<br>Alt: "); html += String(SITE_ALT, 0); html += F(" m n.p.m.");
  html += F("</div>");

  html += F("<div class='info'><b>📌 Mapa pinów:</b>"
    "<table class='pin-table'>"
    "<tr><td>CC1101 SCK</td> <td class='pin-ok'>GPIO14 D5</td></tr>"
    "<tr><td>CC1101 MISO</td><td class='pin-ok'>GPIO12 D6</td></tr>"
    "<tr><td>CC1101 MOSI</td><td class='pin-ok'>GPIO13 D7</td></tr>"
    "<tr><td>CC1101 CS</td>  <td class='pin-ok'>GPIO15 D8</td></tr>"
    "<tr><td>CC1101 GDO0</td><td class='pin-ok'>GPIO4  D2 ← IRQ</td></tr>"
    "<tr><td>CC1101 GDO2</td><td class='pin-ok'>GPIO5  D1</td></tr>"
    "<tr><td>BME280 SDA</td> <td class='pin-ok'>GPIO0  D3</td></tr>"
    "<tr><td>BME280 SCL</td> <td class='pin-ok'>GPIO3  RX</td></tr>"
    "<tr><td>UART WX TX</td> <td class='pin-ok'>GPIO2  D4  9600</td></tr>"
    "<tr><td>Debug TX</td>   <td class='pin-ok'>GPIO1  TX  115200</td></tr>"
    "</table></div>"
    "</div></div></body></html>");

  server.send(200, "text/html", html);
}

void handleData() {
  String uart_str = buildUartString();
  String json = "{";
  json += "\"wxtime\":\""  + get_time_hhmm() + "\",";
  json += "\"wxdate\":\""  + get_date_ddmmyyyy() + "\",";
  json += "\"tz\":\""      + get_tz_label() + "\",";
  json += "\"temp\":\""   + (!isnan(current_wx.temp_c)       ? String(current_wx.temp_c, 1) + " °C"        : "---") + "\",";
  json += "\"hum\":\""    + (current_wx.humidity > 0          ? String(current_wx.humidity) + " %"          : "---") + "\",";
  json += "\"wdir\":\""   + (!isnan(current_wx.wind_dir)      ? String((int)current_wx.wind_dir) + " °"     : "---") + "\",";
  json += "\"wspd\":\""   + (!isnan(current_wx.wind_speed)    ? String(current_wx.wind_speed, 1) + " m/s"   : "---") + "\",";
  json += "\"wgst\":\""   + (!isnan(current_wx.wind_gust)     ? String(current_wx.wind_gust, 1) + " m/s"    : "---") + "\",";
  json += "\"rain\":\""   + (!isnan(current_wx.rain_total_mm) ? String(current_wx.rain_total_mm, 1) + " mm" : "---") + "\",";
  json += "\"uv\":\""     + (!isnan(current_wx.uv_index)      ? String(current_wx.uv_index, 1)               : "---") + "\",";
  json += "\"lux\":\""    + (!isnan(current_wx.light_klx)     ? String(current_wx.light_klx, 1) + " klx"    : "---") + "\",";
  json += "\"rssi\":\""   + String(current_wx.radio_rssi) + " dBm\",";
  json += "\"bat\":\""    + String(current_wx.battery_ok ? "OK" : "LOW") + "\",";
  json += "\"btemp\":\""  + (!isnan(current_wx.bme_temp)      ? String(current_wx.bme_temp, 1) + " °C"      : "---") + "\",";
  json += "\"bhum\":\""   + (!isnan(current_wx.bme_humidity)  ? String(current_wx.bme_humidity, 1) + " %"   : "---") + "\",";
  json += "\"bpress\":\"" + (!isnan(current_wx.bme_pressure)  ? String(current_wx.bme_pressure, 1) + " hPa" : "---") + "\",";
  json += "\"bqnh\":\""   + (!isnan(current_wx.bme_qnh)       ? String(current_wx.bme_qnh, 1) + " hPa"      : "---") + "\",";
  json += "\"upd\":\""    + (current_wx.last_update > 0
    ? String((millis() - current_wx.last_update) / 1000) + " s temu" : "brak") + "\",";
  json += "\"uart\":\"" + uart_str + "\"";
  json += "}";
  server.send(200, "application/json", json);
}

void handleSave() {
  if (server.hasArg("ssid"))       WIFI_SSID     = server.arg("ssid");
  if (server.hasArg("pass"))       WIFI_PASS     = server.arg("pass");
  if (server.hasArg("call"))       APRS_CALLSIGN = server.arg("call");
  if (server.hasArg("passcode"))   APRS_PASSCODE = server.arg("passcode");
  if (server.hasArg("lat"))        SITE_LAT      = server.arg("lat").toDouble();
  if (server.hasArg("lon"))        SITE_LON      = server.arg("lon").toDouble();
  if (server.hasArg("alt"))        SITE_ALT      = server.arg("alt").toFloat();
  if (server.hasArg("interval"))   REPORT_INTERVAL_MS = server.arg("interval").toInt() * 60000UL;
  if (server.hasArg("utc_offset")) UTC_OFFSET    = server.arg("utc_offset").toInt();
  if (server.hasArg("mqtt_en"))    MQTT_ENABLED  = (server.arg("mqtt_en") == "1");
  if (server.hasArg("mqtt_host"))  MQTT_HOST     = server.arg("mqtt_host");
  if (server.hasArg("mqtt_port"))  MQTT_PORT     = server.arg("mqtt_port").toInt();
  if (server.hasArg("mqtt_user"))  MQTT_USER     = server.arg("mqtt_user");
  if (server.hasArg("mqtt_pass"))  MQTT_PASS     = server.arg("mqtt_pass");
  if (server.hasArg("mqtt_topic")) MQTT_TOPIC    = server.arg("mqtt_topic");

  server.send(200, "text/html",
    F("<!DOCTYPE html><html><head><meta charset='UTF-8'>"
      "<meta http-equiv='refresh' content='3;url=/'>"
      "<style>body{font-family:Arial;text-align:center;padding:50px;background:#000;color:#fff}"
      ".msg{background:#1a1a1a;padding:30px;border-radius:8px;display:inline-block;"
      "box-shadow:0 2px 8px rgba(0,170,255,0.3);border:1px solid #00aaff}"
      "h1{color:#0f0}</style></head><body><div class='msg'>"
      "<h1>✅ Zapisano!</h1><p>Przekierowanie za 3 sekundy...</p>"
      "</div></body></html>"));

  saveConfig();
  mqttClient.setServer(MQTT_HOST.c_str(), MQTT_PORT);

  Serial.print(F("[CFG] UTC_OFFSET = ")); Serial.println(UTC_OFFSET);

/*
  server.send(200, "text/html",
    F("<!DOCTYPE html><html><head><meta charset='UTF-8'>"
      "<meta http-equiv='refresh' content='3;url=/'>"
      "<style>body{font-family:Arial;text-align:center;padding:50px;background:#000;color:#fff}"
      ".msg{background:#1a1a1a;padding:30px;border-radius:8px;display:inline-block;"
      "box-shadow:0 2px 8px rgba(0,170,255,0.3);border:1px solid #00aaff}"
      "h1{color:#0f0}</style></head><body><div class='msg'>"
      "<h1>✅ Zapisano!</h1><p>Przekierowanie za 3 sekundy...</p>"
      "</div></body></html>"));
*/
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (int i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void handleReset() {

  server.send(200, "text/html",
    F("<!DOCTYPE html><html><head><meta charset='UTF-8'>"
      "<meta http-equiv='refresh' content='0;url=/'>"
      "</head></html>"));

  WiFiManager wm;
  wm.resetSettings(); // Czyści dane WiFi z pamięci NVS
  // Po tym kroku warto też skasować plik config.json z LittleFS
  LittleFS.remove("/config.json");
  ESP.restart();

}

// Wyswietlenie danych na wyswietlaczu
void LCDShow() {

    //Wersja dla Heltec lub Lilygo
#if defined(HARDWARE_HELTEC_V2) || defined(HARDWARE_HELTEC_V3) || defined(ARDUINO_LILYGO_T3S3_SX1262) || defined(ARDUINO_LILYGO_T3S3_SX1276) || defined(ARDUINO_LILYGO_T3S3_LR1121)

    display.clearDisplay();
    display.setCursor(0, 0);  //kursor do lini 1

    //temperatura
    display.drawBitmap(0, 0, thermo_icon, 16, 16, WHITE);

    display.setTextSize(2);
    display.setCursor(20, 0);             // Pozycja linii numer 1
    display.print(" ");
    display.print( !isnan(current_wx.temp_c) ? String(current_wx.temp_c, 1) + " C" : "---" );
    display.setTextSize(1);

    //wilgotnosc
    display.setCursor(0, 19);             // Pozycja linii numer 2
    display.print("Wilg: ");
    display.print( current_wx.humidity > 0 ? String(current_wx.humidity) + " %" : "---" );
//    display.print(" %");

    //Sredni wiatr
    display.setCursor(0, 31);             // Pozycja linii numer 3
    display.print("Wiatr sr: ");
    display.print( !isnan(current_wx.wind_speed) ? String(current_wx.wind_speed, 1) + " m/s" : "---" );
//    display.print(" m/s");

    //opady lacznie
    display.setCursor(0, 43);             // Pozycja linii numer 4
    display.print("Opady: ");
    display.print( !isnan(current_wx.rain_total_mm) ? String(current_wx.rain_total_mm, 1) + " mm" : "---" );
//    display.print(" mm ");

    // promieniowanie UV
    display.setCursor(0, 55);             // Pozycja linii numer 5
    display.print("Swiatlo: ");
    display.print( !isnan(current_wx.light_klx) ? String(current_wx.light_klx, 1) + " klx" : "---" );
//    display.print(" klx ");

    display.display(); //wyświetlenie danych na wyświetlaczu

// wszystkie wartosci ktore mozna uzyc:
//
// !isnan(current_wx.temp_c) ? String(current_wx.temp_c, 1) + " °C" : "---";
// current_wx.humidity > 0 ? String(current_wx.humidity) + " %" : "---";
// !isnan(current_wx.wind_dir) ? String((int)current_wx.wind_dir) + " °" : "---";
// !isnan(current_wx.wind_speed) ? String(current_wx.wind_speed, 1) + " m/s" : "---";
// !isnan(current_wx.wind_gust) ? String(current_wx.wind_gust, 1) + " m/s" : "---";
// !isnan(current_wx.rain_total_mm) ? String(current_wx.rain_total_mm, 1) + " mm" : "---";
// !isnan(current_wx.uv_index) ? String(current_wx.uv_index, 1) : "---";
// !isnan(current_wx.light_klx) ? String(current_wx.light_klx, 1) + " klx" : "---";
// 
// String(current_wx.radio_rssi) + " dBm";
// current_wx.battery_ok ? F("status-ok'>OK") : F("status-warn'>LOW");
// 
// !isnan(current_wx.bme_temp) ? String(current_wx.bme_temp, 1) + " °C" : "---";
// !isnan(current_wx.bme_humidity) ? String(current_wx.bme_humidity, 1) + " %" : "---";
// !isnan(current_wx.bme_pressure) ? String(current_wx.bme_pressure, 1) + " hPa" : "---";
// !isnan(current_wx.bme_qnh) ? String(current_wx.bme_qnh, 1) + " hPa" : "---";
// 
// //ostatnia aktualizacja:
// current_wx.last_update > 0
//   ? String((millis() - current_wx.last_update) / 1000) + " s temu" : "brak danych";
//

    //Wersja dla CYD
#elif defined(HARDWARE_CYD)
/*
  tft.fillScreen(TFT_BLACK);

//1. godzina

  String godzina = "12:12";

  tft.setTextColor(TFT_GREEN, TFT_BLACK);   // Set text color to green and padding to back
  tft.setTextSize(4);

  tft.setTextDatum(TC_DATUM);
//  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.setTextPadding(tft.textWidth(" 44:44 "));  // String width + margin
  tft.drawString(godzina, 70, 10);

//2. Temperatura

  String temp = "21,7 oC"; // + strDate(current->time);

  tft.setTextSize(4);
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.setTextPadding(tft.textWidth("-21,6"));  // String width + margin
  tft.drawString("-21,6", 280, 10);
  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(2);
  tft.drawString("o", 285, 2);
  tft.setTextSize(4);
  tft.drawString("C", 300, 10);
//  tft.setTextPadding(tft.textWidth(" -21,6 oC "));  // String width + margin
  tft.setTextDatum(TL_DATUM);

  tft.setTextSize(2);

  tft.drawLine(145, 45, 145, 5, TFT_WHITE);
  tft.drawLine(20, 50, 300, 50, TFT_WHITE);

//3. cisnienie

  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Cisnienie:", 160, 60);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("1034 hPa", 180, 60);

//3. predkosc wiatru

  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Pr. wiatru:", 160, 80);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("12,3 m/s", 180, 80);

//3. kierunek wiatru

  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Kier. wiat.:", 160, 100);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("145", 180, 100);






  tft.setTextSize(1);

  String date = "Zaktualizowano: Apr 04 14:00"; // + strDate(current->time);

  tft.setTextDatum(BC_DATUM);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.setTextPadding(tft.textWidth(" Zaktualizowano: Mmm 44 44:44 "));  // String width + margin
  tft.drawString(date, 230, 235);
*/

//---------------------- V2 ----------------------
  // --- 1. CZAS (Górny Pasek) ---
//  tft.setFreeFont(&FreeSans28pt7b);
  tft.setFreeFont(&FreeSans14pt7b);
  tft.setTextColor(ACCENT_YELLOW, CARD_COLOR);
  tft.setTextDatum(CC_DATUM); 
  tft.drawString("--:--", 160, 18);

  // --- 2. SEKCJA NA ZEWNĄTRZ ---
//  tft.setFreeFont(&FreeSans28pt7b);
  tft.setFreeFont(&FreeSans14pt7b);
  tft.setTextColor(TEXT_MAIN, BG_COLOR);
  tft.setTextDatum(TL_DATUM);
  String t_out = String(current_wx.temp_c, 1) + "C"; // Zastąpiono ° symbolem C, czcionki GFX rzadko mają znak stopnia
  tft.drawString(t_out, 15, 58);

  tft.setFreeFont(&FreeSans14pt7b);
  tft.setTextColor(TEXT_SUB, BG_COLOR);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("Wilg: " + String(current_wx.humidity) + "%", 15, 105);

  // --- 3. SEKCJA W DOMU (BME) ---
//  tft.setFreeFont(&FreeSans28pt7b);
  tft.setFreeFont(&FreeSans14pt7b);
  tft.setTextColor(TEXT_MAIN, BG_COLOR);
  tft.setTextDatum(TL_DATUM);
  String t_in = String(current_wx.bme_temp, 1) + "C";
  tft.drawString(t_in, 173, 58);

  tft.setFreeFont(&FreeSans14pt7b);
  tft.setTextColor(TEXT_SUB, BG_COLOR);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("Wilg: " + String(current_wx.bme_humidity) + "%", 173, 100);
  tft.drawString(String(current_wx.bme_pressure, 0) + " hPa", 173, 116);

  // --- 4. SEKCJA WIATR ---
//  tft.setFreeFont(&FreeSans28pt7b);
  tft.setFreeFont(&FreeSans14pt7b);
  tft.setTextColor(TEXT_MAIN, BG_COLOR);
  tft.setTextDatum(TL_DATUM);
  tft.drawString(String(current_wx.wind_speed, 1), 15, 152); 

  tft.setFreeFont(&FreeSans14pt7b);
  tft.setTextColor(TEXT_SUB, BG_COLOR);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("km/h", 95, 165); 
  tft.drawString("Poryw: " + String(current_wx.wind_gust, 1), 15, 195);
  tft.drawString("Kier: " + String(current_wx.wind_dir) + " ", 15, 212);

  // --- 5. SEKCJA SŁOŃCE / OPAD ---
  tft.setFreeFont(&FreeSans14pt7b);
  tft.setTextColor(TEXT_MAIN, BG_COLOR);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("Opad: " + String(current_wx.rain_total_mm, 1) + "mm", 173, 155);
  tft.drawString("UV: " + String(current_wx.uv_index), 173, 180);
  
  String lux_str;
  if(current_wx.light_klx > 9.9) {
    lux_str = String(current_wx.light_klx, 1) + " kLx";
  } else {
    lux_str = String(current_wx.light_klx * 1000.0, 0) + " Lx";
  }
  tft.drawString("Jas: " + lux_str, 173, 205);

#endif

}

void LCDShow_status_searchwifi() {

#if defined(HARDWARE_CYD)
  tft.setFreeFont(&FreeSans14pt7b);
  tft.setTextColor(ACCENT_YELLOW, CARD_COLOR);
  tft.setTextDatum(CC_DATUM); 
  tft.drawString("Connecting to WiFi", 160, 18);
#endif

}

void LCDShow_status_enableap() {

#if defined(HARDWARE_CYD)
  tft.setFreeFont(&FreeSans14pt7b);
  tft.setTextColor(ACCENT_YELLOW, CARD_COLOR);
  tft.setTextDatum(CC_DATUM); 
  tft.drawString("Waiting on config, AP: MeteoStation", 160, 18);
#endif

}


void LCDShow_status(String komunikat) {

#if defined(HARDWARE_CYD)
  tft.setFreeFont(&FreeSans14pt7b);
  tft.setTextColor(ACCENT_YELLOW, CARD_COLOR);
  tft.setTextDatum(CC_DATUM); 
  tft.drawString(komunikat, 160, 18);

#endif

}
// wyczyszczenie paska statusu
void LCDShow_status_clear() {

#if defined(HARDWARE_CYD)
  tft.setFreeFont(&FreeSans14pt7b);
  tft.setTextColor(ACCENT_YELLOW, CARD_COLOR);
  tft.setTextDatum(CC_DATUM); 
  // tft.drawString("", 160, 18);
  tft.fillRect(0, 0, 320, 35, CARD_COLOR);

#endif

}

#if defined(HARDWARE_CYD)

// Pomocnicza funkcja do rysowania ramek sekcji (używa czcionki z pamięci Flash)
void rysujBoks(int x, int y, int w, int h, String naglowek, uint16_t kolorRamki) {
  tft.setFreeFont(NULL);
  tft.setTextFont(2);

  tft.drawRoundRect(x, y, w, h, 6, kolorRamki);
  tft.fillRect(x+5, y, w-10, 15, BG_COLOR); // Wycięcie miejsca na napis
  
  // Ładowanie czcionki bezpośrednio z tablicy wkompilowanej w kod programu
  //tft.loadFont(FreeSans14pt7bBitmaps); 
  tft.setTextFont(2); // 4 to duża wbudowana czcionka, 2 to mała czcionka
  tft.setTextSize(1);
  tft.setTextColor(kolorRamki, BG_COLOR);
  tft.setTextDatum(TC_DATUM); 
  tft.drawString(naglowek, x + (w / 2), y);
  //tft.unloadFont();
}
#endif

//funkcje pomocnicze do rysowanie na ekranie graficznym 320x240 px
#if defined(HARDWARE_CYD)

// Funkcja rysuje siatkę i tła dla sekcji
void rysujSzkieletInterfejsu() {
  tft.fillRect(0, 0, 320, 35, CARD_COLOR);
  tft.setTextPadding(0);
  
  // Rysowanie zaokrąglonych boksów ze statycznymi nagłówkami
  rysujBoks(5, 40, 152, 92, "NA ZEWNATRZ", ACCENT_BLUE);
  rysujBoks(163, 40, 152, 92, "W DOMU", ACCENT_GREEN);
  rysujBoks(5, 137, 152, 98, "WIATR", ACCENT_BLUE);
  rysujBoks(163, 137, 152, 98, "SLONCE / OPAD", ACCENT_YELLOW);
}


// Pole tekstowe z czyszczeniem tła (padding = szerokość pola w px)
void drawField(const String& s, int x, int y, int w, const GFXfont* f,
               uint16_t fg, uint16_t bg = BG_COLOR) {
  tft.setFreeFont(f);
  tft.setTextColor(fg, bg);
  tft.setTextDatum(TL_DATUM);
  tft.setTextPadding(w);
  tft.drawString(s, x, y);
  tft.setTextPadding(0);
}

// Liczba lub "---" gdy brak danych
String fmt(float v, int dec, const char* unit = "") {
  if (isnan(v)) return "---";
  return String(v, dec) + unit;
}


// Funkcja nadpisuje stare wartości nowymi
void aktualizujDaneNaEkranie() {

  const GFXfont* big   = &FreeSans14pt7b;
  //const GFXfont* small = FSS9;              // FreeSans 9pt z TFT_eSPI
  const GFXfont* small = &FreeSans9pt7b;
  const int W = 138;                        // szerokość pola w ramce

  // 1. Czas (pasek na górze)
  tft.setFreeFont(NULL);
  tft.setTextFont(4);
  tft.setTextSize(1);
  tft.setTextColor(ACCENT_YELLOW, CARD_COLOR);
  tft.setTextDatum(CC_DATUM);
  tft.setTextPadding(120);
  tft.drawString(get_time_hhmm(), 160, 18);
  tft.setTextPadding(0);

  // 2. NA ZEWNATRZ (ramka x=5..157, y=40..132)
  drawField(fmt(current_wx.temp_c, 1, " C"), 15, 58, W, big, TEXT_MAIN);
  drawField("Wilg: " + (current_wx.humidity > 0 ? String(current_wx.humidity) + "%" : String("---")),
            15, 98, W, small, TEXT_SUB);

  // 3. W DOMU (ramka x=163..315)
  drawField(fmt(current_wx.bme_temp, 1, " C"), 173, 58, W, big, TEXT_MAIN);
  drawField("Wilg: " + fmt(current_wx.bme_humidity, 0, "%"), 173, 90, W, small, TEXT_SUB);
  drawField(fmt(current_wx.bme_pressure, 0, " hPa"),         173, 110, W, small, TEXT_SUB);

  // 4. WIATR (ramka x=5..157, y=137..235)
  float kmh = isnan(current_wx.wind_speed) ? NAN : current_wx.wind_speed * 3.6f;
  float gst = isnan(current_wx.wind_gust)  ? NAN : current_wx.wind_gust  * 3.6f;
  drawField(fmt(kmh, 1, " km/h"), 15, 158, W, big, TEXT_MAIN);
  drawField("Poryw: " + fmt(gst, 1), 15, 192, W, small, TEXT_SUB);
  drawField("Kier: " + (isnan(current_wx.wind_dir) ? String("---")
                        : String((int)current_wx.wind_dir) + " st."), 15, 212, W, small, TEXT_SUB);

  // 5. SLONCE / OPAD (ramka x=163..315)
  drawField("Opad: " + fmt(current_wx.rain_total_mm, 1, " mm"), 173, 158, W, small, TEXT_MAIN);
  drawField("UV: " + fmt(current_wx.uv_index, 1),               173, 184, W, small, TEXT_MAIN);
  String lux = "---";
  if (!isnan(current_wx.light_klx)) {
    lux = current_wx.light_klx > 9.9 ? String(current_wx.light_klx, 1) + " kLx"
                                     : String(current_wx.light_klx * 1000.0, 0) + " Lx";
  }
  drawField("Jasn.: " + lux, 173, 210, W, small, TEXT_MAIN);

}

#endif


void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());

  Serial.println(myWiFiManager->getConfigPortalSSID());
}


void setup() {
  delay(3000);  // oczekiwanie na całkowity start serial monitora
  Serial.begin(115200);
  Serial.setDebugOutput(true);

//  LittleFS.begin();

  if (!LittleFS.begin(true)) {
    Serial.println("Błąd LittleFS!");
    return;
  }

  loadConfig();

  Serial.print(F("[CFG] UTC_OFFSET = ")); Serial.println(UTC_OFFSET);


#if defined(ARDUINO_TTGO_LoRa32_V1) || \
    defined(ARDUINO_TTGO_LoRa32_V2) || \
    defined(ARDUINO_TTGO_LoRa32_v21new)
    // I2C pin configuration
    Wire.begin(OLED_SDA, OLED_SCL);
#elif defined(ARDUINO_LILYGO_T3S3_SX1262) || \
    defined(ARDUINO_LILYGO_T3S3_SX1276) ||   \
    defined(ARDUINO_LILYGO_T3S3_LR1121)
    // I2C pin configuration
    Wire.begin(SDA, SCL);
#elif defined(HARDWARE_HELTEC_V2)
    // I2C pin configuration
    twi.begin(OLED_SDA, OLED_SCL);
    //Wire.begin(SDA, SCL);
#elif defined(HARDWARE_HELTEC_V3)
    // WŁĄCZENIE ZASILANIA EKRANU (Kluczowy krok dla Heltec V3)
    pinMode(VEXT_PIN, OUTPUT);
    digitalWrite(VEXT_PIN, LOW); // Stan NISKI włącza zasilanie Vext 
    delay(100); // Krótka przerwa, aby ekran się ustabilizował
    // I2C Konfiguracja pinow
    twi.begin(OLED_SDA, OLED_SCL);
#endif



#if defined(HARDWARE_HELTEC_V2) || defined(HARDWARE_HELTEC_V3) || defined(ARDUINO_LILYGO_T3S3_SX1262) || defined(ARDUINO_LILYGO_T3S3_SX1276) || defined(ARDUINO_LILYGO_T3S3_LR1121)
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        log_e("SSD1306 allocation failed");
    }

    // Clear the buffer
    display.clearDisplay();

    display.setTextColor(SSD1306_WHITE); // Draw white text
    display.setCursor(0, 0);             // Start at top-left corner
    display.cp437(true);                 // Use full 256 char 'Code Page 437' font
    display.setTextSize(2);
    display.println("Bresser");
    display.setCursor(0, 19);             // Pozycja linii numer 2
    display.setTextSize(1);
    display.println("Weather Sensor");
    display.setCursor(0, 31);             // Pozycja linii numer 3
    display.println("");
    display.setCursor(0, 43);             // Pozycja linii numer 4
    display.println(" ");
    display.setCursor(0, 55);             // Pozycja linii numer 5
    display.println("Wersja DarKo");
    display.display(); // Show initial text

#elif defined(HARDWARE_CYD)

  // Start the tft display and set it to black
  tft.init();
  tft.setRotation(1); //This is the display in landscape
  tft.invertDisplay(1);

  // Clear the screen before writing to it
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Stacja Meteo Garni / Bresser", 5, 10, 2); // Left Aligned
  tft.setTextColor(TFT_BLUE, TFT_BLACK);
  tft.drawCentreString("DarKo (SQ3HTZ)", 230, 215, 2);

  delay(2000);

//  tft.fillScreen(TFT_BLACK);
  tft.fillScreen(BG_COLOR);

  // Rysowanie stałego szkieletu interfejsu (wywoływane RAZ na starcie)
  rysujSzkieletInterfejsu();

  Serial.print("zakonczono rysowanie szkieletu");
#endif

// konfiguracja i połączenie Wi-Fi

  // definicja callback WiFi - czyli jak nie dojdzie do połączenia z zdefiniowaną Wi-Fi to odpali się punk dostępowy i strona do konfiguracji
  WiFiManager wm;
  wm.setSaveConfigCallback(saveConfigCallback);
  // allow to address the device by the given name e.g. http://webserver
  WiFi.setHostname(HOSTNAME);

  WiFi.mode(WIFI_STA);

  // funkcja wymazujaca ustawienia wi-fi == odkomentowac ponizej w razie potrzeby
  //wm.resetSettings();

  // --- 1. Przygotowanie buforów tekstowych dla liczb ---
  char c_lat[15], c_lon[15], c_alt[10], c_utc[5], c_int[10], c_port[6];
  
  dtostrf(SITE_LAT, 1, 6, c_lat);
  dtostrf(SITE_LON, 1, 6, c_lon);
  dtostrf(SITE_ALT, 1, 1, c_alt);
  sprintf(c_utc, "%d", UTC_OFFSET);
  sprintf(c_int, "%lu", REPORT_INTERVAL_MS / 60000);
  sprintf(c_port, "%u", MQTT_PORT);

  // --- 2. Definicja wszystkich 11+ parametrów ---
  // Składnia: WiFiManagerParameter(ID, Etykieta, Wartość, Długość)
  WiFiManagerParameter p_aprs_call("aprs_c", "APRS Callsign", APRS_CALLSIGN.c_str(), 15);
  WiFiManagerParameter p_aprs_pass("aprs_p", "APRS Passcode", APRS_PASSCODE.c_str(), 10);
  WiFiManagerParameter p_lat("lat", "Latitude", c_lat, 15);
  WiFiManagerParameter p_lon("lon", "Longitude", c_lon, 15);
  WiFiManagerParameter p_alt("alt", "Altitude (m)", c_alt, 10);
  WiFiManagerParameter p_utc("utc", "UTC Offset", c_utc, 5);
  WiFiManagerParameter p_interval("int", "Report Interval (min)", c_int, 10);
  
  WiFiManagerParameter p_mqtt_host("m_host", "MQTT Host", MQTT_HOST.c_str(), 40);
  WiFiManagerParameter p_mqtt_port("m_port", "MQTT Port", c_port, 6);
  WiFiManagerParameter p_mqtt_user("m_user", "MQTT User", MQTT_USER.c_str(), 20);
  WiFiManagerParameter p_mqtt_pass("m_pass", "MQTT Pass", MQTT_PASS.c_str(), 20);
  WiFiManagerParameter p_mqtt_topic("m_top", "MQTT Topic", MQTT_TOPIC.c_str(), 50);

  // Specjalna obsługa dla checkboxa (MQTT_ENABLED)
  // W WiFiManager najprościej zrobić to jako tekst "1" lub "0" lub niestandardowy HTML
  const char* mqtt_en_str = MQTT_ENABLED ? "checked" : "";
  char custom_html_mqtt[100];
  sprintf(custom_html_mqtt, "type='checkbox' %s", mqtt_en_str);
  WiFiManagerParameter p_mqtt_en("m_en", "Enable MQTT", "T", 2, custom_html_mqtt, WFM_LABEL_AFTER);

  // --- 3. Rejestracja parametrów w panelu ---
  wm.addParameter(&p_aprs_call);
  wm.addParameter(&p_aprs_pass);
  wm.addParameter(&p_lat);
  wm.addParameter(&p_lon);
  wm.addParameter(&p_alt);
  wm.addParameter(&p_utc);
  wm.addParameter(&p_interval);
  wm.addParameter(&p_mqtt_host);
  wm.addParameter(&p_mqtt_port);
  wm.addParameter(&p_mqtt_user);
  wm.addParameter(&p_mqtt_pass);
  wm.addParameter(&p_mqtt_topic);
  wm.addParameter(&p_mqtt_en);


// ----- polaczenie z Wi-Fi za pomoca WiFiManagera
// jeśli urządzenie nioe połączy się z zapisaną siecią Wi-Fi to uruchomi się w trybie AP z nazwą SSID: "WeatherStation" oraz hasłem: "meteo123"
//  if (!wm.autoConnect("WeatherStation","meteo123")) {
//    delay(3000);
//    ESP.restart();
//  }

  // --- USTAWIENIA TIMEOUTÓW DLA Wi-Fi ---
  wm.setTimeout(60);              // Próbuj łączyć ze znanym Wi-Fi przez 30 sekund
  wm.setConfigPortalTimeout(300); // Jeśli uruchomi się AP, wyłącz go po 3 minutach (180s) braku akcji

  LCDShow_status_searchwifi(); // wyswietlenie komunikatu o rozpoczeciu laczenia z Wi-Fi

  Serial.println("Lazczenie z Wi-Fi lub ustawienie w tryb AP");
  //rozpoczecie laczenie z zapisanym Wi-fi - jak nie ma danych lub przez 30-60 sek. nie moze sie polaczyć to przechodzi w tryB AP
  //  gdzie mozna skonfigurowac nowa siec Wi-Fi
  bool res;
  res = wm.autoConnect("WeatherStation","meteo123"); // password protected ap

  LCDShow_status("Ustawiono tryb AP - skonfiguruj");

  // Wylaczenie Wi-Fi jesli nie polaczylo się z zapisanym Wi-Fi i nikt nie podlaczyl sie z AP w celu konfiguracji 
  if(!res) {
    Serial.println("Failed to connect");
    WiFi.mode(WIFI_OFF);  // Wylaczenie Wi-Fi
    Serial.println("Wi-Fi turned OFF. Proceeding with the rest of the program...");
  }

  LCDShow_status_clear();

  WIFI_SSID = WiFi.SSID();
  WIFI_PASS = WiFi.psk();

  // --- 5. Pobranie danych po zapisie ---
  if (shouldSaveConfig) {
    APRS_CALLSIGN = p_aprs_call.getValue();
    APRS_PASSCODE = p_aprs_pass.getValue();
    SITE_LAT      = atof(p_lat.getValue());
    SITE_LON      = atof(p_lon.getValue());
    SITE_ALT      = atof(p_alt.getValue());
    UTC_OFFSET    = atoi(p_utc.getValue());
    REPORT_INTERVAL_MS = atol(p_interval.getValue()) * 60000UL;
    
    MQTT_HOST     = p_mqtt_host.getValue();
    MQTT_PORT     = atoi(p_mqtt_port.getValue());
    MQTT_USER     = p_mqtt_user.getValue();
    MQTT_PASS     = p_mqtt_pass.getValue();
    MQTT_TOPIC    = p_mqtt_topic.getValue();
    
    // Sprawdzenie czy checkbox został kliknięty
    MQTT_ENABLED = (String(p_mqtt_en.getValue()) == "T"); 

    saveConfig(); // Zapis do LittleFS
  }

  TRACE("connected.\n");

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID.c_str());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

//  if (MDNS.begin("esp32")) {
//    Serial.println("MDNS responder started");
//  }

  // Ask for the current time using NTP request builtin into ESP firmware.
  TRACE("Setup ntp...\n");
  configTzTime(TIMEZONE, "pool.ntp.org");

  TRACE("Register service handlers...\n");

  // register a redirect handler when only domain name is given.
//  server.on("/", HTTP_GET, handleRoot);
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/save", HTTP_POST, handleSave);
  server.on("/resetconfig", handleReset);

  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  delay(1000);

  //inicjalizacja BME280
  Serial.println(F("BME280 inicjalizacja"));

  unsigned status;

  //inicjalizacja BME
  Wire.begin(I2C_SDA, I2C_SCL);
  // default settings
//  status = bme.begin(0x76);  

  if (bme.begin(0x76)) {
    Serial.println(F("[BME] OK 0x76")); bme_available = true;
  } else if (bme.begin(0x77)) {
    Serial.println(F("[BME] OK 0x77")); bme_available = true;
  } else {
    Serial.println(F("[BME] NIE WYKRYTO!"));
  }


//  if (!status) {
//    Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
//    Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
//    Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
//    Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
//    Serial.print("        ID of 0x60 represents a BME 280.\n");
//    Serial.print("        ID of 0x61 represents a BME 680.\n");
//    while (1) delay(10);
//  }

  Serial.printf("Ustawienia zakonczone ...\n");
  initBoard();
  Serial.printf("Po initBoard ...\n");
  ws.begin();
  Serial.printf("Po ws.begin, Koniec BOOT ...\n");
}


void loop() 
{   
    server.handleClient();

    // This example uses only a single slot in the sensor data array
    int const i=0;

    if (MQTT_ENABLED) {
        if (!mqttClient.connected()) {
            static unsigned long lastTry = 0;
            if (millis() - lastTry > 5000) { lastTry = millis(); mqttReconnect(); }
        } else {
            mqttClient.loop();
        }
    }

    // Clear all sensor data
    ws.clearSlots();

    // Tries to receive radio message (non-blocking) and to decode it.
    // Timeout occurs after a small multiple of expected time-on-air.
    int decode_status = ws.getMessage();

    if (decode_status == DECODE_OK) {

        char batt_ok[] = "OK ";
        char batt_low[] = "Low";
        char batt_inv[] = "---";
        char * batt;

        current_wx.last_update = millis();

        current_wx.radio_rssi = ws.sensor[0].rssi;
        current_wx.battery_ok = ws.sensor[0].battery_ok;

//        display.clearDisplay();
//        display.setCursor(0, 0);  //kursor do lini 1

//        display.drawBitmap(0, 0, thermo_icon, 16, 16, WHITE);

        if ((ws.sensor[i].s_type == SENSOR_TYPE_WEATHER1) && !ws.sensor[i].w.temp_ok) {
            // Special handling for 6-in-1 decoder
            batt = batt_inv;
        } else if (ws.sensor[i].battery_ok) {
            batt = batt_ok;
        } else {
            batt = batt_low;
        }
        Serial.printf("Id: [%8X] Typ: [%X] Ch: [%d] St: [%d] Bat: [%-3s] RSSI: [%6.1fdBm] ",
            static_cast<int> (ws.sensor[i].sensor_id),
            ws.sensor[i].s_type,
            ws.sensor[i].chan,
            ws.sensor[i].startup,
            batt,
            ws.sensor[i].rssi);
           
        if (ws.sensor[i].s_type == SENSOR_TYPE_LIGHTNING) {
            // Lightning Sensor
            Serial.printf("Lightning Counter: [%4d] ", ws.sensor[i].lgt.strike_count);
            if (ws.sensor[i].lgt.distance_km != 0) {
                Serial.printf("Distance: [%2dkm] ", ws.sensor[i].lgt.distance_km);
            } else {
                Serial.printf("Distance: [----] ");
            }
            Serial.printf("unknown1: [0x%03X] ", ws.sensor[i].lgt.unknown1);
            Serial.printf("unknown2: [0x%04X]\n", ws.sensor[i].lgt.unknown2);

        }
        else if (ws.sensor[i].s_type == SENSOR_TYPE_LEAKAGE) {
            // Water Leakage Sensor
            Serial.printf("Leakage: [%-5s]\n", (ws.sensor[i].leak.alarm) ? "ALARM" : "OK");
      
        }
        else if (ws.sensor[i].s_type == SENSOR_TYPE_AIR_PM) {
            // Air Quality (Particular Matter) Sensor
            if (ws.sensor[i].pm.pm_1_0_init) {
                Serial.printf("PM1.0: [init] ");
            } else {
                Serial.printf("PM1.0: [%uµg/m³] ", ws.sensor[i].pm.pm_1_0);
            }
            if (ws.sensor[i].pm.pm_2_5_init) {
                Serial.printf("PM2.5: [init] ");
            } else {
                Serial.printf("PM2.5: [%uµg/m³] ", ws.sensor[i].pm.pm_2_5);
            }
            if (ws.sensor[i].pm.pm_10_init) {
                Serial.printf("PM10: [init]\n");
            } else {
                Serial.printf("PM10: [%uµg/m³]\n", ws.sensor[i].pm.pm_10);
            }
            
        }
        else if (ws.sensor[i].s_type == SENSOR_TYPE_CO2) {
            // CO2 Sensor
            if (ws.sensor[i].co2.co2_init) {
                Serial.printf("CO2: [init]\n");
            } else {
                Serial.printf("CO2: [%uppm]\n", ws.sensor[i].co2.co2_ppm);
            }

        }
        else if (ws.sensor[i].s_type == SENSOR_TYPE_HCHO_VOC) {
            // HCHO / VOC Sensor
            if (ws.sensor[i].voc.hcho_init) {
                Serial.printf("HCHO: [init] ");
            } else {
                Serial.printf("HCHO: [%uppb] ", ws.sensor[i].voc.hcho_ppb);
            }
            if (ws.sensor[i].voc.voc_init) {
                Serial.printf("VOC: [init]\n");
            } else {
                Serial.printf("VOC: [%u]\n", ws.sensor[i].voc.voc_level);
            }

        }
        else if (ws.sensor[i].s_type == SENSOR_TYPE_SOIL) {
            Serial.printf("Temp: [%5.1fC] ", ws.sensor[i].soil.temp_c);
            Serial.printf("Moisture: [%2d%%]\n", ws.sensor[i].soil.moisture);

        } else {
            // Any other (weather-like) sensor is very similar
            if (ws.sensor[i].w.temp_ok) {
                Serial.printf("Temp: [%5.1fC] ", ws.sensor[i].w.temp_c);
                current_wx.temp_c = ws.sensor[0].w.temp_c;
            } else {
 
                Serial.printf("Temp: [---.-C] ");
            }
            if (ws.sensor[i].w.humidity_ok) {
                Serial.printf("Hum: [%3d%%] ", ws.sensor[i].w.humidity);
                current_wx.humidity = ws.sensor[0].w.humidity;
            }
            else {
                Serial.printf("Hum: [---%%] ");
            }
            if (ws.sensor[i].w.wind_ok) {
                Serial.printf("Wmax: [%4.1fm/s] Wavg: [%4.1fm/s] Wdir: [%5.1fdeg] ",
                        ws.sensor[i].w.wind_gust_meter_sec,
                        ws.sensor[i].w.wind_avg_meter_sec,
                        ws.sensor[i].w.wind_direction_deg);
                
                current_wx.wind_dir   = ws.sensor[0].w.wind_direction_deg;
                current_wx.wind_speed = ws.sensor[0].w.wind_avg_meter_sec;
                current_wx.wind_gust  = ws.sensor[0].w.wind_gust_meter_sec;
                wind_speed_sum       += current_wx.wind_speed;
                wind_sample_count++;
                if (current_wx.wind_gust > wind_gust_max_period)
                    wind_gust_max_period = current_wx.wind_gust;
            } else {
                Serial.printf("Wmax: [--.-m/s] Wavg: [--.-m/s] Wdir: [---.-deg] ");
            }
            if (ws.sensor[i].w.rain_ok) {
                Serial.printf("Rain: [%7.1fmm] ",  
                    ws.sensor[i].w.rain_mm);
                current_wx.rain_total_mm = ws.sensor[0].w.rain_mm;
            } else {
                Serial.printf("Rain: [-----.-mm] ");
            }
        
            #if defined BRESSER_6_IN_1 || defined BRESSER_7_IN_1
            if (ws.sensor[i].w.uv_ok) {
                Serial.printf("UVidx: [%2.1f] ",
                    ws.sensor[i].w.uv);
                current_wx.uv_index = ws.sensor[0].w.uv;
            }
            else {
                Serial.printf("UVidx: [--.-] ");
            }
            #endif
            #ifdef BRESSER_7_IN_1
            if (ws.sensor[i].w.light_ok) {
                Serial.printf("Light: [%2.1fklx] ",
                    ws.sensor[i].w.light_klx);
                current_wx.light_klx = ws.sensor[0].w.light_klx;
            }
            else {
                Serial.printf("Light: [--.-klx] ");
            }
            if (ws.sensor[i].s_type == SENSOR_TYPE_WEATHER8) {
                if (ws.sensor[i].w.tglobe_ok) {
                    Serial.printf("T_globe: [%3.1fC] ",
                    ws.sensor[i].w.tglobe_c);
                }
                else {
                    Serial.printf("T_globe: [--.-C] ");
                }
            }
            #endif
            Serial.printf("\n");

      }

      current_wx.valid_data = true;

//    //wyświetlenie aktualnych danych na wyświetlaczu
//#if defined(HARDWARE_HELTEC_V2) || defined(ARDUINO_LILYGO_T3S3_SX1262) || defined(ARDUINO_LILYGO_T3S3_SX1276) || defined(ARDUINO_LILYGO_T3S3_LR1121)
//      LCDShow();
//#elif defined(HARDWARE_CYD)
//      aktualizujDaneNaEkranie();
//#endif
  


    } // if (decode_status == DECODE_OK)

  // UART TX co 5 sekund
  if (millis() - last_uart_send >= UART_INTERVAL_MS) {
      last_uart_send = millis();
      sendUartWX();
      Serial.print(F("[UART] ")); Serial.println(buildUartString());

//Wyświetlenie danych z BME
// =============================================!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//   ustawic przypisanie do odpowiednich zmiennych
//================================-------------------------------------------------------
/*
    Serial.print("Temperature = ");
    Serial.print(bme.readTemperature());
    Serial.println(" °C");
    current_wx.bme_temp = bme.readTemperature();

    Serial.print("Pressure = ");
    Serial.print(bme.readPressure() / 100.0F);
    Serial.println(" hPa");
    float raw_p = bme.readPressure() / 100.0F;

    Serial.print("Approx. Altitude = ");
    Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    Serial.println(" m");

    Serial.print("Humidity = ");
    Serial.print(bme.readHumidity());
    Serial.println(" %");
    current_wx.bme_humidity = bme.readHumidity();

    Serial.println();
    Serial.println("Proba odczytu danych BME");
*/
  }

  if (bme_available) {
    float raw_p             = bme.readPressure() / 100.0;
    current_wx.bme_temp     = bme.readTemperature();
    current_wx.bme_humidity = bme.readHumidity();
    current_wx.bme_pressure = raw_p;
    float t_ref = !isnan(current_wx.temp_c) ? current_wx.temp_c : current_wx.bme_temp;
    current_wx.bme_qnh = calc_qnh(raw_p, t_ref, SITE_ALT);
  }

  // APRS + MQTT
  if (millis() - last_report_time >= REPORT_INTERVAL_MS) {
      if (current_wx.valid_data && WiFi.status() == WL_CONNECTED) {
          send_aprs();
          mqttPublishWX();
      }
      last_report_time = millis();
  }

  //LCD refresh
  if (millis() - last_LCD_refresh >= LCD_INTERVAL_MS) {
  
    // funkcja wyświetlająca aktualne dane na wyświetlaczu
#if defined(HARDWARE_HELTEC_V2) || defined(HARDWARE_HELTEC_V3) || defined(ARDUINO_LILYGO_T3S3_SX1262) || defined(ARDUINO_LILYGO_T3S3_SX1276) || defined(ARDUINO_LILYGO_T3S3_LR1121)
      LCDShow();
#elif defined(HARDWARE_CYD)
      aktualizujDaneNaEkranie();
#endif

    last_LCD_refresh = millis();
//    unsigned long last_LCD_refresh = millis();
  }

  delay(50);

} // loop()
