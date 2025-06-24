// Sistem Praktikum Impuls dan Momentum
// Menggunakan Load Cell + HX711, LCD I2C, LED indikator, dan buzzer

#include <HX711.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Inisialisasi HX711
#define DT 3
#define SCK 2
HX711 scale;
float calibration_factor = 411.15; // Disesuaikan dengan hasil kalibrasi

// LCD I2C 
LiquidCrystal_I2C lcd(0x27, 16, 2);

// LED dan buzzer
#define LED_HIJAU 7
#define LED_KUNING 6
#define LED_MERAH 5
#define LED_PUTIH 4
//#define BUZZER 4

// Variabel penyimpanan
float massa = 0;
float berat = 0;
float F1 = 0, F2 = 0;
unsigned long t1 = 0, t2 = 0;
float delta_t1 = 0, delta_t2 = 0;
float h1 = 0, h2 = 0;
float e = 0;
bool tumbukan1_terdeteksi = false, tumbukan2_terdeteksi = false;

void setup() {
  Serial.begin(9600);
  scale.begin(DT, SCK);
  scale.set_scale(calibration_factor);
  scale.tare(); // set nol awal

  lcd.begin(16,2);
  lcd.backlight();

  pinMode(LED_HIJAU, OUTPUT);
  pinMode(LED_KUNING, OUTPUT);
  pinMode(LED_MERAH, OUTPUT);
  //pinMode(BUZZER, OUTPUT);

  digitalWrite(LED_MERAH, LOW);
  digitalWrite(LED_KUNING, LOW);
  digitalWrite(LED_HIJAU, LOW);
  //digitalWrite(BUZZER, LOW);
}

void loop() {
  digitalWrite(LED_PUTIH, HIGH);
  delay(500);
  digitalWrite(LED_PUTIH, LOW);
  delay(500);
  
  // 1. TAHAP PENIMBANGAN BENDA
  while (true) {
    massa = getStableMass();
    if (massa >= 1.0) break;
    kedipMerah();
    lcd.setCursor(0, 0);
    lcd.print("Timbang beban!   ");
    lcd.setCursor(0, 1);
    lcd.print("                ");
  }

  delay(200);
  flashLed(LED_HIJAU);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Massa (gram):");
  lcd.setCursor(0, 1);
  lcd.print(massa, 2);
  delay(7000);

  // 2. TAHAP MENUNGGU TUMBUKAN
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Jatuhkan benda!");
  digitalWrite(LED_KUNING, LOW);

  while (!tumbukan1_terdeteksi || !tumbukan2_terdeteksi) {
    float gaya = bacaGayaInstan();
    if (gaya > berat + 100) {
      if (!tumbukan1_terdeteksi) {
        F1 = gaya;
        t1 = millis();
        tumbukan1_terdeteksi = true;
        Serial.println("Tumbukan 1 terdeteksi.");
        flashLed(LED_KUNING);
      } else if (!tumbukan2_terdeteksi && millis() - t1 > 100) {
        F2 = gaya;
        t2 = millis();
        tumbukan2_terdeteksi = true;
        Serial.println("Tumbukan 2 terdeteksi.");
        flashLed(LED_KUNING);
      }
    }
  }

  // 3. PROSES PENGHITUNGAN
  digitalWrite(LED_KUNING, HIGH);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sedang mengukur");
  lcd.setCursor(0, 1);
  lcd.print("Mohon tunggu...");

  delta_t1 = (t2 - t1) / 1000.0;
  delta_t2 = delta_t1 / 2.0;

  float impuls1 = F1 * delta_t1;
  float impuls2 = F2 * delta_t2;
  float v1 = impuls1 / (massa / 1000.0);
  float v2 = impuls2 / (massa / 1000.0);

  h1 = (v1 * v1) / (2 * 9.8);
  h2 = (v2 * v2) / (2 * 9.8);
  e = sqrt(h2 / h1);

  digitalWrite(LED_KUNING, LOW);

  // 4. TAMPILKAN HASIL
  tampilkanHasil("F1: ", F1, "dt1: ", delta_t1);
  tampilkanHasil("F2: ", F2, "dt2: ", delta_t2);
  tampilkanHasil("h1: ", h1, "h2: ", h2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("e: "); lcd.print(e, 2);
  delay(7000);
  flashLed(LED_HIJAU);

  // RESET
  tumbukan1_terdeteksi = false;
  tumbukan2_terdeteksi = false;
  scale.tare();
  delay(1000);
}

float getStableMass() {
  float sum = 0;
  int count = 50;
  for (int i = 0; i < count; i++) {
    sum += scale.get_units();
    delay(5);
  }
  berat = sum / count * 980; // simpan gaya berat untuk acuan
  return sum / count * 1000; // gram
}

float bacaGayaInstan() {
  if (scale.is_ready()) {
    return scale.get_units(1) * 980; // dyne
  }
  return 0;
}

void flashLed(int pin) {
  digitalWrite(pin, HIGH);
  delay(50);
  digitalWrite(pin, LOW);
}

void kedipMerah() {
  digitalWrite(LED_MERAH, HIGH);
  delay(200);
  digitalWrite(LED_MERAH, LOW);
  delay(200);
}

void tampilkanHasil(String label1, float val1, String label2, float val2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(label1); lcd.print(val1, 2);
  lcd.setCursor(0, 1);
  lcd.print(label2); lcd.print(val2, 3);
  flashLed(LED_HIJAU);
  delay(7000);
}
