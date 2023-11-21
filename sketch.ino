#include "DHT.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <utility>

const char* ssid = "Wokwi-GUEST";
const char* password = "";

#define DHTPIN 16
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

int iotDeviceId = 1;

const int sensorPin = 35;
int sensorValue;
const int ledv = 18;

float value;
const int sensorv = 2;
const int sensorc = 34;

unsigned long startTime = 0;  // Variable para almacenar el tiempo de inicio del evento
bool highValueState = false;  // Estado indicando si estamos en un periodo con value alto

std::pair<float, unsigned long> sensorvoltaje() {
  sensorValue = analogRead(sensorPin);
  value = fmap(sensorValue, 0, 4095, 5.0, 1000.0);

  unsigned long elapsedTime = 0;
  if (value > 5 && !highValueState) {  // Si value está por encima del umbral y no estamos en un periodo alto
    startTime = millis();  // Almacena el tiempo de inicio
    highValueState = true;  // Establece el estado a alto
    Serial.println("Entró en estado alto");
  }

  if (value <= 5 && highValueState) {  // Si value vuelve a bajar y estamos en un periodo alto
    elapsedTime = millis() - startTime;  // Calcula el tiempo transcurrido
    Serial.print("Tiempo alto: ");
    Serial.print(elapsedTime);
    Serial.println(" ms");
    highValueState = false;  // Restablece el estado a bajo
  }


  delay(1000);
  return std::make_pair(value, elapsedTime);
}

float fmap(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float corriente() {
  int sensorValue = analogRead(sensorc);
  float corriente = map(sensorValue, 0, 4095, 5.0, 80.0);


  return corriente;
}

void connnecToWiFi() {
  Serial.begin(3000);
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println(" Connected!");
}

void setup() {


  connnecToWiFi();
  analogReadResolution(12);
  Serial.print("IoTheraphy Device ID: ");
  Serial.println(iotDeviceId);
  Serial.begin(115200);

}

void loop() {
  float humidity = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float degreesC = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float degreesF = dht.readTemperature(true);

  //float amplitud = sensorvoltaje();
  auto sensorVoltageResult = sensorvoltaje();
  float amplitud = sensorVoltageResult.first;
  unsigned long elapsedTime = sensorVoltageResult.second;

  float frecuencia = corriente();

  String iotDeviceIdString = String(iotDeviceId);
  String humidityString = String(humidity);
  String degreesCString = String(degreesC);
  String degreesFString = String(degreesF);
  // Obtener el pulso base
  int pulseBase = 70;

  // Ajustar el pulso en función del voltaje
  int adjustedPulse = pulseBase + map(amplitud, 5.0, 1000.0, 0, 30);


  String pulse = String(adjustedPulse);


  Serial.print(humidityString);
  Serial.println(" %");

  Serial.print(degreesCString);
  Serial.println(" °C");

  Serial.print(pulse);
  Serial.println(" BPM");

  Serial.print(amplitud);
  Serial.println(" µV");

  Serial.print(frecuencia);
  Serial.println(" Hz");

  if (elapsedTime != 0) {
    Serial.print(elapsedTime);
    Serial.println(" ms");
  }




  // Construye el JSON manualmente
  String jsonStr = "{\"iotDeviceId\":\"" + iotDeviceIdString + "\"" +
                   ",\"humidity\":\"" + humidityString + "\"" +
                   ",\"temperature\":\"" + degreesCString + "\"" +
                   ",\"pulse\":\"" + pulse + "\"" +
                   ",\"mapAmplitude\":\"" + amplitud + "\"" +
                   ",\"mapFrequency\":\"" + frecuencia + "\"" +
                   ",\"mapDuration\":\"" + elapsedTime  + "\"}";


  // Crea la solicitud HTTP
  HTTPClient http;
  http.begin("https://api-iotheraphy-production.up.railway.app/api/v1/iotResults"); // Reemplaza con la URL de tu endpoint
  http.addHeader("Content-Type", "application/json");

  // Envía la solicitud POST con los datos JSON
  int httpCode = http.POST(jsonStr);
  if (httpCode == HTTP_CODE_CREATED) {
    Serial.println("Solicitud exitosa. Datos enviados.");
  } else {
    Serial.print("Error en la solicitud. Código de estado: ");
    Serial.println(httpCode);
    String errorMsg = http.errorToString(httpCode);
    Serial.println("Error: " + errorMsg);
  }

  http.end();
  delay(5000); // Espera un segundo antes de volver a enviar los datos
}