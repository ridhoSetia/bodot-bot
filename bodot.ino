#include <driver/i2s.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

// ================= CONFIG =================
const char* ssid = "NAMA_WIFI_KAMU";
const char* password = "PASSWORD_WIFI_KAMU";

// Ganti dengan IP Address PC/Server yang menjalankan Docker
// Jika pakai Tailscale/VPN, masukkan IP dari VPN tersebut
const char* serverHost = "IP_ADDRESS_SERVER_KAMU"; 
const int serverPort = 8010;

#define BUTTON_PIN    4
#define RELAY_PIN     2
#define I2S_MIC_WS   25
#define I2S_MIC_SD   32
#define I2S_MIC_SCK  33
#define I2S_SPK_BCLK 14
#define I2S_SPK_LRC  27
#define I2S_SPK_DIN  26
#define SAMPLE_RATE  16000
#define I2S_PORT     I2S_NUM_0
#define READ_BUF     1024

int ledConn = 18;
int ledThinking = 5;

WiFiClientSecure client;
Adafruit_SSD1306 display(128, 64, &Wire, -1);

// ================= DISPLAY =================
void showStatus(String line1, String line2) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0); display.println(line1);
  display.setCursor(0, 10); display.println(line2);
  display.display();
}

// ================= I2S =================
void setupI2S(i2s_mode_t mode) {
  i2s_driver_uninstall(I2S_PORT);
  i2s_config_t cfg = {
    .mode = mode,
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = (mode & I2S_MODE_RX) ? I2S_BITS_PER_SAMPLE_32BIT : I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 16,
    .dma_buf_len = 512,
    .use_apll = false,
    .tx_desc_auto_clear = true
  };
  i2s_pin_config_t pins = {
    .bck_io_num  = (mode & I2S_MODE_RX) ? I2S_MIC_SCK  : I2S_SPK_BCLK,
    .ws_io_num   = (mode & I2S_MODE_RX) ? I2S_MIC_WS   : I2S_SPK_LRC,
    .data_out_num= (mode & I2S_MODE_TX) ? I2S_SPK_DIN  : I2S_PIN_NO_CHANGE,
    .data_in_num = (mode & I2S_MODE_RX) ? I2S_MIC_SD   : I2S_PIN_NO_CHANGE
  };
  i2s_driver_install(I2S_PORT, &cfg, 0, NULL);
  i2s_set_pin(I2S_PORT, &pins);
  if (mode & I2S_MODE_TX)
    i2s_set_clk(I2S_PORT, SAMPLE_RATE, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
}

// ================= SERVER =================
bool connectToServer() {
  if (client.connected()) return true;
  client.stop();
  delay(100);
  digitalWrite(ledConn, HIGH);
  client.setInsecure();
  client.setTimeout(30000);
  for (int i = 1; i <= 3; i++) {
    showStatus("SERVER", "Connecting " + String(i) + "/3");
    if (client.connect(serverHost, serverPort)) {
      client.setNoDelay(true);
      Serial.println("Connected!");
      digitalWrite(ledConn, LOW);
      return true;
    }
    digitalWrite(ledConn, LOW);
    delay(1000 * i);
  }
  return false;
}

// ================= SMART HOME =================
void handleSmartHome(String text) {
  text.toLowerCase();
  if (text.indexOf("nyalakan lampu") >= 0) digitalWrite(RELAY_PIN, HIGH);
  else if (text.indexOf("matikan lampu") >= 0) digitalWrite(RELAY_PIN, LOW);
}

// ================= PLAYBACK =================
// Server FastAPI pakai Response biasa → Content-Length, bukan chunked
void playAudio(int contentLength) {
  if (contentLength <= 0) {
    Serial.println("[PLAY] Content-Length tidak valid, skip.");
    return;
  }

  uint8_t* buf = (uint8_t*)malloc(READ_BUF);
  if (!buf) { Serial.println("[PLAY] malloc gagal!"); return; }

  size_t written;
  int remaining = contentLength;
  unsigned long lastData = millis();

  Serial.printf("[PLAY] Mulai playback %d bytes\n", contentLength);

  while (remaining > 0 && client.connected() && millis() - lastData < 8000) {
    int avail = client.available();
    if (avail <= 0) { delay(1); continue; }

    int toRead = min(avail, min(READ_BUF, remaining));
    int len = client.read(buf, toRead);
    if (len <= 0) { delay(1); continue; }

    lastData = millis();
    remaining -= len;
    i2s_write(I2S_PORT, buf, len, &written, portMAX_DELAY);
  }

  free(buf);
  Serial.printf("[PLAY] Selesai. Sisa=%d bytes\n", remaining);
  delay(300);
  i2s_zero_dma_buffer(I2S_PORT);
  delay(200);
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT); digitalWrite(RELAY_PIN, LOW);
  pinMode(ledConn, OUTPUT);   digitalWrite(ledConn, LOW);
  pinMode(ledThinking, OUTPUT); digitalWrite(ledThinking, LOW);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextColor(WHITE);
  showStatus("WIFI", "Connecting...");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.println("WiFi: " + WiFi.localIP().toString());

  showStatus("SERVER", "Pre-connecting...");
  if (connectToServer()) showStatus("BODOT READY", "Tahan Tombol");
  else { showStatus("BODOT READY", "Server offline?"); digitalWrite(ledConn, HIGH); }

  setupI2S(I2S_MODE_RX);
}

// ================= LOOP =================
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    showStatus("WIFI LOST", "Reconnecting...");
    WiFi.disconnect(); WiFi.begin(ssid, password);
    unsigned long t = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t < 10000) delay(500);
    if (WiFi.status() != WL_CONNECTED) return;
  }

  if (digitalRead(BUTTON_PIN) == LOW) {

    if (!connectToServer()) {
      digitalWrite(ledConn, HIGH);
      showStatus("CONN FAILED", "Cek server/wifi");
      delay(1000); return;
    }

    // ---- FASE 1: REKAM & KIRIM AUDIO ----
    client.println("POST /process-audio HTTP/1.1");
    client.print("Host: "); client.println(serverHost);
    client.println("Content-Type: application/octet-stream");
    client.println("Transfer-Encoding: chunked");
    client.println("Connection: keep-alive");
    client.println();

    setupI2S(I2S_MODE_RX);
    showStatus("MENDENGAR", "...");

    int32_t* mBuf = (int32_t*)malloc(512 * 4);
    uint8_t* sBuf = (uint8_t*)malloc(512 * 2);
    size_t br;

    while (digitalRead(BUTTON_PIN) == LOW) {
      i2s_read(I2S_PORT, mBuf, 512 * 4, &br, 20);
      if (br > 0) {
        int samples = br / 4;
        for (int i = 0; i < samples; i++) {
          int16_t s = mBuf[i] >> 14;
          sBuf[i*2]   = s & 0xFF;
          sBuf[i*2+1] = (s >> 8) & 0xFF;
        }
        client.print(samples * 2, HEX);
        client.print("\r\n");
        client.write(sBuf, samples * 2);
        client.print("\r\n");
      }
    }
    client.print("0\r\n\r\n");
    client.flush();
    free(mBuf); free(sBuf);

    // ---- FASE 2: BACA HEADER RESPONSE ----
    digitalWrite(ledThinking, HIGH);
    showStatus("BODOT", "Berpikir...");

    String aiReply = "";
    int contentLength = -1;
    bool headerEnded = false;
    unsigned long timeout = millis();
    uint8_t pBuf[512];

    while (client.connected() && millis() - timeout < 10000) {
      if (!client.available()) continue;
      String line = client.readStringUntil('\n');
      line.trim();
      Serial.println("[HDR] " + line);

      if (line.startsWith("Content-Length:")) {
        { String cl = line.substring(15); cl.trim(); contentLength = cl.toInt(); }
      }
      if (line.startsWith("X-Reply:")) {
        aiReply = line.substring(8); aiReply.trim();
        handleSmartHome(aiReply);
      }
      if (line.length() == 0) {
        headerEnded = true;
        break;
      }
    }

    Serial.printf("[HDR] Content-Length=%d | Reply=%s\n", contentLength, aiReply.c_str());

    // ---- FASE 3: PLAYBACK ----
    if (headerEnded) {
      digitalWrite(ledThinking, LOW);
      showStatus("BODOT:", aiReply);
      setupI2S(I2S_MODE_TX);
      delay(100);
      playAudio(contentLength);
      setupI2S(I2S_MODE_RX);
      
      // --- TAMBAHAN LOGIKA BARU ---
      // 1. Tunggu dulu sampai tombol benar-benar dilepas (antisipasi kalau masih ditekan)
      while (digitalRead(BUTTON_PIN) == LOW) { delay(10); } 
      
      // 2. Berhenti di sini! Tetap tampilkan balasan AI sampai tombol ditekan lagi
      // Ini akan membuat layar tetap pada "BODOT: [aiReply]"
      while (digitalRead(BUTTON_PIN) == HIGH) { delay(10); } 
      
      // 3. Setelah ditekan lagi, baru bersihkan layar untuk masuk ke mode SIAP
      showStatus("SIAP", "Tahan tombol...");
      
      // Tunggu sebentar agar tidak langsung mentrigger rekaman baru
      delay(500); 
    } else {
      digitalWrite(ledThinking, LOW);
      showStatus("ERROR", "No Response");
      client.stop();
      // Tambahkan delay agar pesan error terbaca
      delay(2000); 
    }

    showStatus("SIAP", "Tahan tombol...");
  }

  delay(1);
}