# 🚀 ESP32 MQTT Setup Guide - Production Ready

## 📋 Arsitektur Sistem

```
┌─────────────┐      MQTT Broker       ┌─────────────┐
│  ESP32 #1   │◄────►(HiveMQ/Mosquitto)◄────►│   Golang    │
│  (Lokasi A) │      Topic: cjack/*    │   Web Server  │
└─────────────┘                        └─────────────┘
                                              ▲
┌─────────────┐                              │
│  ESP32 #2   │                              │ HTTP/WS
│  (Lokasi B) │◄─────────────────────────────┤
└─────────────┘                         ┌────┴────┐
                                        │ Browser │
┌─────────────┐                         │  User   │
│  ESP32 #n   │                         └─────────┘
│  (Lokasi n) │
└─────────────┘
```

## ✅ Keuntungan MQTT vs HTTP Direct:

| Aspek | HTTP Direct | MQTT |
|-------|-------------|------|
| **Jarak** | Harus satu WiFi | Internet (global) |
| **Multi-device** | Perlu tahu IP tiap device | Subscribe 1x dapat semua |
| **Real-time** | Polling manual | Push otomatis |
| **Firewall** | Masalah dengan NAT | Broker handle routing |
| **Scalability** | 10-20 device max | Ribuan device |
| **Reliability** | Putus = lost data | QoS support |

## 🔧 Setup

### 1️⃣ Install Library di Arduino IDE

1. **Buka Arduino IDE**
2. **Tools → Manage Libraries**
3. **Search: "PubSubClient"**
4. **Install: PubSubClient by Nick O'Leary**

### 2️⃣ Upload ESP32 Code

File: `esp32_mqtt.ino`

**Edit baris 12-14 (WiFi):**
```cpp
const char* wifi_ssid = "UMAIR";        // WiFi di lokasi device
const char* wifi_password = "12345678"; // Password WiFi
```

**Edit baris 20 (Device ID) - PENTING!**
```cpp
const char* device_id = "esp32_001";  // Beda per device: esp32_001, esp32_002, dst
```

**Upload ke ESP32:**
- Klik Upload
- Hold BOOT button saat "Connecting..."
- Tunggu "Done uploading"

### 3️⃣ Cek Serial Monitor

Set baud rate: **115200**

Output sukses:
```
=== ESP32 MQTT Control ===
[INFO] Device ID: esp32_001
[INFO] Control Topic: cjack/esp32_001/control
[INFO] Status Topic: cjack/esp32_001/status
[WIFI] Connecting to: UMAIR
..........
[OK] WiFi Connected!
[INFO] IP Address: 192.168.x.x
[MQTT] Connecting to broker... Connected!
[MQTT] Subscribed to: cjack/esp32_001/control
[OK] System Ready!
==========================
```

### 4️⃣ Update config.env

File: `config.env`

```env
MQTT_BROKER=tcp://broker.hivemq.com:1883
MQTT_CLIENT_ID=cjack-web-server
MQTT_TOPIC=cjack/#
```

**Pilihan MQTT Broker:**

**A. Public Broker (Testing):**
- HiveMQ: `tcp://broker.hivemq.com:1883`
- Eclipse: `tcp://mqtt.eclipseprojects.io:1883`
- Mosquitto: `tcp://test.mosquitto.org:1883`

⚠️ Warning: Public broker tidak secure, semua orang bisa lihat data!

**B. Deploy Sendiri (Production):**
- Install Mosquitto di server
- URL: `tcp://your-server-ip:1883`
- Add authentication untuk keamanan

### 5️⃣ Run Web Server

```bash
go run main.go
```

Server akan:
- ✅ Connect ke MQTT broker
- ✅ Subscribe ke `cjack/#` (semua device)
- ✅ Forward data ke WebSocket clients

## 📱 Cara Pakai

### Control dari Web (pengujian.html)

Browser akan kirim MQTT message:

```javascript
// Publish ke topic: cjack/esp32_001/control
{
  "action": "inject"  // atau "blink" atau "stop"
}
```

ESP32 akan:
1. Terima message dari broker
2. Execute command (LED ON/OFF/BLINK)
3. Publish status ke `cjack/esp32_001/status`
4. Web terima status via WebSocket

### MQTT Topics Structure

```
cjack/
├── esp32_001/
│   ├── control  (Web → ESP32: commands)
│   ├── status   (ESP32 → Web: device status)
│   └── sensor   (ESP32 → Web: sensor data)
│
├── esp32_002/
│   ├── control
│   ├── status
│   └── sensor
│
└── esp32_xxx/
    └── ...
```

## 🧪 Testing dengan MQTT Explorer

1. **Download MQTT Explorer**: http://mqtt-explorer.com/
2. **Connect ke broker:**
   - Host: `broker.hivemq.com`
   - Port: `1883`
3. **Subscribe ke:** `cjack/#`
4. **Publish test command:**
   - Topic: `cjack/esp32_001/control`
   - Message: `{"action":"inject"}`
5. **Lihat response:**
   - Topic: `cjack/esp32_001/status`
   - Payload: `{"device":"esp32_001","injecting":true,...}`

## 🔐 Production Security Checklist

- [ ] Deploy private MQTT broker (jangan pakai public)
- [ ] Enable username/password authentication
- [ ] Enable TLS/SSL encryption (`mqtts://`)
- [ ] Implementasi access control per device
- [ ] Add device authentication (JWT/token)
- [ ] Monitor broker logs
- [ ] Setup firewall rules
- [ ] Regular password rotation

## 📊 Monitoring

ESP32 kirim status tiap 5 detik:

```json
{
  "device": "esp32_001",
  "injecting": false,
  "blinking": false,
  "wifi_rssi": -45,
  "uptime": 12345,
  "ip": "192.168.1.100"
}
```

Sensor data (tiap 5 detik):

```json
{
  "device": "esp32_001",
  "voltage": 220.5,
  "current": 0.2,
  "timestamp": 123456789
}
```

## 🐛 Troubleshooting

**ESP32 tidak connect ke broker:**
- Cek WiFi connect (LED blink 3x = success)
- Cek broker URL di baris 16
- Test broker dengan MQTT Explorer dulu
- Coba broker lain (Eclipse/Mosquitto)

**Web tidak terima data:**
- Cek `config.env` MQTT_BROKER sama dengan ESP32
- Cek Golang console: "Connected to MQTT broker"
- Cek WebSocket connected di browser console
- Test dengan MQTT Explorer

**Message hilang:**
- Set QoS level di code (default 0)
- Check broker connection stability
- Monitor broker logs

## 📝 Next Steps

1. **Upload esp32_mqtt.ino ke semua device**
2. **Ganti device_id unik per device**
3. **Update pengujian.html untuk publish MQTT**
4. **Test control dari web**
5. **Deploy production broker jika perlu**
6. **Implementasi sensor reading (HLK2)**
7. **Add data logging ke database**

## 💡 Tips

- Device ID format: `esp32_001`, `esp32_002`, dst
- WiFi bisa beda per lokasi (hardcode di code masing-masing)
- Broker bisa di cloud (AWS/Azure/DigitalOcean)
- Untuk production: pakai TLS dan authentication!
