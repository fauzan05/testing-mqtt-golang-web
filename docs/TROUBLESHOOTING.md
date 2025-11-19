# 🔧 Troubleshooting: "tidak bisa" - ESP32 Connection Failed

## ❌ Problem
```
⚠️ AP mode failed: Get "http://192.168.4.1/status": context deadline exceeded
```

Server Go jalan, tapi tidak bisa konek ke ESP32.

---

## ✅ Solution Checklist

### 1️⃣ **Upload Code ESP32 yang Baru**

```bash
# Pastikan file main.cpp sudah di-upload ke ESP32
# Via Arduino IDE:
- Open: esp32/main.cpp
- Select Board: ESP32 Dev Module
- Select Port: COMx (lihat di Device Manager)
- Click Upload
```

**Ciri code berhasil upload:**
```
✅ Connecting....._____...._
✅ Writing at 0x00010000... (100%)
✅ Hash of data verified.
✅ Leaving...
✅ Hard resetting via RTS pin...
```

### 2️⃣ **Aktifkan Web Server Mode**

ESP32 harus dalam **web server mode** untuk bisa diakses.

```cpp
// GPIO 22 switch harus DITEKAN (active LOW)
// Atau hubungkan GPIO 22 ke GND
```

**Cara cek:**
1. Buka **Serial Monitor** (115200 baud)
2. Tekan **GPIO 22 switch**
3. Lihat output:

```
✅ Switch pressed! Activating web server mode...
✅ Starting Access Point...
✅ Access Point started! AP IP: 192.168.4.1
🌐 Connecting to router WiFi: PDKB_INTERNET_G
✅ Connected to router! Station IP: 192.168.1.50
📡 ESP32 now accessible from 2 networks:
   1. AP Mode: 192.168.4.1 (Direct WiFi: CJack_ESP32)
   2. Station Mode: 192.168.1.50 (Router: PDKB_INTERNET_G)
✅ Web server started
```

### 3️⃣ **Konek Laptop ke WiFi ESP32**

**Pilih salah satu:**

#### **Option A: WiFi Lokal (AP Mode)**
```
1. Disconnect dari WiFi lain
2. Connect ke WiFi: CJack_ESP32
3. Password: 12345678
4. IP ESP32: 192.168.4.1
```

#### **Option B: WiFi Router (Station Mode)**
```
1. Connect ke WiFi: PDKB_INTERNET_G
2. Password: uptpulogadung
3. IP ESP32: 192.168.1.x (cek di Serial Monitor)
```

### 4️⃣ **Test Koneksi Manual**

```powershell
# Test ping ke ESP32
ping 192.168.4.1

# Test HTTP endpoint
curl http://192.168.4.1/status

# Jika sukses, output:
# {"voltage":0.00,"current":0.00,"resistance":0.00,...}
```

### 5️⃣ **Restart Go Server**

```powershell
# Ctrl+C untuk stop server
# Jalankan ulang
go run main.go
```

---

## 🔍 Debug Steps

### **Cek Serial Monitor ESP32**

Buka Serial Monitor (115200 baud), lihat output:

**Normal**:
```
Starting JSY1050 Dashboard...
✅ Web server switch configured on pin 22
Waiting for switch press on GPIO 22 (active low)...
```

**Web Server Active**:
```
Switch pressed! Activating web server mode...
✅ Access Point started! AP IP: 192.168.4.1
✅ Connected to router! Station IP: 192.168.1.50
✅ Web server started
```

### **Cek Go Server Log**

**Bad** (timeout):
```
⚠️ AP mode failed: context deadline exceeded
```

**Good** (sukses):
```
✅ Trying AP mode: http://192.168.4.1/status
✅ AP mode success
```

---

## 🎯 Quick Fix (Step by Step)

```bash
# 1. Upload ESP32 code
# Arduino IDE → Upload esp32/main.cpp

# 2. Open Serial Monitor (115200 baud)
# Tools → Serial Monitor

# 3. Tekan GPIO 22 switch
# Lihat: "Web server started"

# 4. Cek IP address dari serial
# Catat: AP IP dan Station IP

# 5. Konek laptop ke WiFi ESP32
# WiFi: CJack_ESP32, Pass: 12345678

# 6. Test ping
ping 192.168.4.1

# 7. Test HTTP
curl http://192.168.4.1/status

# 8. Restart Go server
go run main.go

# 9. Open browser
# http://localhost:8000/pengujian
```

---

## 🔥 Common Issues

### **Issue 1: Serial Monitor tidak ada output**
```
❌ Problem: ESP32 tidak booting
✅ Fix: 
   - Cek koneksi USB
   - Tekan tombol EN/Reset di ESP32
   - Re-upload code
```

### **Issue 2: "Switch pressed" tapi tidak muncul IP**
```
❌ Problem: WiFi tidak start
✅ Fix:
   - Cek library TFT_eSPI installed
   - Cek library WiFi.h compatible
   - Uncomment Serial.println untuk debug
```

### **Issue 3: WiFi CJack_ESP32 tidak muncul**
```
❌ Problem: Access Point tidak jalan
✅ Fix:
   - Cek credentials: "CJack_ESP32" / "12345678"
   - Restart ESP32 (tombol EN)
   - Cek dualWiFiMode = true
```

### **Issue 4: Ping 192.168.4.1 timeout**
```
❌ Problem: Laptop tidak konek ke ESP32
✅ Fix:
   - Disconnect WiFi lain
   - Connect hanya ke CJack_ESP32
   - Disable firewall temporary
   - ipconfig /all → cek IP laptop (harus 192.168.4.x)
```

### **Issue 5: HTTP curl timeout tapi ping OK**
```
❌ Problem: Web server tidak listen
✅ Fix:
   - Cek serial: "Web server started"
   - GPIO 22 switch harus LOW
   - Restart ESP32
   - Check handleGetStatus() registered
```

---

## 📊 Expected Serial Output (Full)

```
Starting JSY1050 Dashboard...
Modbus JSY1050 initialized.
Web server switch configured on pin 22
Waiting for switch press on GPIO 22 (active low)...

[USER PRESS GPIO 22 SWITCH]

Switch pressed! Activating web server mode...
Web server mode: ON
Starting Access Point...
✅ Access Point started! AP IP: 192.168.4.1
🌐 Connecting to router WiFi: PDKB_INTERNET_G
..........
✅ Connected to router! Station IP: 192.168.1.50
📡 ESP32 now accessible from 2 networks:
   1. AP Mode: 192.168.4.1 (Direct WiFi: CJack_ESP32)
   2. Station Mode: 192.168.1.50 (Router: PDKB_INTERNET_G)
Web server started

[READY TO USE]
```

---

## 🚀 Next Steps After Fix

1. Buka browser: `http://localhost:8000/pengujian`
2. Pilih nama baju dan titik R1
3. Klik "Inject 200mA"
4. Tunggu tombol "Record" muncul
5. Klik "Record" → timer jalan
6. Selesai!

---

**Problem**: ESP32 tidak bisa diakses
**Root Cause**: Code belum upload / Switch belum ditekan / WiFi belum konek
**Solution**: Follow checklist 1-5 di atas
**Status**: ⏳ Waiting for ESP32 upload & activation
