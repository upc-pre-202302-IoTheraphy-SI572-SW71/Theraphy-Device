#include "DHT.h"
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";

#define DHTPIN 16   
#define DHTTYPE DHT22 
DHT dht(DHTPIN, DHTTYPE);

void setup() {

  Serial.begin(9600);
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println(" Connected!");
}

void loop() {
  float humidity = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float degreesC = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float degreesF = dht.readTemperature(true);

  String humidityString = String(humidity) + "%";
  String degreesCString = String(degreesC) + " °C";
  String degreesFString = String(degreesF) + " °F";
  String pulse = "80 BPM";

 // Construye el JSON manualmente
  String jsonStr = "{\"temperature\":\"" +degreesCString   + "\"" +
                  ",\"distance\":\"" + degreesFString + "\"" +
                  ",\"pulse\":\"" + pulse + "\"" +
                  ",\"humidity\":\"" + humidityString  + "\"}";


  // Crea la solicitud HTTP
  HTTPClient http;
  http.begin("https://api-iotheraphy-production-909e.up.railway.app/api/v1/iotDevice"); // Reemplaza con la URL de tu endpoint
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

  delay(1000); // Espera un segundo antes de volver a enviar los datos
}
