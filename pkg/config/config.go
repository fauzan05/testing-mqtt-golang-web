package config

import "os"

// Config holds all application configuration
type Config struct {
	Server ServerConfig
	MQTT   MQTTConfig
	Auth   AuthConfig
	ESP32  ESP32Config
}

// ServerConfig holds server configuration
type ServerConfig struct {
	Port    string
	AppName string
}

// MQTTConfig holds MQTT broker configuration
type MQTTConfig struct {
	Broker   string
	ClientID string
	Topic    string
}

// AuthConfig holds authentication defaults
type AuthConfig struct {
	DefaultUsername string
	DefaultPassword string
}

// ESP32Config holds ESP32 connection configuration
type ESP32Config struct {
	APIP      string
	StationIP string
}

// LoadConfig loads configuration from environment variables
func LoadConfig() *Config {
	return &Config{
		Server: ServerConfig{
			Port:    getEnv("PORT", "8000"),
			AppName: "CORE - Conductive Suit Reliability Evaluator",
		},
		MQTT: MQTTConfig{
			Broker:   getEnv("MQTT_BROKER", "tcp://localhost:1884"),
			ClientID: getEnv("MQTT_CLIENT_ID", "go-mqtt-client"),
			Topic:    getEnv("MQTT_TOPIC", "golang-webserver/topic"),
		},
		Auth: AuthConfig{
			DefaultUsername: "admin",
			DefaultPassword: "12345",
		},
		ESP32: ESP32Config{
			APIP:      "192.168.4.1",
			StationIP: "20.20.20.56",
		},
	}
}

func getEnv(key, defaultValue string) string {
	value := os.Getenv(key)
	if value == "" {
		return defaultValue
	}
	return value
}
