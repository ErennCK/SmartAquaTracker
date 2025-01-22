#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <ESP8266WebServer.h>
#include <TimeLib.h> // Zaman işlemleri için
#include <NTPClient.h> // NTP sunucusundan zaman almak için
#include <WiFiUdp.h>
#include <time.h>

// WiFi ağ bilgileri
const char *ssid = "Ny2";               // Kablosuz ağınızın SSID'si
const char *password = "2002s2ll";   // Kablosuz ağınızın şifresi

int waterLevel = 100;                  // Başlangıç su seviyesi (%100)
int totalWaterUsed = 0;                // Toplam kullanılan su miktarı (Manuel olarak siz ekleyeceksiniz)

// Firebase bilgileri
#define FIREBASE_HOST "pisipisi-4e990-default-rtdb.firebaseio.com"  // Firebase Realtime Database URL
#define FIREBASE_AUTH "DHNOhqMlom6O9oUFhbl9gvgJHb5HvOT8SsvBImfO" // Firebase Secret Key

// Firebase sunucusunun SHA-1 fingerprint'i
const char *FIREBASE_FINGERPRINT = "6D:29:9B:3F:09:A5:1E:7C:70:08:E0:00:0F:74:D6:4E:39:12:2E:A7"; // Firebase güvenli bağlantı için kullanılan parmak izi

ESP8266WebServer server(80);  // Web sunucusu nesnesi oluşturuluyor

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600 * 3); // Türkiye için UTC+3

// Firebase'den su seviyesi ve kullanılan su miktarını al
void getFirebaseData() {
  // Geçerli tarihi al (sadece tarih kısmı, örneğin: "2024-12-23")
  String currentDate = getFormattedDateTime().substring(0, 10);  // "2024-12-23"
  
  // Firebase yolunu oluştur
  String path = "waterUsage/" + currentDate;  // "waterUsage/2024-12-23" gibi

  // Firebase'den verileri al
  waterLevel = Firebase.getInt(path + "/waterLevel");  // Güncel su seviyesi
  totalWaterUsed = Firebase.getInt(path + "/usedAmount");  // Günlük toplam su kullanımı

  // Hata kontrolü
  if (Firebase.failed()) {
    Serial.println("Firebase'den veri alınamadı!");
    Serial.println(Firebase.error());
  } else {
    Serial.println("Firebase'den veriler başarıyla alındı!");
    Serial.print("Su Seviyesi: ");
    Serial.println(waterLevel);
    Serial.print("Kullanılan Su Miktarı: ");
    Serial.println(totalWaterUsed);
  }
}

// Firebase güncelleme fonksiyonu
void updateFirebase() {
  // Geçerli tarihi al (sadece tarih kısmı, örneğin: "2024-12-23")
  String currentDate = getFormattedDateTime().substring(0, 10);  // "2024-12-23"

  // Firebase veritabanına kaydedilecek veriler
  String path = "waterUsage/" + currentDate;  // "waterUsage/2024-12-23" gibi

  // Günlük su tüketimi verilerini Firebase'e yaz
  Firebase.setInt(path + "/waterLevel", waterLevel);  // Güncel su seviyesi
  Firebase.setInt(path + "/usedAmount", totalWaterUsed);  // Günlük toplam kullanılan su
  Firebase.setString(path + "/lastUpdate", getFormattedDateTime());  // Son güncelleme tarihi

  // Toplam su kullanımını dışarıda tutuyoruz (örneğin, global bir değişken olarak)
  Firebase.setInt("totalWaterUsed", totalWaterUsed);  // Bu her zaman toplam kullanım olabilir

  // Hata kontrolü
  if (Firebase.failed()) {
    Serial.print("Error writing to Firebase: ");
    Serial.println(Firebase.error());
  } else {
    Serial.println("Veriler başarıyla Firebase'e kaydedildi.");
  }
}

// Su seviyesi arttırma fonksiyonu (kaptaki su artacak, toplam su kullanılmaz)
void increase() {
  // Global su seviyesini güncelle
  if (waterLevel < 100) {
    waterLevel += 10;  // Su seviyesini 10 arttır
    updateFirebase();  // Firebase'e veri gönder
  }

  // Web sayfasını tekrar göster
  anasayfa();  // Ana sayfayı tekrar göster
}

// Su seviyesi azaltma fonksiyonu (su içildiği için toplam su kullanımı artacak)
void decrease() {
  // Global su seviyesini güncelle
  if (waterLevel > 0) {
    waterLevel -= 10;  // Su seviyesini 10 azalt

    // Toplam kullanılan suyu artır (kedinin içtiği suyu kaydediyoruz)
    totalWaterUsed += 10;  // Kullanılan toplam suyu 10 arttır

    updateFirebase();  // Firebase'e veri gönder
  }

  // Web sayfasını tekrar göster
  anasayfa();  // Ana sayfayı tekrar göster
}








void criticalLevelFunction() {
  // Burada kritik seviyeye gelindiğinde yapılacak işlemler olacak.
  
}











String getFormattedDateTime() {
  time_t rawTime = timeClient.getEpochTime(); // Epoch zamanını al
  struct tm *timeInfo = localtime(&rawTime); // Epoch zamanını yerel zamana dönüştür

  char buffer[20];
  // Format: YYYY-MM-DDTHH:MM:SS
  sprintf(buffer, "%04d-%02d-%02dT%02d:%02d:%02d", 
          timeInfo->tm_year + 1900, 
          timeInfo->tm_mon + 1, 
          timeInfo->tm_mday, 
          timeInfo->tm_hour, 
          timeInfo->tm_min, 
          timeInfo->tm_sec);
  
  return String(buffer); // Formatlanmış tarih ve saat
}


































// Ana sayfa fonksiyonu
void anasayfa() {
  getFirebaseData();

  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>Akıllı Kedi Su Takibi</title>";
  html += "<script src='https://cdnjs.cloudflare.com/ajax/libs/Chart.js/3.7.0/chart.min.js'></script>";
  html += "<style>";
  
  // Modern ve temiz tasarım için CSS
  html += "* { margin: 0; padding: 0; box-sizing: border-box; }";
  html += "body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background: linear-gradient(135deg, #f5f7fa 0%, #c3cfe2 100%); min-height: 100vh; padding: 20px; }";
  html += ".container { max-width: 1000px; margin: 0 auto; background: white; padding: 30px; border-radius: 20px; box-shadow: 0 10px 20px rgba(0,0,0,0.1); }";
  
  // Başlık stili
  html += ".header { text-align: center; margin-bottom: 40px; }";
  html += ".header h1 { font-size: 2.5em; color: #2c3e50; margin-bottom: 10px; }";
  html += ".header p { color: #7f8c8d; }";
  
  // Dashboard grid layout
  html += ".dashboard { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 30px; margin: 40px 0; }";
  html += ".card { background: #fff; padding: 25px; border-radius: 15px; box-shadow: 0 5px 15px rgba(0,0,0,0.05); }";
  
  // Su seviyesi göstergesi
  html += ".water-container { position: relative; width: 200px; height: 200px; margin: 0 auto; }";
  html += ".water-level { width: 100%; height: 100%; border-radius: 50%; border: 8px solid #eee; position: relative; overflow: hidden; }";
  html += ".water { position: absolute; bottom: 0; left: 0; right: 0; background: linear-gradient(to bottom, #4facfe 0%, #00f2fe 100%); transition: height 0.5s ease; }";
  html += ".water-text { position: absolute; width: 100%; height: 100%; display: flex; align-items: center; justify-content: center; font-size: 2em; font-weight: bold; color: #2c3e50; }";
  
  // İstatistik kartları
  html += ".stat-value { font-size: 2em; font-weight: bold; color: #2c3e50; margin: 10px 0; text-align: center; }";
  html += ".stat-label { color: #7f8c8d; font-size: 0.9em; text-align: center; }";
  
  // Butonlar
  html += ".buttons { display: flex; justify-content: center; gap: 20px; margin: 30px 0; }";
  html += ".btn { padding: 12px 30px; border: none; border-radius: 50px; cursor: pointer; font-size: 1em; font-weight: 600; transition: all 0.3s ease; }";
  html += ".btn:hover { transform: translateY(-2px); box-shadow: 0 5px 15px rgba(0,0,0,0.1); }";
  html += ".btn-primary { background: linear-gradient(to right, #4facfe 0%, #00f2fe 100%); color: white; }";
  html += ".btn-danger { background: linear-gradient(to right, #ff416c 0%, #ff4b2b 100%); color: white; }";
  
  // Kritik seviye uyarısı
  html += ".alert { padding: 20px; border-radius: 15px; margin: 20px 0; text-align: center; }";
  html += ".alert-danger { background: linear-gradient(to right, #ff416c 0%, #ff4b2b 100%); color: white; animation: pulse 2s infinite; }";
  html += "@keyframes pulse { 0% { transform: scale(1); } 50% { transform: scale(1.02); } 100% { transform: scale(1); } }";
  
  html += "</style></head><body>";
  
  // Ana içerik
  html += "<div class='container'>";
  
  // Başlık
  html += "<div class='header'>";
  html += "<h1>🐱 Akıllı Kedi Su Takibi</h1>";
  html += "<p>Kedilerinizin su tüketimini akıllıca takip edin</p>";
  html += "</div>";
  
  // Kritik seviye uyarısı
  if (waterLevel <= 20) {
    html += "<div class='alert alert-danger'>";
    html += "<h3>⚠ Kritik Su Seviyesi!</h3>";
    html += "<p>Lütfen en kısa sürede su kabını doldurun.</p>";
    html += "</div>";
  }
  
  // Dashboard grid
  html += "<div class='dashboard'>";
  
  // Su seviyesi göstergesi
  html += "<div class='card'>";
  html += "<div class='water-container'>";
  html += "<div class='water-level'>";
  html += "<div class='water' style='height: " + String(waterLevel) + "%;'></div>";
  html += "<div class='water-text'>" + String(waterLevel) + "%</div>";
  html += "</div></div></div>";
  
  // İstatistikler
  html += "<div class='card'>";
  html += "<div class='stat-value'>" + String(totalWaterUsed) + " ml</div>";
  html += "<div class='stat-label'>Toplam Kullanılan Su</div>";
  html += "<div class='stat-value'>" + String(totalWaterUsed / 24) + " ml/saat</div>";
  html += "<div class='stat-label'>Ortalama Tüketim</div>";
  html += "</div>";
  
  // Grafik
  html += "<div class='card'>";
  html += "<canvas id='consumptionChart'></canvas>";
  html += "</div>";
  
  html += "</div>";
  
  // Kontrol butonları
  html += "<div class='buttons'>";
  html += "<a href='/increase'><button class='btn btn-primary'>Su Ekle</button></a>";
  html += "<a href='/decrease'><button class='btn btn-danger'>Su Azalt</button></a>";
  html += "</div>";
  
  // Örnek grafik için JavaScript
  html += "<script>";
  html += "const ctx = document.getElementById('consumptionChart').getContext('2d');";
  html += "new Chart(ctx, {";
  html += "  type: 'line',";
  html += "  data: {";
  html += "    labels: ['00:00', '03:00', '06:00', '09:00', '12:00', '15:00', '18:00', '21:00'],";
  html += "    datasets: [{";
  html += "      label: 'Su Tüketimi (ml)',";
  html += "      data: [65, 59, 80, 81, 56, 55, 40, 70],";
  html += "      fill: true,";
  html += "      borderColor: '#4facfe',";
  html += "      tension: 0.4,";
  html += "      backgroundColor: 'rgba(79, 172, 254, 0.1)'";
  html += "    }]";
  html += "  },";
  html += "  options: {";
  html += "    responsive: true,";
  html += "    plugins: {";
  html += "      legend: { position: 'top' }";
  html += "    },";
  html += "    scales: {";
  html += "      y: { beginAtZero: true }";
  html += "    }";
  html += "  }";
  html += "});";
  html += "</script>";
  
  html += "</div></body></html>";
  
  server.send(200, "text/html", html);
}

// Su seviyesi artırma fonksiyonu


































// Bilinmeyen sayfalar için 404 hatası
void bilinmeyen() {
  server.send(404, "text/html", "<h1>Sayfa Bulunamadı</h1>");
}

// Setup fonksiyonu
void setup() {

    timeClient.begin();
  
  // Zamanı güncelle
   timeClient.update();
  Serial.begin(9600);   // Seri portu başlatıyoruz
  Serial.println("Wi-Fi bağlantısı kuruluyor...");

  // Wi-Fi ağına bağlanıyoruz
  WiFi.begin(ssid, password);

  // Wi-Fi bağlantısı sağlanana kadar bekliyoruz
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Bağlantı bekleniyor...");
  }

  // Wi-Fi bağlantısı sağlandıktan sonra IP adresini yazdırıyoruz
  Serial.println("Bağlantı başarılı!");
  Serial.print("IP Adresi: ");
  Serial.println(WiFi.localIP());

  // Web sunucusu için yolları tanımlıyoruz
  server.on("/", anasayfa);            // Ana sayfa
  server.on("/increase", increase);    // Su seviyesini arttırma
  server.on("/decrease", decrease);    // Su seviyesini azaltma
  server.onNotFound(bilinmeyen);       // Bilinmeyen sayfalar için 404 sayfası

  server.begin(); // Web sunucusunu başlatıyoruz
  
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);  // Firebase'e bağlanmak için gerekli parametrelerle başlatılır
  Serial.println("Firebase başlatıldı.");  // Firebase'in başarıyla başlatıldığını belirtir





}

// Loop fonksiyonu
void loop() {
  timeClient.update();  // NTP saatini güncelle
  server.handleClient();  // Web sunucusu üzerinden gelen istekleri işliyoruz
}
