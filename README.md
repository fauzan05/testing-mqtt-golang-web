# CORE - Conductive Suit Reliability Evaluator

Aplikasi web berbasis Go untuk monitoring dan kontrol perangkat ESP32 dengan MQTT dan WebSocket, dibangun menggunakan **Clean Architecture**.

[![Go Version](https://img.shields.io/badge/Go-1.24-00ADD8?style=flat&logo=go)](https://golang.org/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

## 📋 Fitur

- ✅ **MQTT Integration** - Real-time messaging dengan auto-reconnect
- ✅ **WebSocket** - Real-time data streaming ke web client
- ✅ **ESP32 Control** - Kontrol perangkat ESP32 via HTTP/MQTT
- ✅ **Authentication** - Session-based authentication
- ✅ **Clean Architecture** - Modular, testable, dan maintainable
- ✅ **Multi-Connection** - Support AP mode, Station mode, dan Serial USB

## 🚀 Quick Start

```bash
# 1. Clone repository
git clone https://github.com/fauzan05/testing-mqtt-golang-web.git
cd testing-mqtt-golang-web

# 2. Install dependencies
go mod tidy

# 3. Run aplikasi
go run cmd/server/main.go
```

Buka browser: **http://localhost:8000**
- Username: `admin`
- Password: `12345`

## 📁 Struktur Project (Clean Architecture)

```
testing-mqtt-golang-web/
├── cmd/server/              # Application entry point
├── internal/
│   ├── domain/             # Business entities & models
│   ├── repository/         # Data access layer
│   ├── usecase/            # Business logic
│   ├── delivery/           # HTTP & WebSocket handlers
│   └── middleware/         # Authentication middleware
├── pkg/
│   ├── config/             # Configuration management
│   └── mqtt/               # MQTT client wrapper
├── web/                    # Frontend assets (HTML, CSS, JS)
├── docs/                   # Documentation
├── esp32/                  # ESP32 firmware
└── README.md
```

> 📖 **Penjelasan detail arsitektur:** [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)

## 🛠️ Development

### Prerequisites
- Go 1.24 atau lebih baru
- MQTT Broker (Mosquitto/EMQX)

### Konfigurasi

Edit `.env` atau set environment variables:

```bash
PORT=8000
MQTT_BROKER=tcp://localhost:1884
MQTT_CLIENT_ID=go-mqtt-client
MQTT_TOPIC=golang-webserver/topic
```

### Build & Run

```bash
# Development mode
make run

# Build binary
make build

# Run tests
make test

# Lihat semua commands
make help
```

## 📡 API Endpoints

### Authentication
- `POST /api/login` - User login
- `GET /api/logout` - User logout
- `GET /api/check-auth` - Check authentication status
- `GET /api/user-info` - Get user information (protected)

### ESP32 Control
- `GET /esp32/status` - Get ESP32 device status
- `POST /esp32/inject` - Send injection command
- `POST /esp32/stop` - Stop device operation
- `POST /esp32/set_amplitude` - Set amplitude value

### WebSocket
- `GET /ws` - WebSocket connection for real-time updates

### System
- `GET /health` - Health check endpoint

> 📖 **API Documentation lengkap:** [docs/README_NEW.md](docs/README_NEW.md)

## 🧪 Testing MQTT

### Start MQTT Broker
```bash
# macOS
brew install mosquitto
brew services start mosquitto

# Docker
docker run -d -p 1883:1883 eclipse-mosquitto
```

### Publish Test Message
```bash
# Install mosquitto client
brew install mosquitto

# Publish message
mosquitto_pub -h localhost -p 1884 -t "golang-webserver/topic" -m "Hello MQTT!"

# Publish JSON
mosquitto_pub -h localhost -p 1884 -t "golang-webserver/topic" -m '{"sensor":"temp","value":25.5}'
```

## 📚 Dokumentasi

| Dokumen | Deskripsi |
|---------|-----------|
| [QUICKSTART.md](docs/QUICKSTART.md) | Panduan cepat memulai |
| [ARCHITECTURE.md](docs/ARCHITECTURE.md) | Detail arsitektur project |
| [MIGRATION_GUIDE.md](docs/MIGRATION_GUIDE.md) | Panduan migrasi dari versi lama |
| [REFACTORING_SUMMARY.md](docs/REFACTORING_SUMMARY.md) | Ringkasan refactoring |
| [MQTT_SETUP.md](docs/MQTT_SETUP.md) | Setup MQTT broker |
| [ESP32_UPLOAD_FIX.md](docs/ESP32_UPLOAD_FIX.md) | Troubleshooting ESP32 |
| [TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md) | Troubleshooting umum |

## 🏗️ Clean Architecture Layers

```
┌─────────────────────────────────────┐
│     Delivery Layer (HTTP/WS)        │  ← Presentation
├─────────────────────────────────────┤
│     Use Case Layer                  │  ← Business Logic
├─────────────────────────────────────┤
│     Repository Layer                │  ← Data Access
├─────────────────────────────────────┤
│     Domain Layer                    │  ← Entities
└─────────────────────────────────────┘
```

**Benefits:**
- ✅ Separation of Concerns
- ✅ Easy to Test
- ✅ Independent of Frameworks
- ✅ Flexible & Maintainable

## 🔐 Security

⚠️ **PENTING:** Ganti credentials default untuk production!

```go
// Default credentials (GANTI!)
Username: admin
Password: 12345
```

## 🐳 Docker Support

```bash
# Build image
make docker-build

# Run container
make docker-run
```

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the project
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 👥 Authors

- **Fauzan Nurhidayat** - [fauzan05](https://github.com/fauzan05)

## 🙏 Acknowledgments

- [Fiber](https://gofiber.io/) - Fast HTTP framework
- [Paho MQTT](https://github.com/eclipse/paho.mqtt.golang) - MQTT client library
- Clean Architecture by Robert C. Martin

---

⭐ **Star this repo** if you find it helpful!
