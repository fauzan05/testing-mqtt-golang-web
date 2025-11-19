# Migrasi ke Clean Architecture

## File yang sudah dibuat:

### 1. Domain Layer
- ✅ `internal/domain/user.go` - User models dan DTOs
- ✅ `internal/domain/message.go` - MQTT message models
- ✅ `internal/domain/esp32.go` - ESP32 models

### 2. Repository Layer
- ✅ `internal/repository/auth_repository.go` - Auth data management
- ✅ `internal/repository/session_repository.go` - Session management

### 3. Use Case Layer
- ✅ `internal/usecase/auth_usecase.go` - Authentication business logic
- ✅ `internal/usecase/websocket_usecase.go` - WebSocket management
- ✅ `internal/usecase/esp32_usecase.go` - ESP32 device control

### 4. Delivery Layer
- ✅ `internal/delivery/http/auth_handler.go` - Auth API handlers
- ✅ `internal/delivery/http/esp32_handler.go` - ESP32 API handlers
- ✅ `internal/delivery/http/router.go` - Route configuration
- ✅ `internal/delivery/websocket/handler.go` - WebSocket handler

### 5. Middleware
- ✅ `internal/middleware/auth.go` - Authentication middleware

### 6. Shared Packages
- ✅ `pkg/config/config.go` - Configuration management
- ✅ `pkg/mqtt/client.go` - MQTT client wrapper

### 7. Main Application
- ✅ `cmd/server/main.go` - New entry point

## Cara Menyelesaikan Migrasi:

### Langkah 1: Jalankan Script Migrasi
```bash
chmod +x migrate.sh
./migrate.sh
```

### Langkah 2: Update Path di Router (Jika Perlu)
Jika file HTML/CSS/JS belum terpindah otomatis, update path di `internal/delivery/http/router.go`:

```go
// Ubah dari:
return c.SendFile("./web/templates/login.html")

// Menjadi (jika file masih di root):
return c.SendFile("./login.html")
```

### Langkah 3: Test Aplikasi
```bash
go run cmd/server/main.go
```

### Langkah 4: Verifikasi
Buka browser dan akses:
- http://localhost:8000
- Login dengan username: `admin`, password: `12345`
- Test semua fitur

## Perubahan Utama:

### Old Structure (Monolithic):
```
main.go (1000+ lines)
*.html (scattered)
*.css (scattered)
*.js (scattered)
```

### New Structure (Clean Architecture):
```
cmd/server/main.go (~60 lines)
internal/
  ├── domain/ (models)
  ├── repository/ (data access)
  ├── usecase/ (business logic)
  └── delivery/ (handlers)
pkg/
  ├── config/
  └── mqtt/
```

## Benefits:

1. **Separation of Concerns**: Setiap layer punya tanggung jawab jelas
2. **Testability**: Mudah untuk unit testing
3. **Maintainability**: Kode lebih mudah di-maintain
4. **Scalability**: Mudah untuk menambah fitur baru
5. **Reusability**: Package di `pkg/` bisa digunakan project lain

## Dependency Flow:

```
Delivery Layer (HTTP/WebSocket)
    ↓
Use Case Layer (Business Logic)
    ↓
Repository Layer (Data Access)
    ↓
Domain Layer (Models)
```

## Next Steps:

1. ✅ Struktur folder sudah dibuat
2. ✅ Semua layer sudah diimplementasi
3. ⏳ Pindahkan file static (HTML/CSS/JS)
4. ⏳ Pindahkan dokumentasi ke docs/
5. ⏳ Test semua endpoint
6. ⏳ Hapus main.go lama

## Troubleshooting:

### File tidak terpindah otomatis?
Pindahkan manual dengan:
```bash
# HTML files
mv *.html web/templates/

# CSS files
mv *.css web/static/css/

# JS files
mv *.js web/static/js/

# Images
mv *.png web/static/images/

# Docs
mv *.md docs/
mv README_NEW.md README.md
```

### Import error?
Pastikan semua import menggunakan `myfiberapp/` sebagai prefix:
```go
import (
    "myfiberapp/internal/domain"
    "myfiberapp/internal/usecase"
    // ...
)
```

### Cannot find package?
Run:
```bash
go mod tidy
```
