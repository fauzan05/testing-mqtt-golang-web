package main

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io"
	"log"
	"net/http"
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

// LoginRequest struct
type LoginRequest struct {
	Username string `json:"username"`
	Password string `json:"password"`
}

// LoginResponse struct
type LoginResponse struct {
	Success bool   `json:"success"`
	Message string `json:"message"`
}

// ChangeUsernameRequest struct
type ChangeUsernameRequest struct {
	NewUsername string `json:"newUsername"`
	Password    string `json:"password"`
}

// ChangePasswordRequest struct
type ChangePasswordRequest struct {
	OldPassword string `json:"oldPassword"`
	NewPassword string `json:"newPassword"`
}

// UserInfo struct
type UserInfo struct {
	Username  string `json:"username"`
	LoginTime string `json:"loginTime"`
}

var (
	// Menyimpan semua WebSocket connections
	clients   = make(map[*websocket.Conn]bool)
	clientsMu sync.Mutex
	broadcast = make(chan Message)

	// Simple credentials (in production, use database and hash passwords)
	validUsername = "admin"
	validPassword = "12345"

	// Store logged in sessions (simple map for demo)
	sessions   = make(map[string]UserInfo)
	sessionsMu sync.Mutex

	// Mutex for credentials
	credentialsMu sync.Mutex

	// Global MQTT client untuk publish dari WebSocket
	globalMQTTClient mqtt.Client

	// Serial port connection for USB fallback
	serialPort      interface{} // Will be serial.Port when library loaded
	serialConnected = false
	serialPortName  = ""

	// ESP32 connection tracking
	esp32APIP         = "192.168.4.1" // Default AP mode IP
	esp32StationIP    = "20.20.20.56" // Station mode IP (from router PDKB_INTERNET_G)
	useSerialFallback = false         // Use serial if HTTP fails
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

// Struct untuk handle MQTT publish dari client
type MQTTPublishCommand struct {
	Command string `json:"command"` // "mqtt_publish"
	Topic   string `json:"topic"`   // "cjack/esp32_001/control"
	Message string `json:"message"` // {"action":"inject"}
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
					if globalMQTTClient != nil && globalMQTTClient.IsConnected() {
						token := globalMQTTClient.Publish(topic, 0, false, message)
						token.Wait()

						if token.Error() != nil {
							log.Printf("MQTT publish error: %v", token.Error())
						} else {
							log.Printf("MQTT published - Topic: %s, Message: %s", topic, message)
						}
					} else {
						log.Printf("MQTT client not connected, cannot publish")
					}
				}
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

// Auth middleware to check if user is logged in (using simple cookie-based session)
func authRequired(c *fiber.Ctx) error {
	sessionID := c.Cookies("session_id")

	sessionsMu.Lock()
	_, isLoggedIn := sessions[sessionID]
	sessionsMu.Unlock()

	if !isLoggedIn {
		return c.Redirect("/login")
	}

	return c.Next()
}

// Generate simple session ID
func generateSessionID(username string) string {
	return fmt.Sprintf("%s_%d", username, time.Now().Unix())
}

func main() {
	// Setup MQTT client
	globalMQTTClient = setupMQTT()
	defer globalMQTTClient.Disconnect(250)

	// Start goroutine untuk broadcast messages
	go broadcastMessages()

	app := fiber.New(fiber.Config{
		AppName: "CORE - Conductive Suit Reliability Evaluator",
	})

	// Serve sidebar.html with no-cache headers
	app.Get("/sidebar.html", func(c *fiber.Ctx) error {
		c.Set("Cache-Control", "no-cache, no-store, must-revalidate")
		c.Set("Pragma", "no-cache")
		c.Set("Expires", "0")
		return c.SendFile("./sidebar.html")
	})

	// Serve sidebar.js with no-cache headers
	app.Get("/sidebar.js", func(c *fiber.Ctx) error {
		c.Set("Cache-Control", "no-cache, no-store, must-revalidate")
		c.Set("Pragma", "no-cache")
		c.Set("Expires", "0")
		return c.SendFile("./sidebar.js")
	})

	// Serve styles.css with no-cache headers
	app.Get("/styles.css", func(c *fiber.Ctx) error {
		c.Set("Cache-Control", "no-cache, no-store, must-revalidate")
		c.Set("Pragma", "no-cache")
		c.Set("Expires", "0")
		return c.SendFile("./styles.css")
	})

	// Serve sidebar.css with no-cache headers
	app.Get("/sidebar.css", func(c *fiber.Ctx) error {
		c.Set("Cache-Control", "no-cache, no-store, must-revalidate")
		c.Set("Pragma", "no-cache")
		c.Set("Expires", "0")
		return c.SendFile("./sidebar.css")
	})

	// Serve static files (CSS, JS, images)
	app.Static("/", "./", fiber.Static{
		Browse: false,
	})

	// Public routes (no auth required)
	app.Get("/login", func(c *fiber.Ctx) error {
		return c.SendFile("./login.html")
	})

	// Login API endpoint
	app.Post("/api/login", func(c *fiber.Ctx) error {
		var req LoginRequest
		if err := c.BodyParser(&req); err != nil {
			return c.Status(400).JSON(LoginResponse{
				Success: false,
				Message: "Invalid request",
			})
		}

		// Validate credentials
		credentialsMu.Lock()
		usernameMatch := req.Username == validUsername
		passwordMatch := req.Password == validPassword
		credentialsMu.Unlock()

		if usernameMatch && passwordMatch {
			// Create session
			sessionID := generateSessionID(req.Username)

			sessionsMu.Lock()
			sessions[sessionID] = UserInfo{
				Username:  req.Username,
				LoginTime: time.Now().Format(time.RFC3339),
			}
			sessionsMu.Unlock()

			// Set cookie
			c.Cookie(&fiber.Cookie{
				Name:     "session_id",
				Value:    sessionID,
				Expires:  time.Now().Add(24 * time.Hour),
				HTTPOnly: true,
				SameSite: "Lax",
			})

			return c.JSON(LoginResponse{
				Success: true,
				Message: "Login successful",
			})
		}

		return c.Status(401).JSON(LoginResponse{
			Success: false,
			Message: "Username atau password salah",
		})
	})

	// Check auth endpoint (for JavaScript auth check)
	app.Get("/api/check-auth", func(c *fiber.Ctx) error {
		sessionID := c.Cookies("session_id")

		sessionsMu.Lock()
		_, isLoggedIn := sessions[sessionID]
		sessionsMu.Unlock()

		if !isLoggedIn {
			return c.Status(401).JSON(fiber.Map{
				"authenticated": false,
			})
		}

		return c.JSON(fiber.Map{
			"authenticated": true,
		})
	})

	// Get user info endpoint
	app.Get("/api/user-info", authRequired, func(c *fiber.Ctx) error {
		sessionID := c.Cookies("session_id")

		sessionsMu.Lock()
		userInfo := sessions[sessionID]
		sessionsMu.Unlock()

		return c.JSON(userInfo)
	})

	// Logout endpoint
	app.Get("/api/logout", func(c *fiber.Ctx) error {
		sessionID := c.Cookies("session_id")

		sessionsMu.Lock()
		delete(sessions, sessionID)
		sessionsMu.Unlock()

		// Clear cookie
		c.Cookie(&fiber.Cookie{
			Name:     "session_id",
			Value:    "",
			Expires:  time.Now().Add(-1 * time.Hour),
			HTTPOnly: true,
		})

		return c.Redirect("/login")
	})

	// Change username endpoint
	app.Post("/api/change-username", authRequired, func(c *fiber.Ctx) error {
		var req ChangeUsernameRequest
		if err := c.BodyParser(&req); err != nil {
			return c.Status(400).JSON(LoginResponse{
				Success: false,
				Message: "Invalid request",
			})
		}

		// Verify password
		credentialsMu.Lock()
		passwordMatch := req.Password == validPassword
		credentialsMu.Unlock()

		if !passwordMatch {
			return c.Status(401).JSON(LoginResponse{
				Success: false,
				Message: "Password salah",
			})
		}

		// Update username
		credentialsMu.Lock()
		validUsername = req.NewUsername
		credentialsMu.Unlock()

		// Update session info
		sessionID := c.Cookies("session_id")
		sessionsMu.Lock()
		if userInfo, exists := sessions[sessionID]; exists {
			userInfo.Username = req.NewUsername
			sessions[sessionID] = userInfo
		}
		sessionsMu.Unlock()

		return c.JSON(LoginResponse{
			Success: true,
			Message: "Username berhasil diubah",
		})
	})

	// Change password endpoint
	app.Post("/api/change-password", authRequired, func(c *fiber.Ctx) error {
		var req ChangePasswordRequest
		if err := c.BodyParser(&req); err != nil {
			return c.Status(400).JSON(LoginResponse{
				Success: false,
				Message: "Invalid request",
			})
		}

		// Verify old password
		credentialsMu.Lock()
		oldPasswordMatch := req.OldPassword == validPassword
		credentialsMu.Unlock()

		if !oldPasswordMatch {
			return c.Status(401).JSON(LoginResponse{
				Success: false,
				Message: "Password lama salah",
			})
		}

		// Update password
		credentialsMu.Lock()
		validPassword = req.NewPassword
		credentialsMu.Unlock()

		return c.JSON(LoginResponse{
			Success: true,
			Message: "Password berhasil diubah",
		})
	})

	// Redirect root to dashboard if authenticated, else to login
	app.Get("/", func(c *fiber.Ctx) error {
		sessionID := c.Cookies("session_id")

		sessionsMu.Lock()
		_, isLoggedIn := sessions[sessionID]
		sessionsMu.Unlock()

		if isLoggedIn {
			return c.Redirect("/dashboard")
		}
		return c.Redirect("/login")
	})

	// Protected routes (auth required)
	app.Get("/dashboard", authRequired, func(c *fiber.Ctx) error {
		return c.SendFile("./dashboard.html")
	})

	app.Get("/pengujian", authRequired, func(c *fiber.Ctx) error {
		return c.SendFile("./pengujian.html")
	})

	app.Get("/config", authRequired, func(c *fiber.Ctx) error {
		return c.SendFile("./config.html")
	})

	app.Get("/daftar", authRequired, func(c *fiber.Ctx) error {
		return c.SendFile("./daftar.html")
	})

	app.Get("/histori", authRequired, func(c *fiber.Ctx) error {
		return c.SendFile("./histori.html")
	})

	app.Get("/foto", authRequired, func(c *fiber.Ctx) error {
		return c.SendFile("./foto.html")
	})

	app.Get("/user", authRequired, func(c *fiber.Ctx) error {
		return c.SendFile("./user.html")
	})

	app.Get("/index.html", authRequired, func(c *fiber.Ctx) error {
		return c.SendFile("./index.html")
	})

	// WebSocket endpoint for real-time updates
	app.Get("/ws", handleWebSocket())

	// Health check endpoint
	app.Get("/health", func(c *fiber.Ctx) error {
		return c.JSON(fiber.Map{
			"status":           "ok",
			"mqtt_connected":   globalMQTTClient.IsConnected(),
			"clients":          len(clients),
			"esp32_ap":         esp32APIP,
			"esp32_station":    esp32StationIP,
			"serial_port":      serialPortName,
			"serial_connected": serialConnected,
		})
	})

	// Proxy endpoints to control ESP32 via HTTP (with multi-connection support)
	// Priority: 1. AP Mode, 2. Station Mode, 3. Serial USB
	httpClient := &http.Client{Timeout: 5 * time.Second}

	// Helper function to try multiple connection methods
	tryESP32Request := func(method, path string, body []byte) (*http.Response, error) {
		// Try AP mode first (direct WiFi)
		apURL := "http://" + esp32APIP + path
		log.Printf("Trying AP mode: %s", apURL)

		var req *http.Request
		var err error
		if len(body) > 0 {
			req, err = http.NewRequest(method, apURL, bytes.NewBuffer(body))
		} else {
			req, err = http.NewRequest(method, apURL, nil)
		}
		if err == nil {
			req.Header.Set("Content-Type", "application/json")
			resp, err := httpClient.Do(req)
			if err == nil {
				log.Printf("✅ AP mode success")
				return resp, nil
			}
			log.Printf("⚠️ AP mode failed: %v", err)
		}

		// Try Station mode if IP available (router WiFi)
		if esp32StationIP != "" {
			stationURL := "http://" + esp32StationIP + path
			log.Printf("Trying Station mode: %s", stationURL)

			if len(body) > 0 {
				req, err = http.NewRequest(method, stationURL, bytes.NewBuffer(body))
			} else {
				req, err = http.NewRequest(method, stationURL, nil)
			}
			if err == nil {
				req.Header.Set("Content-Type", "application/json")
				resp, err := httpClient.Do(req)
				if err == nil {
					log.Printf("✅ Station mode success")
					return resp, nil
				}
				log.Printf("⚠️ Station mode failed: %v", err)
			}
		}

		// Serial fallback would go here (future implementation)
		return nil, fmt.Errorf("all connection methods failed")
	}

	// Proxy GET /esp32/status -> Try multiple connection methods
	app.Get("/esp32/status", func(c *fiber.Ctx) error {
		resp, err := tryESP32Request("GET", "/status", nil)
		if err != nil {
			return c.Status(502).JSON(fiber.Map{"error": "failed to reach esp32", "detail": err.Error()})
		}
		defer resp.Body.Close()
		body, _ := io.ReadAll(resp.Body)
		return c.Status(resp.StatusCode).Send(body)
	})

	// Proxy POST /esp32/inject - body JSON {mode:'quick'|'special', amplitude:int, duration:int}
	app.Post("/esp32/inject", func(c *fiber.Ctx) error {
		var req map[string]interface{}
		if err := c.BodyParser(&req); err != nil {
			// Accept form values too
			req = make(map[string]interface{})
			_ = json.Unmarshal(c.Body(), &req)
		}

		// Forward langsung ke ESP32 /api/inject endpoint (multi-connection)
		jsonData, _ := json.Marshal(req)

		resp, err := tryESP32Request("POST", "/api/inject", jsonData)
		if err != nil {
			return c.Status(502).JSON(fiber.Map{"error": "failed to reach ESP32", "detail": err.Error()})
		}
		defer resp.Body.Close()

		body, _ := io.ReadAll(resp.Body)
		return c.Status(resp.StatusCode).Send(body)
	})

	// Proxy POST /esp32/stop -> stop running (multi-connection)
	app.Post("/esp32/stop", func(c *fiber.Ctx) error {
		resp, err := tryESP32Request("POST", "/api/stop", []byte("{}"))
		if err != nil {
			return c.Status(502).JSON(fiber.Map{"error": "failed to stop device", "detail": err.Error()})
		}
		defer resp.Body.Close()
		body, _ := io.ReadAll(resp.Body)
		return c.Status(resp.StatusCode).Send(body)
	})

	// Proxy POST /esp32/set_amplitude -> {value: number} (multi-connection)
	app.Post("/esp32/set_amplitude", func(c *fiber.Ctx) error {
		var req map[string]interface{}
		if err := c.BodyParser(&req); err != nil {
			req = make(map[string]interface{})
			_ = json.Unmarshal(c.Body(), &req)
		}
		amp := 0
		if a, ok := req["value"].(float64); ok {
			amp = int(a)
		} else if aStr := c.Query("value"); aStr != "" {
			fmt.Sscanf(aStr, "%d", &amp)
		}
		path := fmt.Sprintf("/set_amplitude?value=%d", amp)
		resp, err := tryESP32Request("GET", path, nil)
		if err != nil {
			return c.Status(502).JSON(fiber.Map{"error": "failed to set amplitude", "detail": err.Error()})
		}
		defer resp.Body.Close()
		body, _ := io.ReadAll(resp.Body)
		return c.Status(resp.StatusCode).Send(body)
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
	fmt.Println("📡 CORE - Conductive Suit Reliability Evaluator is running...")
	fmt.Printf("🔗 Open http://localhost:%s in your browser\n\n", port)

	log.Fatal(app.Listen(":" + port))
}
