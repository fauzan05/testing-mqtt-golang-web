package usecase

import (
	"log"
	"sync"

	"myfiberapp/internal/domain"

	"github.com/gofiber/websocket/v2"
)

// WebSocketUseCase handles WebSocket connections and broadcasting
type WebSocketUseCase struct {
	clients   map[*websocket.Conn]bool
	mu        sync.Mutex
	broadcast chan domain.Message
}

// NewWebSocketUseCase creates a new WebSocket use case
func NewWebSocketUseCase() *WebSocketUseCase {
	uc := &WebSocketUseCase{
		clients:   make(map[*websocket.Conn]bool),
		broadcast: make(chan domain.Message, 100),
	}

	// Start broadcast goroutine
	go uc.handleBroadcast()

	return uc
}

// RegisterClient adds a new WebSocket client
func (uc *WebSocketUseCase) RegisterClient(conn *websocket.Conn) {
	uc.mu.Lock()
	uc.clients[conn] = true
	uc.mu.Unlock()

	log.Printf("New WebSocket client connected. Total clients: %d", uc.GetClientCount())
}

// UnregisterClient removes a WebSocket client
func (uc *WebSocketUseCase) UnregisterClient(conn *websocket.Conn) {
	uc.mu.Lock()
	delete(uc.clients, conn)
	uc.mu.Unlock()

	conn.Close()
	log.Printf("WebSocket client disconnected. Total clients: %d", uc.GetClientCount())
}

// BroadcastMessage sends a message to all connected clients
func (uc *WebSocketUseCase) BroadcastMessage(msg domain.Message) {
	uc.broadcast <- msg
}

// GetBroadcastChannel returns the broadcast channel
func (uc *WebSocketUseCase) GetBroadcastChannel() chan<- domain.Message {
	return uc.broadcast
}

// GetClientCount returns the number of connected clients
func (uc *WebSocketUseCase) GetClientCount() int {
	uc.mu.Lock()
	defer uc.mu.Unlock()

	return len(uc.clients)
}

func (uc *WebSocketUseCase) handleBroadcast() {
	for message := range uc.broadcast {
		uc.mu.Lock()
		for client := range uc.clients {
			err := client.WriteJSON(message)
			if err != nil {
				log.Printf("Error sending message to client: %v", err)
				client.Close()
				delete(uc.clients, client)
			}
		}
		uc.mu.Unlock()
	}
}
