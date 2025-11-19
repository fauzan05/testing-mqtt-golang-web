package main

import (
	"fmt"
	"log"

	"myfiberapp/internal/delivery/http"
	wsHandler "myfiberapp/internal/delivery/websocket"
	"myfiberapp/internal/domain"
	"myfiberapp/internal/repository"
	"myfiberapp/internal/usecase"
	"myfiberapp/pkg/config"
	"myfiberapp/pkg/mqtt"

	"github.com/gofiber/fiber/v2"
)

func main() {
	// Load configuration
	cfg := config.LoadConfig()

	// Initialize repositories
	authRepo := repository.NewAuthRepository(cfg.Auth.DefaultUsername, cfg.Auth.DefaultPassword)
	sessionRepo := repository.NewSessionRepository()

	// Initialize use cases
	authUC := usecase.NewAuthUseCase(authRepo, sessionRepo)
	wsUC := usecase.NewWebSocketUseCase()
	esp32UC := usecase.NewESP32UseCase(domain.ESP32Config{
		APIP:      cfg.ESP32.APIP,
		StationIP: cfg.ESP32.StationIP,
	})

	// Initialize MQTT client with callback to broadcast messages
	mqttClient := mqtt.NewClient(cfg.MQTT, func(msg domain.Message) {
		wsUC.BroadcastMessage(msg)
	})
	defer mqttClient.Disconnect()

	// Initialize handlers
	authHandler := http.NewAuthHandler(authUC)
	esp32Handler := http.NewESP32Handler(esp32UC)
	websocketHandler := wsHandler.NewHandler(wsUC, mqttClient)

	// Initialize Fiber app
	app := fiber.New(fiber.Config{
		AppName: cfg.Server.AppName,
	})

	// Setup router
	router := http.NewRouter(
		app,
		authHandler,
		esp32Handler,
		websocketHandler,
		authUC,
		mqttClient.IsConnected,
		wsUC.GetClientCount,
	)
	router.Setup()

	// Start server
	fmt.Printf("\n🚀 Server started on http://localhost:%s\n", cfg.Server.Port)
	fmt.Println("📡 CORE - Conductive Suit Reliability Evaluator is running...")
	fmt.Printf("🔗 Open http://localhost:%s in your browser\n\n", cfg.Server.Port)

	log.Fatal(app.Listen(":" + cfg.Server.Port))
}
