package repository

import (
	"sync"

	"myfiberapp/internal/domain"
)

// AuthRepository handles user credentials
type AuthRepository struct {
	user domain.User
	mu   sync.Mutex
}

// NewAuthRepository creates a new auth repository with default credentials
func NewAuthRepository(username, password string) *AuthRepository {
	return &AuthRepository{
		user: domain.User{
			Username: username,
			Password: password,
		},
	}
}

// ValidateCredentials checks if credentials are valid
func (r *AuthRepository) ValidateCredentials(username, password string) bool {
	r.mu.Lock()
	defer r.mu.Unlock()

	return r.user.Username == username && r.user.Password == password
}

// UpdateUsername updates the username
func (r *AuthRepository) UpdateUsername(newUsername string) {
	r.mu.Lock()
	r.user.Username = newUsername
	r.mu.Unlock()
}

// UpdatePassword updates the password
func (r *AuthRepository) UpdatePassword(newPassword string) {
	r.mu.Lock()
	r.user.Password = newPassword
	r.mu.Unlock()
}

// GetUsername returns current username
func (r *AuthRepository) GetUsername() string {
	r.mu.Lock()
	defer r.mu.Unlock()

	return r.user.Username
}

// ValidatePassword checks if password is correct
func (r *AuthRepository) ValidatePassword(password string) bool {
	r.mu.Lock()
	defer r.mu.Unlock()

	return r.user.Password == password
}
