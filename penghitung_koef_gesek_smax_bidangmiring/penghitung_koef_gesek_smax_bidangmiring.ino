#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "HX711.h"

// ==== DEFINISI PERANGKAT KERAS ====
// Servo
Servo servo1;
Servo servo2;

// Load cell dan HX711
#define DT 3
#define SCK 2
HX711 scale;

// LED indikator (sesuai wiring yang telah terbukti)
#define ledWhite 4
const int ledRed = 5;
const int ledYellow = 6;
const int ledGreen = 7;

// LCD I2C (alamat 0x27, 16 kolom x 2 baris)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ==== VARIABEL SISTEM ====
int angle = 0;        // sudut bidang (dalam derajat)
int mode = 0;         // mode: 0 = STOP, 1 = MENANJAK, 2 = MELANDAI
bool loadDetected = false;  // flag jika load cell mendeteksi beban
float miu = 0.0;    // koefisien gesek
const float g = 9.8;  // percepatan gravitasi (m/s^2)
float threshold = 1.0;  // ambang deteksi load cell (gram)

// ==== SETUP ====
void setup() {
  Serial.begin(9600);
  
  // Pasang servo
  servo1.attach(9);
  servo2.attach(10);
  
  // Inisialisasi LED
  pinMode (ledWhite, OUTPUT);
  pinMode(ledRed, OUTPUT);
  pinMode(ledYellow, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  
  // Inisialisasi LCD
  lcd.begin(16, 2);
  lcd.backlight();

  // Inisialisasi load cell
  scale.begin(DT, SCK);
  scale.set_scale(411.15); // nilai hasil kalibrasi
  scale.tare();

  // Posisi awal servo
  angle = 0;
  servo1.write(angle);
  servo2.write(angle);
  
  // Countdown awal 20 detik
  lcd.setCursor(0, 0);
  lcd.print("Letakkan bidang!");
  for (int i = 20; i >= 0; i--) {
    lcd.setCursor(0, 1);
    lcd.print("Mulai dalam: ");
    lcd.print(i);
    lcd.print("s ");
    
    // LED merah berkedip cepat
    digitalWrite(ledRed, HIGH);
    delay(150);
    digitalWrite(ledRed, LOW);
    delay(150);
  }

  // Hapus pesan dan tampilkan posisi awal
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("0 deg, STOP");
  delay(1000);

  mode = 1; // mulai naik
}


void flashLed(int pin) {
  digitalWrite(pin, HIGH);
  delay(300);
  digitalWrite(pin, LOW);
}

// ==== LOOP UTAMA ====
void loop() {

  if (!loadDetected) {
    if (mode == 1) { // MENANJAK
      for (int a = angle; a <= 80 && !loadDetected; a++) {
        angle = a;
        servo1.write(angle);
        servo2.write(angle);
        updateLED();
        updateDisplay("MENANJAK");
        clearBarisKedua();
        delay(500);
        
        float reading = scale.get_units();
        if (reading > threshold) {
          loadDetected = true;
          lcd.setCursor(0, 1);
          lcd.print("Menghitung...   "); // Tambahan tampilan
          digitalWrite(ledRed, HIGH);
          digitalWrite(ledGreen, LOW);
          digitalWrite(ledYellow, LOW);
          processLoad();
          break;
        }
      }
      if (!loadDetected) mode = 0;
    }

    else if (mode == 2) { // MELANDAI
      for (int a = angle; a >= 0 && !loadDetected; a--) {
        angle = a;
        servo1.write(angle);
        servo2.write(angle);
        updateLED();
        updateDisplay("MELANDAI");
        clearBarisKedua();
        delay(500);

        float reading = scale.get_units();
        if (reading > threshold) {
          loadDetected = true;
          lcd.setCursor(0, 1);
          lcd.print("Menghitung...   ");
          processLoad();
          break;
        }
      }
      if (!loadDetected) mode = 0;
    }

    if (angle >= 80 && !loadDetected) {
      updateDisplay("MAX!");
      digitalWrite(ledRed, HIGH);
      delay(1000);
      mode = 2;
    } else if (angle <= 0 && mode == 0 && !loadDetected) {
      updateDisplay("MAX!");
      digitalWrite(ledRed, HIGH);
      delay(1000);
      mode = 1;
    }
  }
}

// ==== PROSES LOAD ====
void processLoad() {
  unsigned long startTime = millis();
  float latestReading = 0;
  while (millis() - startTime < 10000) {
    latestReading = scale.get_units();
    delay(200);
  }

  float massa = latestReading;
  float massaKg = massa / 1000.0;
  float currentTheta = angle;
  float gaya = massaKg * g * sin(radians(currentTheta));
  float gayaNormal = massaKg * g * cos(radians(currentTheta));
  if (gayaNormal == 0) miu = 0;
  else miu = gaya / gayaNormal;

  Serial.print("Sudut: ");
  Serial.print(currentTheta);
  Serial.println(" deg");

  Serial.print("Massa: ");
  Serial.print(massa, 1);
  Serial.println(" gram");

  Serial.print("μ: ");
  Serial.println(miu, 3);

  lcd.setCursor(0, 1);
  lcd.print("μ = ");
  lcd.print(miu, 3);
  lcd.print("          ");
  delay(10000);

  for (int i = 0; i < 10; i++) {
    lcd.setCursor(0, 1);
    lcd.print("Ambil beban     ");
    digitalWrite(ledRed, HIGH);
    delay(250);
    digitalWrite(ledRed, LOW);
    delay(250);
  }

  // === RESET SISTEM ===
  lcd.setCursor(0, 1);
  lcd.print("Reset posisi... ");
  angle = 0;
  servo1.write(angle);
  servo2.write(angle);
  digitalWrite(ledYellow, HIGH);
  digitalWrite(ledGreen, LOW);
  delay(10000);//silakan letak beban lagi di bidang miring
  digitalWrite(ledYellow, LOW);

  clearBarisKedua();
  loadDetected = false;
  mode = 1;
}

// ==== LCD & LED SUPPORT ====
void updateDisplay(String status) {
  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 0);
  lcd.print(String(angle) + " deg, " + status);
  
  Serial.print("Sudut: ");
  Serial.print(angle);
  Serial.print(" deg, Status: ");
  Serial.println(status);
}

void clearBarisKedua() {
  lcd.setCursor(0, 1);
  lcd.print("                ");
}

void updateLED() {
  // Reset semua LED
  digitalWrite(ledRed, LOW);
  digitalWrite(ledYellow, LOW);
  digitalWrite(ledGreen, LOW);

  // mode 0 (STOP) → kuning
  // mode 1 (MENANJAK) → hijau
  // mode 2 (MELANDAI) → kuning (DIPERBAIKI)
  if (mode == 0) {
    digitalWrite(ledYellow, HIGH);
    flashLed(ledWhite);}
  else if (mode == 1) {
    digitalWrite(ledGreen, HIGH);
    flashLed(ledWhite);}
  else if (mode == 2) {
    digitalWrite(ledYellow, HIGH);  // perbaikan!
    flashLed(ledWhite);}
}
