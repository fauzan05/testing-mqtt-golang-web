package http

import (
	"myfiberapp/internal/domain"
	"myfiberapp/internal/usecase"

	"github.com/gofiber/fiber/v2"
)

// AuthHandler handles authentication endpoints
type AuthHandler struct {
	authUC *usecase.AuthUseCase
}

// NewAuthHandler creates a new auth handler
func NewAuthHandler(authUC *usecase.AuthUseCase) *AuthHandler {
	return &AuthHandler{
		authUC: authUC,
	}
}

// Login handles user login
func (h *AuthHandler) Login(c *fiber.Ctx) error {
	var req domain.LoginRequest
	if err := c.BodyParser(&req); err != nil {
		return c.Status(400).JSON(domain.LoginResponse{
			Success: false,
			Message: "Invalid request",
		})
	}

	sessionID, err := h.authUC.Login(req)
	if err != nil {
		return c.Status(401).JSON(domain.LoginResponse{
			Success: false,
			Message: err.Error(),
		})
	}

	// Set cookie
	c.Cookie(h.authUC.CreateSessionCookie(sessionID))

	return c.JSON(domain.LoginResponse{
		Success: true,
		Message: "Login successful",
	})
}

// Logout handles user logout
func (h *AuthHandler) Logout(c *fiber.Ctx) error {
	sessionID := c.Cookies("session_id")
	h.authUC.Logout(sessionID)

	// Clear cookie
	c.Cookie(h.authUC.ClearSessionCookie())

	return c.Redirect("/login")
}

// CheckAuth checks if user is authenticated
func (h *AuthHandler) CheckAuth(c *fiber.Ctx) error {
	sessionID := c.Cookies("session_id")

	if !h.authUC.ValidateSession(sessionID) {
		return c.Status(401).JSON(fiber.Map{
			"authenticated": false,
		})
	}

	return c.JSON(fiber.Map{
		"authenticated": true,
	})
}

// GetUserInfo retrieves user information
func (h *AuthHandler) GetUserInfo(c *fiber.Ctx) error {
	sessionID := c.Cookies("session_id")

	userInfo, exists := h.authUC.GetUserInfo(sessionID)
	if !exists {
		return c.Status(401).JSON(fiber.Map{
			"error": "Session not found",
		})
	}

	return c.JSON(userInfo)
}

// ChangeUsername changes user's username
func (h *AuthHandler) ChangeUsername(c *fiber.Ctx) error {
	var req domain.ChangeUsernameRequest
	if err := c.BodyParser(&req); err != nil {
		return c.Status(400).JSON(domain.LoginResponse{
			Success: false,
			Message: "Invalid request",
		})
	}

	sessionID := c.Cookies("session_id")

	if err := h.authUC.ChangeUsername(sessionID, req); err != nil {
		return c.Status(401).JSON(domain.LoginResponse{
			Success: false,
			Message: err.Error(),
		})
	}

	return c.JSON(domain.LoginResponse{
		Success: true,
		Message: "Username berhasil diubah",
	})
}

// ChangePassword changes user's password
func (h *AuthHandler) ChangePassword(c *fiber.Ctx) error {
	var req domain.ChangePasswordRequest
	if err := c.BodyParser(&req); err != nil {
		return c.Status(400).JSON(domain.LoginResponse{
			Success: false,
			Message: "Invalid request",
		})
	}

	if err := h.authUC.ChangePassword(req); err != nil {
		return c.Status(401).JSON(domain.LoginResponse{
			Success: false,
			Message: err.Error(),
		})
	}

	return c.JSON(domain.LoginResponse{
		Success: true,
		Message: "Password berhasil diubah",
	})
}
