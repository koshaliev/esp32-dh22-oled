#include <DHT.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <cmath>
#include <WiFi.h>
#include <WebServer.h>

#define DHTPIN 32
#define DHTTYPE DHT22
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

// сенсор DHT 
DHT dhtSensor(DHTPIN, DHTTYPE);
float temperature = 0.0;
float humidity = 0.0;


// OLED дисплей
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


// Данные от точку доступа (AP)
const String ssid = "esp32-dev-wifi";
const String password = "esp32-dev";

// веб-сервер
WebServer server(80);


void updateSensorData() {
  humidity = dhtSensor.readHumidity();
  temperature = dhtSensor.readTemperature();
}

void showDataOnDisplay() {
  Serial.println("-----");
  Serial.println("Температура: " + String(temperature, 2) + " °C");
  Serial.println("Влажность: " + String(humidity, 2) + " %");

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 22);

  String text = "Temp: " + String(temperature, 2) + " C\n" +
                "Humidity: " + String(humidity, 2) + " %";

  display.println(text);
  display.display();
}

void setupSensorAndDisplay() {
  Serial.println("-----------");
  Serial.println("Запуск дисплея...");
  dhtSensor.begin();
  
  // инициализация дисплея
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Ошибка при инициализации OLED дисплей"));
    for (;;);
  }
  display.display();
  Serial.println("Дисплей запущен.");
  delay(1000);
  display.clearDisplay();
}

void showNetworks() {
  int networkCount = WiFi.scanNetworks();
  Serial.println("-----------");
  if (networkCount < 1) {
    Serial.println("Нет доступных сетей.");
  } else {
    Serial.println("Найдено сетей: " + String(networkCount));

    for (int i = 0; i < networkCount; i++) {
      Serial.println(String(i+1) + ": " + WiFi.SSID(i));
    }
  }
}

void sendRootHtml() {
  String response = R"(
    <!DOCTYPE html>
    <html>
      <head>
        <title>Веб-сервер на ESP32 DEV</title>
        <meta name="viewport" content="width=device-width, initial-scale=1">
      </head>
            
      <body>
        <h1>Данные с датчика</h1>

        <div>
          <p>Температура: TEMPERATURE_TEXT °C</p>
          <p>Влажность: HUMIDITY_TEXT %</p>
        </div>
      </body>
    </html>
  )";
  response.replace("TEMPERATURE_TEXT", String(temperature));
  response.replace("HUMIDITY_TEXT", String(humidity));
  server.send(200, "text/html", response);
}

void setupWebServer() {  
  Serial.println("-----------");
  Serial.println("Запуск точки доступа...");
  WiFi.softAP(ssid, password);
  
  Serial.println("SSID: " + ssid);
  Serial.print("IPv4: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", sendRootHtml);
  
  server.begin();
  Serial.println("Веб-сервер запущен.");
}

void setup() {
  Serial.begin(115200);
  setupSensorAndDisplay();
  setupWebServer();
  showNetworks();
}

void loop() {     
  updateSensorData();
  showDataOnDisplay();  
  server.handleClient();
  delay(3000);           
}