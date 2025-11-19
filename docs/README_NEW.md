# CORE - Conductive Suit Reliability Evaluator

Aplikasi web berbasis Go untuk monitoring dan kontrol perangkat ESP32 dengan MQTT dan WebSocket.

## 📁 Struktur Project (Clean Architecture)

```
testing-mqtt-golang-web/
├── cmd/
│   └── server/
│       └── main.go                 # Entry point aplikasi
├── internal/
│   ├── domain/                     # Domain models (entities)
│   │   ├── user.go
│   │   ├── message.go
│   │   └── esp32.go
│   ├── repository/                 # Data access layer
│   │   ├── auth_repository.go
│   │   └── session_repository.go
│   ├── usecase/                    # Business logic layer
│   │   ├── auth_usecase.go
│   │   ├── websocket_usecase.go
│   │   └── esp32_usecase.go
│   ├── delivery/                   # Presentation layer
│   │   ├── http/
│   │   │   ├── auth_handler.go
│   │   │   ├── esp32_handler.go
│   │   │   └── router.go
│   │   └── websocket/
│   │       └── handler.go
│   └── middleware/                 # HTTP middlewares
│       └── auth.go
├── pkg/                            # Shared packages
│   ├── config/
│   │   └── config.go              # Configuration management
│   └── mqtt/
│       └── client.go              # MQTT client wrapper
├── web/                            # Frontend assets
│   ├── templates/                  # HTML templates
│   └── static/
│       ├── css/                    # CSS files
│       ├── js/                     # JavaScript files
│       └── images/                 # Images
├── docs/                           # Documentation
├── esp32/                          # ESP32 firmware
│   └── main.cpp
├── go.mod
├── go.sum
└── README.md
```

## 🏗️ Penjelasan Clean Architecture

### 1. **Domain Layer** (`internal/domain/`)
- Berisi entity dan business models
- Tidak bergantung pada layer lain
- Merupakan inti dari business logic

### 2. **Repository Layer** (`internal/repository/`)
- Menangani data access dan storage
- Abstraksi untuk data persistence
- Saat ini menggunakan in-memory storage (bisa diganti dengan database)

### 3. **Use Case Layer** (`internal/usecase/`)
- Berisi business logic aplikasi
- Menggunakan repository untuk data access
- Independen dari delivery mechanism

### 4. **Delivery Layer** (`internal/delivery/`)
- HTTP handlers untuk REST API
- WebSocket handlers untuk real-time communication
- Routing dan request/response handling

### 5. **Middleware** (`internal/middleware/`)
- Authentication middleware
- Request validation
- Logging (bisa ditambahkan)

### 6. **Package** (`pkg/`)
- Shared utilities yang bisa digunakan oleh project lain
- Config management
- MQTT client wrapper

## 🚀 Cara Menjalankan

### Prasyarat
- Go 1.24 atau lebih baru
- MQTT Broker (contoh: Mosquitto)

### Instalasi

1. Clone repository:
```bash
git clone <repository-url>
cd testing-mqtt-golang-web
```

2. Install dependencies:
```bash
go mod tidy
```

3. Jalankan aplikasi:
```bash
go run cmd/server/main.go
```

Atau build terlebih dahulu:
```bash
go build -o bin/server cmd/server/main.go
./bin/server
```

### Konfigurasi

Aplikasi dapat dikonfigurasi menggunakan environment variables:

| Variable | Default | Deskripsi |
|----------|---------|-----------|
| `PORT` | `8000` | Port server web |
| `MQTT_BROKER` | `tcp://localhost:1884` | URL MQTT broker |
| `MQTT_CLIENT_ID` | `go-mqtt-client` | Client ID MQTT |
| `MQTT_TOPIC` | `golang-webserver/topic` | Topic MQTT |

Contoh:
```bash
PORT=3000 MQTT_BROKER="tcp://broker.emqx.io:1883" go run cmd/server/main.go
```

## 📡 API Endpoints

### Authentication
- `POST /api/login` - Login user
- `GET /api/logout` - Logout user
- `GET /api/check-auth` - Check authentication status
- `GET /api/user-info` - Get user information (protected)
- `POST /api/change-username` - Change username (protected)
- `POST /api/change-password` - Change password (protected)

### ESP32 Control
- `GET /esp32/status` - Get ESP32 status
- `POST /esp32/inject` - Send injection command
- `POST /esp32/stop` - Stop device
- `POST /esp32/set_amplitude` - Set amplitude value

### WebSocket
- `GET /ws` - WebSocket connection for real-time updates

### Health Check
- `GET /health` - Health check endpoint

## 🧪 Testing MQTT

### Menggunakan mosquitto_pub
```bash
# Publish message
mosquitto_pub -h localhost -p 1884 -t "golang-webserver/topic" -m "Hello MQTT"

# Publish JSON
mosquitto_pub -h localhost -p 1884 -t "golang-webserver/topic" -m '{"sensor":"temp","value":25.5}'
```

### Menggunakan MQTTX CLI
```bash
# Publish message
mqttx pub -h localhost -p 1884 -t "golang-webserver/topic" -m "Hello MQTT"
```

## 🔐 Default Credentials

- Username: `admin`
- Password: `12345`

**⚠️ PENTING:** Ganti credentials default untuk production!

## 📚 Dokumentasi Tambahan

Dokumentasi lengkap tersedia di folder `docs/`:
- [MQTT Setup Guide](docs/MQTT_SETUP.md)
- [ESP32 Setup Guide](docs/ESP32_UPLOAD_FIX.md)
- [Multi-Connection Guide](docs/MULTI_CONNECTION_GUIDE.md)
- [Troubleshooting](docs/TROUBLESHOOTING.md)

## 🛠️ Development

### Menambahkan Fitur Baru

1. **Tambah Domain Model**: Buat file baru di `internal/domain/`
2. **Tambah Repository**: Jika perlu data storage, buat di `internal/repository/`
3. **Tambah Use Case**: Business logic di `internal/usecase/`
4. **Tambah Handler**: HTTP/WebSocket handler di `internal/delivery/`
5. **Update Router**: Tambahkan route di `internal/delivery/http/router.go`

### Best Practices

- Setiap layer tidak boleh bergantung pada layer di atasnya
- Domain layer harus bebas dari dependency eksternal
- Use case layer tidak boleh tahu tentang HTTP/WebSocket
- Handler harus tipis, hanya handle request/response

## 📝 License

MIT License - Bebas digunakan untuk project pribadi maupun komersial.

## 👥 Contributing

Pull requests are welcome! Untuk perubahan besar, silakan buat issue terlebih dahulu.

## 🎯 Roadmap

- [ ] Implementasi database (PostgreSQL/MySQL)
- [ ] Unit testing untuk semua layer
- [ ] Docker containerization
- [ ] CI/CD pipeline
- [ ] API documentation dengan Swagger
- [ ] Logging yang lebih baik
- [ ] Metrics dan monitoring
