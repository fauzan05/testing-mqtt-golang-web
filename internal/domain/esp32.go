package domain

// ESP32InjectionRequest represents injection command parameters
type ESP32InjectionRequest struct {
	Mode      string `json:"mode"`      // "quick" or "special"
	Amplitude int    `json:"amplitude"` // amplitude value
	Duration  int    `json:"duration"`  // duration in ms
}

// ESP32Config holds ESP32 connection settings
type ESP32Config struct {
	APIP       string // AP mode IP address
	StationIP  string // Station mode IP address
	SerialPort string // Serial port name for USB fallback
}
