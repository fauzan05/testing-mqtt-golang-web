# 🔌 ESP32 LED Control with WiFi Manager

ESP32 dengan WiFi configuration untuk connect ke WiFi yang sama dengan laptop (tethering HP).

## 📦 File: esp32_test_led.ino

Features:
- ✅ WiFi Manager - Connect ke WiFi tethering HP
- ✅ Auto-save WiFi credentials
- ✅ Fallback ke AP mode jika gagal connect
- ✅ LED control (built-in GPIO 2 + external GPIO 32)
- ✅ Blink mode (kedip 500ms)
- ✅ Web interface untuk config WiFi
- ✅ CORS enabled untuk cross-origin

## 🌐 Mode WiFi

### Mode 1: Station Mode (Recommended)
- ESP32 connect ke WiFi tethering HP
- Laptop juga connect ke WiFi yang sama
- IP Address: Dynamic (dari router/HP)
- Akses: `http://[IP_ESP32]` (cek di Serial Monitor)

### Mode 2: AP Mode (Fallback)
- ESP32 buat Access Point sendiri
- SSID: **CJack_ESP32**
- Password: **12345678**
- IP: **192.168.4.1**

## 🚀 Cara Upload ke ESP32

### 1. Open Arduino IDE
- Buka file `esp32_test_led.ino`

### 2. Setup Arduino IDE
**Install ESP32 Board:**
- File → Preferences → Additional Board Manager URLs
- Tambahkan: `https://dl.espressif.com/dl/package_esp32_index.json`
- Tools → Board → Boards Manager → Search "ESP32" → Install

### 3. Select Board & Port
```
Tools → Board → ESP32 Dev Module
Tools → Port → (Pilih COM port ESP32)
```

### 4. Upload Code
- Klik tombol **Upload** (→)
- Tunggu sampai "Done uploading"
- Open **Serial Monitor** (Ctrl+Shift+M)
- Set baud rate: **115200**

### 5. Verify Upload Success
Di Serial Monitor akan muncul:

```
====================================
ESP32 LED Control Test
====================================

[GPIO] LED Built-in: GPIO 2
[GPIO] LED External: GPIO 32

[WiFi] Creating Access Point...
[WiFi] ✓ AP Created Successfully!
[WiFi] SSID: CJack_ESP32
[WiFi] Password: 12345678
[WiFi] IP Address: 192.168.4.1
[WiFi] Connect to this network and access:
[WiFi] http://192.168.4.1

[SERVER] HTTP Server started on port 80
[SERVER] Ready to receive commands!
====================================
```

LED akan **blink 3x** sebagai tanda ESP32 ready!

## 🧪 Testing

### Test 1: Test langsung dari ESP32
1. Connect laptop ke WiFi: **CJack_ESP32** (password: 12345678)
2. Buka browser: **http://192.168.4.1**
3. Klik tombol **"⚡ Start Inject"**
4. **LED ESP32 harus menyala!** 💡
5. Klik **"⏹️ Stop"** → LED mati

### Test 2: Config WiFi (Connect ke Tethering HP)
1. **Nyalakan tethering/hotspot di HP**
2. Connect laptop ke WiFi tethering HP
3. Access ESP32 config page:
   - Jika masih di AP mode: http://192.168.4.1/config
   - Atau dari web: http://localhost:8000/config
4. Klik **"🔍 Scan WiFi"**
5. Pilih WiFi tethering HP dari list
6. Masukkan password
7. Klik **"📡 Connect"**
8. Tunggu konfirmasi **"✓ Berhasil! IP ESP32: xxx.xxx.xxx.xxx"**
9. **Catat IP address ESP32!**

### Test 3: Test dari Halaman Pengujian (WiFi yang sama)
1. **Pastikan laptop dan ESP32 connect ke WiFi tethering HP yang sama**
2. Start Golang server:
   ```powershell
   cd C:\Users\PLN\Documents\GitHub\testing-mqtt-golang-web
   go run main.go
   ```
3. Buka browser: **http://localhost:8000/pengujian**
4. Login jika diminta
5. **Cek status koneksi ESP32** (di atas monitor):
   - Status: **Server Connected** (WebSocket ke Golang)
   - IP: **xxx.xxx.xxx.xxx (Station)** (ESP32 IP & mode)
   - Jika belum connect, klik **"⚙️ Config WiFi"**
6. Pilih baju dan titik pengukuran
7. Klik **"⚡ Inject 200mA"**
8. **LED ESP32 harus menyala selama 2 detik!** ⚡
9. Lihat notifikasi "✅ ESP32 LED Menyala!"
10. Setelah 2 detik, LED otomatis mati
11. Lihat notifikasi "⏹️ ESP32 LED Mati"

## 📊 Monitor Serial

Saat inject diklik dari web, Serial Monitor akan tampilkan:

```
[INJECT] Command received!
========================================
[LED] ⚡ MENYALA - Inject Started!
[LED] Duration: 2 seconds
========================================

[AUTO-STOP] Injection duration completed
========================================
[LED] ⏹️ MATI - Inject Stopped!
[LED] Duration: 2000 ms
========================================
```

## 🔧 Hardware

### LED yang Menyala:
1. **LED Built-in (GPIO 2)** - LED biru kecil di board ESP32
2. **LED External (GPIO 32)** - Optional, bisa ditambahkan:
   ```
   ESP32 GPIO 32 → LED Anode (+) → 220Ω Resistor → GND
   ```

## 🐛 Troubleshooting

### Problem: ESP32 tidak muncul di WiFi list
**Solution:**
- Reset ESP32 (tekan tombol EN)
- Re-upload code
- Check Serial Monitor untuk error

### Problem: LED tidak menyala
**Solution:**
- Check LED built-in di board (LED biru kecil)
- LED built-in kadang redup, check di ruangan gelap
- Verify di Serial Monitor: apakah "[LED] MENYALA" muncul?
- Test dengan LED eksternal di GPIO 32

### Problem: Web tidak bisa connect ke ESP32
**Solution:**
- **Cek WiFi sama atau tidak:**
  - Laptop connect ke WiFi: `ipconfig` (lihat WiFi adapter)
  - ESP32 IP: Cek di Serial Monitor atau halaman config
- **Jika ESP32 di AP mode:**
  - Laptop HARUS connect ke "CJack_ESP32"
  - Test ping: `ping 192.168.4.1`
- **Jika ESP32 di Station mode:**
  - Laptop dan ESP32 HARUS di WiFi yang sama
  - Test ping: `ping [IP_ESP32]`
  - Lihat IP ESP32 di: Serial Monitor atau http://localhost:8000/config
- Check firewall Windows tidak block
- Test direct access: http://[IP_ESP32]

### Problem: "CORS Error" di browser
**Solution:**
- Code sudah include CORS headers
- Try different browser (Chrome, Firefox, Edge)
- Clear browser cache (Ctrl+Shift+Delete)

## ✅ Expected Results

### Di ESP32:
- ✅ Connect ke WiFi tethering HP (atau fallback ke AP)
- ✅ IP address assigned (dynamic atau 192.168.4.1)
- ✅ LED menyala saat inject command diterima
- ✅ LED kedip saat mode blink
- ✅ LED mati setelah 2 detik atau saat stop
- ✅ Serial Monitor menampilkan log detail
- ✅ Auto-reconnect setiap restart

### Di Web (pengujian.html):
- ✅ Indikator "Server Connected" (WebSocket)
- ✅ Indikator IP ESP32 dan mode (Station/AP)
- ✅ Notifikasi "ESP32 LED Menyala" muncul
- ✅ Timer countdown 2 detik
- ✅ Data sensor (simulated) update setiap 500ms
- ✅ Auto-stop setelah 2 detik
- ✅ Notifikasi "ESP32 LED Mati"

### Di Config Page:
- ✅ Status koneksi ESP32 (Connected/AP/Disconnected)
- ✅ WiFi scan menampilkan networks tersedia
- ✅ Connect ke WiFi berhasil dengan IP baru
- ✅ Config tersimpan dan auto-reconnect

## 📝 Notes

- **Durasi inject**: 2 detik (untuk testing cepat)
- **Auto-stop**: LED otomatis mati setelah 2 detik
- **LED Built-in**: LED biru kecil di ESP32 board
- **Optional**: Bisa tambah LED eksternal di GPIO 32 untuk lebih jelas
- **WiFi Credentials**: Tersimpan di ESP32 EEPROM, auto-reconnect setiap restart
- **IP Dynamic**: IP ESP32 akan berubah setiap connect ke WiFi baru

## 🔄 WiFi Configuration Flow

```
┌─────────────────────────────────────────────┐
│ 1. ESP32 Startup                            │
│    - Cek saved WiFi di EEPROM               │
│    - Ada? → Try connect                     │
│    - Tidak ada? → AP mode                   │
└─────────────────────────────────────────────┘
                    ↓
┌─────────────────────────────────────────────┐
│ 2. Connect ke WiFi                          │
│    - Berhasil? → Station mode (use WiFi IP) │
│    - Gagal? → Fallback ke AP mode           │
└─────────────────────────────────────────────┘
                    ↓
┌─────────────────────────────────────────────┐
│ 3. Web Access                               │
│    - Station: http://[IP dari router]       │
│    - AP mode: http://192.168.4.1            │
└─────────────────────────────────────────────┘
```

## 🌐 Network Scenarios

### Scenario 1: Tethering HP (Recommended)
```
HP (Hotspot) ←→ WiFi ←→ Laptop
                  ↓
                ESP32

- HP: 192.168.43.1 (gateway)
- Laptop: 192.168.43.x
- ESP32: 192.168.43.y
- Semua di network yang sama!
```

### Scenario 2: AP Mode (Fallback)
```
ESP32 (AP) ←→ Laptop only

- ESP32: 192.168.4.1
- Laptop connect langsung ke ESP32
- Tidak bisa akses internet
```

## 🎯 Next Steps

Setelah test berhasil:
1. ✅ Konfirmasi LED menyala dari web
2. ✅ Konfirmasi auto-stop berfungsi
3. Lanjut ke integrasi sensor voltage/current (HLK2)
4. Lanjut ke integrasi Arduino Nano untuk SPWM
5. Full integration test

---

**Quick Test Command:**
```powershell
# Test dengan curl
curl -X POST http://192.168.4.1/inject

# Check status
curl http://192.168.4.1/status
```

🎉 **Happy Testing!**
