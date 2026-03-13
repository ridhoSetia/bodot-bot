# 🤖 Bodot AI: Open-Source Voice Assistant Robot

Bodot AI adalah asisten suara pintar berbasis AI yang dirancang untuk diintegrasikan ke dalam hardware (ESP32). Sistem ini menggunakan **FastAPI** sebagai otak pusat, **Groq (Llama 3.3 & Whisper)** untuk kecerdasan buatan, dan **Edge-TTS** untuk memberikan suara alami.

## 🚀 Fitur Utama

* **Speech-to-Text (STT):** Konversi suara ke teks menggunakan Whisper Large V3 Turbo.
* **Otak AI (LLM):** Menggunakan Llama 3.3 70B untuk respon yang cerdas, santai, dan cepat.
* **Text-to-Speech (TTS):** Output suara manusia yang jernih menggunakan teknologi Edge-TTS.
* **Streaming I2S:** Mendukung transmisi audio dua arah langsung dari ESP32.
* **Smart Home Control:** Mampu mengontrol Relay (Lampu/Kipas) melalui perintah suara.

---

## 🔌 Panduan Wiring

Berikut adalah skema wiring untuk menghubungkan ESP32 dengan modul audio I2S:

### 1. I2S Microphone (INMP441)

| INMP441 Pin | ESP32 Pin | Keterangan |
| --- | --- | --- |
| **VDD** | 3.3V | Daya |
| **GND** | GND | Ground |
| **L/R** | GND | Mono (Left Channel) |
| **WS** | GPIO 25 | Word Select |
| **SCK** | GPIO 33 | Serial Clock |
| **SD** | GPIO 32 | Serial Data |

### 2. I2S Speaker Amplifier (MAX98357A)

| MAX98357A Pin | ESP32 Pin | Keterangan |
| --- | --- | --- |
| **Vin** | 5V | Daya (Gunakan 5V untuk suara keras) |
| **GND** | GND | Ground |
| **GAIN**| GND
| **LRC** | GPIO 27 | Left/Right Clock |
| **BCLK** | GPIO 14 | Bit Clock |
| **DIN** | GPIO 26 | Data In |

### 3. Komponen Tambahan

* **Push Button:** GPIO 4 (Mode: Active Low / Pull-up).
* **Relay (Lampu):** GPIO 2.
* **OLED Display (I2C):** SDA (GPIO 21), SCL (GPIO 22).
* **LED Status:** LED Koneksi (GPIO 18), LED Berpikir (GPIO 5).

---

## 💻 Instalasi Perangkat Lunak

### 1. Persiapan Backend (Server)

Pastikan kamu sudah menginstal **Docker** dan **Docker Compose**.

1. Dapatkan API Key dari [Groq Cloud](https://console.groq.com/).
2. Buat file `.env` di direktori utama:
```env
GROQ_API_KEY=isi_dengan_api_key_kamu
```


3. Jalankan server menggunakan Docker:
```bash
docker-compose up -d
```



### 2. Konfigurasi Firmware (ESP32)

Buka file `bodot.ino` di Arduino IDE dan **sesuaikan bagian berikut** agar sesuai dengan environtment kamu:

```cpp
// ================= SESUAIKAN DI SINI =================
const char* ssid = "NAMA_WIFI_KAMU";
const char* password = "PASSWORD_WIFI_KAMU";

const char* serverHost = "IP_ADDRESS_SERVER_KAMU"; 
const int serverPort = 8010; 

// Sesuaikan Pin jika kamu menggunakan layout berbeda
#define BUTTON_PIN    4
// =====================================================

```

---

## 🛠️ Cara Menggunakan

1. **Nyalakan Server:** Pastikan container Docker sudah berjalan.
2. **Flash ESP32:** Upload kode `.ino` yang sudah dikonfigurasi.
3. **Bicara:** * Tekan dan tahan tombol.
* Layar OLED akan menampilkan **"MENDENGAR"**.
* Bicaralah (Contoh: *"Bodot, tolong nyalakan lampu"*).
* Lepas tombol untuk mengirim suara ke server.


4. **Respon:** Bodot akan memproses suara, dan suaranya akan terdengar melalui speaker MAX98357A.

---

## 📂 Struktur Proyek

* `main.py`: Backend FastAPI untuk memproses alur AI.
* `bodot.ino`: Kode firmware untuk ESP32.
* `Dockerfile` & `docker-compose.yml`: Konfigurasi deployment server.
* `requirements.txt`: Daftar library Python yang dibutuhkan.
* `recordings/`: Folder otomatis untuk menyimpan log suara (debugging).

---

## 🤝 Kontribusi

Proyek ini bersifat open-source. Jika kamu ingin mengembangkan fitur baru seperti integrasi sensor IoT lainnya atau memperbaiki logika AI, silakan lakukan *Pull Request*.

---

**Catatan:** Pastikan server dan ESP32 berada dalam jaringan yang sama agar bisa saling berkomunikasi!