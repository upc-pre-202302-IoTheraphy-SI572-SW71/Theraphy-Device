#include <LiquidCrystal_I2C.h>
#include "DHTesp.h"
#include <HTTPClient.h>

#define Pot 35
#define Ledpot 4
const float Res = 4095.0;
const float Vmax = 3.3;
const String serverUrl = "http://localhost:8080/api/v1/iotDevice";
#define direccion 0x27
#define Sensor 23
#define col 16
#define ren 2
#define tri 12
#define eco 14
LiquidCrystal_I2C lcd(direccion, col, ren);
DHTesp DHT;


class IoTData {
  private:
    String temperature;
    String distance;
    String pulse;
    String humidity;
  public:
    IoTData() {}

    void setTemperature(String temperature) { this->temperature = temperature; }
    void setDistance(String distance) { this->distance = distance; }
    void setPulse(String pulse) { this->pulse = pulse; }
    void setHumidity(String humidity) { this->humidity = humidity; }

    String getTemperature() { return this->temperature; }
    String getDistance() { return this->distance; }
    String getPulse() { return this->pulse; }
    String getHumidity() { return this->humidity; }
};

void setup(){
  Serial.begin(115200);
  pinMode(Ledpot, OUTPUT);
  lcd.init();
  lcd.backlight();

  DHT.setup(Sensor, DHTesp::DHT22);

  pinMode(tri, OUTPUT);
  pinMode(eco, INPUT);
  digitalWrite(tri, LOW);
}
void loop(){
  int x =  analogRead(Pot);
  float volt = (x*Vmax)/Res;
  float luz = ((x*255)/4095);
  analogWrite(Ledpot, luz);
  TempAndHumidity  lectura = DHT.getTempAndHumidity();

  Serial.println("Temperatura[Celcius]: " + String(lectura.temperature, 2) + "°");
  Serial.println("Humidity: " + String(lectura.humidity, 1) + "%");

  delay(3000);

  digitalWrite(tri, LOW);
  digitalWrite(tri, HIGH);
  delayMicroseconds(10);
  digitalWrite(tri, LOW);
  long tiempo = pulseIn(eco, HIGH);
  long distancia = (tiempo/29.35)/2;
  Serial.print("Distancia cm: ");
  Serial.println(distancia);
  Serial.print("tiempo: ");
  Serial.println(tiempo);

  lcd.setCursor(1, 0);
  lcd.print("Pulso cardiaco");
  lcd.setCursor(6, 1);
  lcd.print(volt);
  delay(1000);
  lcd.clear();
  delay(100);

  lcd.setCursor(0, 0);
  lcd.print("Humedad "+ String(lectura.humidity, 1));
  lcd.setCursor(0, 1);
  lcd.print("Temperatura "+ String(lectura.temperature, 2));
  delay(1000);
  lcd.clear();
  delay(100);

  lcd.setCursor(0, 0);
  lcd.print("Distancia ");
  lcd.setCursor(0, 1);
  lcd.print(distancia);
  delay(1000);
  lcd.clear();
  delay(10);

  // Crear un objeto IoTData y asignar los datos
  IoTData data;
  data.setTemperature(String(lectura.temperature, 2));
  data.setDistance(String(distancia));
  data.setPulse(String(volt));
  data.setHumidity(String(lectura.humidity, 1));

  // Crear un objeto JSON con los datos
  String objetData = 
  "{\"temperature\":" + data.getTemperature() + 
  ",\"distance\":" + data.getDistance() + 
  ",\"pulse\":" + data.getPulse() + 
  ",\"humidity\":" + data.getHumidity() + "}";
}
