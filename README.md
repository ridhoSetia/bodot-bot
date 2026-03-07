# 🎤 ESP32 Voice Assistant

Voice assistant berbahasa Indonesia menggunakan ESP32, dengan STT (Groq Whisper), AI response (Groq LLaMA), dan TTS (Edge-TTS).

## 📋 Fitur

- ✅ **Voice Recording** - Rekam audio 3 detik via INMP441 microphone
- ✅ **Speech-to-Text** - Groq Whisper large-v3-turbo
- ✅ **AI Response** - LLaMA 3.3 70B untuk jawaban cerdas
- ✅ **Text-to-Speech** - Edge-TTS Indonesian voice
- ✅ **Audio Playback** - Speaker MAX98357A I2S
- ✅ **Smart Commands** - Kontrol lampu, waktu, dll
- ✅ **Error Handling** - LED feedback untuk status
- ✅ **WiFi Auto-reconnect** - Stabil dan reliable

## 🛠️ Hardware Requirements

### ESP32 Board
- ESP32 DevKit (atau variant lain)
- Minimal 4MB Flash

### Peripherals
- **Microphone**: INMP441 I2S MEMS
- **Speaker**: MAX98357A I2S Amplifier + Speaker 3-8Ω
- **Button**: Push button (NO - Normally Open)
- **LED**: Built-in LED atau eksternal

### Wiring Diagram

#### INMP441 Microphone
```
INMP441          ESP32
────────────────────────
VDD         →    3.3V
GND         →    GND
SD          →    GPIO 32
WS          →    GPIO 25
SCK         →    GPIO 33
L/R         →    GND
```

#### MAX98357A Speaker
```
MAX98357A        ESP32
────────────────────────
VIN         →    5V
GND         →    GND
BCLK        →    GPIO 14
LRC         →    GPIO 27
DIN         →    GPIO 26
SD          →    (tidak dipakai)
GAIN        →    (tidak dipakai, atau GND untuk 9dB)
```

#### Button & LED
```
Button           ESP32
────────────────────────
Pin 1       →    GPIO 5
Pin 2       →    GND

LED (opsional)   ESP32
────────────────────────
Anode (+)   →    GPIO 2
Cathode (-) →    GND (dengan resistor 220Ω)
```

## 💻 Software Requirements

### Server (Python)
- Python 3.8+
- FFmpeg (untuk audio processing)
- Dependencies: lihat `requirements.txt`

### ESP32 (Arduino)
- Arduino IDE 2.0+ atau PlatformIO
- ESP32 Board Support
- Library yang dibutuhkan (biasanya sudah include di ESP32 core):
  - WiFi
  - HTTPClient
  - driver/i2s

## 🚀 Setup Instructions

### 1. Setup Server

#### Install FFmpeg
**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install ffmpeg
```

**macOS:**
```bash
brew install ffmpeg
```

**Windows:**
Download dari [ffmpeg.org](https://ffmpeg.org/download.html)

#### Install Python Dependencies
```bash
cd server
pip install -r requirements.txt
```

#### Configure Environment
```bash
# Copy template
cp .env.example .env

# Edit .env dan isi:
nano .env
```

Isi file `.env`:
```env
GROQ_API_KEY=gsk_your_actual_groq_api_key_here
SERVER_HOST=192.168.2.64  # IP komputer Anda di jaringan lokal
SERVER_PORT=8000
```

**Cara mendapatkan Groq API Key:**
1. Buka [console.groq.com](https://console.groq.com/)
2. Sign up / Login
3. Buat API key baru
4. Copy dan paste ke `.env`

#### Run Server
```bash
python server.py
```

Jika berhasil, akan muncul:
```
🚀 ESP32 Voice Assistant Server Starting...
📡 Server: http://192.168.2.64:8000
✅ Server ready to accept requests
```

### 2. Setup ESP32

#### Install Arduino IDE & ESP32 Board
1. Download [Arduino IDE 2.0+](https://www.arduino.cc/en/software)
2. Install ESP32 board:
   - File → Preferences
   - Additional Board Manager URLs: 
     `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - Tools → Board → Boards Manager
   - Cari "esp32" dan install

#### Configure WiFi Credentials
```bash
# Copy template
cp secrets.h.example secrets.h

# Edit secrets.h
nano secrets.h
```

Isi file `secrets.h`:
```cpp
#define WIFI_SSID "NamaWiFiAnda"
#define WIFI_PASS "PasswordWiFiAnda"
```

#### Update Server URL
Edit file `esp32_voice_assistant.ino` baris 27:
```cpp
String serverUrl = "http://192.168.2.64:8000/process-audio";
//                         ^^^^^^^^^^^^^^
//                         Ganti dengan IP server Anda
```

#### Upload ke ESP32
1. Open `esp32_voice_assistant.ino` di Arduino IDE
2. Tools → Board → ESP32 Dev Module (sesuaikan board Anda)
3. Tools → Port → pilih port COM ESP32
4. Upload (Ctrl+U)

#### Monitor Serial
```
Tools → Serial Monitor (Ctrl+Shift+M)
Baud rate: 115200
```

Output yang diharapkan:
```
=================================
ESP32 Voice Assistant
=================================
Connecting to WiFi.........
✅ WiFi Connected
IP Address: 192.168.2.xxx
Server: http://192.168.2.64:8000/process-audio
✅ Microphone initialized
✅ Speaker initialized
=================================
Ready! Press button to talk...
=================================
```

## 📱 Usage

### Basic Usage
1. **Tekan dan tahan tombol** untuk mulai merekam
2. **Bicara** dengan jelas (max 3 detik atau lepas tombol lebih cepat)
3. **Lepas tombol** untuk mengirim audio
4. **Tunggu respon** diputar melalui speaker

### LED Status
- **Menyala terus**: WiFi connected, ready
- **Berkedip cepat**: Recording
- **Berkedip lambat**: Processing
- **Kedip-kedip cepat**: Error
- **3x kedip**: Success

### Perintah yang Didukung

#### Simple Commands (tanpa AI)
```
"Halo" / "Hai"              → Greeting
"Nyalakan lampu"            → Turn on light
"Matikan lampu"             → Turn off light
"Jam berapa?"               → Current time
"Tanggal berapa?"           → Current date
"Bantuan"                   → Help message
```

#### AI-Powered (untuk pertanyaan lain)
```
"Siapa presiden Indonesia?"
"Jelaskan tentang fotosintesis"
"Berapa hasil 25 kali 4?"
"Ceritakan tentang Jakarta"
```

## 🔧 Troubleshooting

### ESP32 tidak connect ke WiFi
- Cek `secrets.h` sudah benar
- Pastikan WiFi 2.4GHz (ESP32 tidak support 5GHz)
- Cek jarak ke router

### Server error "GROQ_API_KEY not found"
- Pastikan file `.env` ada dan terisi
- Cek API key valid di [console.groq.com](https://console.groq.com/)

### Audio tidak terdengar / pecah
- Cek wiring speaker
- Pastikan power supply cukup (min 5V 2A)
- Coba turunkan volume (edit gain di hardware)

### "Not enough RAM" error
- ESP32 kehabisan memory
- Reduce `MAX_DURATION` dari 3 ke 2 detik
- Pastikan tidak ada program lain yang berjalan

### Audio terlalu pelan/keras
**Server side** - Edit `server.py` line 93:
```python
audio = audio + 3  # Ubah angka (0-10 dB)
```

**ESP32 side** - Tambahkan resistor pada GAIN pin MAX98357A:
- Open (tidak connect): 9dB
- GND: 9dB  
- 100K to GND: 12dB
- 100K to VDD: 15dB

## 📊 Performance

- **Recording**: 3 detik
- **Upload**: ~1-2 detik (tergantung WiFi)
- **STT**: ~0.5-1 detik
- **AI Response**: ~1-2 detik
- **TTS**: ~0.5-1 detik
- **Total**: ~5-8 detik per request

## 🔐 Security Notes

- ⚠️ **Jangan commit** file `secrets.h` atau `.env` ke Git
- ⚠️ **Jangan share** API key Groq Anda
- ✅ Gunakan file `.gitignore`:
```
secrets.h
.env
```

## 📝 Customization

### Ganti Voice (Suara TTS)
Edit `server.py` line 26:
```python
VOICE_ID = "id-ID-GadisNeural"  # Suara perempuan
# atau
VOICE_ID = "id-ID-ArdiNeural"   # Suara laki-laki
```

### Ganti AI Model
Edit `server.py` line 54:
```python
model="llama-3.3-70b-versatile"  # Default
# atau
model="llama-3.1-70b-versatile"
model="mixtral-8x7b-32768"
```

### Custom Commands
Edit fungsi `process_simple_command()` di `server.py` line 68-118.

Contoh menambahkan perintah baru:
```python
# AC control
if "nyalakan ac" in text_lower:
    return "Baik, AC telah dinyalakan.", False

if "matikan ac" in text_lower:
    return "Baik, AC telah dimatikan.", False
```

## 🤝 Contributing

Contributions welcome! Silakan buat issue atau pull request.

## 📄 License

MIT License - Feel free to use in your projects!

## 🙏 Credits

- **Groq** - STT & AI processing
- **Edge-TTS** - Text-to-speech
- **ESP32** - Microcontroller
- **INMP441** - MEMS microphone
- **MAX98357A** - I2S audio amplifier

## 📞 Support

Jika ada pertanyaan atau masalah, silakan buat issue di GitHub repository.

---

Made with ❤️ for Indonesian voice assistant enthusiasts
