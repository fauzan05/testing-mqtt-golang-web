# 🔌 Multi-Connection Setup - CORE System

## ✅ Ketiga Mode Koneksi Sudah Diaktifkan!

### 1️⃣ **WiFi Lokal ESP32 (Access Point)**
- **SSID**: `CJack_ESP32`
- **Password**: `12345678`
- **IP ESP32**: `192.168.4.1`
- **Status**: ✅ Aktif
- **Cara pakai**: Laptop konek ke WiFi `CJack_ESP32`

### 2️⃣ **WiFi Router (Station Mode)**
- **Router**: `PDKB_INTERNET_G`
- **Password**: `uptpulogadung`
- **IP ESP32**: Auto (dari DHCP router)
- **Status**: ✅ Aktif (Dual mode dengan AP)
- **Cara pakai**: ESP32 dan laptop sama-sama konek ke router

### 3️⃣ **USB Serial (Fallback)**
- **Port**: COM3 (default, bisa diubah)
- **Baud Rate**: 115200
- **Status**: ⚠️ Perlu install library
- **Cara pakai**: Konek ESP32 ke laptop via USB

---

## 🚀 Quick Start

### Upload Code ESP32
```bash
# Via Arduino IDE atau PlatformIO
# Upload esp32/main.cpp ke ESP32
```

### Install Serial Library (Optional)
```powershell
cd testing-mqtt-golang-web
go get go.bug.st/serial
go mod tidy
```

### Run Go Server
```powershell
# Set environment variables (optional)
$env:ESP32_SERIAL_PORT="COM3"

# Run server
go run main.go
```

### Test Koneksi
```powershell
# Health check - lihat semua koneksi
curl http://localhost:8000/health

# Test ESP32 status
curl http://localhost:8000/esp32/status
```

---

## 📊 Cara Kerja Auto-Fallback

Go server akan **otomatis coba semua method** ini:

```
1. HTTP ke 192.168.4.1 (AP Mode)
   ↓ Gagal?
2. HTTP ke 192.168.x.x (Station Mode dari router)
   ↓ Gagal?
3. Serial USB COM port
   ↓ Gagal?
4. Return error 502
```

**Priority**:
- ✅ **WiFi AP** (tercepat, tidak butuh router)
- ✅ **WiFi Station** (multi-device, ada internet)
- ✅ **Serial USB** (backup, debugging)

---

## 🔧 Configuration Files

### ESP32 (main.cpp)
```cpp
// WiFi credentials
String wifiSSID = "PDKB_INTERNET_G";
String wifiPassword = "uptpulogadung";

// Dual mode enabled
bool dualWiFiMode = true;
```

### Go Server (main.go)
```go
// ESP32 IP tracking
esp32APIP = "192.168.4.1"
esp32StationIP = "" // Auto-detected

// Serial port
serialPortName = "COM3" // From env var
```

---

## 📱 Skenario Penggunaan

### **Skenario 1: Lab Testing (No Router)**
```
Laptop ←→ WiFi ←→ ESP32 (AP Mode: 192.168.4.1)
```
- Pakai WiFi lokal ESP32
- Portable, tidak butuh internet
- 1 laptop = 1 ESP32

### **Skenario 2: Production (With Router)**
```
Laptop A ←→ Router ←→ ESP32 (Station: 192.168.1.50)
Laptop B ←→ Router ←→ ESP32 (Station: 192.168.1.50)
```
- ESP32 konek ke PDKB_INTERNET_G
- Multi-user bisa akses bersamaan
- Ada internet untuk cloud upload

### **Skenario 3: Debugging (USB Cable)**
```
Laptop ←→ USB Cable ←→ ESP32 (Serial: COM3)
```
- Fallback jika WiFi bermasalah
- Lihat serial monitor langsung
- Upload code sambil test

### **Skenario 4: Hybrid (All 3 Active)**
```
Laptop A ←→ WiFi AP ←→ ESP32 ←→ Router WiFi ←→ Laptop B
                        ↓
                    USB ←→ Laptop C (Debug)
```
- 3 koneksi aktif bersamaan!
- Maximum flexibility

---

## 🔍 Monitoring

### Log ESP32 (Serial Monitor)
```
✅ Access Point started! AP IP: 192.168.4.1
🌐 Connecting to router WiFi: PDKB_INTERNET_G
✅ Connected to router! Station IP: 192.168.1.50
📡 ESP32 now accessible from 2 networks:
   1. AP Mode: 192.168.4.1 (Direct WiFi: CJack_ESP32)
   2. Station Mode: 192.168.1.50 (Router: PDKB_INTERNET_G)
```

### Log Go Server
```
✅ Trying AP mode: http://192.168.4.1/status
✅ AP mode success
```
atau
```
⚠️ AP mode failed
✅ Trying Station mode: http://192.168.1.50/status
✅ Station mode success
```

### Web Interface (pengujian.html)
```
Server: Connected ✅
ESP32: Connected ✅
```

---

## ⚠️ Troubleshooting

### ESP32 tidak konek ke router
```cpp
// Cek credentials di main.cpp
String wifiSSID = "PDKB_INTERNET_G"; // ← Pastikan benar
String wifiPassword = "uptpulogadung"; // ← Pastikan benar
```

### Laptop tidak bisa akses 192.168.4.1
```
1. Disconnect dari WiFi lain
2. Konek ke "CJack_ESP32"
3. Password: 12345678
4. Ping 192.168.4.1
```

### Serial port tidak terdeteksi
```powershell
# Install driver CP210x
# Download dari: https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers

# Cek port
mode

# Set environment
$env:ESP32_SERIAL_PORT="COM3"
```

### Go server error "library not found"
```powershell
# Install serial library
go get go.bug.st/serial
go mod tidy
go run main.go
```

---

## 📈 Next Steps

1. **Upload `main.cpp`** ke ESP32
2. **Tekan GPIO 22 switch** untuk aktifkan web server
3. **Cek Serial Monitor** untuk melihat IP addresses
4. **Run Go server**: `go run main.go`
5. **Open browser**: `http://localhost:8000/pengujian`
6. **Test inject 200mA** - sistem otomatis pilih koneksi terbaik!

---

## 🎯 Benefits Multi-Connection

✅ **Reliability**: 3x backup connections
✅ **Flexibility**: Pilih mode sesuai situasi
✅ **Scalability**: Multi-user via router
✅ **Debugging**: Serial monitor always available
✅ **Portability**: Tidak harus butuh router

---

**Status**: ✅ All 3 modes ready to use!
**Last Updated**: 2025-11-18
