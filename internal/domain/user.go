package domain

// User represents user credentials
type User struct {
	Username string
	Password string
}

// UserInfo contains logged-in user information
type UserInfo struct {
	Username  string `json:"username"`
	LoginTime string `json:"loginTime"`
}

// LoginRequest struct for login API
type LoginRequest struct {
	Username string `json:"username"`
	Password string `json:"password"`
}

// LoginResponse struct for login API response
type LoginResponse struct {
	Success bool   `json:"success"`
	Message string `json:"message"`
}

// ChangeUsernameRequest struct for changing username
type ChangeUsernameRequest struct {
	NewUsername string `json:"newUsername"`
	Password    string `json:"password"`
}

// ChangePasswordRequest struct for changing password
type ChangePasswordRequest struct {
	OldPassword string `json:"oldPassword"`
	NewPassword string `json:"newPassword"`
}
