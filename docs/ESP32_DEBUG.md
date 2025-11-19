# 🐛 Troubleshooting - ESP32 MQTT Not Working

## Issue 1: ESP32 LED Tidak Nyala

### Checklist:

#### ✅ 1. ESP32 Hardware Check
```
□ ESP32 terhubung ke USB?
□ LED on-board ada di pin GPIO 2?
□ Power LED ESP32 nyala?
```

**Test Manual:**
```cpp
// Upload kode test sederhana:
void setup() {
  pinMode(2, OUTPUT);
}
void loop() {
  digitalWrite(2, HIGH);
  delay(1000);
  digitalWrite(2, LOW);
  delay(1000);
}
```
Jika LED blink = Hardware OK ✅

---

#### ✅ 2. ESP32 Code Upload Check

**Arduino IDE:**
```
Tools → Board → ESP32 Dev Module
Tools → Port → COM5 (atau port yang terdeteksi)
Upload Speed → 115200
```

**Upload `esp32_mqtt.ino`:**
1. Klik Upload
2. Hold BOOT button saat "Connecting..."
3. Tunggu "Done uploading"
4. Press EN button (reset)

**Serial Monitor (115200 baud):**
```
Harus muncul:
=== ESP32 MQTT Control ===
[INFO] Device ID: esp32_001
[WIFI] Connecting to: UMAIR
..........
[OK] WiFi Connected!
[INFO] IP Address: 192.168.x.x
[MQTT] Connecting to broker... Connected!
[MQTT] Subscribed to: cjack/esp32_001/control
[OK] System Ready!
```

❌ **Jika tidak muncul:**
- ESP32 tidak running
- Upload gagal
- Wrong board selection

---

#### ✅ 3. WiFi Connection Check

**Serial Monitor:**
```
[WIFI] Connecting to: UMAIR
....................
[ERROR] WiFi gagal connect!
```

**Fix:**
Edit `esp32_mqtt.ino` baris 13-14:
```cpp
const char* wifi_ssid = "UMAIR";        // ← GANTI dengan nama WiFi HP kamu
const char* wifi_password = "12345678"; // ← GANTI dengan password WiFi HP
```

**Re-upload** setelah edit!

---

#### ✅ 4. MQTT Broker Connection Check

**Serial Monitor:**
```
[MQTT] Connecting to broker... Connected!
```

❌ **Jika muncul:**
```
[MQTT] Connecting to broker... FAILED! rc=-2
Retrying in 5s...
```

**Coba broker alternatif:**
```cpp
// Edit esp32_mqtt.ino baris 17:
const char* mqtt_server = "test.mosquitto.org";
// atau
const char* mqtt_server = "mqtt.eclipseprojects.io";
```

---

#### ✅ 5. Test Manual MQTT Command

**Serial Monitor sudah show:**
```
[MQTT] Subscribed to: cjack/esp32_001/control
```

**Download MQTT Explorer:**
http://mqtt-explorer.com/

**Connect:**
- Host: `broker.hivemq.com`
- Port: `1883`
- Click "Connect"

**Publish Manual Command:**
- Topic: `cjack/esp32_001/control`
- Message: `{"action":"inject"}`
- Click "Publish"

**Serial Monitor harus show:**
```
[MQTT] Message arrived [cjack/esp32_001/control]: {"action":"inject"}
[LED] INJECT ON
[MQTT] Published status: {"device":"esp32_001","injecting":true,...}
```

**LED ESP32 HARUS NYALA!**

✅ Jika nyala = ESP32 MQTT working! Problem di Web/Golang

❌ Jika tidak nyala = Check code/wiring

---

## Issue 2: Indikator "Server Connected" Palsu

### Checklist:

#### ✅ 1. Golang Server Running?

**Terminal:**
```bash
cd C:\Users\PLN\Documents\GitHub\testing-mqtt-golang-web
go run main.go
```

**Output harus:**
```
Connecting to MQTT broker: tcp://broker.hivemq.com:1883
Client ID: cjack-web-server
Connected to MQTT broker
Subscribed to topic: cjack/#
Server running on http://localhost:8000
```

❌ **Jika error:**
```
panic: ... config.env not found
```

**Fix:** Buat file `config.env`:
```env
MQTT_BROKER=tcp://broker.hivemq.com:1883
MQTT_CLIENT_ID=cjack-web-server
MQTT_TOPIC=cjack/#
```

---

#### ✅ 2. WebSocket Connection Real Check

**Browser Console (F12):**
```javascript
// Harus ada log:
WebSocket connected
```

**Jika ada error:**
```
WebSocket connection failed
```

**Fix:** Refresh browser atau restart Golang server

---

#### ✅ 3. MQTT Indicator Update Check

**Indikator harus berubah otomatis:**

| Kondisi | Server | MQTT | ESP32 |
|---------|--------|------|-------|
| Initial load | ⚫ Checking | ⚫ Disconnected | ⚫ Waiting |
| Golang connected | 🟢 Connected | 🟢 Connected | ⚫ Waiting |
| ESP32 kirim status | 🟢 Connected | 🟢 Connected | 🟢 esp32_001 |

**Jika stuck di "Checking":**
- WebSocket tidak connect
- Check browser console
- Restart Golang server

**Jika stuck di "MQTT: Disconnected":**
- Golang tidak connect ke broker
- Check `config.env`
- Check terminal Golang

**Jika stuck di "ESP32: Waiting":**
- ESP32 belum publish status
- Tunggu 5 detik (auto-report)
- Check Serial Monitor ESP32

---

## Full Test Flow

### Step 1: Upload & Check ESP32

```bash
1. Arduino IDE → Open esp32_mqtt.ino
2. Edit WiFi credentials (baris 13-14)
3. Upload
4. Open Serial Monitor (115200)
5. Verify output:
   ✅ WiFi Connected
   ✅ MQTT Connected
   ✅ System Ready
   ✅ LED blink 3x
```

### Step 2: Test with MQTT Explorer

```bash
1. Open MQTT Explorer
2. Connect to broker.hivemq.com:1883
3. Subscribe to: cjack/#
4. Publish to: cjack/esp32_001/control
   Message: {"action":"inject"}
5. Check Serial Monitor:
   ✅ [MQTT] Message arrived
   ✅ [LED] INJECT ON
   ✅ LED ESP32 nyala!
```

### Step 3: Start Golang Server

```bash
1. Terminal: go run main.go
2. Verify output:
   ✅ Connected to MQTT broker
   ✅ Subscribed to topic: cjack/#
   ✅ Server running on :8000
```

### Step 4: Test Web Interface

```bash
1. Browser: http://localhost:8000/pengujian
2. Login: admin / 12345
3. Check 3 indicators:
   ✅ Server: Connected (hijau)
   ✅ MQTT: Connected (kuning)
   ✅ ESP32: esp32_001 (biru) ← Tunggu 5 detik
4. Pilih baju dan titik
5. Klik "Inject 200mA"
6. Check:
   ✅ Alert: "📡 MQTT: Injecting..."
   ✅ Alert: "💡 ESP32 LED ON - Injecting!"
   ✅ Cards beranimasi pulse
   ✅ LED ESP32 nyala!
   ✅ Serial Monitor: [LED] INJECT ON
```

---

## Quick Diagnostic

**Problem:** "Command sent" tapi LED tidak nyala

**Check Serial Monitor:**

```
✅ GOOD: [MQTT] Message arrived [cjack/esp32_001/control]: {"action":"inject"}
   → ESP32 terima command
   
❌ BAD: No message
   → ESP32 tidak terima / tidak subscribe
   → Check: MQTT connected? Topic correct?
```

**Check MQTT Explorer:**

```
✅ GOOD: Topic cjack/esp32_001/status shows: {"injecting":true}
   → ESP32 execute command & publish status
   
❌ BAD: No status update
   → ESP32 not publishing
   → Check: mqtt.publish() called?
```

**Check Browser Console:**

```
✅ GOOD: Received: {topic: "cjack/esp32_001/status", payload: "{\"injecting\":true}"}
   → Web terima MQTT message dari Golang
   
❌ BAD: No message
   → Golang tidak broadcast
   → WebSocket disconnected?
```

---

## Common Mistakes

### ❌ 1. WiFi credentials salah
```cpp
const char* wifi_ssid = "UMAIR";  // ← Typo? Case sensitive!
```

### ❌ 2. Device ID tidak match
```cpp
// ESP32:
const char* device_id = "esp32_001";

// Web (pengujian.html baris 209):
const ESP32_DEVICE_ID = 'esp32_002';  // ← HARUS SAMA!
```

### ❌ 3. MQTT broker berbeda
```cpp
// ESP32:
const char* mqtt_server = "broker.hivemq.com";

// config.env:
MQTT_BROKER=tcp://test.mosquitto.org:1883  // ← HARUS SAMA!
```

### ❌ 4. Port salah
```cpp
const int mqtt_port = 1883;  // ✅ Correct
// NOT 8883 (that's for TLS)
```

### ❌ 5. Mode HTTP bukan MQTT
```javascript
// pengujian.html baris 208:
const CONTROL_MODE = 'http';  // ❌ SALAH!
const CONTROL_MODE = 'mqtt';  // ✅ BENAR!
```

---

## Debug Output Examples

### ✅ SUCCESS - ESP32 Serial Monitor:
```
=== ESP32 MQTT Control ===
[INFO] Device ID: esp32_001
[INFO] Control Topic: cjack/esp32_001/control
[INFO] Status Topic: cjack/esp32_001/status
[WIFI] Connecting to: UMAIR
..........
[OK] WiFi Connected!
[INFO] IP Address: 192.168.1.100
[MQTT] Connecting to broker... Connected!
[MQTT] Subscribed to: cjack/esp32_001/control
[OK] System Ready!
==========================
[MQTT] Published status: {"device":"esp32_001","injecting":false,"blinking":false,"wifi_rssi":-45,"uptime":5,"ip":"192.168.1.100"}
[MQTT] Message arrived [cjack/esp32_001/control]: {"action":"inject"}
[LED] INJECT ON
[MQTT] Published status: {"device":"esp32_001","injecting":true,"blinking":false,"wifi_rssi":-45,"uptime":10,"ip":"192.168.1.100"}
[LED] INJECT OFF
[MQTT] Published status: {"device":"esp32_001","injecting":false,"blinking":false,"wifi_rssi":-45,"uptime":12,"ip":"192.168.1.100"}
```

### ✅ SUCCESS - Golang Terminal:
```
Connecting to MQTT broker: tcp://broker.hivemq.com:1883
Client ID: cjack-web-server
Connected to MQTT broker
Subscribed to topic: cjack/#
Server running on http://localhost:8000

Received MQTT message - Topic: cjack/esp32_001/status, Payload: {"device":"esp32_001","injecting":false,...}
MQTT published - Topic: cjack/esp32_001/control, Message: {"action":"inject"}
Received MQTT message - Topic: cjack/esp32_001/status, Payload: {"device":"esp32_001","injecting":true,...}
```

### ✅ SUCCESS - Browser Console (F12):
```
WebSocket connected
MQTT command sent: {command: "mqtt_publish", topic: "cjack/esp32_001/control", message: "{\"action\":\"inject\"}"}
Received: {topic: "cjack/esp32_001/status", payload: "{\"device\":\"esp32_001\",\"injecting\":true,...}"}
ESP32 Status: {device: "esp32_001", injecting: true, blinking: false, ...}
```

---

## Still Not Working?

**Reset Everything:**

1. **ESP32:** Press EN button (hardware reset)
2. **Golang:** Ctrl+C → `go run main.go`
3. **Browser:** Hard refresh (Ctrl+Shift+R)
4. **Try Manual Test:** MQTT Explorer → Publish command
5. **Check Each Layer:**
   - Layer 1: ESP32 hardware (LED blink test)
   - Layer 2: ESP32 WiFi (Serial Monitor)
   - Layer 3: ESP32 MQTT (MQTT Explorer)
   - Layer 4: Golang MQTT (Terminal logs)
   - Layer 5: WebSocket (Browser console)
   - Layer 6: Web UI (Visual indicators)

**Share Debug Info:**
```
Serial Monitor: [paste ESP32 output]
Golang Terminal: [paste server logs]
Browser Console: [paste F12 logs]
MQTT Explorer: [screenshot topics]
```
