// Include Libraries
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED screen size in pixels
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Variable Declarations
unsigned long previousMillisGetHR = 0;     // Store last time Millis (to get Heartbeat) was updated.
unsigned long previousMillisResultHR = 0; // Store last time Millis (to get BPM) was updated.

const long intervalGetHR = 20;     // Interval for reading heart rate (Heartbeat) = 10ms.
const long intervalResultHR = 10000; // The reading interval for the result of the Heart Rate calculation is in 10 seconds.

int PulseSensorSignal;      // Variable to accommodate the signal value from the sensor
const int PulseSensorHRWire = 0;  // PulseSensor connected to ANALOG PIN 0 (A0 / ADC 0).
const int LED_A1 = A1;            // LED to detect when the heart is beating. The LED is connected to PIN A1 on the Arduino UNO.
int UpperThreshold = 550;         // Determine which Signal to "count as a beat", and which to ignore.
int LowerThreshold = 500;

int cntHB = 0;                     // Variable for counting the number of heartbeats.
boolean ThresholdStat = true;     // Variable for triggers in calculating heartbeats.
int BPMval = 0;                   // Variable to hold the result of heartbeats calculation.

int x=0;            // Variable axis x graph values to display on OLED
int y=0;            // Variable axis y graph values to display on OLED
int lastx=0;        // The graph's last x axis variable value to display on the OLED
int lasty=0;        // The graph's last y axis variable value to display on the OLED

// 'Heart_Icon', 16x16px - I drew this heart icon at: http://dotmatrixtool.com/
const unsigned char Heart_Icon [] PROGMEM = {
  0x00, 0x00, 0x18, 0x30, 0x3c, 0x78, 0x7e, 0xfc, 0xff, 0xfe, 0xff, 0xfe, 0xee, 0xee, 0xd5, 0x56, 
  0x7b, 0xbc, 0x3f, 0xf8, 0x1f, 0xf0, 0x0f, 0xe0, 0x07, 0xc0, 0x03, 0x80, 0x01, 0x00, 0x00, 0x00
};

// Setup function
void setup() {
  pinMode(LED_A1,OUTPUT);  // Set LED_3 PIN as Output.
  Serial.begin(9600);      // Set up Serial Communication at a certain speed.
  
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  // Address 0x3C for 128x32 and Address 0x3D for 128x64.
  // But on my 128x64 module the 0x3D address doesn't work. What works is the 0x3C address.
  // So please try which address works on your module.
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); //--> Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(1000);

  // Displays BPM value reading information
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 12); //--> (x position, y position)
  display.print("     Please wait");
  display.setCursor(0, 22); //--> (x position, y position)
  display.print("     10  seconds");
  display.setCursor(0, 32); //--> (x position, y position)
  display.print("       to get");
  display.setCursor(0, 42); //--> (x position, y position)
  display.print(" the Heart Rate value");
  display.display(); 
  delay(3000);

  // Displays the initial display of BPM value
  display.clearDisplay(); //--> for Clearing the display
  display.drawBitmap(0, 47, Heart_Icon, 16, 16, WHITE); //--> display.drawBitmap(x position, y position, bitmap data, bitmap width, bitmap height, color)
  display.drawLine(0, 43, 127, 43, WHITE); //--> drawLine(x1, y1, x2, y2, color)
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(20, 48); //--> (x position, y position)
  display.print(": 0 BPM");
  display.display(); 

  Serial.println();
  Serial.println("Please wait 10 seconds to get the BPM Value");
}
void loop() {
  GetHeartRate(); // Calling the GetHeartRate() subroutine
}

void GetHeartRate() {
  // Process of reading heart rate.
  unsigned long currentMillisGetHR = millis();

  if (currentMillisGetHR - previousMillisGetHR >= intervalGetHR) {
    previousMillisGetHR = currentMillisGetHR;

    PulseSensorSignal = analogRead(PulseSensorHRWire);

    if (PulseSensorSignal > UpperThreshold && ThresholdStat == true) {
      cntHB++;
      ThresholdStat = false;
      digitalWrite(LED_A1, HIGH);
    }

    if (PulseSensorSignal < LowerThreshold) {
      ThresholdStat = true;
      digitalWrite(LED_A1, LOW);
    }

    DrawGraph(); // Calling the DrawGraph() subroutine
  }

  // Process for getting the BPM value.
  unsigned long currentMillisResultHR = millis();

  if (currentMillisResultHR - previousMillisResultHR >= intervalResultHR) {
    previousMillisResultHR = currentMillisResultHR;

    BPMval = cntHB * 6; // The taken heart rate is for 10 seconds. So to get the BPM value, the total heart rate in 10 seconds x 6.
    Serial.print("BPM : ");
    Serial.println(BPMval);

    display.fillRect(20, 48, 108, 18, BLACK);

    display.drawBitmap(0, 47, Heart_Icon, 16, 16, WHITE);

    display.drawLine(0, 43, 127, 43, WHITE);

    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(20, 48);
    display.print(": ");
    display.print(BPMval);
    display.print(" BPM");
    display.display();

    cntHB = 0;
  }
}

void DrawGraph() {
  // Condition to reset the graphic display if it fills the width of the OLED screen
  if (x > 127) {
    display.fillRect(0, 0, 128, 42, BLACK);
    x = 0;
    lastx = 0;
  }

  // Process signal data to be displayed on OLED in graphic form
  int ySignal = PulseSensorSignal;
  if (ySignal > 850) ySignal = 850;
  if (ySignal < 350) ySignal = 350;
  int ySignalMap = map(ySignal, 350, 850, 0, 40);
  y = 40 - ySignalMap;
  // Displays the heart rate graph
  display.writeLine(lastx, lasty, x, y, WHITE);
  display.display();
  lastx = x;
  lasty = y;
  x++;
}