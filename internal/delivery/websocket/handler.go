package websocket

import (
	"log"

	"myfiberapp/internal/domain"
	"myfiberapp/internal/usecase"
	"myfiberapp/pkg/mqtt"

	"github.com/gofiber/fiber/v2"
	"github.com/gofiber/websocket/v2"
)

// Handler handles WebSocket connections
type Handler struct {
	wsUC       *usecase.WebSocketUseCase
	mqttClient *mqtt.Client
}

// NewHandler creates a new WebSocket handler
func NewHandler(wsUC *usecase.WebSocketUseCase, mqttClient *mqtt.Client) *Handler {
	return &Handler{
		wsUC:       wsUC,
		mqttClient: mqttClient,
	}
}

// HandleConnection returns a WebSocket handler function
func (h *Handler) HandleConnection() func(*fiber.Ctx) error {
	return websocket.New(func(c *websocket.Conn) {
		// Register client
		h.wsUC.RegisterClient(c)

		defer func() {
			// Unregister client
			h.wsUC.UnregisterClient(c)
		}()

		// Handle messages from client
		for {
			var msg map[string]interface{}
			err := c.ReadJSON(&msg)
			if err != nil {
				if websocket.IsUnexpectedCloseError(err, websocket.CloseGoingAway, websocket.CloseAbnormalClosure) {
					log.Printf("WebSocket error: %v", err)
				}
				break
			}

			// Check if it's MQTT publish command
			if cmd, ok := msg["command"].(string); ok && cmd == "mqtt_publish" {
				topic, _ := msg["topic"].(string)
				message, _ := msg["message"].(string)

				if topic != "" && message != "" {
					// Publish to MQTT broker
					if h.mqttClient != nil && h.mqttClient.IsConnected() {
						err := h.mqttClient.Publish(topic, message)
						if err != nil {
							log.Printf("MQTT publish error: %v", err)
						}
					} else {
						log.Printf("MQTT client not connected, cannot publish")
					}
				}
			}
		}
	})
}

// BroadcastMessage broadcasts a message to all connected clients
func (h *Handler) BroadcastMessage(msg domain.Message) {
	h.wsUC.BroadcastMessage(msg)
}
