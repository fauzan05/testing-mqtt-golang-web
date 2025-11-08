package main

import (
	"encoding/json"
	"fmt"
	"log"
	"os"
	"sync"
	"time"

	mqtt "github.com/eclipse/paho.mqtt.golang"
	"github.com/gofiber/fiber/v2"
	"github.com/gofiber/websocket/v2"
)

// Message struct untuk data yang dikirim ke WebSocket
type Message struct {
	Topic   string `json:"topic"`
	Payload string `json:"payload"`
}

var (
	// Menyimpan semua WebSocket connections
	clients   = make(map[*websocket.Conn]bool)
	clientsMu sync.Mutex
	broadcast = make(chan Message)
)

// MQTT message handler
var messagePubHandler mqtt.MessageHandler = func(client mqtt.Client, msg mqtt.Message) {
	message := Message{
		Topic:   msg.Topic(),
		Payload: string(msg.Payload()),
	}

	log.Printf("Received MQTT message - Topic: %s, Payload: %s\n", message.Topic, message.Payload)

	// Broadcast ke semua WebSocket clients
	broadcast <- message
}

// MQTT connection handler
var connectHandler mqtt.OnConnectHandler = func(client mqtt.Client) {
	log.Println("Connected to MQTT broker")
}

// MQTT connection lost handler
var connectLostHandler mqtt.ConnectionLostHandler = func(client mqtt.Client, err error) {
	log.Printf("Connection lost: %v", err)
}

func setupMQTT() mqtt.Client {
	// Konfigurasi MQTT broker
	// Ganti dengan broker MQTT Anda (contoh: "tcp://broker.emqx.io:1884" atau "tcp://localhost:1884")
	broker := getEnvironment("MQTT_BROKER", "tcp://localhost:1884")
	clientID := getEnvironment("MQTT_CLIENT_ID", "go-mqtt-client")
	topic := getEnvironment("MQTT_TOPIC", "golang-webserver/topic")

	log.Printf("Connecting to MQTT broker: %s", broker)
	log.Printf("Client ID: %s", clientID)
	log.Printf("Subscribing to topic: %s", topic)

	opts := mqtt.NewClientOptions()
	opts.AddBroker(broker)
	opts.SetClientID(clientID)
	opts.SetDefaultPublishHandler(messagePubHandler)
	opts.OnConnect = connectHandler
	opts.OnConnectionLost = connectLostHandler
	opts.SetAutoReconnect(true)
	opts.SetConnectRetry(true)
	opts.SetConnectRetryInterval(5 * time.Second)

	client := mqtt.NewClient(opts)
	if token := client.Connect(); token.Wait() && token.Error() != nil {
		log.Printf("Error connecting to MQTT broker: %v", token.Error())
		log.Println("Server will continue running, waiting for MQTT connection...")
	}

	// Subscribe ke topic
	if token := client.Subscribe(topic, 1, nil); token.Wait() && token.Error() != nil {
		log.Printf("Error subscribing to topic: %v", token.Error())
	} else {
		log.Printf("Successfully subscribed to topic: %s", topic)
	}

	return client
}

func handleWebSocket() func(*fiber.Ctx) error {
	return websocket.New(func(c *websocket.Conn) {
		// Register client
		clientsMu.Lock()
		clients[c] = true
		clientsMu.Unlock()

		log.Printf("New WebSocket client connected. Total clients: %d", len(clients))

		defer func() {
			// Unregister client
			clientsMu.Lock()
			delete(clients, c)
			clientsMu.Unlock()
			c.Close()
			log.Printf("WebSocket client disconnected. Total clients: %d", len(clients))
		}()

		// Keep connection alive
		for {
			_, _, err := c.ReadMessage()
			if err != nil {
				break
			}
		}
	})
}

func broadcastMessages() {
	for {
		message := <-broadcast

		clientsMu.Lock()
		for client := range clients {
			data, err := json.Marshal(message)
			if err != nil {
				log.Printf("Error marshaling message: %v", err)
				continue
			}

			err = client.WriteMessage(websocket.TextMessage, data)
			if err != nil {
				log.Printf("Error sending message to client: %v", err)
				client.Close()
				delete(clients, client)
			}
		}
		clientsMu.Unlock()
	}
}

func getEnvironment(key, defaultValue string) string {
	value := os.Getenv(key)
	if value == "" {
		return defaultValue
	}
	return value
}

func main() {
	// Setup MQTT client
	mqttClient := setupMQTT()
	defer mqttClient.Disconnect(250)

	// Start goroutine untuk broadcast messages
	go broadcastMessages()

	app := fiber.New(fiber.Config{
		AppName: "MQTT Real-time Monitor",
	})

	// Serve HTML file
	app.Get("/", func(c *fiber.Ctx) error {
		return c.SendFile("./index.html")
	})

	// WebSocket endpoint
	app.Get("/ws", handleWebSocket())

	// Health check endpoint
	app.Get("/health", func(c *fiber.Ctx) error {
		return c.JSON(fiber.Map{
			"status":         "ok",
			"mqtt_connected": mqttClient.IsConnected(),
			"clients":        len(clients),
		})
	})

	// Info endpoint
	app.Get("/info", func(c *fiber.Ctx) error {
		broker := getEnvironment("MQTT_BROKER", "tcp://localhost:1884")
		topic := getEnvironment("MQTT_TOPIC", "golang-webserver/topic")

		return c.JSON(fiber.Map{
			"broker":  broker,
			"topic":   topic,
			"clients": len(clients),
		})
	})

	port := getEnvironment("PORT", "8000")
	fmt.Printf("\n🚀 Server started on http://localhost:%s\n", port)
	fmt.Println("📡 MQTT Real-time Monitor is running...")
	fmt.Printf("🔗 Open http://localhost:%s in your browser\n\n", port)

	log.Fatal(app.Listen(":" + port))
}
