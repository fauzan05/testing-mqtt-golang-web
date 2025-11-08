# MQTT Real-time Web Monitor

Aplikasi Go yang menerima data dari MQTT broker dan menampilkannya secara real-time di web menggunakan WebSocket.

## 📋 Fitur

- ✅ MQTT Subscriber yang auto-reconnect
- ✅ Real-time data streaming via WebSocket
- ✅ Tampilan web yang modern dan responsif
- ✅ Statistik message counter
- ✅ Support multiple WebSocket clients
- ✅ Auto-scroll untuk message terbaru

## 🚀 Cara Menggunakan

### 1. Install Dependencies

```bash
go mod tidy
```

### 2. Jalankan Aplikasi Subscriber/Web Server

```bash
go run main.go
```

Atau dengan konfigurasi custom:

```bash
MQTT_BROKER="tcp://localhost:1883" MQTT_TOPIC="sensor/data" PORT="3000" go run main.go
```

### 3. Buka Browser

Akses: http://localhost:3000

### 4. Testing dengan MQTT Client di Terminal

#### Menggunakan mosquitto_pub (Install dulu jika belum ada)

```bash
# Install mosquitto di macOS
brew install mosquitto

# Publish message ke broker
mosquitto_pub -h broker.emqx.io -t "test/topic" -m "Hello from terminal!"

# Publish JSON data
mosquitto_pub -h broker.emqx.io -t "test/topic" -m '{"temperature": 25.5, "humidity": 60}'

# Publish dengan QoS
mosquitto_pub -h broker.emqx.io -t "test/topic" -q 1 -m "Important message"
```

#### Menggunakan MQTTX CLI

```bash
# Install MQTTX CLI
brew install emqx/mqttx/mqttx-cli

# Publish message
mqttx pub -h broker.emqx.io -t "test/topic" -m "Hello from MQTTX!"

# Publish JSON
mqttx pub -h broker.emqx.io -t "test/topic" -m '{"sensor": "temp", "value": 23.4}'
```

## ⚙️ Konfigurasi Environment Variables

| Variable | Default | Deskripsi |
|----------|---------|-----------|
| `MQTT_BROKER` | `tcp://broker.emqx.io:1883` | URL MQTT broker |
| `MQTT_TOPIC` | `test/topic` | Topic untuk subscribe/publish |
| `MQTT_CLIENT_ID` | `go-mqtt-client` | Client ID untuk subscriber |
| `PORT` | `3000` | Port untuk web server |

## 📡 MQTT Broker

### Menggunakan Public Broker (Default)

Aplikasi menggunakan broker public EMQX secara default: `broker.emqx.io:1883`

### Menggunakan Local Broker

1. Install Mosquitto MQTT broker:

```bash
# macOS
brew install mosquitto

# Jalankan broker
mosquitto -v
```

2. Jalankan aplikasi dengan local broker:

```bash
MQTT_BROKER="tcp://localhost:1883" go run main.go
```

### Menggunakan MQTT Broker Online Lainnya

- HiveMQ: `tcp://broker.hivemq.com:1883`
- Eclipse: `tcp://mqtt.eclipseprojects.io:1883`
- test.mosquitto.org: `tcp://test.mosquitto.org:1883`

## 🧪 Testing

### Testing dengan MQTT Client Tool

Gunakan MQTT client tool seperti:

- **MQTT Explorer** (GUI) - Download dari: http://mqtt-explorer.com/
- **mosquitto_pub** (CLI):
  ```bash
  mosquitto_pub -h broker.emqx.io -t "test/topic" -m "Hello from mosquitto"
  ```

### Testing dengan MQTTX (Recommended)

1. Download MQTTX: https://mqttx.app/
2. Buat connection baru ke broker
3. Subscribe ke topic `test/topic`
4. Publish message ke topic `test/topic`
5. Lihat hasilnya di web browser

## 📁 Struktur File

```
.
├── main.go         # Aplikasi utama (subscriber + web server)
├── index.html      # Frontend untuk menampilkan data
├── go.mod          # Go modules
└── README.md       # Dokumentasi
```

## 🔍 API Endpoints

- `GET /` - Halaman utama (HTML)
- `GET /ws` - WebSocket endpoint
- `GET /health` - Health check & status
- `GET /info` - Informasi konfigurasi

## 💡 Tips

1. **Multiple Tabs**: Buka beberapa tab browser untuk melihat broadcast ke multiple clients
2. **Custom Topics**: Gunakan topic sesuai kebutuhan, misalnya `sensor/temperature`, `device/status`, dll
3. **Monitoring**: Gunakan endpoint `/health` untuk monitoring status koneksi

## 🐛 Troubleshooting

### Tidak bisa connect ke MQTT broker

- Pastikan broker URL benar
- Check firewall/network
- Coba gunakan public broker untuk testing

### WebSocket tidak connect

- Pastikan server sudah running
- Check console browser untuk error
- Refresh halaman browser

## 📝 Contoh Penggunaan Real-world

### Sensor IoT

```bash
# Jalankan subscriber untuk data sensor
MQTT_TOPIC="sensor/temperature" go run main.go

# Publish dari terminal
mosquitto_pub -h broker.emqx.io -t "sensor/temperature" -m '{"temp": 25.5, "unit": "celsius"}'
```

### Chat/Messaging

```bash
# Subscribe ke chat topic
MQTT_TOPIC="chat/room1" go run main.go

# Publish chat message dari terminal
mosquitto_pub -h broker.emqx.io -t "chat/room1" -m "Hello everyone!"
```

### Device Status Monitoring

```bash
# Monitor status devices
MQTT_TOPIC="devices/+/status" go run main.go

# Publish device status
mosquitto_pub -h broker.emqx.io -t "devices/device1/status" -m '{"online": true, "battery": 85}'
```

## 📚 Library yang Digunakan

- [Fiber](https://gofiber.io/) - Web framework
- [Paho MQTT](https://github.com/eclipse/paho.mqtt.golang) - MQTT client
- [Fiber WebSocket](https://github.com/gofiber/websocket) - WebSocket support

## 📄 License

MIT License - Bebas digunakan untuk project pribadi maupun komersial.
