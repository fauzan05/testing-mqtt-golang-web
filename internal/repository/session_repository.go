package repository

import (
	"fmt"
	"sync"
	"time"

	"myfiberapp/internal/domain"
)

// SessionRepository handles session storage
type SessionRepository struct {
	sessions map[string]domain.UserInfo
	mu       sync.Mutex
}

// NewSessionRepository creates a new session repository
func NewSessionRepository() *SessionRepository {
	return &SessionRepository{
		sessions: make(map[string]domain.UserInfo),
	}
}

// Create creates a new session
func (r *SessionRepository) Create(username string) string {
	sessionID := r.generateSessionID(username)

	r.mu.Lock()
	r.sessions[sessionID] = domain.UserInfo{
		Username:  username,
		LoginTime: time.Now().Format(time.RFC3339),
	}
	r.mu.Unlock()

	return sessionID
}

// Get retrieves session info
func (r *SessionRepository) Get(sessionID string) (domain.UserInfo, bool) {
	r.mu.Lock()
	defer r.mu.Unlock()

	userInfo, exists := r.sessions[sessionID]
	return userInfo, exists
}

// Update updates session info
func (r *SessionRepository) Update(sessionID string, userInfo domain.UserInfo) {
	r.mu.Lock()
	r.sessions[sessionID] = userInfo
	r.mu.Unlock()
}

// Delete removes a session
func (r *SessionRepository) Delete(sessionID string) {
	r.mu.Lock()
	delete(r.sessions, sessionID)
	r.mu.Unlock()
}

// IsValid checks if session exists
func (r *SessionRepository) IsValid(sessionID string) bool {
	r.mu.Lock()
	defer r.mu.Unlock()

	_, exists := r.sessions[sessionID]
	return exists
}

func (r *SessionRepository) generateSessionID(username string) string {
	return fmt.Sprintf("%s_%d", username, time.Now().Unix())
}
