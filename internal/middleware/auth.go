package middleware

import (
	"myfiberapp/internal/usecase"

	"github.com/gofiber/fiber/v2"
)

// AuthMiddleware creates a middleware for authentication
func AuthMiddleware(authUC *usecase.AuthUseCase) fiber.Handler {
	return func(c *fiber.Ctx) error {
		sessionID := c.Cookies("session_id")

		if !authUC.ValidateSession(sessionID) {
			return c.Redirect("/login")
		}

		return c.Next()
	}
}
