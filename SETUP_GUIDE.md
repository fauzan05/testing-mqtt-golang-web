# CORE Testing System - Setup Guide

## Persiapan Hardware

### 1. ESP32 Setup
- Install library di Arduino IDE:
  - WiFi (built-in)
  - PubSubClient (MQTT) - Install via Library Manager
  - ArduinoJson - Install via Library Manager

### 2. Arduino Nano Setup
- Install library:
  - ArduinoJson - Install via Library Manager

### 3. Koneksi Serial ESP32 ↔ Nano
```
ESP32 TX2 (GPIO17) → Nano RX (D0)
ESP32 RX2 (GPIO16) → Nano TX (D1)
GND ESP32 → GND Nano
```

### 4. Koneksi Sensor & Relay
```
ESP32:
- GPIO32 (atau 35) → Relay IN
- GPIO34 (A0) → Voltage Sensor OUT
- GPIO35 (A1) → Current Sensor OUT
- GND → Common Ground

Arduino Nano:
- D9 (PWM) → Driver Injeksi Arus
- D13 → LED Indicator
```

---

## Software Configuration

### 1. Edit ESP32 Code (`esp32_core_tester.ino`)

**Line 13-16: WiFi Configuration**
```cpp
const char* WIFI_SSID = "NamaWiFiAnda";        // Ganti dengan nama WiFi
const char* WIFI_PASS = "PasswordWiFiAnda";    // Ganti dengan password WiFi
```

**Line 17: MQTT Server**
```cpp
const char* MQTT_SERVER = "192.168.1.100";     // Ganti dengan IP komputer yang jalankan server Golang
```

Cara cek IP komputer:
- Windows: Buka CMD → ketik `ipconfig` → lihat IPv4 Address
- Linux/Mac: Buka Terminal → ketik `ifconfig` atau `ip addr`

**Line 23-25: Pin Configuration** (sesuaikan dengan wiring Anda)
```cpp
#define RELAY_PIN 32        // Pin relay (cek schematic)
#define VOLTAGE_PIN 34      // Pin sensor tegangan
#define CURRENT_PIN 35      // Pin sensor arus
```

**Line 31-32: Sensor Calibration**
```cpp
const float VOLTAGE_RATIO = 5.0 / 1023.0;  // Sesuaikan dengan voltage divider
const float CURRENT_RATIO = 0.185;         // Sesuaikan dengan sensor arus (ACS712)
```

### 2. Arduino Nano Code (`arduino_nano_injector.ino`)

**Line 10: Pin Configuration**
```cpp
#define INJECT_CONTROL_PIN 9  // Pin output ke driver injeksi
```

---

## Cara Upload Code

### 1. Upload ke ESP32
1. Buka `esp32_core_tester.ino` di Arduino IDE
2. Pilih Board: **ESP32 Dev Module**
3. Pilih Port: (COM port ESP32 Anda)
4. Klik Upload
5. Buka Serial Monitor (115200 baud) untuk cek koneksi WiFi & MQTT

### 2. Upload ke Arduino Nano
1. Buka `arduino_nano_injector.ino` di Arduino IDE
2. Pilih Board: **Arduino Nano**
3. Pilih Processor: **ATmega328P (Old Bootloader)** atau **ATmega328P**
4. Pilih Port: (COM port Nano Anda)
5. Klik Upload
6. Buka Serial Monitor (9600 baud) untuk cek response

---

## Cara Menjalankan Sistem

### 1. Start MQTT Broker (jika belum running)
```bash
# Windows
mosquitto -v -c "C:\Program Files\mosquitto\mosquitto.conf"

# Linux
sudo systemctl start mosquitto

# Mac
brew services start mosquitto
```

### 2. Start Golang Server
```bash
cd testing-mqtt-golang-web
go run main.go
```

Server akan jalan di: `http://localhost:8000`

### 3. Power ON Hardware
1. Nyalakan ESP32 (via USB atau power supply)
2. Tunggu ESP32 connect ke WiFi (cek LED indicator)
3. Nyalakan Arduino Nano
4. Cek Serial Monitor ESP32 - harus muncul "MQTT Connected!"

### 4. Test dari Web
1. Buka browser → `http://localhost:8000`
2. Login dengan: username `admin`, password `12345`
3. Klik menu **Pengujian**
4. Pilih nama baju konduktif
5. Pilih titik pengukuran (R1-R8)
6. Klik **"⚡ Inject 200mA"**
7. Lihat nilai real-time dari sensor
8. Tunggu 2 menit, sistem auto-stop dan simpan hasil

---

## Troubleshooting

### ESP32 tidak connect WiFi
- Cek SSID dan password sudah benar
- Pastikan WiFi 2.4GHz (ESP32 tidak support 5GHz)
- Cek jarak ESP32 ke router

### MQTT Failed
- Pastikan MQTT broker (mosquitto) sudah running
- Cek IP server sudah benar di kode ESP32
- Test ping dari ESP32 ke server: `ping 192.168.1.100`

### Data sensor tidak muncul di web
- Buka Serial Monitor ESP32, cek apakah ada data sensor
- Buka browser console (F12), cek WebSocket connection
- Cek topik MQTT sudah sama antara ESP32 dan server

### Relay tidak switching
- Cek koneksi pin GPIO → Relay
- Cek relay membutuhkan level trigger (active HIGH atau LOW)
- Test manual dengan digitalWrite di loop()

### Arduino Nano tidak response
- Cek koneksi Serial RX/TX
- Pastikan GND sudah terhubung
- Cek baud rate sama (9600)
- Buka Serial Monitor Nano untuk debug

---

## Testing Manual

### Test MQTT (tanpa ESP32)
Gunakan MQTT client seperti MQTT Explorer atau mosquitto_pub:

```bash
# Test publish
mosquitto_pub -h localhost -t "golang-webserver/topic" -m '{"voltage":5.2,"current":198,"resistance":26.3}'

# Test subscribe
mosquitto_sub -h localhost -t "golang-webserver/topic"
```

### Test WebSocket (browser console)
Buka browser console (F12) di halaman Pengujian, ketik:

```javascript
// Cek WebSocket connection
console.log(ws.readyState); // 1 = OPEN

// Send test command
ws.send(JSON.stringify({command: 'inject', point: 'R1'}));
```

---

## Production Checklist

- [ ] Kalibrasi sensor tegangan (bandingkan dengan multimeter)
- [ ] Kalibrasi sensor arus (gunakan beban known)
- [ ] Test dengan baju konduktif real (8 titik pengukuran)
- [ ] Verifikasi threshold Good/Bad (10 Ohm)
- [ ] Test durasi 2 menit full
- [ ] Test auto-stop dan save ke database
- [ ] Test export PDF
- [ ] Test multi-user concurrent

---

## Kalibrasi Sensor

### Voltage Sensor
1. Ukur tegangan output dengan multimeter
2. Baca nilai ADC dari ESP32
3. Hitung ratio: `VOLTAGE_RATIO = Vmultimeter / Vadc`
4. Update di kode ESP32 line 31

### Current Sensor (ACS712)
1. Tidak ada beban: ADC harus ~512 (2.5V)
2. Dengan beban known (misal 1A): ADC berubah
3. Hitung sensitivity: `CURRENT_RATIO = (Vadc - 2.5) / Ireal`
4. Update di kode ESP32 line 32

---

## Next Steps

1. **Database Migration**: Pindah dari localStorage ke MySQL
2. **Cloud Deployment**: Deploy ke VPS untuk akses remote
3. **Multi-device**: Support banyak ESP32 dengan Device ID berbeda
4. **Data Analytics**: Dashboard grafik trend Good/Bad over time
5. **Mobile App**: React Native untuk monitoring mobile

---

## Support

Jika ada masalah, cek:
1. Serial Monitor ESP32 (115200 baud)
2. Serial Monitor Nano (9600 baud)
3. Browser Console (F12)
4. Server log (terminal tempat `go run main.go`)

File log error akan membantu troubleshooting.
