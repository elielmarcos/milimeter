// Milimeter V1.0
//
// Power monitor using INA219 module, 0.96" 128x64 and ESP32 OLED display
//
// Author: Eliel Marcos Romancini
//
// Date: 21/07/2021
//
// Released for UFSC

// CAL:
//
// 400mA: 20340
// 800mA: 16300
// 1600mA: 8130
// 3200mA: 4063
//
//

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_INA219.h>
#include <Preferences.h>  // EEPROM

// Instantiate eeprom
Preferences preferences;

// macros from DateTime.h 
/* Useful Constants */
#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24L)
 
/* Useful Macros for getting elapsed time */
#define numberOfSeconds(_time_) (_time_ % SECS_PER_MIN)  
#define numberOfMinutes(_time_) ((_time_ / SECS_PER_MIN) % SECS_PER_MIN) 
#define numberOfHours(_time_) (( _time_% SECS_PER_DAY) / SECS_PER_HOUR)
#define elapsedDays(_time_) ( _time_ / SECS_PER_DAY)  

// Configure orientation of the display.
// 0 = none, 1 = 90 degrees clockwise, 2 = 180 degrees, 3 = 270 degrees CW
#define ROTATION     2

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Declaration for an INA219 module connected to I2C (SDA, SCL pins)
#define INA_219_ADDRESS 0x40
Adafruit_INA219 ina219(INA_219_ADDRESS);

// Declaration buttons input
#define BTN_LEFT 32
#define BTN_RIGHT 33

// Declaration LED
#define LED 2

// Declaration About
#define DEVICE "MILIMETER"
#define VERSION "V1.0"
#define NAME "ELIEL M. ROMANCINI"
#define DATE "30/07/21"
#define MAX_VOLT "16.0V"
#define MAX_CURR "3.00A"
#define LOAD "100R/2W"

// Variables
#define warning_width 24
#define warning_height 20
const uint8_t PROGMEM warning[] = { 0x00, 0x18, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x7E, 0x00, 0x00, 0xFF, 0x00, 0x00,
0xE7, 0x00, 0x01, 0xC3, 0x80, 0x03, 0xC3, 0xC0, 0x03, 0xC3, 0xC0, 0x07, 0xC3, 0xE0, 0x07, 0xC3,
0xE0, 0x0F, 0xC3, 0xF0, 0x1F, 0xC3, 0xF8, 0x1F, 0xE7, 0xF8, 0x3F, 0xFF, 0xFC, 0x7F, 0xE7, 0xFE,
0x7F, 0xC3, 0xFE, 0xFF, 0xE7, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFE };

#define save_width 22
#define save_height 20
const uint8_t PROGMEM save[] = { 0x3f, 0xff, 0x80, 0x7f, 0xff, 0xc0, 0x60, 0x39, 0xe0, 0x60, 0x39, 0xf0, 0x60, 0x39, 0xf8, 0x60,
0x01, 0xf8, 0x60, 0x01, 0xf8, 0x7f, 0xff, 0xf8, 0x7f, 0xff, 0xf8, 0x7f, 0xff, 0xf8, 0x7f, 0xff,
0xf8, 0x7f, 0x87, 0xf8, 0x7f, 0x03, 0xf8, 0x7f, 0x03, 0xf8, 0x7f, 0x03, 0xf8, 0x7f, 0x03, 0xf8,
0x7f, 0x87, 0xf8, 0x7f, 0xff, 0xf8, 0x7f, 0xff, 0xf8, 0x3f, 0xff, 0xf0 };

#define trash_width 16
#define trash_height 20
const uint8_t PROGMEM trash[] = { 0x07, 0xE0, 0x07, 0xE0, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x7F, 0xFE, 0x7F, 0xFE, 0x77, 0xEE,
0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66,
0x66, 0x66, 0x7F, 0xFE, 0x7F, 0xFE, 0x3F, 0xFC };

#define milimeter_width 32
#define milimeter_height 24
const uint8_t PROGMEM milimeter[] = { 0x00, 0x0F, 0xF0, 0x00, 0x00, 0x3F, 0xFC, 0x00, 0x00, 0xF8, 0x1F, 0x00, 0x01, 0xE0, 0x07, 0x80,
0x03, 0xC0, 0x03, 0xC0, 0x03, 0x80, 0x01, 0xC0, 0x07, 0x0E, 0x78, 0xE0, 0x06, 0x11, 0x88, 0x60,
0x0E, 0x11, 0x88, 0x70, 0x0C, 0x31, 0x8C, 0x30, 0x7F, 0xE1, 0x87, 0xFE, 0x80, 0x20, 0x04, 0x01,
0x80, 0x04, 0x20, 0x01, 0x7F, 0x84, 0x21, 0xFE, 0x0C, 0x8E, 0x71, 0x30, 0x0E, 0x8A, 0x51, 0x70,
0x06, 0xCA, 0x53, 0x60, 0x07, 0x73, 0xCE, 0xE0, 0x03, 0x80, 0x01, 0xC0, 0x03, 0xC0, 0x03, 0xC0,
0x01, 0xE0, 0x07, 0x80, 0x00, 0xF8, 0x1F, 0x00, 0x00, 0x3F, 0xFC, 0x00, 0x00, 0x0F, 0xF0, 0x00 };

bool  last_btn_left = true; //pullup
bool  last_btn_right = true; //pullup
bool  communication = false;
bool  new_connection = false;
uint8_t no_reply = 0; // UART unanswered number
uint8_t attempts = 3; // Maximum unanswered sends
uint8_t menu = 0;
uint8_t toggle = 0;
uint64_t now_millis = 0;
uint64_t last_millis_post = 0;
uint64_t last_millis_sample = 0;
uint64_t last_millis_ping = 0;
uint64_t last_millis_monitor = 0;
uint64_t post = 0;
String post_rate_tag[8] = {"1 seg", "30 seg", "1 min", "5 min", "10 min", "15 min", "30 min", "1 hour"};
uint32_t post_rate_value[] = {1000, 30000, 60000, 300000, 600000, 900000, 1800000, 3600000};  //ms
uint8_t post_rate_index = 0;
uint8_t post_rate = 0;
String sample_rate_tag[7] = {"50 ms", "100 ms", "250 ms", "500 ms", "1 seg", "2 seg", "5 seg"};
uint32_t sample_rate_value[] = {50, 100, 250, 500, 1000, 2000, 5000};  //ms
uint8_t sample_rate_index = 0;
uint8_t sample_rate = 0;
uint32_t monitor_rate = 1000; // ms
String mm_display_tag[2] = {"Off", "On"};
uint8_t mm_display_index = 0;
uint8_t mm_display = 0;
uint8_t change_cal = 0;
uint32_t ina219_cal_400mA  = 0;
uint32_t ina219_cal_800mA  = 0;
uint32_t ina219_cal_1600mA = 0;
uint32_t ina219_cal_3200mA = 0;


// Struct Queue Data
typedef struct {
  float vshunt = 0;       // Voltage Shunt
  float vbus = 0;         // Voltage Bus
  float cshunt = 0;       // Current Shunt
  float pshunt = 0;       // Power Shunt
  float vload = 0;        // Voltage Load
  float max_voltage = 0;
  float min_voltage = 0;
  float max_current = 0;
  float min_current = 0;
  float max_power = 0;
  float min_power = 0;
} DataInfo;

// Queue Data
QueueHandle_t dataPipe;

// Task User Interface
TaskHandle_t UI_Handle = NULL;


// Button 1 read Falling
bool btn_left_falling() {
  bool now_btn = digitalRead(BTN_LEFT);

  if ((now_btn != last_btn_left) and (now_btn == true)) {
    last_btn_left = now_btn;
    return true;
  }

  last_btn_left = now_btn;

  return false;
  
}

// Button 2 read Falling
bool btn_right_falling() {
  bool now_btn = digitalRead(BTN_RIGHT);

  if ((now_btn != last_btn_right) and (now_btn == true)) {
    last_btn_right = now_btn;
    return true;
  }

  last_btn_right = now_btn;

  return false;
  
}

// Button 1 read Rising
bool btn_left_rising() {
  bool now_btn = digitalRead(BTN_LEFT);

  if ((now_btn != last_btn_left) and (now_btn == false)) {
    last_btn_left = now_btn;
    return true;
  }

  last_btn_left = now_btn;

  return false;
  
}

// Button 2 read Rising
bool btn_right_rising() {
  bool now_btn = digitalRead(BTN_RIGHT);

  if ((now_btn != last_btn_right) and (now_btn == false)) {
    last_btn_right = now_btn;
    return true;
  }

  last_btn_right = now_btn;

  return false;
  
}


// Read eeprom
void load_eeprom() {
  // Init preference eeprom
  preferences.begin("EEPROM", false); // RW-mode (second parameter has to be false).

  post_rate = preferences.getUChar("post_rate", 0);     // load uint8_t
  sample_rate = preferences.getUChar("sample_rate", 4); // load uint8_t
  mm_display = preferences.getUChar("mm_display", 1);   // load uint8_t

  // Close the preferences eeprom
  preferences.end();
}


// Write eeprom
void write_eeprom(String address = "", uint8_t value = 0) {
  // Init preference eeprom
  preferences.begin("EEPROM", false); // RW-mode (second parameter has to be false).

  if (address != "") {
    preferences.putUChar(address.c_str(), value);  // save uint8_t
  }

  // Close the preferences eeprom
  preferences.end();
}


// Reset Factory
void reset_factory() {
  write_eeprom("post_rate", 0);
  write_eeprom("sample_rate", 4);
  write_eeprom("mm_display", 1);
   
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(64-(8*6*2)/2, 32-(7*2)/2);
  display.print("RESET...");
  display.display();

  vTaskDelay(2000 / portTICK_PERIOD_MS);
  ESP.restart();
}


// Read Calibration eeprom
void cal_load_eeprom() {
  // Init preference cal eeprom
  preferences.begin("CAL", false); // RW-mode (second parameter has to be false).

  ina219_cal_400mA  = preferences.getUInt("cal_400mA", 20480);   // load uint32_t
  ina219_cal_800mA  = preferences.getUInt("cal_800mA", 16384);   // load uint32_t
  ina219_cal_1600mA = preferences.getUInt("cal_1600mA", 8192);   // load uint32_t
  ina219_cal_3200mA = preferences.getUInt("cal_3200mA", 4096);   // load uint32_t

  // Close the preferences cal eeprom
  preferences.end();
}


// Write Calibration eeprom
void cal_write_eeprom(String address = "", uint32_t value = 0) {
  // Init preference eeprom
  preferences.begin("CAL", false); // RW-mode (second parameter has to be false).

  if (address != "") {
    preferences.putUInt(address.c_str(), value);  // save uint32_t
  }

  // Close the preferences eeprom
  preferences.end();
}



// Header display
void header(String TITLE = "") {
  if (TITLE != "") {
    display.setTextSize(1);
    display.setCursor(64-int((TITLE.length()*6)/2), 0);
    display.setTextColor(WHITE);
    display.print(TITLE);
    display.drawLine(0, 8, 128, 8, WHITE);
  }
}

// Footer display
void footer(String LEFT = "", bool L_GROUND = WHITE, String RIGHT = "", bool R_GROUND = WHITE) { // Text1, color1, text2, color2
  display.setTextSize(1);
  if (LEFT != "") {
    if (L_GROUND == WHITE)
      display.fillRoundRect(0, 55, 61, 9, 2, WHITE);  
    else
      display.drawRoundRect(0, 55, 61, 9, 2, WHITE);  
    display.setCursor(32-int((LEFT.length()*6)/2), 56);
    if (L_GROUND == WHITE)
      display.setTextColor(BLACK);
    else
      display.setTextColor(WHITE);
    display.print(LEFT);
    display.drawLine(64, 55, 64, 64, WHITE); 
  }

  if (RIGHT != "") {
    display.drawLine(64, 55, 64, 64, WHITE);  
    if (R_GROUND == WHITE)
      display.fillRoundRect(68, 55, 60, 9, 2, WHITE);  
    else
      display.drawRoundRect(68, 55, 60, 9, 2, WHITE);  
    display.setCursor(99-int((RIGHT.length()*6)/2), 56);
    if (R_GROUND == WHITE)
      display.setTextColor(BLACK);
    else
      display.setTextColor(WHITE);
    display.print(RIGHT);
    display.setTextColor(WHITE);
  }
}

void MENU_00(DataInfo *datas) {
  
  display.clearDisplay();
  header("MEASUREMENT");

  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 13);  
  display.printf("Volt:   % 2.3f V", datas->vload);
  display.setCursor(0, 25);
  display.printf("Curr:   % 4.2f mA", datas->cshunt);
  display.setCursor(0, 37);
  display.printf("Power:  % 5.1f mW", datas->pshunt);

  footer("MODE",digitalRead(BTN_LEFT),"SETTING",digitalRead(BTN_RIGHT));
  display.display();

  return;
}


void MENU_01(DataInfo *datas, uint8_t check = 0) {
  uint8_t px = 12*2; // font size 2

  if (check == 0) {
     px = 18*1; // font size 3
  }

  if (abs(datas->vload)>=10.000f)
    px -= check == 1 ? 12 : 18;
  
  display.clearDisplay();
  header("VOLTAGE");

  display.setTextColor(WHITE);
  display.setTextSize(2);
  if (check == 1) display.setCursor(px, 13);
    else {
      display.setCursor(px, 30); 
      display.setTextSize(3);
    }
  display.printf("% .3f", datas->vload);
  display.setTextSize(2);
  display.setCursor(104, 13);
  display.print(" V");

  if (check == 1) {
    display.setTextSize(1);
    display.setCursor(0, 38);
    display.printf("Max: % 2.3fV\nMin: % 2.3fV", datas->max_voltage, datas->min_voltage);
    
    footer("MODE",digitalRead(BTN_LEFT),"CLEAR",digitalRead(BTN_RIGHT));
  } else
    footer("MODE",digitalRead(BTN_LEFT),"",0);
    
  display.display();
  
  return;
}


void MENU_02(DataInfo *datas, uint8_t check = 0) {
  uint8_t px = 12*3; // font size 2

  if (check == 0) {
     px = 18*2; // font size 3
  }

  if (abs(datas->cshunt)>=10.00f)
    px -= check == 1 ? 12 : 18;
  if (abs(datas->cshunt)>=100.00f)
    px -= check == 1 ? 12 : 18;
  if (abs(datas->cshunt)>=1000.00f)
    px -= check == 1 ? 12 : 18;
    
  display.clearDisplay();
  header("CURRENT");

  display.setTextColor(WHITE);
  display.setTextSize(2);
  if (check == 1) display.setCursor(px, 13);
    else {
      display.setCursor(px, 30); 
      display.setTextSize(3);
    }
  display.printf("% .2f", datas->cshunt);
  display.setTextSize(2);
  display.setCursor(104, 13);
  display.print("mA");

  if (check == 1) {
    display.setTextSize(1);
    display.setCursor(0, 38);
    display.printf("Max: % 4.2fmA\nMin: % 4.2fmA", datas->max_current, datas->min_current);
    
    footer("MODE",digitalRead(BTN_LEFT),"CLEAR",digitalRead(BTN_RIGHT));
  } else
    footer("MODE",digitalRead(BTN_LEFT),"",0);
    
  display.display();
  
  return;
}


void MENU_03(DataInfo *datas, uint8_t check = 0) {
  uint8_t px = 12*4; // font size 2

  if (check == 0) {
     px = 18*3; // font size 3
  }

  if (abs(datas->pshunt)>=10.0f)
    px -= check == 1 ? 12 : 18;
  if (abs(datas->pshunt)>=100.0f)
    px -= check == 1 ? 12 : 18;
  if (abs(datas->pshunt)>=1000.0f)
    px -= check == 1 ? 12 : 18;
  if (abs(datas->pshunt)>=10000.0f)
    px -= check == 1 ? 12 : 18;
    
  display.clearDisplay();
  header("POWER");

  display.setTextColor(WHITE);
  display.setTextSize(2);
  if (check == 1) display.setCursor(px, 13);
    else {
      display.setCursor(px, 30); 
      display.setTextSize(3);
    }
  display.printf("% .1f", datas->pshunt);
  display.setTextSize(2);
  display.setCursor(104, 13);
  display.print("mW");

  if (check == 1) {
    display.setTextSize(1);
    display.setCursor(0, 38);
    display.printf("Max: % 4.1fmW\nMin: % 4.1fmW", datas->max_power, datas->min_power);
    
    footer("MODE",digitalRead(BTN_LEFT),"CLEAR",digitalRead(BTN_RIGHT));
  } else
    footer("MODE",digitalRead(BTN_LEFT),"",0);
    
  display.display();
  
  return;
}


void MENU_04() {
  long val = millis() / 1000;
  int days = elapsedDays(val);
  int hours = numberOfHours(val);
  int minutes = numberOfMinutes(val);
  int seconds = numberOfSeconds(val);
  
  display.clearDisplay();
  header("STATUS");

  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 13);  
  if (days >= 10) { display.printf("Timer: %dd %dh %dm %s",days,hours,minutes, (seconds % 2) == 0 ? "s" : ""); }
  else if (days > 0) { display.printf("Timer: %dd %dh %dm %ds",days,hours,minutes,seconds); }
  else if (hours > 0) { display.printf("Timer: %dh %dm %ds",hours,minutes,seconds); }
  else if (minutes > 0) { display.printf("Timer: %dm %ds",minutes,seconds); }
  else { display.printf("Timer: %ds",seconds); }
  display.setCursor(0, 25);
  display.printf("Posts: %llu pck",post);
  display.setCursor(0, 37);
  display.printf("Communication: %s", communication ? String("OK").c_str() : String("No").c_str());

  footer("MODE",digitalRead(BTN_LEFT),"",0);
  display.display();
  return;
}


void MENU_10() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1); 

  display.drawRoundRect(0, 0, 128, 52, 3, WHITE);
  display.drawRoundRect(1, 1, 128-2, 52-2, 2, WHITE);
  display.drawRoundRect(2, 2, 128-4, 52-4, 2, WHITE);

  display.drawBitmap(64-(trash_width/2), 4, trash, trash_width, trash_height, 1);

  display.setCursor(26, 28);
  display.print("Clear Max/Min");
  display.setCursor(40, 38);
  display.print("VOLTAGE?");
  
  footer("YES",digitalRead(BTN_LEFT),"NO",digitalRead(BTN_RIGHT));
  display.display();
  
  return;
}


void MENU_11() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1); 

  display.drawRoundRect(0, 0, 128, 52, 3, WHITE);
  display.drawRoundRect(1, 1, 128-2, 52-2, 2, WHITE);
  display.drawRoundRect(2, 2, 128-4, 52-4, 2, WHITE);

  display.drawBitmap(64-(trash_width/2), 4, trash, trash_width, trash_height, 1);

  display.setCursor(26, 28);
  display.print("Clear Max/Min");
  display.setCursor(40, 38);
  display.print("CURRENT?");
  
  footer("YES",digitalRead(BTN_LEFT),"NO",digitalRead(BTN_RIGHT));
  display.display();
  
  return;
}


void MENU_12() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1); 

  display.drawRoundRect(0, 0, 128, 52, 3, WHITE);
  display.drawRoundRect(1, 1, 128-2, 52-2, 2, WHITE);
  display.drawRoundRect(2, 2, 128-4, 52-4, 2, WHITE);

  display.drawBitmap(64-(trash_width/2), 4, trash, trash_width, trash_height, 1);

  display.setCursor(26, 28);
  display.print("Clear Max/Min");
  display.setCursor(46, 38);
  display.print("POWER?");
  
  footer("YES",digitalRead(BTN_LEFT),"NO",digitalRead(BTN_RIGHT));
  display.display();
  
  return;
}


void MENU_20() {
  display.clearDisplay();
  header("SETTINGS");

  display.setTextColor(WHITE);
  display.setTextSize(1);

  display.fillRect(0, 10, 128, 11, WHITE);

  display.setCursor(2, 12); 
  display.setTextColor(BLACK);
  display.print("1 - Post Rate");
  display.setTextColor(WHITE);
  display.setCursor(2, 23); 
  display.print("2 - Sample Rate");
  display.setCursor(2, 34); 
  display.print("3 - Max/Min Display");
  display.setCursor(2, 45); 
  display.print("4 - Reset Factory");
  
  footer("CHANGE",digitalRead(BTN_LEFT),"SELECT",digitalRead(BTN_RIGHT));
  display.display();
  
  return;
}


void MENU_21() {
  display.clearDisplay();
  header("SETTINGS");

  display.setTextColor(WHITE);
  display.setTextSize(1);

  display.fillRect(0, 21, 128, 11, WHITE);

  display.setCursor(2, 12); 
  display.print("1 - Post Rate");
  display.setCursor(2, 23); 
  display.setTextColor(BLACK);
  display.print("2 - Sample Rate");
  display.setTextColor(WHITE);
  display.setCursor(2, 34); 
  display.print("3 - Max/Min Display");
  display.setCursor(2, 45); 
  display.print("4 - Reset Factory");
  
  footer("CHANGE",digitalRead(BTN_LEFT),"SELECT",digitalRead(BTN_RIGHT));
  display.display();
  
  return;
}


void MENU_22() {
  display.clearDisplay();
  header("SETTINGS");

  display.setTextColor(WHITE);
  display.setTextSize(1);

  display.fillRect(0, 32, 128, 11, WHITE);

  display.setCursor(2, 12); 
  display.print("1 - Post Rate");
  display.setCursor(2, 23); 
  display.print("2 - Sample Rate");
  display.setCursor(2, 34); 
  display.setTextColor(BLACK);
  display.print("3 - Max/Min Display");
  display.setTextColor(WHITE);
  display.setCursor(2, 45); 
  display.print("4 - Reset Factory");
  
  footer("CHANGE",digitalRead(BTN_LEFT),"SELECT",digitalRead(BTN_RIGHT));
  display.display();
  
  return;
}


void MENU_23() {
  display.clearDisplay();
  header("SETTINGS");

  display.setTextColor(WHITE);
  display.setTextSize(1);

  display.fillRect(0, 43, 128, 11, WHITE);

  display.setCursor(2, 12); 
  display.print("1 - Post Rate");
  display.setCursor(2, 23); 
  display.print("2 - Sample Rate");
  display.setCursor(2, 34); 
  display.print("3 - Max/Min Display");
  display.setCursor(2, 45); 
  display.setTextColor(BLACK);
  display.print("4 - Reset Factory");
  display.setTextColor(WHITE);
  
  footer("CHANGE",digitalRead(BTN_LEFT),"SELECT",digitalRead(BTN_RIGHT));
  display.display();
  
  return;
}

void MENU_24() {
  display.clearDisplay();
  header("SETTINGS");

  display.setTextColor(WHITE);
  display.setTextSize(1);

  display.fillRect(0, 10, 128, 11, WHITE);

  display.setCursor(2, 12); 
  display.setTextColor(BLACK);
  display.print("5 - About");
  display.setTextColor(WHITE);
  display.setCursor(2, 23); 
  display.print("6 - Exit");
  display.setCursor(2, 34); 
  
  footer("CHANGE",digitalRead(BTN_LEFT),"SELECT",digitalRead(BTN_RIGHT));
  display.display();
  
  return;
}


void MENU_25() {
  display.clearDisplay();
  header("SETTINGS");

  display.setTextColor(WHITE);
  display.setTextSize(1);

  display.fillRect(0, 21, 128, 11, WHITE);

  display.setCursor(2, 12); 
  display.print("5 - About");
  display.setCursor(2, 23); 
  display.setTextColor(BLACK);
  display.print("6 - Exit");
  display.setTextColor(WHITE);
  display.setCursor(2, 34); 
  
  footer("CHANGE",digitalRead(BTN_LEFT),"SELECT",digitalRead(BTN_RIGHT));
  display.display();
  
  return;
}


void MENU_30(uint8_t check) {
  display.clearDisplay();
  header("POST RATE");

  display.setTextColor(WHITE);
  display.setTextSize(1);

  display.drawCircle(4, 14, 4, WHITE);
  if (check == 0) display.fillCircle(4, 14, 2, WHITE);
  display.setCursor(15, 11);
  display.printf("%s", post_rate_tag[0].c_str());

  display.drawCircle(4, 25, 4, WHITE);
  if (check == 1) display.fillCircle(4, 25, 2, WHITE);
  display.setCursor(15, 22);
  display.printf("%s", post_rate_tag[1].c_str());

  display.drawCircle(4, 36, 4, WHITE);
  if (check == 2) display.fillCircle(4, 36, 2, WHITE);
  display.setCursor(15, 33);
  display.printf("%s", post_rate_tag[2].c_str());

  display.drawCircle(4, 47, 4, WHITE);
  if (check == 3) display.fillCircle(4, 47, 2, WHITE);
  display.setCursor(15, 44);
  display.printf("%s", post_rate_tag[3].c_str());

  display.drawCircle(74, 14, 4, WHITE);
  if (check == 4) display.fillCircle(74, 14, 2, WHITE);
  display.setCursor(85, 11);
  display.printf("%s", post_rate_tag[4].c_str());

  display.drawCircle(74, 25, 4, WHITE);
  if (check == 5) display.fillCircle(74, 25, 2, WHITE);
  display.setCursor(85, 22);
  display.printf("%s", post_rate_tag[5].c_str());

  display.drawCircle(74, 36, 4, WHITE);
  if (check == 6) display.fillCircle(74, 36, 2, WHITE);
  display.setCursor(85, 33);
  display.printf("%s", post_rate_tag[6].c_str());

  display.drawCircle(74, 47, 4, WHITE);
  if (check == 7) display.fillCircle(74, 47, 2, WHITE);
  display.setCursor(85, 44);
  display.printf("%s", post_rate_tag[7].c_str());
  
  footer("CHANGE",digitalRead(BTN_LEFT),"SET",digitalRead(BTN_RIGHT));
  display.display();
  
  return;
}


void MENU_31(uint8_t check) {
  display.clearDisplay();
  header("SAMPLE RATE");

  display.setTextColor(WHITE);
  display.setTextSize(1);

  display.drawCircle(4, 14, 4, WHITE);
  if (check == 0) display.fillCircle(4, 14, 2, WHITE);
  display.setCursor(15, 11);
  display.printf("%s", sample_rate_tag[0].c_str());

  display.drawCircle(4, 25, 4, WHITE);
  if (check == 1) display.fillCircle(4, 25, 2, WHITE);
  display.setCursor(15, 22);
  display.printf("%s", sample_rate_tag[1].c_str());

  display.drawCircle(4, 36, 4, WHITE);
  if (check == 2) display.fillCircle(4, 36, 2, WHITE);
  display.setCursor(15, 33);
  display.printf("%s", sample_rate_tag[2].c_str());

  display.drawCircle(4, 47, 4, WHITE);
  if (check == 3) display.fillCircle(4, 47, 2, WHITE);
  display.setCursor(15, 44);
  display.printf("%s", sample_rate_tag[3].c_str());

  display.drawCircle(74, 14, 4, WHITE);
  if (check == 4) display.fillCircle(74, 14, 2, WHITE);
  display.setCursor(85, 11);
  display.printf("%s", sample_rate_tag[4].c_str());

  display.drawCircle(74, 25, 4, WHITE);
  if (check == 5) display.fillCircle(74, 25, 2, WHITE);
  display.setCursor(85, 22);
  display.printf("%s", sample_rate_tag[5].c_str());

  display.drawCircle(74, 36, 4, WHITE);
  if (check == 6) display.fillCircle(74, 36, 2, WHITE);
  display.setCursor(85, 33);
  display.printf("%s", sample_rate_tag[6].c_str());
  
  footer("CHANGE",digitalRead(BTN_LEFT),"SET",digitalRead(BTN_RIGHT));
  display.display();
  
  return;
}


void MENU_32(uint8_t check) {
  display.clearDisplay();
  header("MAX/MIN DISPLAY");

  display.setTextColor(WHITE);
  display.setTextSize(1);

  display.drawCircle(4, 14, 4, WHITE);
  if (check == 0) display.fillCircle(4, 14, 2, WHITE);
  display.setCursor(15, 11);
  display.printf("%s", mm_display_tag[0].c_str());

  display.drawCircle(74, 14, 4, WHITE);
  if (check == 1) display.fillCircle(74, 14, 2, WHITE);
  display.setCursor(85, 11);
  display.printf("%s", mm_display_tag[1].c_str());

  footer("CHANGE",digitalRead(BTN_LEFT),"SET",digitalRead(BTN_RIGHT));
  display.display();
  
  return;
}


void MENU_33() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1); 

  display.drawRoundRect(0, 0, 128, 52, 3, WHITE);
  display.drawRoundRect(1, 1, 128-2, 52-2, 2, WHITE);
  display.drawRoundRect(2, 2, 128-4, 52-4, 2, WHITE);

  display.drawBitmap(5, 4, warning, warning_width, warning_height, 1);
  display.setCursor(34, 8);
  display.setTextSize(2); 
  display.print("WARNING");
  display.setTextSize(1); 
  display.setCursor(10, 26);
  display.print("RETURN ALL FACTORY");
  display.setCursor(37, 36);
  display.print("SETTINGS?");
  
  footer("YES",digitalRead(BTN_LEFT),"NO",digitalRead(BTN_RIGHT));
  display.display();
  
  return;
}


void MENU_34() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1); 

  display.drawBitmap(0, 0, milimeter, milimeter_width, milimeter_height, 1);
  display.setCursor(35, 4);
  display.printf("%s %s",DEVICE,VERSION);
  display.setCursor(50, 14); 
  display.print("Build by");
  display.setCursor(10, 30);
  display.printf("%s",NAME);
  display.setCursor(22, 44);
  display.printf("Date: %s ",DATE);
  
  footer("",0,"NEXT",digitalRead(BTN_RIGHT));
  display.display();
  
  return;
}


void MENU_35(uint8_t blink=0) {
  display.clearDisplay();
  header("ABOUT");

  display.setTextColor(WHITE);
  display.setTextSize(1);

  display.setCursor(0, 10);
  display.print("With modules:  ESP32; Oled_128x64; INA219.");

  if (blink >= 15) {
    display.fillRect(0, 28, 128, 25, WHITE);
    display.setTextColor(BLACK);
  }
  display.setCursor(6, 29);
  display.printf("MAX VOLTAGE:  %s  MAX CURRENT:  %s  LOAD:       %s",MAX_VOLT,MAX_CURR,LOAD);
  display.setTextColor(WHITE);
  
  footer("PREVIOUS",digitalRead(BTN_LEFT),"EXIT",digitalRead(BTN_RIGHT));
  display.display();
  
  return;
}


void MENU_40() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1); 

  display.drawRoundRect(0, 0, 128, 52, 3, WHITE);
  display.drawRoundRect(1, 1, 128-2, 52-2, 2, WHITE);
  display.drawRoundRect(2, 2, 128-4, 52-4, 2, WHITE);

  display.drawBitmap(64-(save_width/2), 4, save, save_width, save_height, 1);

  display.setCursor(19, 28);
  display.print("Save Poste Rate");
  display.setCursor(64-(post_rate_tag[post_rate_index].length()+1)*6/2, 38);
  display.printf("%s?",post_rate_tag[post_rate_index].c_str());
  
  footer("YES",digitalRead(BTN_LEFT),"NO",digitalRead(BTN_RIGHT));
  display.display();
  
  return;
}


void MENU_41() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1); 

  display.drawRoundRect(0, 0, 128, 52, 3, WHITE);
  display.drawRoundRect(1, 1, 128-2, 52-2, 2, WHITE);
  display.drawRoundRect(2, 2, 128-4, 52-4, 2, WHITE);

  display.drawBitmap(64-(save_width/2), 4, save, save_width, save_height, 1);

  display.setCursor(16, 28);
  display.print("Save Sample Rate");
  display.setCursor(64-(sample_rate_tag[sample_rate_index].length()+1)*6/2, 38);
  display.printf("%s?",sample_rate_tag[sample_rate_index].c_str());
  
  footer("YES",digitalRead(BTN_LEFT),"NO",digitalRead(BTN_RIGHT));
  display.display();
  
  return;
}


void MENU_42() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1); 

  display.drawRoundRect(0, 0, 128, 52, 3, WHITE);
  display.drawRoundRect(1, 1, 128-2, 52-2, 2, WHITE);
  display.drawRoundRect(2, 2, 128-4, 52-4, 2, WHITE);

  display.drawBitmap(64-(save_width/2), 4, save, save_width, save_height, 1);

  display.setCursor(4, 28);
  display.print("Save Max/Min Display");
  display.setCursor(64-(mm_display_tag[mm_display_index].length()+1)*6/2, 38);
  display.printf("%s?",mm_display_tag[mm_display_index].c_str());
  
  footer("YES",digitalRead(BTN_LEFT),"NO",digitalRead(BTN_RIGHT));
  display.display();
  
  return;
}


// Menu of the calibration
void MENU_CAL_00(DataInfo *datas, uint32_t t=0, int sm=0) {
  display.clearDisplay();
  header("CALIBRATION");

  display.setTextColor(WHITE);
  display.setTextSize(1); 

  display.setCursor(0, 10);
  display.printf("VSHUNT: %3.2f mV\n", datas->vshunt);
  display.printf("VBUS:   %2.3f V\n", datas->vbus);
  display.printf("CSHUNT: %4.2f mA\n", datas->cshunt);
  display.printf("PSHUNT: %5.1f mW\n", datas->pshunt);
  display.printf("CORE1: %d ms| S_M: %d\n", t, sm);
  
  footer("CAL",digitalRead(BTN_LEFT),"EXIT",digitalRead(BTN_RIGHT));
  display.display();
  
  return;
}


void MENU_CAL_01(DataInfo *datas, uint32_t t=0, int sm=0, int change=0, uint8_t blink=0) {
  
  char digits[5];
  sprintf(digits,"%05d",ina219_cal_400mA);
  
  display.clearDisplay();
  header("CAL=400mA | S_M=0");

  display.setTextColor(WHITE);
  display.setTextSize(1); 

  display.setCursor(0, 14);
  display.printf("  CAL_INA => ");
  if (change == 5 and blink >= 15) {
    display.setTextColor(BLACK,WHITE);
  }
  display.printf("%c",digits[0]);
  display.setTextColor(WHITE);
  if (change == 4 and blink >= 15) {
    display.setTextColor(BLACK,WHITE);
  }
  display.printf("%c",digits[1]);
  display.setTextColor(WHITE);
  if (change == 3 and blink >= 15) {
    display.setTextColor(BLACK,WHITE);
  }
  display.printf("%c",digits[2]);
  display.setTextColor(WHITE);
  if (change == 2 and blink >= 15) {
    display.setTextColor(BLACK,WHITE);
  }
  display.printf("%c",digits[3]);
  display.setTextColor(WHITE);
  if (change == 1 and blink >= 15) {
    display.setTextColor(BLACK,WHITE);
  }
  display.printf("%c",digits[4]);
  display.setTextColor(WHITE);

  display.setCursor(0, 26);
  display.printf("VSHUNT: %3.2f mV\n", datas->vshunt);
  display.printf("CSHUNT: %4.2f mA\n", datas->cshunt);
  display.printf("CORE1: %d ms| S_M: %d\n", t, sm);

  if (change == 0) {
    footer("CHANGE",digitalRead(BTN_LEFT),"NEXT",digitalRead(BTN_RIGHT));
  } else {
    footer("CHANGE",digitalRead(BTN_LEFT),"UP",digitalRead(BTN_RIGHT));
  }
  display.display();
  
  return;
}

void MENU_CAL_02(DataInfo *datas, uint32_t t=0, int sm=0, int change=0, uint8_t blink=0) {
  
  char digits[5];
  sprintf(digits,"%05d",ina219_cal_800mA);
  
  display.clearDisplay();
  header("CAL=800mA | S_M=1");

  display.setTextColor(WHITE);
  display.setTextSize(1); 

  display.setCursor(0, 14);
  display.printf("  CAL_INA => ");
  if (change == 5 and blink >= 15) {
    display.setTextColor(BLACK,WHITE);
  }
  display.printf("%c",digits[0]);
  display.setTextColor(WHITE);
  if (change == 4 and blink >= 15) {
    display.setTextColor(BLACK,WHITE);
  }
  display.printf("%c",digits[1]);
  display.setTextColor(WHITE);
  if (change == 3 and blink >= 15) {
    display.setTextColor(BLACK,WHITE);
  }
  display.printf("%c",digits[2]);
  display.setTextColor(WHITE);
  if (change == 2 and blink >= 15) {
    display.setTextColor(BLACK,WHITE);
  }
  display.printf("%c",digits[3]);
  display.setTextColor(WHITE);
  if (change == 1 and blink >= 15) {
    display.setTextColor(BLACK,WHITE);
  }
  display.printf("%c",digits[4]);
  display.setTextColor(WHITE);

  display.setCursor(0, 26);
  display.printf("VSHUNT: %3.2f mV\n", datas->vshunt);
  display.printf("CSHUNT: %4.2f mA\n", datas->cshunt);
  display.printf("CORE1: %d ms| S_M: %d\n", t, sm);

  if (change == 0) {
    footer("CHANGE",digitalRead(BTN_LEFT),"NEXT",digitalRead(BTN_RIGHT));
  } else {
    footer("CHANGE",digitalRead(BTN_LEFT),"UP",digitalRead(BTN_RIGHT));
  }
  display.display();
  
  return;
}

void MENU_CAL_03(DataInfo *datas, uint32_t t=0, int sm=0, int change=0, uint8_t blink=0) {
  
  char digits[5];
  sprintf(digits,"%05d",ina219_cal_1600mA);
  
  display.clearDisplay();
  header("CAL=1600mA | S_M=2");

  display.setTextColor(WHITE);
  display.setTextSize(1); 

  display.setCursor(0, 14);
  display.printf("  CAL_INA => ");
  if (change == 5 and blink >= 15) {
    display.setTextColor(BLACK,WHITE);
  }
  display.printf("%c",digits[0]);
  display.setTextColor(WHITE);
  if (change == 4 and blink >= 15) {
    display.setTextColor(BLACK,WHITE);
  }
  display.printf("%c",digits[1]);
  display.setTextColor(WHITE);
  if (change == 3 and blink >= 15) {
    display.setTextColor(BLACK,WHITE);
  }
  display.printf("%c",digits[2]);
  display.setTextColor(WHITE);
  if (change == 2 and blink >= 15) {
    display.setTextColor(BLACK,WHITE);
  }
  display.printf("%c",digits[3]);
  display.setTextColor(WHITE);
  if (change == 1 and blink >= 15) {
    display.setTextColor(BLACK,WHITE);
  }
  display.printf("%c",digits[4]);
  display.setTextColor(WHITE);

  display.setCursor(0, 26);
  display.printf("VSHUNT: %3.2f mV\n", datas->vshunt);
  display.printf("CSHUNT: %4.2f mA\n", datas->cshunt);
  display.printf("CORE1: %d ms| S_M: %d\n", t, sm);

  if (change == 0) {
    footer("CHANGE",digitalRead(BTN_LEFT),"NEXT",digitalRead(BTN_RIGHT));
  } else {
    footer("CHANGE",digitalRead(BTN_LEFT),"UP",digitalRead(BTN_RIGHT));
  }
  display.display();
  
  return;
}

void MENU_CAL_04(DataInfo *datas, uint32_t t=0, int sm=0, int change=0, uint8_t blink=0) {
  
  char digits[5];
  sprintf(digits,"%05d",ina219_cal_3200mA);
  
  display.clearDisplay();
  header("CAL=3200mA | S_M=3");

  display.setTextColor(WHITE);
  display.setTextSize(1); 

  display.setCursor(0, 14);
  display.printf("  CAL_INA => ");
  if (change == 5 and blink >= 15) {
    display.setTextColor(BLACK,WHITE);
  }
  display.printf("%c",digits[0]);
  display.setTextColor(WHITE);
  if (change == 4 and blink >= 15) {
    display.setTextColor(BLACK,WHITE);
  }
  display.printf("%c",digits[1]);
  display.setTextColor(WHITE);
  if (change == 3 and blink >= 15) {
    display.setTextColor(BLACK,WHITE);
  }
  display.printf("%c",digits[2]);
  display.setTextColor(WHITE);
  if (change == 2 and blink >= 15) {
    display.setTextColor(BLACK,WHITE);
  }
  display.printf("%c",digits[3]);
  display.setTextColor(WHITE);
  if (change == 1 and blink >= 15) {
    display.setTextColor(BLACK,WHITE);
  }
  display.printf("%c",digits[4]);
  display.setTextColor(WHITE);

  display.setCursor(0, 26);
  display.printf("VSHUNT: %4.2f mV\n", datas->vshunt);
  display.printf("CSHUNT: %3.2f mA\n", datas->cshunt);
  display.printf("CORE1: %d ms| S_M: %d\n", t, sm);

  if (change == 0) {
    footer("CHANGE",digitalRead(BTN_LEFT),"NEXT",digitalRead(BTN_RIGHT));
  } else {
    footer("CHANGE",digitalRead(BTN_LEFT),"UP",digitalRead(BTN_RIGHT));
  }
  display.display();
  
  return;
}

void MENU_CAL_05() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1); 

  display.drawRoundRect(0, 0, 128, 52, 3, WHITE);
  display.drawRoundRect(1, 1, 128-2, 52-2, 2, WHITE);
  display.drawRoundRect(2, 2, 128-4, 52-4, 2, WHITE);

  display.drawBitmap(64-(save_width/2), 4, save, save_width, save_height, 1);

  display.setCursor(16, 28);
  display.print("Save Calibration");
  display.setCursor(43, 38);
  display.printf("values?",sample_rate_tag[sample_rate_index].c_str());
  
  footer("YES",digitalRead(BTN_LEFT),"NO",digitalRead(BTN_RIGHT));
  display.display();
  
  return;
}


// Setup ESP32
void setup()   {

  dataPipe = xQueueCreate(10,sizeof(DataInfo));

  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  
  Serial.begin(115200, SERIAL_8N1); // Speed baudrate, [8 send bits, No parity, 1 stop bit]
  while (!Serial) {
      // will pause, until serial console opens
      delay(1);
  }

  // Load eeprom
  load_eeprom();

  // Load CAL eeprom
  cal_load_eeprom();
  
  // Setup the OLED display
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  display.setRotation(ROTATION);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed: restart."));
    delay(2000);
    ESP.restart();     // Don't proceed, restart
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(4000); // Pause for 4 seconds

  // Clear the buffer
  display.clearDisplay();

  // Try to initialize the INA219
  if (! ina219.begin()) {
    Serial.println("Failed to find INA219 chip: restart.");
    delay(2000);
    ESP.restart();     // Don't proceed, restart
  }
  // By default the INA219 will be calibrated with a range of 32V, 2A.
  // However uncomment one of the below to change the range.  A smaller
  // range can't measure as large of values but will measure with slightly
  // better precision.
  //ina219.setCalibration_32V_1A();
  //ina219.setCalibration_32V_2A();
  ina219.setCalibration_16V_400mA(ina219_cal_400mA);
  //ina219.setCalibration_16V_800mA(ina219_cal_800mA);
  //ina219.setCalibration_16V_1600mA(ina219_cal_1600mA);
  //ina219.setCalibration_16V_3200mA(ina219_cal_3200mA);

  // Enter mode calibration
  while (!digitalRead(BTN_LEFT) and !digitalRead(BTN_RIGHT))
  {
    menu = 90;
    display.setCursor(0, 0);
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.printf("Enter mode Cal...");
    display.display();
    delay(1000);
    display.clearDisplay();
  }

  xTaskCreatePinnedToCore(UserInterfaceTask, "ui_task", 4096, NULL, 3, &UI_Handle, 1);

}


// Main loop ESP32
void loop() {

  DataInfo dataSend;   

  while(true) {

    xQueueReceive( dataPipe, &( dataSend ), (TickType_t) pdMS_TO_TICKS(1)); // Read pipe data measurement (wait 1ms if busy or full) 
     
    now_millis = millis();

    // PING
    if (Serial.available()) {
      String input = "";
      while(Serial.available())
        input += String(char(Serial.read()));
      //if (Serial.readStringUntil('\n') == "OK") {
      //Serial.flush();
      //Serial.print(input);
      if (input == "OK\n") {  
        digitalWrite(LED, HIGH);
        communication = true;
        no_reply = 0;
      }
    }


    // POST and MONITOR
    if ((now_millis - last_millis_monitor) >= monitor_rate) {

      Serial.flush();
      last_millis_monitor = now_millis;
      
      if (communication and (no_reply < attempts)) { // Communication: OK

        no_reply++;
        
        if (((now_millis - last_millis_post) >= post_rate_value[post_rate]) or new_connection) {
          new_connection = false;
          last_millis_post = now_millis;
          Serial.printf("%.3f;%.2f;%.1f;P\n",dataSend.vload,dataSend.cshunt,dataSend.pshunt);
          post++;
        } else {
          Serial.printf("%.3f;%.2f;%.1f;M\n",dataSend.vload,dataSend.cshunt,dataSend.pshunt);
        }
        
      } else { // Communication: NO

        Serial.printf("%.3f;%.2f;%.1f;M\n",dataSend.vload,dataSend.cshunt,dataSend.pshunt);

        digitalWrite(LED, !digitalRead(LED));
        communication = false;
        new_connection = true;
        last_millis_post = 0;
        
      }
    }
  
  
    vTaskDelay(1 / portTICK_PERIOD_MS);
  
  }
  
}


void UserInterfaceTask( void *arg )
{

  DataInfo dataRead, dataSum, dataMeans;
  uint64_t CORE_1 = 0;
  uint64_t timer_millis = 0;
  int loop_time = 50; // ms
  int loop_delay = 0;
  int means_count = 0;
  int state_machine = 0; // State machine scale, 0 = 400mA; 1 = 800mA; 2 = 1600mA; 3 = 3200mA

  // Init Max and Min
  dataRead.max_voltage = -16.000f;
  dataRead.min_voltage = +16.000f;
  dataRead.max_current = -3200.00f;
  dataRead.min_current = +3200.00f;
  dataRead.max_power   = -51200.0f;
  dataRead.min_power   = +51200.0f;
  
  while(true) {

    timer_millis = millis();

    dataRead.vshunt = ina219.getShuntVoltage_mV();
    dataRead.vbus   = ina219.getBusVoltage_V();
    dataRead.cshunt = ina219.getCurrent_mA();
    dataRead.pshunt = ina219.getPower_mW();

    // STATE MACHINE SCALE
    // STATE=0; Scale 16V / 400mA
    if (state_machine == 0) {
      if (dataRead.vshunt >= 39.50f) {
        ina219.setCalibration_16V_800mA(ina219_cal_800mA);
        state_machine = 1;
      }
    }

    // STATE=1; Scale 16V / 800mA
    if (state_machine == 1) {
      if (dataRead.vshunt >= 79.50f) {
        ina219.setCalibration_16V_1600mA(ina219_cal_1600mA);
        state_machine = 2;
      }
      if (dataRead.vshunt < 39.50f) {
        ina219.setCalibration_16V_400mA(ina219_cal_400mA);
        state_machine = 0;
      }
    }

    // STATE=2; Scale 16V / 1600mA
    if (state_machine == 2) {
      if (dataRead.vshunt >= 159.50f) {
        ina219.setCalibration_16V_3200mA(ina219_cal_3200mA);
        state_machine = 3;
      }
      if (dataRead.vshunt < 79.50f) {
        ina219.setCalibration_16V_800mA(ina219_cal_800mA);
        state_machine = 1;
      }
    }

    // STATE=3; Scale 16V / 3200mA
    if (state_machine == 3) {
      if (dataRead.vshunt < 159.50f) {
        ina219.setCalibration_16V_1600mA(ina219_cal_1600mA);
        state_machine = 2;
      }
    }

    // STATE=4; Scale 16V / 400mA (FORCED)
    if (state_machine == 4) {
       ina219.setCalibration_16V_400mA(ina219_cal_400mA);
    }

    // STATE=5; Scale 16V / 800mA (FORCED)
    if (state_machine == 5) {
       ina219.setCalibration_16V_800mA(ina219_cal_800mA);
    }

    // STATE=6; Scale 16V / 1600mA (FORCED)
    if (state_machine == 6) {
       ina219.setCalibration_16V_1600mA(ina219_cal_1600mA);
    }

    // STATE=7; Scale 16V / 3200mA (FORCED)
    if (state_machine == 7) {
       ina219.setCalibration_16V_3200mA(ina219_cal_3200mA);
    }
    
    // ------------

    dataSum.vshunt += dataRead.vshunt; 
    dataSum.vbus += dataRead.vbus; 
    dataSum.cshunt += dataRead.cshunt; 
    dataSum.pshunt += dataRead.pshunt;
    means_count++;

    if (means_count >= (sample_rate_value[sample_rate] / loop_time)) {
      dataMeans.vshunt = dataSum.vshunt / means_count; 
      dataMeans.vbus = dataSum.vbus / means_count; 
      dataMeans.cshunt = dataSum.cshunt / means_count; 
      dataMeans.pshunt = dataSum.pshunt / means_count;
      dataMeans.vload = dataMeans.vbus + (dataMeans.vshunt / 1000.000f);
      dataSum.vshunt = 0; 
      dataSum.vbus = 0; 
      dataSum.cshunt = 0; 
      dataSum.pshunt = 0;
      means_count = 0;

      // Restore Max and Min
      if (!mm_display) {
        dataRead.max_voltage = -16.000f;
        dataRead.min_voltage = +16.000f;
        dataRead.max_current = -3200.00f;
        dataRead.min_current = +3200.00f;
        dataRead.max_power   = -51200.0f;
        dataRead.min_power   = +51200.0f;
        dataMeans.max_voltage = 0.000f;
        dataMeans.min_voltage = 0.000f;
        dataMeans.max_current = 0.00f;
        dataMeans.min_current = 0.00f;
        dataMeans.max_power   = 0.0f;
        dataMeans.min_power   = 0.0f;
      } else {
      // Set Max and Min
        dataMeans.max_voltage = dataRead.max_voltage = dataMeans.vload > dataRead.max_voltage ? dataMeans.vload : dataRead.max_voltage;
        dataMeans.min_voltage = dataRead.min_voltage = dataMeans.vload < dataRead.min_voltage ? dataMeans.vload : dataRead.min_voltage;
        dataMeans.max_current = dataRead.max_current = dataMeans.cshunt > dataRead.max_current ? dataMeans.cshunt : dataRead.max_current;
        dataMeans.min_current = dataRead.min_current = dataMeans.cshunt < dataRead.min_current ? dataMeans.cshunt : dataRead.min_current;
        dataMeans.max_power   = dataRead.max_power   = dataMeans.pshunt > dataRead.max_power ? dataMeans.pshunt : dataRead.max_power;
        dataMeans.min_power   = dataRead.min_power   = dataMeans.pshunt < dataRead.min_power ? dataMeans.pshunt : dataRead.min_power;
      }
    }

    
    
    xQueueSend( dataPipe, (void *) &dataMeans, (TickType_t) 0); // Write pipe data measurement (not wait if busy or full) 

    toggle = (++toggle) % 30;
    
    if (menu == 0)  { MENU_00(&dataMeans); if (btn_left_falling()) menu=1; if (btn_right_falling()) menu=20;}                                   // Measurement
    if (menu == 1)  { MENU_01(&dataMeans, mm_display); if (btn_left_falling()) menu=2; if (btn_right_falling() and mm_display == 1) menu=10;}   // Voltage
    if (menu == 2)  { MENU_02(&dataMeans, mm_display); if (btn_left_falling()) menu=3; if (btn_right_falling() and mm_display == 1) menu=11;}   // Current
    if (menu == 3)  { MENU_03(&dataMeans, mm_display); if (btn_left_falling()) menu=4; if (btn_right_falling() and mm_display == 1) menu=12;}   // Power
    if (menu == 4)  { MENU_04(); if (btn_left_falling()) menu=0; }                                                                              // Status
  
    if (menu == 10) { MENU_10(); if (btn_left_falling()) {menu=1; dataRead.max_voltage = -16.000f; dataRead.min_voltage = +16.000f; 
                                                          dataMeans.max_voltage = 0.000f; dataMeans.min_voltage = 0.000f;} if (btn_right_falling()) menu=1;}  // Voltage Clear Max/Min?
    if (menu == 11) { MENU_11(); if (btn_left_falling()) {menu=2; dataRead.max_current = -3200.00f; dataRead.min_current = +3200.00f;
                                                          dataMeans.max_current = 0.00f; dataMeans.min_current = 0.00f;}   if (btn_right_falling()) menu=2;}  // Current Clear Max/Min?
    if (menu == 12) { MENU_12(); if (btn_left_falling()) {menu=3; dataRead.max_power = -51200.0f; dataRead.min_power = +51200.0f;
                                                          dataMeans.max_power = 0.0f; dataMeans.min_power = 0.0f;}         if (btn_right_falling()) menu=3;}  // Power Clear Max/Min?
  
    if (menu == 20) { MENU_20(); if (btn_left_falling()) menu=21; if (btn_right_falling()) {menu=30; post_rate_index=post_rate;}}      // Settings Post Rate
    if (menu == 21) { MENU_21(); if (btn_left_falling()) menu=22; if (btn_right_falling()) {menu=31; sample_rate_index=sample_rate;}}  // Settings Sample Rate
    if (menu == 22) { MENU_22(); if (btn_left_falling()) menu=23; if (btn_right_falling()) {menu=32; mm_display_index=mm_display;}}    // Settings Max/Min Display
    if (menu == 23) { MENU_23(); if (btn_left_falling()) menu=24; if (btn_right_falling()) menu=33;}                                // Settings Reset Factory
    if (menu == 24) { MENU_24(); if (btn_left_falling()) menu=25; if (btn_right_falling()) menu=34;}                                // Settings About
    if (menu == 25) { MENU_25(); if (btn_left_falling()) menu=20; if (btn_right_falling()) menu=0;}                                 // Settings Exit
  
    if (menu == 30) { MENU_30(post_rate_index);   if (btn_left_falling()) {post_rate_index = (++post_rate_index) % 8;}     if (btn_right_falling()) menu=40;} // Post Rate
    if (menu == 31) { MENU_31(sample_rate_index); if (btn_left_falling()) {sample_rate_index = (++sample_rate_index) % 7;} if (btn_right_falling()) menu=41;} // Sample Rate
    if (menu == 32) { MENU_32(mm_display_index);  if (btn_left_falling()) {mm_display_index = (++mm_display_index) % 2;}   if (btn_right_falling()) menu=42;} // Max/Min Display
    if (menu == 33) { MENU_33(); if (btn_left_falling()) {menu=23; reset_factory();} if (btn_right_falling()) menu=23;}             // Reset Factory?
    if (menu == 34) { MENU_34(); if (btn_right_falling()) menu=35;}                                                                 // About 1
    if (menu == 35) { MENU_35(toggle); if (btn_left_falling()) menu=34;              if (btn_right_falling()) menu=24;}             // About 2
  
    if (menu == 40) { MENU_40(); if (btn_left_falling()) {menu=20; post_rate=post_rate_index; write_eeprom("post_rate",post_rate_index);}         if (btn_right_falling()) menu=20;}// Post Rate Save?
    if (menu == 41) { MENU_41(); if (btn_left_falling()) {menu=21; sample_rate=sample_rate_index; write_eeprom("sample_rate",sample_rate_index);} if (btn_right_falling()) menu=21;}// Sample Rate Save?
    if (menu == 42) { MENU_42(); if (btn_left_falling()) {menu=22; mm_display=mm_display_index; write_eeprom("mm_display",mm_display_index);}     if (btn_right_falling()) menu=22;}// Max/Min Display Save?

    if (menu == 90) { MENU_CAL_00(&dataRead, CORE_1, state_machine); if (btn_left_falling()) {menu=91; change_cal=0;} if (btn_right_falling()) menu=95;}  // Calibration
    if (menu == 91) { state_machine = 4; MENU_CAL_01(&dataRead, CORE_1, state_machine-4, change_cal, toggle); if (btn_left_falling()) {change_cal = (++change_cal) % 6;}
                      if (btn_right_falling()) {if (change_cal==0) menu=92; else {ina219_cal_400mA = int(ina219_cal_400mA + pow(10,change_cal-1)) % 100000;}}}  // Calibration 400mA (State Machine 0)
    if (menu == 92) { state_machine = 5; MENU_CAL_02(&dataRead, CORE_1, state_machine-4, change_cal, toggle); if (btn_left_falling()) {change_cal = (++change_cal) % 6;}
                      if (btn_right_falling()) {if (change_cal==0) menu=93; else {ina219_cal_800mA = int(ina219_cal_800mA + pow(10,change_cal-1)) % 100000;}}}  // Calibration 800mA (State Machine 1)
    if (menu == 93) { state_machine = 6; MENU_CAL_03(&dataRead, CORE_1, state_machine-4, change_cal, toggle); if (btn_left_falling()) {change_cal = (++change_cal) % 6;}
                      if (btn_right_falling()) {if (change_cal==0) menu=94; else {ina219_cal_1600mA = int(ina219_cal_1600mA + pow(10,change_cal-1)) % 100000;}}}  // Calibration 1600mA (State Machine 2)
    if (menu == 94) { state_machine = 7; MENU_CAL_04(&dataRead, CORE_1, state_machine-4, change_cal, toggle); if (btn_left_falling()) {change_cal = (++change_cal) % 6;}
                      if (btn_right_falling()) {if (change_cal==0) {menu=90; state_machine = 3;} else {ina219_cal_3200mA = int(ina219_cal_3200mA + pow(10,change_cal-1)) % 100000;}}}  // Calibration 3200mA (State Machine 3)

    if (menu == 95) { MENU_CAL_05(); if (btn_left_falling()) {cal_write_eeprom("cal_400mA", ina219_cal_400mA); 
                                                              cal_write_eeprom("cal_800mA", ina219_cal_800mA);
                                                              cal_write_eeprom("cal_1600mA", ina219_cal_1600mA);
                                                              cal_write_eeprom("cal_3200mA", ina219_cal_3200mA); ESP.restart();} if (btn_right_falling()) ESP.restart();}  //Calibration Save?
    
    loop_delay = loop_time - ( millis() - timer_millis );
    
    if (loop_delay < 1)  loop_delay = 1;

    vTaskDelay(loop_delay / portTICK_PERIOD_MS);

    CORE_1 = millis() - timer_millis;

  }
}
