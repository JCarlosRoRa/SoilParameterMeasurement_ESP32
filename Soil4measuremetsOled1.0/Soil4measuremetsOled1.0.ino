#include <Adafruit_SSD1306.h>
#include <splash.h>

#include <Adafruit_GFX.h>
#include <Adafruit_GrayOLED.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <gfxfont.h>


#include <HardwareSerial.h>
#include <Wire.h>



// RO to pin 8 & DI to pin 9 when using AltSoftSerial
#define RE 19
#define DE 18

#define SCREEN_WIDTH 128 // OLED width,  in pixels EL MAXIMO DE COLUMNAS
#define SCREEN_HEIGHT 64 // OLED height, in pixels EL MAXIMO DE FINAL 

Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

/*
  // const byte temp[] = {0x02,0x03, 0x00, 0x13, 0x00, 0x01, 0x75, 0xcf};//
  const byte temp[] = {0x01, 0x03, 0x00, 0x01, 0x00, 0x01, 0xD5, 0xCA};//
  // const byte mois[]  = {0x02,0x03,0x00,0x12,0x00,0x01,0x24,0x0F};
  const byte mois[]  = {0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x84, 0x0A};
  // const byte econ[] = {0x02,0x03, 0x00, 0x15, 0x00, 0x01, 0x95, 0xce};
  const byte econ[] = {0x01, 0x03, 0x00, 0x02, 0x00, 0x01, 0x25, 0xCA};
  // const byte ph[] = {0x02,0x03, 0x00, 0x06, 0x00, 0x01, 0x64, 0x0b};//0x0B64
  const byte ph[] = {0x01, 0x03, 0x00, 0x03, 0x00, 0x01, 0x74, 0x0A};//0x0B64

  const byte nitro[] = {0x01, 0x03, 0x00, 0x04, 0x00, 0x01, 0xC5, 0xCB};
  const byte phos[] = {0x01, 0x03, 0x00, 0x05, 0x00, 0x01, 0x94, 0x0B};
  const byte pota[] = {0x01, 0x03, 0x00, 0x06, 0x00, 0x01, 0x64, 0x0B};
*/


const uint32_t TIMEOUT = 500UL;

const byte sensorVals[4][8] = {
  {0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x84, 0x0A},//moist
  {0x01, 0x03, 0x00, 0x01, 0x00, 0x01, 0xD5, 0xCA},//temp
  {0x01, 0x03, 0x00, 0x02, 0x00, 0x01, 0x25, 0xCA},//econ
  {0x01, 0x03, 0x00, 0x03, 0x00, 0x01, 0x74, 0x0A},//ph
};

//byte values[11];
HardwareSerial mod(2);

float envhumidity = 0.0, envtemperature = 0.0, soil_ph = 0.0, soil_mois = 0.0, soil_temp = 0.0;
uint16_t val1 = 0, val2 = 0, val3 = 0, val4 = 0;

unsigned long delayOneK = 1000;
unsigned long pastTime = millis();
int state = 0;

void setup() {

  Serial.begin(115200);
  mod.begin(4800, SERIAL_8N1,16,17);

  pinMode(RE, OUTPUT);
  pinMode(DE, OUTPUT);

  // put RS-485 into receive mode
  digitalWrite(DE, LOW);
  digitalWrite(RE, LOW);

  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("failed to start SSD1306 OLED"));
    while (1);
  }

  delay(3000);

  oled.clearDisplay(); // clear display

  oled.setTextSize(1);         // set text size
  oled.setTextColor(WHITE);    // set text color
  oled.setCursor(20, 30);       // set position to display columna fila 
  oled.println("MEDIDOR"); // set text
  oled.setCursor(20, 50);       // set position to display
  oled.println("AGRICOLA"); // set text
  oled.display();              // display on OLED
  Serial.println("Agriculture Kit Sensor Ready");
  delay(1000);
  
}


void loop() {

  switch ( state )
  {
    case 0:
      if ( (millis() - pastTime ) >= delayOneK )
      {
        val1 = GetValue(state);
        soil_mois = val1 / 10.0;
        state = 1;
        Serial.print("Moisture: "); Serial.print(soil_mois); Serial.println(" %");
        pastTime = millis();
      }
      break;
    case 1:
      if ( (millis() - pastTime) >= delayOneK )
      {
        soil_temp = GetValue(state) / 10.0;
        Serial.print("Temperature: "); Serial.print(soil_temp); Serial.println(" C");
        pastTime = millis();
        state = 2;
      }
      break;
    case 2:
      if ( (millis() - pastTime) >= delayOneK )
      {
        val3 = GetValue(state);
        Serial.print("EC: "); Serial.print(val3); Serial.println(" us/cm");
        pastTime = millis();
        state = 3;
      }
      break;
    case 3:
      if ( (millis() - pastTime) >= delayOneK )
      {
        val4 = GetValue(state) / 10;
        soil_ph = val4;
        Serial.print("ph: "); Serial.print(soil_ph); Serial.println(" ph");
        pastTime = millis();
        state = 4;
      }
      break;
    default:
       state = 0;
      break;
  }
  datos_oled();

}


uint16_t GetValue(byte val) {
  if (val > 6) return 0;
  uint32_t startTime = 0;
  uint8_t  byteCount = 0;
  byte buff[20];

  digitalWrite(DE, HIGH);
  digitalWrite(RE, HIGH);
  delay(10);
  mod.write(sensorVals[val], sizeof(sensorVals[val]));
  mod.flush();
  digitalWrite(DE, LOW);
  digitalWrite(RE, LOW);

  startTime = millis();
  while (millis() - startTime <= TIMEOUT) {
    if (mod.available() && byteCount < sizeof(buff)) {
      buff[byteCount++] = mod.read();
      Serial.print(buff[byteCount - 1], HEX); Serial.print(":");
    }
  }
  Serial.println();
  return (int16_t)(buff[3] << 8 | buff[4]);
}



void datos_oled(){

  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setCursor(10, 5);
  oled.print("Temp: ");
  oled.print(soil_temp);
  oled.setTextSize(1);
  oled.print(" C");
  

  oled.setTextSize(1);
  oled.setCursor(10, 20);
  oled.print("Humedad: ");
  oled.print(soil_mois);
  oled.setTextSize(1);
  oled.print("%");
  

  oled.setTextSize(1);
  oled.setCursor(10, 35);
  oled.print("Ec: ");
  oled.print(val3);
  oled.setTextSize(1);
  oled.print(" us/cm");
              
  
  oled.setTextSize(1);
  oled.setCursor(10, 50);
  oled.print("Ph: ");
  oled.print(soil_ph);
  oled.setTextSize(1);
  oled.print(" ph");
  oled.display();
}
 