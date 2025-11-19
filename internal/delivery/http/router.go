package http

import (
	wsHandler "myfiberapp/internal/delivery/websocket"
	"myfiberapp/internal/middleware"
	"myfiberapp/internal/usecase"

	"github.com/gofiber/fiber/v2"
)

// Router sets up all HTTP routes
type Router struct {
	app           *fiber.App
	authHandler   *AuthHandler
	esp32Handler  *ESP32Handler
	wsHandler     *wsHandler.Handler
	authUC        *usecase.AuthUseCase
	mqttConnected func() bool
	clientCount   func() int
}

// NewRouter creates a new router
func NewRouter(
	app *fiber.App,
	authHandler *AuthHandler,
	esp32Handler *ESP32Handler,
	wsHandler *wsHandler.Handler,
	authUC *usecase.AuthUseCase,
	mqttConnected func() bool,
	clientCount func() int,
) *Router {
	return &Router{
		app:           app,
		authHandler:   authHandler,
		esp32Handler:  esp32Handler,
		wsHandler:     wsHandler,
		authUC:        authUC,
		mqttConnected: mqttConnected,
		clientCount:   clientCount,
	}
}

// Setup configures all routes
func (r *Router) Setup() {
	// Serve static files with no-cache headers
	r.setupStaticRoutes()

	// Public routes
	r.setupPublicRoutes()

	// Protected routes
	r.setupProtectedRoutes()

	// API routes
	r.setupAPIRoutes()

	// WebSocket route
	r.app.Get("/ws", r.wsHandler.HandleConnection())

	// Health check
	r.app.Get("/health", r.healthCheck)
}

func (r *Router) setupStaticRoutes() {
	// Serve CSS files with no-cache
	cssFiles := []string{"styles.css", "sidebar.css", "form.css"}
	for _, file := range cssFiles {
		filename := file
		r.app.Get("/"+filename, func(c *fiber.Ctx) error {
			c.Set("Cache-Control", "no-cache, no-store, must-revalidate")
			c.Set("Pragma", "no-cache")
			c.Set("Expires", "0")
			c.Set("Content-Type", "text/css")
			return c.SendFile("./" + filename)
		})
	}

	// Serve JS files with no-cache
	jsFiles := []string{"sidebar.js", "auth-check.js"}
	for _, file := range jsFiles {
		filename := file
		r.app.Get("/"+filename, func(c *fiber.Ctx) error {
			c.Set("Cache-Control", "no-cache, no-store, must-revalidate")
			c.Set("Pragma", "no-cache")
			c.Set("Expires", "0")
			c.Set("Content-Type", "application/javascript")
			return c.SendFile("./" + filename)
		})
	}

	// Serve HTML fragments with no-cache
	htmlFiles := []string{"sidebar.html"}
	for _, file := range htmlFiles {
		filename := file
		r.app.Get("/"+filename, func(c *fiber.Ctx) error {
			c.Set("Cache-Control", "no-cache, no-store, must-revalidate")
			c.Set("Pragma", "no-cache")
			c.Set("Expires", "0")
			return c.SendFile("./" + filename)
		})
	}

	// Serve images
	r.app.Static("/images", "./", fiber.Static{
		Browse: false,
	})

	// Serve all static files from root for now (compatibility)
	r.app.Static("/", "./", fiber.Static{
		Browse: false,
	})
}

func (r *Router) setupPublicRoutes() {
	r.app.Get("/login", func(c *fiber.Ctx) error {
		return c.SendFile("./login.html")
	})

	// Redirect root based on auth status
	r.app.Get("/", func(c *fiber.Ctx) error {
		sessionID := c.Cookies("session_id")
		if r.authUC.ValidateSession(sessionID) {
			return c.Redirect("/dashboard")
		}
		return c.Redirect("/login")
	})
}

func (r *Router) setupProtectedRoutes() {
	authMW := middleware.AuthMiddleware(r.authUC)

	pages := map[string]string{
		"/dashboard":  "dashboard.html",
		"/pengujian":  "pengujian.html",
		"/config":     "config.html",
		"/daftar":     "daftar.html",
		"/histori":    "histori.html",
		"/foto":       "foto.html",
		"/user":       "user.html",
		"/index.html": "index.html",
	}

	for route, file := range pages {
		filename := file
		r.app.Get(route, authMW, func(c *fiber.Ctx) error {
			return c.SendFile("./" + filename)
		})
	}
}

func (r *Router) setupAPIRoutes() {
	api := r.app.Group("/api")

	// Auth endpoints
	api.Post("/login", r.authHandler.Login)
	api.Get("/logout", r.authHandler.Logout)
	api.Get("/check-auth", r.authHandler.CheckAuth)

	authMW := middleware.AuthMiddleware(r.authUC)
	api.Get("/user-info", authMW, r.authHandler.GetUserInfo)
	api.Post("/change-username", authMW, r.authHandler.ChangeUsername)
	api.Post("/change-password", authMW, r.authHandler.ChangePassword)

	// ESP32 endpoints
	esp32 := r.app.Group("/esp32")
	esp32.Get("/status", r.esp32Handler.GetStatus)
	esp32.Post("/inject", r.esp32Handler.Inject)
	esp32.Post("/stop", r.esp32Handler.Stop)
	esp32.Post("/set_amplitude", r.esp32Handler.SetAmplitude)
}

func (r *Router) healthCheck(c *fiber.Ctx) error {
	return c.JSON(fiber.Map{
		"status":         "ok",
		"mqtt_connected": r.mqttConnected(),
		"clients":        r.clientCount(),
	})
}
