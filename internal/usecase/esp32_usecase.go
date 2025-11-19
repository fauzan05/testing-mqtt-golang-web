package usecase

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io"
	"log"
	"net/http"
	"time"

	"myfiberapp/internal/domain"
)

// ESP32UseCase handles ESP32 device communication
type ESP32UseCase struct {
	config     domain.ESP32Config
	httpClient *http.Client
}

// NewESP32UseCase creates a new ESP32 use case
func NewESP32UseCase(config domain.ESP32Config) *ESP32UseCase {
	return &ESP32UseCase{
		config:     config,
		httpClient: &http.Client{Timeout: 5 * time.Second},
	}
}

// GetStatus retrieves ESP32 status
func (uc *ESP32UseCase) GetStatus() ([]byte, error) {
	resp, err := uc.tryRequest("GET", "/status", nil)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	return io.ReadAll(resp.Body)
}

// SendInjectCommand sends injection command to ESP32
func (uc *ESP32UseCase) SendInjectCommand(req domain.ESP32InjectionRequest) ([]byte, error) {
	jsonData, err := json.Marshal(req)
	if err != nil {
		return nil, err
	}

	resp, err := uc.tryRequest("POST", "/api/inject", jsonData)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	return io.ReadAll(resp.Body)
}

// StopDevice stops the running device
func (uc *ESP32UseCase) StopDevice() ([]byte, error) {
	resp, err := uc.tryRequest("POST", "/api/stop", []byte("{}"))
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	return io.ReadAll(resp.Body)
}

// SetAmplitude sets device amplitude
func (uc *ESP32UseCase) SetAmplitude(value int) ([]byte, error) {
	path := fmt.Sprintf("/set_amplitude?value=%d", value)
	resp, err := uc.tryRequest("GET", path, nil)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	return io.ReadAll(resp.Body)
}

// tryRequest tries multiple connection methods to reach ESP32
func (uc *ESP32UseCase) tryRequest(method, path string, body []byte) (*http.Response, error) {
	// Try AP mode first (direct WiFi)
	apURL := "http://" + uc.config.APIP + path
	log.Printf("Trying AP mode: %s", apURL)

	req, err := uc.createRequest(method, apURL, body)
	if err == nil {
		resp, err := uc.httpClient.Do(req)
		if err == nil {
			log.Printf("✅ AP mode success")
			return resp, nil
		}
		log.Printf("⚠️ AP mode failed: %v", err)
	}

	// Try Station mode if IP available (router WiFi)
	if uc.config.StationIP != "" {
		stationURL := "http://" + uc.config.StationIP + path
		log.Printf("Trying Station mode: %s", stationURL)

		req, err := uc.createRequest(method, stationURL, body)
		if err == nil {
			resp, err := uc.httpClient.Do(req)
			if err == nil {
				log.Printf("✅ Station mode success")
				return resp, nil
			}
			log.Printf("⚠️ Station mode failed: %v", err)
		}
	}

	return nil, fmt.Errorf("all connection methods failed")
}

func (uc *ESP32UseCase) createRequest(method, url string, body []byte) (*http.Request, error) {
	var req *http.Request
	var err error

	if len(body) > 0 {
		req, err = http.NewRequest(method, url, bytes.NewBuffer(body))
	} else {
		req, err = http.NewRequest(method, url, nil)
	}

	if err == nil {
		req.Header.Set("Content-Type", "application/json")
	}

	return req, err
}
