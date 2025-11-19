# 🚀 Quick Start - MQTT Setup

## 1️⃣ Install Library ESP32

**Arduino IDE:**
1. Tools → Manage Libraries
2. Search: `PubSubClient`
3. Install: **PubSubClient by Nick O'Leary**

## 2️⃣ Upload ESP32 Code

**File:** `esp32_mqtt.ino`

**Edit WiFi (baris 12-14):**
```cpp
const char* wifi_ssid = "UMAIR";        // Ganti dengan WiFi kamu
const char* wifi_password = "12345678";
```

**Edit Device ID (baris 20) - PENTING!**
```cpp
const char* device_id = "esp32_001";  // Harus unik per device!
```

**Upload:**
- Klik Upload
- Hold BOOT button
- Tunggu selesai

## 3️⃣ Check Serial Monitor

Buka Serial Monitor (115200 baud):

```
=== ESP32 MQTT Control ===
[INFO] Device ID: esp32_001
[WIFI] Connecting to: UMAIR
..........
[OK] WiFi Connected!
[INFO] IP Address: 192.168.1.100
[MQTT] Connecting to broker... Connected!
[MQTT] Subscribed to: cjack/esp32_001/control
[OK] System Ready!
```

✅ Jika muncul ini = **SUCCESS!**

## 4️⃣ Run Web Server

**Terminal:**
```bash
cd C:\Users\PLN\Documents\GitHub\testing-mqtt-golang-web
go run main.go
```

**Output:**
```
Connecting to MQTT broker: tcp://broker.hivemq.com:1883
Connected to MQTT broker
Subscribed to topic: cjack/#
Server running on http://localhost:8000
```

## 5️⃣ Test dari Browser

1. **Buka:** http://localhost:8000/pengujian
2. **Login:** admin / 12345
3. **Edit baris 208-209 di pengujian.html:**
   ```javascript
   const CONTROL_MODE = 'mqtt';        // Aktifkan MQTT mode
   const ESP32_DEVICE_ID = 'esp32_001'; // Sesuaikan dengan device ID kamu
   ```
4. **Pilih baju** dari dropdown
5. **Pilih titik** pengukuran (R1-R8)
6. **Klik:** "⚡ Inject 200mA"

**Expected Result:**
- ✅ Alert: "📡 Command sent via MQTT: inject"
- ✅ LED ESP32 menyala
- ✅ Serial Monitor: `[LED] INJECT ON`
- ✅ Setelah 2 detik: LED mati otomatis
- ✅ Serial Monitor: `[LED] INJECT OFF`

## 🐛 Troubleshooting

**ESP32 tidak connect ke broker:**
```
[MQTT] Connecting to broker... FAILED! rc=-2
```
→ Cek WiFi connected dulu (IP address muncul)
→ Coba broker lain: `tcp://mqtt.eclipseprojects.io:1883`

**Web tidak terima data:**
```
WebSocket error: ...
```
→ Refresh browser
→ Cek Golang console: "Connected to MQTT broker"

**MQTT command tidak sampai:**
→ Cek browser console (F12): "MQTT command sent: ..."
→ Cek Serial Monitor ESP32
→ Test dengan MQTT Explorer dulu

## 📊 Monitor dengan MQTT Explorer

1. Download: http://mqtt-explorer.com/
2. Connect:
   - Host: `broker.hivemq.com`
   - Port: `1883`
3. Subscribe: `cjack/#`
4. Lihat semua message real-time!

## 🎯 Multi-Device Setup

**Device 1:**
```cpp
const char* device_id = "esp32_001";
const char* wifi_ssid = "WiFi_Lokasi_A";
```

**Device 2:**
```cpp
const char* device_id = "esp32_002";
const char* wifi_ssid = "WiFi_Lokasi_B";
```

**Web:**
```javascript
const ESP32_DEVICE_ID = 'esp32_001'; // Ganti sesuai device yang mau dikontrol
```

## ✨ Features

- ✅ Control dari mana aja (selama ada internet)
- ✅ Bisa kontrol banyak device
- ✅ Real-time status monitoring
- ✅ Auto-reconnect WiFi & MQTT
- ✅ Command history di broker
- ✅ Scalable untuk production

## 🔒 Production Notes

Untuk production, ganti ke private broker:
1. Deploy Mosquitto di server sendiri
2. Enable authentication
3. Enable TLS/SSL
4. Update `config.env` dan ESP32 code

File: `MQTT_SETUP.md` untuk detail lengkap!
