# Serial USB Connection Setup

## 📦 Install Serial Library

Untuk mengaktifkan koneksi USB Serial ke ESP32, install library Go:

```powershell
# Install serial library
go get go.bug.st/serial

# Update go.mod
go mod tidy
```

## 🔌 Cara Menggunakan Serial USB

### 1. **Cek COM Port**
```powershell
# Windows - cek port yang tersedia
mode

# Atau lihat di Device Manager → Ports (COM & LPT)
# ESP32 biasanya muncul sebagai: Silicon Labs CP210x (COMx)
```

### 2. **Set Environment Variable**
```powershell
# Set COM port untuk ESP32
$env:ESP32_SERIAL_PORT="COM3"

# Atau permanent via System Settings
```

### 3. **Run Server**
```powershell
go run main.go
```

Server akan otomatis:
- ✅ Coba HTTP ke ESP32 AP (192.168.4.1)
- ✅ Coba HTTP ke ESP32 Station (IP dari router)
- ✅ Fallback ke Serial USB jika HTTP gagal

## 🎯 Priority Order

1. **HTTP - AP Mode** (192.168.4.1)
2. **HTTP - Station Mode** (IP dari router PDKB_INTERNET_G)
3. **Serial USB** (COM port fallback)

## 🔧 Troubleshooting

### Driver ESP32 tidak terdeteksi
```powershell
# Download CP210x driver dari:
# https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers
```

### Permission denied on COM port
```powershell
# Pastikan tidak ada aplikasi lain yang pakai port
# Tutup Arduino IDE, PlatformIO, Serial Monitor, dll
```

### Baud rate error
```go
// Ubah baud rate di main.go jika perlu
// Default: 115200
```

## 📊 Status Check

Lihat log server:
```
✅ ESP32 HTTP connected (AP: 192.168.4.1)
✅ ESP32 HTTP connected (Station: 192.168.1.50)
✅ Serial USB connected (COM3)
```

## 🚀 Testing

```powershell
# Test HTTP connection
curl http://localhost:8000/esp32/status

# Check health endpoint
curl http://localhost:8000/health
```
