package usecase

import (
	"time"

	"myfiberapp/internal/domain"
	"myfiberapp/internal/repository"

	"github.com/gofiber/fiber/v2"
)

// AuthUseCase handles authentication business logic
type AuthUseCase struct {
	authRepo    *repository.AuthRepository
	sessionRepo *repository.SessionRepository
}

// NewAuthUseCase creates a new auth use case
func NewAuthUseCase(authRepo *repository.AuthRepository, sessionRepo *repository.SessionRepository) *AuthUseCase {
	return &AuthUseCase{
		authRepo:    authRepo,
		sessionRepo: sessionRepo,
	}
}

// Login authenticates user and creates session
func (uc *AuthUseCase) Login(req domain.LoginRequest) (string, error) {
	if !uc.authRepo.ValidateCredentials(req.Username, req.Password) {
		return "", fiber.NewError(fiber.StatusUnauthorized, "Username atau password salah")
	}

	sessionID := uc.sessionRepo.Create(req.Username)
	return sessionID, nil
}

// Logout removes user session
func (uc *AuthUseCase) Logout(sessionID string) {
	uc.sessionRepo.Delete(sessionID)
}

// ValidateSession checks if session is valid
func (uc *AuthUseCase) ValidateSession(sessionID string) bool {
	return uc.sessionRepo.IsValid(sessionID)
}

// GetUserInfo retrieves user info from session
func (uc *AuthUseCase) GetUserInfo(sessionID string) (domain.UserInfo, bool) {
	return uc.sessionRepo.Get(sessionID)
}

// ChangeUsername changes user's username
func (uc *AuthUseCase) ChangeUsername(sessionID string, req domain.ChangeUsernameRequest) error {
	if !uc.authRepo.ValidatePassword(req.Password) {
		return fiber.NewError(fiber.StatusUnauthorized, "Password salah")
	}

	uc.authRepo.UpdateUsername(req.NewUsername)

	// Update session info
	if userInfo, exists := uc.sessionRepo.Get(sessionID); exists {
		userInfo.Username = req.NewUsername
		uc.sessionRepo.Update(sessionID, userInfo)
	}

	return nil
}

// ChangePassword changes user's password
func (uc *AuthUseCase) ChangePassword(req domain.ChangePasswordRequest) error {
	if !uc.authRepo.ValidatePassword(req.OldPassword) {
		return fiber.NewError(fiber.StatusUnauthorized, "Password lama salah")
	}

	uc.authRepo.UpdatePassword(req.NewPassword)
	return nil
}

// CreateSessionCookie creates a session cookie
func (uc *AuthUseCase) CreateSessionCookie(sessionID string) *fiber.Cookie {
	return &fiber.Cookie{
		Name:     "session_id",
		Value:    sessionID,
		Expires:  time.Now().Add(24 * time.Hour),
		HTTPOnly: true,
		SameSite: "Lax",
	}
}

// ClearSessionCookie clears the session cookie
func (uc *AuthUseCase) ClearSessionCookie() *fiber.Cookie {
	return &fiber.Cookie{
		Name:     "session_id",
		Value:    "",
		Expires:  time.Now().Add(-1 * time.Hour),
		HTTPOnly: true,
	}
}
