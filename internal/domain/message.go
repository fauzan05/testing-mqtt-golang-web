package domain

// Message struct untuk data MQTT yang dikirim ke WebSocket
type Message struct {
	Topic   string `json:"topic"`
	Payload string `json:"payload"`
}

// MQTTPublishCommand struct untuk handle MQTT publish dari client
type MQTTPublishCommand struct {
	Command string `json:"command"` // "mqtt_publish"
	Topic   string `json:"topic"`   // topic MQTT
	Message string `json:"message"` // payload message
}
