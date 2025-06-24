//KODE KALIBRASI LOAD CELL DAN HX711 DALAM MENGUKUR MASSA BEBAN
//CREATED BY SEKAR K.N. & RETNO S. (S2 P. FISIKA ITB'24)

#include "HX711.h"

HX711 scale;

// Ganti pin sesuai dengan yang kamu pakai
#define DT 3//pin DT modul ke D3 arduino
#define SCK 2//pin SCK modul ke D2 arduino

void setup() {
  Serial.begin(9600);
  scale.begin(DT, SCK);

  scale.set_scale(411.15);      /*nilai 411.15 dihapus dulu saat kalibrasi, lalu unggah kode
  siapkan benda dengan massa yang sudah diketahui. lalu ukur dengan sistem ini.
  lihat nilai analog yang terbaca pada serial monitor. 
  lalu bagikan nilai analog yang terbaca dengan massa asli benda.
  nilai tersebutlah yang selanjutnya menjadi nilai kalibrasi.
  Misalnya, pada saat pembuatan kode kalibrasi ini digunakan beban bermassa 4 gram dengan nilai analog 1644.6
  Maka nilai kalibrasinya adalah membagi 1644.6 dengan 4 gram, atau 411.15/gram
  Lalu isikan nilai kalibrasinya ke dalam baris "scale.set_scale(nilai kalibrasi);", unggah kembali kode.
  */ 
  scale.tare();           // set titik nol (tanpa beban)

  Serial.println("Tempatkan beban yang diketahui massanya (misal 75.9 gram)...");
}

void loop() {
  const int jumlahPembacaan = 50;/*jika terjadi fluktuasi pembacaan sensor, 
  maka nilai yang dimunculkan pada serial monitor adalah rata-rata dari 50 kali pembacaan
  */
  float total = 0;

  for (int i = 0; i < jumlahPembacaan; i++) {
    total += scale.get_units();
    delay(50); // jeda antar pembacaan
  }

  float massa_rata2 = total / jumlahPembacaan;

  Serial.print("massa rata-rata: ");
  Serial.print(massa_rata2, 1);  // 1 angka di belakang koma
  Serial.println(" gram");

  delay(100);//jeda setiap hasil rata-rata pembacaan untuk ditampilkan ke serial monitor
  //serial monitor akan memunculkan nilai setiap = jeda antar pembacaan x jeda hasil rata-rata = 5000 ms
}
