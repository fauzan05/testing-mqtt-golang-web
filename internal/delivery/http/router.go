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
	// Common CSS files (shared across multiple pages)
	commonCSSFiles := map[string]string{
		"styles.css":  "./web/static/css/common/styles.css",
		"sidebar.css": "./web/static/css/common/sidebar.css",
		"form.css":    "./web/static/css/common/form.css",
	}
	for route, path := range commonCSSFiles {
		filePath := path
		r.app.Get("/css/"+route, func(c *fiber.Ctx) error {
			c.Set("Cache-Control", "no-cache, no-store, must-revalidate")
			c.Set("Pragma", "no-cache")
			c.Set("Expires", "0")
			c.Set("Content-Type", "text/css")
			return c.SendFile(filePath)
		})
	}

	// Page-specific CSS files
	pageCSSFiles := map[string]string{
		"data-conductive.css": "./web/static/css/pages/data-conductive/data-conductive.css",
	}
	for route, path := range pageCSSFiles {
		filePath := path
		r.app.Get("/css/"+route, func(c *fiber.Ctx) error {
			c.Set("Cache-Control", "no-cache, no-store, must-revalidate")
			c.Set("Pragma", "no-cache")
			c.Set("Expires", "0")
			c.Set("Content-Type", "text/css")
			return c.SendFile(filePath)
		})
	}

	// Common JS files (shared across multiple pages)
	commonJSFiles := map[string]string{
		"sidebar.js":    "./web/static/js/common/sidebar.js",
		"auth-check.js": "./web/static/js/common/auth-check.js",
	}
	for route, path := range commonJSFiles {
		filePath := path
		r.app.Get("/js/"+route, func(c *fiber.Ctx) error {
			c.Set("Cache-Control", "no-cache, no-store, must-revalidate")
			c.Set("Pragma", "no-cache")
			c.Set("Expires", "0")
			c.Set("Content-Type", "application/javascript")
			return c.SendFile(filePath)
		})
	}

	// Page-specific JS files
	pageJSFiles := map[string]string{
		"dashboard.js":       "./web/static/js/pages/dashboard/dashboard.js",
		"data-conductive.js": "./web/static/js/pages/data-conductive/data-conductive.js",
		"history.js":         "./web/static/js/pages/history/history.js",
		"user.js":            "./web/static/js/pages/user/user.js",
		"login.js":           "./web/static/js/pages/auth/login.js",
	}
	for route, path := range pageJSFiles {
		filePath := path
		r.app.Get("/js/"+route, func(c *fiber.Ctx) error {
			c.Set("Cache-Control", "no-cache, no-store, must-revalidate")
			c.Set("Pragma", "no-cache")
			c.Set("Expires", "0")
			c.Set("Content-Type", "application/javascript")
			return c.SendFile(filePath)
		})
	}

	// Serve HTML fragments with no-cache (for includes/partials)
	htmlFiles := []string{"sidebar.html", "header.html", "footer.html", "auth-header.html", "auth-footer.html"}
	for _, file := range htmlFiles {
		filename := file
		r.app.Get("/"+filename, func(c *fiber.Ctx) error {
			c.Set("Cache-Control", "no-cache, no-store, must-revalidate")
			c.Set("Pragma", "no-cache")
			c.Set("Expires", "0")
			return c.SendFile("./web/templates/" + filename)
		})
	}

	// Serve image files
	imageFiles := []string{"foto_conductive.png", "Logo_PLN.png", "logo-1.png"}
	for _, file := range imageFiles {
		filename := file
		r.app.Get("/images/"+filename, func(c *fiber.Ctx) error {
			c.Set("Cache-Control", "public, max-age=31536000") // Cache images for 1 year
			return c.SendFile("./web/static/images/" + filename)
		})
	}

	// Serve images folder (fallback for any other images)
	r.app.Static("/images", "./web/static/images", fiber.Static{
		Browse: false,
	})

	// Serve static files from web/static
	r.app.Static("/static", "./web/static", fiber.Static{
		Browse: false,
	})
}

func (r *Router) setupPublicRoutes() {
	r.app.Get("/login", func(c *fiber.Ctx) error {
		return c.SendFile("./web/templates/login.html")
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
		"/dashboard":       "dashboard.html",
		"/data-conductive": "data-conductive.html",
		"/history":         "history.html",
		"/user":            "user.html",
	}

	for route, file := range pages {
		filename := file
		r.app.Get(route, authMW, func(c *fiber.Ctx) error {
			return c.SendFile("./web/templates/" + filename)
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
