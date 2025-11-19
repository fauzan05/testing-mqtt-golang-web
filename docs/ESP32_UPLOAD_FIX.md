# 🔧 Fix ESP32 Upload Error

## ❌ Error: "The chip stopped responding"

Error ini terjadi saat upload ESP32 di tengah jalan (10.8% progress).

## ✅ Solusi (coba satu per satu):

### 1️⃣ Hold BOOT Button (Paling Sering Berhasil)
```
1. Disconnect ESP32 dari USB
2. Reconnect ESP32
3. Di Arduino IDE, klik Upload
4. SEGERA setelah muncul "Connecting...."
5. TEKAN DAN TAHAN tombol BOOT di ESP32
6. Tahan sampai muncul "Writing at 0x00010000..."
7. Lepas tombol BOOT
8. Tunggu upload selesai
```

### 2️⃣ Lower Upload Speed
```
Arduino IDE:
Tools → Upload Speed → 115200
(Default biasanya 921600 - terlalu cepat untuk beberapa board)

Coba upload lagi.
```

### 3️⃣ Change USB Cable
```
- Pakai kabel USB yang bagus (bukan kabel charge only)
- Kabel pendek lebih bagus dari kabel panjang
- Coba port USB lain di laptop
```

### 4️⃣ Reduce Code Size (Temporary)
Code sekarang: **951KB (72%)** - masih OK tapi besar.

Bisa comment bagian HTML di `handleConfig()` untuk test upload:

```cpp
void handleConfig() {
  // Temporary: simple response
  String html = "<html><body><h1>Config Page</h1>";
  html += "<p>WiFi Config temporarily disabled for testing</p></body></html>";
  server.send(200, "text/html", html);
}
```

Setelah berhasil upload, uncomment lagi.

### 5️⃣ Enable Verbose Upload
```
Arduino IDE:
File → Preferences → Show verbose output during: [✓] upload

Upload lagi, lihat error detail.
```

### 6️⃣ Erase Flash First
```
Arduino IDE:
Tools → Erase Flash → "All Flash Contents"

Upload lagi.
```

### 7️⃣ Manual Flash Mode
```
1. Disconnect ESP32
2. Tekan dan tahan tombol BOOT
3. Sambil tahan BOOT, tekan tombol EN (reset)
4. Lepas EN, tapi TETAP tahan BOOT
5. Connect USB (masih tahan BOOT)
6. Lepas BOOT
7. ESP32 sekarang di flash mode
8. Upload dari Arduino IDE
```

## 🎯 Recommended Steps:

1. **Coba #1 (Hold BOOT)** ← Paling sering berhasil!
2. Jika gagal, coba **#2 (Lower speed to 115200)**
3. Jika masih gagal, coba **#3 (Ganti USB cable)**
4. Last resort: **#6 (Erase flash)** + **#1 (Hold BOOT)**

## 📊 Current Stats:
- Sketch size: **951KB (72%)** ✅ OK
- Global variables: **45KB (13%)** ✅ OK
- Chip: **ESP32-D0WD-V3** ✅ Good
- Upload stopped at: **10.8%** (while writing flash)

## 💡 Tips:
- **Jangan gerakkan** ESP32 saat upload
- **Tutup Serial Monitor** sebelum upload
- **Disconnect sensor/device lain** dari ESP32
- Pastikan **power supply cukup** (USB 2.0 minimal)

## ✅ Upload Berhasil Jika:
```
Writing at 0x000f8fff [==============================] 100.0%
Wrote 951376 bytes (604983 compressed)
Hash of data verified.

Leaving...
Hard resetting via RTS pin...
```

Coba solusi #1 dulu (hold BOOT button)! 🚀
