# 📡 MQTT Terminal Commands Cheatsheet

Panduan lengkap untuk testing MQTT subscriber menggunakan command line tools.

## 🔧 Install MQTT Client Tools

### macOS

```bash
# Install Mosquitto (mosquitto_pub & mosquitto_sub)
brew install mosquitto

# Install MQTTX CLI (alternative modern)
brew install emqx/mqttx/mqttx-cli
```

### Linux (Ubuntu/Debian)

```bash
sudo apt-get update
sudo apt-get install mosquitto-clients
```

## 📤 Mosquitto Commands

### Basic Publish

```bash
# Publish simple message
mosquitto_pub -h broker.emqx.io -t "test/topic" -m "Hello MQTT!"

# Publish dengan broker lokal
mosquitto_pub -h localhost -t "test/topic" -m "Local message"

# Publish dengan QoS level 1
mosquitto_pub -h broker.emqx.io -t "test/topic" -q 1 -m "Important message"

# Publish dengan retain flag
mosquitto_pub -h broker.emqx.io -t "test/topic" -r -m "Retained message"
```

### Publish JSON Data

```bash
# Simple JSON
mosquitto_pub -h broker.emqx.io -t "test/topic" -m '{"temperature": 25.5, "humidity": 60}'

# Sensor data
mosquitto_pub -h broker.emqx.io -t "sensor/data" -m '{"sensor_id": "temp01", "value": 23.4, "unit": "celsius", "timestamp": "2025-11-08T10:30:00Z"}'

# Device status
mosquitto_pub -h broker.emqx.io -t "device/status" -m '{"device_id": "device001", "online": true, "battery": 85, "signal": "strong"}'

# Multiple sensors
mosquitto_pub -h broker.emqx.io -t "sensors/living-room" -m '{"temperature": 22.5, "humidity": 55, "co2": 450, "timestamp": "2025-11-08T10:35:00Z"}'
```

### Publish Multiple Messages (Loop)

```bash
# Publish 10 messages dengan interval 1 detik
for i in {1..10}; do
  mosquitto_pub -h broker.emqx.io -t "test/topic" -m "Message #$i"
  sleep 1
done

# Publish dengan timestamp
for i in {1..5}; do
  timestamp=$(date +"%Y-%m-%d %H:%M:%S")
  mosquitto_pub -h broker.emqx.io -t "test/topic" -m "Message at $timestamp"
  sleep 2
done

# Publish random temperature data
for i in {1..20}; do
  temp=$((RANDOM % 15 + 20))  # Random temp between 20-35
  mosquitto_pub -h broker.emqx.io -t "sensor/temperature" -m "{\"temp\": $temp, \"unit\": \"celsius\"}"
  sleep 1
done
```

### Publish dari File

```bash
# Publish isi file
mosquitto_pub -h broker.emqx.io -t "test/topic" -f message.txt

# Publish multiple lines dari file
while IFS= read -r line; do
  mosquitto_pub -h broker.emqx.io -t "test/topic" -m "$line"
  sleep 0.5
done < data.txt
```

## 🎯 MQTTX CLI Commands

### Basic Usage

```bash
# Publish message
mqttx pub -h broker.emqx.io -t "test/topic" -m "Hello from MQTTX"

# Publish JSON
mqttx pub -h broker.emqx.io -t "test/topic" -m '{"message": "Hello MQTT"}'

# Publish dengan QoS
mqttx pub -h broker.emqx.io -t "test/topic" -q 1 -m "Important"

# Multiple messages
mqttx pub -h broker.emqx.io -t "test/topic" -m "msg1" -m "msg2" -m "msg3"
```

### Subscribe untuk Testing

```bash
# Subscribe ke topic (untuk melihat messages)
mqttx sub -h broker.emqx.io -t "test/topic"

# Subscribe multiple topics
mqttx sub -h broker.emqx.io -t "sensor/#" -t "device/#"

# Subscribe dengan verbose
mqttx sub -h broker.emqx.io -t "test/topic" -v
```

## 🌐 Testing dengan Different Brokers

### Public Broker EMQX (Default)

```bash
mosquitto_pub -h broker.emqx.io -p 1883 -t "test/topic" -m "EMQX Test"
```

### HiveMQ Public Broker

```bash
mosquitto_pub -h broker.hivemq.com -p 1883 -t "test/topic" -m "HiveMQ Test"
```

### Eclipse Public Broker

```bash
mosquitto_pub -h mqtt.eclipseprojects.io -p 1883 -t "test/topic" -m "Eclipse Test"
```

### Local Mosquitto Broker

```bash
# Start local broker (di terminal terpisah)
mosquitto -v

# Publish ke local broker
mosquitto_pub -h localhost -p 1883 -t "test/topic" -m "Local Test"
```

## 🔐 Authentication (Jika broker memerlukan)

```bash
# Dengan username & password
mosquitto_pub -h broker.example.com -p 1883 -u username -P password -t "test/topic" -m "Authenticated"

# Dengan TLS
mosquitto_pub -h broker.example.com -p 8883 --cafile ca.crt -t "test/topic" -m "Secure"

# Dengan client certificate
mosquitto_pub -h broker.example.com -p 8883 --cert client.crt --key client.key -t "test/topic" -m "Cert auth"
```

## 📊 Testing Scenarios

### 1. Sensor Data Simulation

```bash
# Temperature sensor
while true; do
  temp=$(awk -v min=20 -v max=30 'BEGIN{srand(); print min+rand()*(max-min)}')
  mosquitto_pub -h broker.emqx.io -t "sensor/temperature" -m "{\"temp\": $temp, \"unit\": \"celsius\"}"
  sleep 2
done
```

### 2. IoT Device Status

```bash
# Device heartbeat
while true; do
  timestamp=$(date -u +"%Y-%m-%dT%H:%M:%SZ")
  mosquitto_pub -h broker.emqx.io -t "device/heartbeat" -m "{\"device_id\": \"dev001\", \"status\": \"online\", \"timestamp\": \"$timestamp\"}"
  sleep 10
done
```

### 3. Multiple Sensors

```bash
# Multi-sensor data
while true; do
  temp=$(awk 'BEGIN{srand(); print 20+rand()*10}')
  humidity=$(awk 'BEGIN{srand(); print 40+rand()*30}')
  mosquitto_pub -h broker.emqx.io -t "sensors/room1" -m "{\"temp\": $temp, \"humidity\": $humidity}"
  sleep 3
done
```

### 4. Event Notifications

```bash
# Simulate events
events=("door_open" "motion_detected" "alarm_triggered" "system_ok")

while true; do
  event=${events[$RANDOM % ${#events[@]}]}
  timestamp=$(date +"%H:%M:%S")
  mosquitto_pub -h broker.emqx.io -t "events/security" -m "{\"event\": \"$event\", \"time\": \"$timestamp\"}"
  sleep 5
done
```

## 🛠️ Troubleshooting Commands

```bash
# Test connection to broker
mosquitto_pub -h broker.emqx.io -t "test" -m "ping" -d

# Verbose output
mosquitto_pub -h broker.emqx.io -t "test/topic" -m "test" -d -v

# Check if message was received (subscribe in same command)
mosquitto_pub -h broker.emqx.io -t "test/topic" -m "test" & mosquitto_sub -h broker.emqx.io -t "test/topic" -C 1
```

## 📝 Quick Reference

| Command | Description |
|---------|-------------|
| `-h` | MQTT broker host |
| `-p` | Port (default: 1883) |
| `-t` | Topic |
| `-m` | Message |
| `-q` | QoS level (0, 1, 2) |
| `-r` | Retain message |
| `-d` | Debug mode |
| `-v` | Verbose |
| `-u` | Username |
| `-P` | Password |
| `-f` | Read message from file |
| `-C` | Count (untuk subscribe) |

## 🎯 Tips

1. **Multiple Terminals**: Buka beberapa terminal untuk simulate multiple devices
2. **JSON Format**: Selalu validate JSON sebelum publish
3. **Topic Naming**: Gunakan struktur yang jelas, misal: `sensor/location/type`
4. **Testing**: Subscribe dulu untuk memastikan messages terkirim
5. **Broker Selection**: Gunakan public broker untuk testing, private untuk production

## 📚 Useful Links

- [Mosquitto Documentation](https://mosquitto.org/man/mosquitto_pub-1.html)
- [MQTTX CLI](https://mqttx.app/cli)
- [MQTT Protocol](https://mqtt.org/)
- [Public MQTT Brokers](https://github.com/mqtt/mqtt.org/wiki/public_brokers)
