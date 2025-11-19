package http

import (
	"encoding/json"
	"fmt"

	"myfiberapp/internal/domain"
	"myfiberapp/internal/usecase"

	"github.com/gofiber/fiber/v2"
)

// ESP32Handler handles ESP32 device endpoints
type ESP32Handler struct {
	esp32UC *usecase.ESP32UseCase
}

// NewESP32Handler creates a new ESP32 handler
func NewESP32Handler(esp32UC *usecase.ESP32UseCase) *ESP32Handler {
	return &ESP32Handler{
		esp32UC: esp32UC,
	}
}

// GetStatus retrieves ESP32 status
func (h *ESP32Handler) GetStatus(c *fiber.Ctx) error {
	body, err := h.esp32UC.GetStatus()
	if err != nil {
		return c.Status(502).JSON(fiber.Map{
			"error":  "failed to reach ESP32",
			"detail": err.Error(),
		})
	}

	return c.Send(body)
}

// Inject handles injection command
func (h *ESP32Handler) Inject(c *fiber.Ctx) error {
	var req domain.ESP32InjectionRequest
	if err := c.BodyParser(&req); err != nil {
		// Try to parse as generic map
		var reqMap map[string]interface{}
		_ = json.Unmarshal(c.Body(), &reqMap)

		// Extract values with defaults
		if mode, ok := reqMap["mode"].(string); ok {
			req.Mode = mode
		}
		if amp, ok := reqMap["amplitude"].(float64); ok {
			req.Amplitude = int(amp)
		}
		if dur, ok := reqMap["duration"].(float64); ok {
			req.Duration = int(dur)
		}
	}

	body, err := h.esp32UC.SendInjectCommand(req)
	if err != nil {
		return c.Status(502).JSON(fiber.Map{
			"error":  "failed to reach ESP32",
			"detail": err.Error(),
		})
	}

	return c.Send(body)
}

// Stop stops the ESP32 device
func (h *ESP32Handler) Stop(c *fiber.Ctx) error {
	body, err := h.esp32UC.StopDevice()
	if err != nil {
		return c.Status(502).JSON(fiber.Map{
			"error":  "failed to stop device",
			"detail": err.Error(),
		})
	}

	return c.Send(body)
}

// SetAmplitude sets device amplitude
func (h *ESP32Handler) SetAmplitude(c *fiber.Ctx) error {
	var req map[string]interface{}
	amp := 0

	if err := c.BodyParser(&req); err == nil {
		if a, ok := req["value"].(float64); ok {
			amp = int(a)
		}
	}

	// Also check query parameter
	if ampStr := c.Query("value"); ampStr != "" {
		var tempAmp int
		if _, err := fmt.Sscanf(ampStr, "%d", &tempAmp); err == nil {
			amp = tempAmp
		}
	}

	body, err := h.esp32UC.SetAmplitude(amp)
	if err != nil {
		return c.Status(502).JSON(fiber.Map{
			"error":  "failed to set amplitude",
			"detail": err.Error(),
		})
	}

	return c.Send(body)
}
