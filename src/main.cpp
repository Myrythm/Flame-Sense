#include <DHT.h>
#include <HTTPClient.h>
#include <MQUnifiedsensor.h>

#define DHTPIN 2
#define DHTTYPE DHT22
#define FIRE 18
#define BUZZER_PIN 5 
#define MQPIN 32


DHT dht(DHTPIN, DHTTYPE);
HTTPClient http;

char ssid[] = "TECNO SPARK 10 Pro";
char pass[] = "teunyaho";
#define BLYNK_TEMPLATE_ID "TMPL6CzAAtefk"
#define BLYNK_TEMPLATE_NAME "Quickstart Template"
#define BLYNK_AUTH_TOKEN "-N7gE8mRNFZrGLXBMEvBgyllfLG53Nko"

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial
#include <esp_log.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

int ppm = 0;
int fireStatus = 0;

BlynkTimer timer;

void teleSendMessage(String payload) {
  http.begin("https://api.telegram.org/"
             "bot6756040579:AAHFEY8OmzrxkH9FJtBqw7ITHkX_j42IZWo/sendMessage");
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(payload);

  if (httpResponseCode > 0) {
    String response = http.getString(); // Get the response to the request

    ESP_LOGI("TELE", "HTTP Response code: %d", httpResponseCode); // Print return code
    ESP_LOGI("TELE", "Response: %s", response.c_str());         // Print request answer
  } else {
    ESP_LOGI("TELE", "Error on sending POST: %d", httpResponseCode);
  }

  http.end();
}

void sendTemperatureToTelegram(float temperature) {
  String message;

  if (temperature >= 65.0) {
    message = "Alert: SUHU DI DAPUR SANGAT PANAS MENCAPAI: " + String(temperature) + " C";
    tone(BUZZER_PIN, 1000); 

  } else if (temperature >= 50.0) {
    message = "Alert: SUHU DI DAPUR PANAS MENCAPAI: " + String(temperature) + " C";
    tone(BUZZER_PIN, 1000); 

  } else {
    noTone(BUZZER_PIN);
    return;
  }

  // Replace YOUR_CHAT_ID with the actual chat ID you obtained
  String telegramPayload = "{\"chat_id\": 6256086672, \"text\":\"" + message + "\"}";
  teleSendMessage(telegramPayload);
  delay(2000);
}

void sendFireAlertToTelegram() {
  String message;

  if (fireStatus) {

    message = "Alert: API TERDETEKSI DI DAPUR";
    // Mengaktifkan buzzer
    tone(BUZZER_PIN, 1000);  // Frequency 1000 Hz
  
  } else {
    noTone(BUZZER_PIN);
    return;
  }

  // Replace YOUR_CHAT_ID with the actual chat ID you obtained
  String telegramPayload = "{\"chat_id\": 6256086672, \"text\":\"" + message + "\"}";
  teleSendMessage(telegramPayload);
  delay(2000);
}


void sendGasAlertToTelegram(int ppm) {
  String message;

  if (ppm >= 1000) {

    message = "Alert: KEBOCORAN GAS TERDETEKSI DI DAPUR MENCAPAI: " + String(ppm) + " ppm";
    // Mengaktifkan buzzer
    tone(BUZZER_PIN, 1000);  // Frequency 1000 Hz
  
  } else {
    noTone(BUZZER_PIN);
    return;
  }

  // Replace YOUR_CHAT_ID with the actual chat ID you obtained
  String telegramPayload = "{\"chat_id\": 6256086672, \"text\":\"" + message + "\"}";
  teleSendMessage(telegramPayload);
  delay(2000);
}

void sendSensor() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  ppm = analogRead(MQPIN);
  fireStatus = digitalRead(FIRE);

  Serial.print("Humidity: ");
  Serial.println(h);
  Serial.print("Temperature: ");
  Serial.println(t);
  Serial.print("MQ Sensor Value: ");
  Serial.println(ppm);
  Serial.print("Fire Status: ");
  Serial.println(fireStatus);
  Serial.println("================================================");


  Blynk.virtualWrite(V0, t);
  Blynk.virtualWrite(V1, h);
  Blynk.virtualWrite(V2, ppm);
  Blynk.virtualWrite(V3, fireStatus);

  sendTemperatureToTelegram(t);
  sendFireAlertToTelegram();
  sendGasAlertToTelegram(ppm);
}



void setup() {
  Serial.begin(9600);
  pinMode(FIRE, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  dht.begin();
  pinMode(MQPIN, INPUT);
  timer.setInterval(1000L, sendSensor);
  
}

void loop() {
  Blynk.run();
  timer.run();
}
