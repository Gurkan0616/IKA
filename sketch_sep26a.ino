#include <SoftwareSerial.h>

// Bluetooth Bağlantısı
SoftwareSerial bluetooth(2, 3); // RX=2, TX=3

// Motor Sürücü Pinleri (Nano)
const int sag_ileri = 4;
const int sag_geri = 7;
const int sol_ileri = 8;
const int sol_geri = 12;
const int sag_enable = 5; // PWM
const int sol_enable = 6; // PWM

// HC-SR04 Sensör Pinleri
const int on_trig = 9;
const int on_echo = 10;
const int arka_trig = 11;
const int arka_echo = A0;

// Güvenli mesafe sınırı (cm cinsinden)
const int guvenli_mesafe = 20;

// Aracın anlık yön durumunu tutan değişken
char son_komut = 'S'; 

void setup() {
  // Motor pinleri
  pinMode(sag_ileri, OUTPUT);
  pinMode(sag_geri, OUTPUT);
  pinMode(sol_ileri, OUTPUT);
  pinMode(sol_geri, OUTPUT);
  pinMode(sag_enable, OUTPUT);
  pinMode(sol_enable, OUTPUT);
  
  // Sensör pinleri
  pinMode(on_trig, OUTPUT);
  pinMode(on_echo, INPUT);
  pinMode(arka_trig, OUTPUT);
  pinMode(arka_echo, INPUT);

  // İletişim
  Serial.begin(9600);
  bluetooth.begin(9600);
  
  // Motorları başlangıçta durdur
  motorDur();
}

void loop() {
  // 1. Bluetooth'tan gelen yeni komut var mı kontrol et
  if (bluetooth.available() > 0) {
    char gelen_veri = bluetooth.read();
    // Gelen karakter geçerli bir yön komutuysa kaydet
    if(gelen_veri == 'F' || gelen_veri == 'B' || gelen_veri == 'L' || gelen_veri == 'R' || gelen_veri == 'S') {
      son_komut = gelen_veri;
      Serial.print("Yeni Komut: ");
      Serial.println(son_komut);
    }
  }

  // 2. Sensörlerden mesafeleri ölç
  int on_mesafe = mesafeOlc(on_trig, on_echo);
  int arka_mesafe = mesafeOlc(arka_trig, arka_echo);

  // 3. Karar Mekanizması (Beyin)
  
  if (son_komut == 'F') {
    if (on_mesafe > 0 && on_mesafe <= guvenli_mesafe) {
      // Çarpışma riski var! Dur.
      motorDur();
      Serial.println("ENGEL! On sensor araci durdurdu.");
    } else {
      // Yol açık, ileri git
      ileriGit();
    }
  }
  else if (son_komut == 'B') {
    if (arka_mesafe > 0 && arka_mesafe <= guvenli_mesafe) {
      // Çarpışma riski var! Dur.
      motorDur();
      Serial.println("ENGEL! Arka sensor araci durdurdu.");
    } else {
      // Yol açık, geri git
      geriGit();
    }
  }
  else if (son_komut == 'L') {
    solaDon();
  }
  else if (son_komut == 'R') {
    sagaDon();
  }
  else if (son_komut == 'S') {
    motorDur();
  }

  // Sensörlerin stabil çalışması için kısa bir gecikme
  delay(50); 
}

// --- MESAFE ÖLÇÜM FONKSİYONU ---
int mesafeOlc(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // 30000 mikrosaniye (yaklaşık 5 metre) zaman aşımı sınırı
  long sure = pulseIn(echoPin, HIGH, 30000); 
  
  if (sure == 0) return 999; // Engel yoksa veya çok uzaktaysa büyük bir değer döndür
  
  int mesafe = sure / 58.2;
  return mesafe;
}

// --- MOTOR HAREKET FONKSİYONLARI ---
void ileriGit() {
  digitalWrite(sag_ileri, 1); digitalWrite(sag_geri, 0);
  digitalWrite(sol_ileri, 1); digitalWrite(sol_geri, 0);
  analogWrite(sag_enable, 255); analogWrite(sol_enable, 255);
}

void geriGit() {
  digitalWrite(sag_ileri, 0); digitalWrite(sag_geri, 1);
  digitalWrite(sol_ileri, 0); digitalWrite(sol_geri, 1);
  analogWrite(sag_enable, 255); analogWrite(sol_enable, 255);
}

void sagaDon() {
  digitalWrite(sag_ileri, 0); digitalWrite(sag_geri, 1);
  digitalWrite(sol_ileri, 1); digitalWrite(sol_geri, 0);
  analogWrite(sag_enable, 200); analogWrite(sol_enable, 200);
}

void solaDon() {
  digitalWrite(sag_ileri, 1); digitalWrite(sag_geri, 0);
  digitalWrite(sol_ileri, 0); digitalWrite(sol_geri, 1);
  analogWrite(sag_enable, 200); analogWrite(sol_enable, 200);
}

void motorDur() {
  digitalWrite(sag_ileri, 0); digitalWrite(sag_geri, 0);
  digitalWrite(sol_ileri, 0); digitalWrite(sol_geri, 0);
  analogWrite(sag_enable, 0); analogWrite(sol_enable, 0);
}