# 🚀 Quick Start Guide

Panduan cepat untuk menjalankan project CORE setelah refactoring ke Clean Architecture.

## ⚡ Super Quick Start (3 Steps)

```bash
# 1. Pindahkan file (jika belum)
chmod +x migrate.sh && ./migrate.sh

# 2. Install dependencies
go mod tidy

# 3. Run!
go run cmd/server/main.go
```

Buka browser: http://localhost:8000
- Username: `admin`
- Password: `12345`

## 📋 Lengkap Step-by-Step

### Step 1: Verifikasi Struktur

Pastikan struktur folder sudah terbuat:
```bash
ls -la cmd/server/
ls -la internal/
ls -la pkg/
```

Jika sudah ada ✅, lanjut ke step 2.

### Step 2: Pindahkan File Lama (Manual)

Karena ada issue dengan terminal, pindahkan manual:

```bash
# Pindahkan HTML
mkdir -p web/templates
mv *.html web/templates/ 2>/dev/null || true

# Pindahkan CSS
mkdir -p web/static/css
mv *.css web/static/css/ 2>/dev/null || true

# Pindahkan JS
mkdir -p web/static/js
mv *.js web/static/js/ 2>/dev/null || true

# Pindahkan Images
mkdir -p web/static/images
mv *.png *.jpg web/static/images/ 2>/dev/null || true

# Pindahkan Docs
mkdir -p docs
mv ESP32_*.md MQTT_*.md MULTI_*.md SERIAL_*.md SETUP_*.md TEST_*.md TROUBLESHOOTING.md docs/ 2>/dev/null || true

# Backup main.go lama
mv main.go main.go.backup 2>/dev/null || true
```

### Step 3: Install Dependencies

```bash
go mod download
go mod tidy
```

### Step 4: Konfigurasi (Opsional)

Copy dan edit environment:
```bash
cp .env.example .env
nano .env  # atau editor favorit Anda
```

Edit sesuai kebutuhan:
```bash
PORT=8000
MQTT_BROKER=tcp://localhost:1884
MQTT_CLIENT_ID=go-mqtt-client
MQTT_TOPIC=golang-webserver/topic
```

### Step 5: Run Aplikasi

**Option A: Development Mode**
```bash
go run cmd/server/main.go
```

**Option B: Build & Run**
```bash
go build -o bin/core-server cmd/server/main.go
./bin/core-server
```

**Option C: Menggunakan Makefile**
```bash
make run
```

### Step 6: Test Aplikasi

1. Buka browser: http://localhost:8000
2. Login dengan:
   - Username: `admin`
   - Password: `12345`
3. Test fitur-fitur:
   - Dashboard
   - Pengujian
   - Config
   - dll.

## 🧪 Testing MQTT

### Start MQTT Broker (Jika Belum Ada)

```bash
# macOS
brew install mosquitto
brew services start mosquitto

# Linux
sudo apt-get install mosquitto
sudo systemctl start mosquitto

# Docker
docker run -d -p 1883:1883 -p 9001:9001 eclipse-mosquitto
```

### Publish Test Message

```bash
# Install mosquitto clients
brew install mosquitto  # macOS

# Publish message
mosquitto_pub -h localhost -p 1884 -t "golang-webserver/topic" -m "Hello MQTT!"

# Publish JSON
mosquitto_pub -h localhost -p 1884 -t "golang-webserver/topic" -m '{"sensor":"temperature","value":25.5}'
```

Lihat message di web dashboard secara real-time!

## 🐛 Troubleshooting

### Error: "cannot find module"

**Problem**: Import path tidak sesuai

**Solution**:
```bash
go mod tidy
```

### Error: "file not found" saat akses web

**Problem**: File HTML/CSS belum dipindahkan atau path salah

**Solution 1**: Pindahkan file manual (lihat Step 2)

**Solution 2**: Sementara, router sudah dikonfigurasi untuk serve dari root directory untuk backward compatibility.

### Error: "cannot connect to MQTT"

**Problem**: MQTT broker belum jalan

**Solution**:
```bash
# Start mosquitto
brew services start mosquitto

# Atau gunakan public broker untuk testing
# Edit .env:
MQTT_BROKER=tcp://broker.emqx.io:1883
```

### Error: "port already in use"

**Problem**: Port 8000 sudah digunakan

**Solution**:
```bash
# Ganti port
PORT=3000 go run cmd/server/main.go

# Atau kill process yang menggunakan port 8000
lsof -ti:8000 | xargs kill -9
```

### File tidak bisa dipindahkan

**Problem**: Permission atau file sudah dipindah

**Solution**:
```bash
# Cek file yang masih di root
ls -la *.html *.css *.js *.png 2>/dev/null

# Jika kosong, berarti sudah dipindah ✅
```

## 📊 Verifikasi Instalasi

```bash
# Check struktur folder
tree -L 3 -I 'node_modules|.git'

# Check dependencies
go list -m all

# Check build
go build -v cmd/server/main.go

# Check tests (jika ada)
go test ./...
```

## 🎯 Next Steps

1. ✅ Aplikasi jalan di http://localhost:8000
2. ✅ Test semua fitur
3. 📝 Baca dokumentasi lengkap di README.md
4. 📚 Pahami struktur di ARCHITECTURE.md
5. 🔧 Customize sesuai kebutuhan

## 📱 Endpoints Reference

### Public Endpoints
- `GET /` - Redirect ke dashboard (jika login) atau login page
- `GET /login` - Login page
- `POST /api/login` - Login API
- `GET /api/check-auth` - Check authentication

### Protected Endpoints (Perlu Login)
- `GET /dashboard` - Dashboard page
- `GET /pengujian` - Pengujian page
- `GET /config` - Config page
- `GET /daftar` - Daftar page
- `GET /histori` - Histori page
- `GET /foto` - Foto page
- `GET /user` - User management page

### API Endpoints
- `GET /api/user-info` - Get user info
- `POST /api/change-username` - Change username
- `POST /api/change-password` - Change password
- `GET /api/logout` - Logout

### ESP32 Endpoints
- `GET /esp32/status` - Get ESP32 status
- `POST /esp32/inject` - Send injection command
- `POST /esp32/stop` - Stop device
- `POST /esp32/set_amplitude` - Set amplitude

### WebSocket
- `GET /ws` - WebSocket connection

### Health Check
- `GET /health` - Health check & status

## 💡 Pro Tips

### Development dengan Hot Reload

```bash
# Install air
go install github.com/cosmtrek/air@latest

# Run dengan hot reload
air
# atau
make dev
```

### Build untuk Production

```bash
# Build binary
make build

# Build untuk Linux
make build-linux

# Build untuk Windows
make build-windows

# Build Docker image
make docker-build
```

### Debugging

```bash
# Run dengan verbose logging
go run -v cmd/server/main.go

# Build dengan debug info
go build -gcflags="all=-N -l" -o bin/debug cmd/server/main.go
```

## 📞 Need Help?

- 📖 Baca `README.md` untuk dokumentasi lengkap
- 🏗️ Baca `ARCHITECTURE.md` untuk detail arsitektur
- 🔄 Baca `MIGRATION_GUIDE.md` untuk panduan migrasi
- 📝 Baca `REFACTORING_SUMMARY.md` untuk ringkasan perubahan

## ✨ You're All Set!

Selamat! Aplikasi Anda sekarang menggunakan Clean Architecture dan siap untuk:
- ✅ Development yang lebih mudah
- ✅ Testing yang lebih baik
- ✅ Maintenance yang lebih simpel
- ✅ Scaling yang lebih fleksibel

**Happy Coding! 🚀**
