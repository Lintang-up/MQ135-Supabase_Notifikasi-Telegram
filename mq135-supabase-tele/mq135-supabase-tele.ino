#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>

// Ganti dengan SSID dan password Wi-Fi Anda
const char *ssid = "P1";
const char *password = "69696969";

// Ganti dengan URL Supabase API dan API Key
const String supabaseURL = "https://fewpwfdauqjatjlpvxzo.supabase.co";
const String apiKey = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6ImZld3B3ZmRhdXFqYXRqbHB2eHpvIiwicm9sZSI6ImFub24iLCJpYXQiOjE3MjY2NDQyODEsImV4cCI6MjA0MjIyMDI4MX0.3X_aiNj8ytx7yOLpTPC_q-wc4XHzUT0HrSGfUs1z6A8";

// Bot Tele
const char *BOT_TOKEN = "7880210514:AAEjsFZ0LOWGNqEJoxn-NG4ojwLbpgedkns";
const char *CHAT_ID = "6926806791";

// Library Sensor asap
#include <TroykaMQ.h>
#define PIN_MQ135 A0
MQ135 mq135(PIN_MQ135);

// Interval pengiriman data ke Supabase dalam milidetik
int sendingInterval = 10000; // Pengiriman data setiap 10 detik
HTTPClient https;
WiFiClientSecure client;

void setup(){
  // Mengatur pin LED bawaan untuk menunjukkan status pengiriman data
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // LED menyala saat tidak mengirim data

  // Menghubungkan ke Wi-Fi
  Serial.begin(9600);

  connectToWiFi();
  // HTTPS tanpa validasi sertifikat
  client.setInsecure();

  // MQ135
  mq135.calibrate();
  Serial.print("Ro = ");
  Serial.println(mq135.getRo());
}

void loop(){
  // Cek status koneksi Wi-Fi
  if (WiFi.status() == WL_CONNECTED)  {
    digitalWrite(LED_BUILTIN, LOW); // LED menyala saat mengirim data

    // kirim notif ke tele
    int gs = senosrGas();
    if (gs > 600){
      sendTelegramMessage("Hello, gas terdeteksi tinggi!!!!");
    }

    // Mengirim data ke Supabase
    sendToSupabase(gs);

    // Matikan LED setelah pengiriman data
    digitalWrite(LED_BUILTIN, HIGH); // LED mati
  }
  else{
    Serial.println("Error in Wi-Fi connection");
  }

  // Tunggu beberapa detik sebelum membaca dan mengirim data lagi
  delay(sendingInterval); // 3 detik sebelum membaca sensor lagi
}

// telegram
void sendTelegramMessage(String message){
  if (WiFi.status() == WL_CONNECTED)  {                          // Pastikan sudah terhubung ke WiFi
    // Format URL Telegram API
    String url = "https://api.telegram.org/bot" + String(BOT_TOKEN) + "/sendMessage?chat_id=" + String(CHAT_ID) + "&text=" + message;
    https.begin(client, url); 

    // Lakukan GET request ke Telegram API
    int httpCode = https.GET();
    // Periksa kode respon HTTP
    if (httpCode > 0)    {
        //      Serial.println("HTTP Response Code: " + String(httpCode));
        String payload = https.getString();
        Serial.println("Respon Telegram       : " + payload);
    }
    else{
      Serial.println("Error Mengirim Tele  : " + String(httpCode));
    }
    https.end(); // Menutup koneksi
  }
  else{
    Serial.println("WiFi not connected");
  }
}

// Fungsi untuk menghubungkan ke Wi-Fi
void connectToWiFi(){
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Wi-Fi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// Fungsi untuk membaca nilai mq135
int senosrGas(){
  int gs = mq135.readCO2();
  Serial.println("");
  Serial.print("kadar gas             : ");
  Serial.println(gs);
  return gs;
}

// Fungsi untuk mengirim data ke Supabase
void sendToSupabase(int sensorGas){
  // Menyusun data JSON untuk dikirim
  String postData = "{\"nilai\":" + String(sensorGas) + "}";
  // Mengirim permintaan POST ke Supabase
  https.begin(client, supabaseURL + "/rest/v1/tabel_mq135");
  https.addHeader("Content-Type", "application/json");
  https.addHeader("Prefer", "return=representation");
  https.addHeader("apikey", apiKey);
  https.addHeader("Authorization", "Bearer " + apiKey);
  // Mengirim data JSON
  int httpCode = https.POST(postData);
  String payload = https.getString();
  // Menampilkan hasil HTTP
  Serial.print("Respon Supabase       : ");
  Serial.println(httpCode);
  Serial.print("Tersimpan di Supabase : ");
  Serial.println(payload);
  Serial.println("____________________________________________________________");
  https.end(); // Menutup koneksi
}
