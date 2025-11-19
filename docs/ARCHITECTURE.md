# Clean Architecture Diagram

## Struktur Project

```
testing-mqtt-golang-web/
│
├── 📁 cmd/                          # Application Entry Points
│   └── server/
│       └── main.go                  # Main entry point (60 lines)
│
├── 📁 internal/                     # Private Application Code
│   │
│   ├── 📁 domain/                   # Enterprise Business Rules (Domain Layer)
│   │   ├── user.go                  # User entities & DTOs
│   │   ├── message.go               # Message models
│   │   └── esp32.go                 # ESP32 models
│   │
│   ├── 📁 repository/               # Interface Adapters (Data Layer)
│   │   ├── auth_repository.go       # User credentials storage
│   │   └── session_repository.go    # Session management
│   │
│   ├── 📁 usecase/                  # Application Business Rules (Use Case Layer)
│   │   ├── auth_usecase.go          # Authentication logic
│   │   ├── websocket_usecase.go     # WebSocket management
│   │   └── esp32_usecase.go         # ESP32 control logic
│   │
│   ├── 📁 delivery/                 # Frameworks & Drivers (Presentation Layer)
│   │   ├── http/
│   │   │   ├── auth_handler.go      # Auth API endpoints
│   │   │   ├── esp32_handler.go     # ESP32 API endpoints
│   │   │   └── router.go            # Route configuration
│   │   └── websocket/
│   │       └── handler.go           # WebSocket handler
│   │
│   └── 📁 middleware/               # Cross-Cutting Concerns
│       └── auth.go                  # Authentication middleware
│
├── 📁 pkg/                          # Public Shared Libraries
│   ├── config/
│   │   └── config.go                # Configuration management
│   └── mqtt/
│       └── client.go                # MQTT client wrapper
│
├── 📁 web/                          # Frontend Assets
│   ├── templates/                   # HTML templates
│   │   ├── login.html
│   │   ├── dashboard.html
│   │   └── ...
│   └── static/
│       ├── css/                     # Stylesheets
│       ├── js/                      # JavaScript files
│       └── images/                  # Images
│
├── 📁 docs/                         # Documentation
│   ├── MQTT_SETUP.md
│   ├── ESP32_UPLOAD_FIX.md
│   └── ...
│
├── 📁 esp32/                        # ESP32 Firmware
│   └── main.cpp
│
├── 📄 go.mod                        # Go module definition
├── 📄 go.sum                        # Dependency checksums
├── 📄 Makefile                      # Build automation
├── 📄 Dockerfile                    # Container definition
├── 📄 .gitignore                    # Git ignore rules
├── 📄 .env.example                  # Environment template
├── 📄 migrate.sh                    # Migration script
└── 📄 README.md                     # Project documentation
```

## Layer Dependencies (Clean Architecture)

```
┌─────────────────────────────────────────────────────────────┐
│                     DELIVERY LAYER                          │
│  (HTTP Handlers, WebSocket, Router, Middleware)             │
│  • auth_handler.go, esp32_handler.go, router.go            │
│  • websocket/handler.go                                     │
└─────────────────────┬───────────────────────────────────────┘
                      │ depends on
                      ↓
┌─────────────────────────────────────────────────────────────┐
│                     USE CASE LAYER                          │
│  (Business Logic, Application Rules)                        │
│  • auth_usecase.go, websocket_usecase.go                   │
│  • esp32_usecase.go                                         │
└─────────────────────┬───────────────────────────────────────┘
                      │ depends on
                      ↓
┌─────────────────────────────────────────────────────────────┐
│                   REPOSITORY LAYER                          │
│  (Data Access, Storage Interface)                           │
│  • auth_repository.go, session_repository.go               │
└─────────────────────┬───────────────────────────────────────┘
                      │ depends on
                      ↓
┌─────────────────────────────────────────────────────────────┐
│                     DOMAIN LAYER                            │
│  (Entities, Business Models - NO DEPENDENCIES)              │
│  • user.go, message.go, esp32.go                           │
└─────────────────────────────────────────────────────────────┘

    ┌────────────────────────────────────────┐
    │         SHARED PACKAGES (pkg/)         │
    │  • config/  (used by all layers)       │
    │  • mqtt/    (used by use case layer)   │
    └────────────────────────────────────────┘
```

## Request Flow

```
┌──────────┐
│ Browser  │
└────┬─────┘
     │ HTTP Request
     ↓
┌─────────────────────────┐
│  Router (router.go)     │ ← Delivery Layer
│  • Route matching       │
│  • Middleware chain     │
└────┬────────────────────┘
     │
     ↓
┌─────────────────────────┐
│  Middleware             │
│  • Auth check           │
│  • Validation           │
└────┬────────────────────┘
     │
     ↓
┌─────────────────────────┐
│  Handler                │ ← Delivery Layer
│  • auth_handler.go      │
│  • esp32_handler.go     │
│  • Parse request        │
└────┬────────────────────┘
     │
     ↓
┌─────────────────────────┐
│  Use Case               │ ← Use Case Layer
│  • Business logic       │
│  • Validation rules     │
│  • Orchestration        │
└────┬────────────────────┘
     │
     ↓
┌─────────────────────────┐
│  Repository             │ ← Repository Layer
│  • Data access          │
│  • Storage operations   │
└────┬────────────────────┘
     │
     ↓
┌─────────────────────────┐
│  Domain                 │ ← Domain Layer
│  • Entities             │
│  • Business rules       │
└─────────────────────────┘
```

## WebSocket Flow

```
┌──────────┐
│  Client  │
└────┬─────┘
     │ WebSocket Connection
     ↓
┌──────────────────────────────┐
│  WebSocket Handler           │ ← Delivery/WebSocket
│  • Connection management     │
│  • Message routing           │
└────┬─────────────────┬───────┘
     │                 │
     ↓                 ↓
┌─────────────┐   ┌──────────────┐
│  WebSocket  │   │  MQTT Client │ ← pkg/mqtt
│  Use Case   │   │  • Subscribe │
│  • Broadcast│   │  • Publish   │
└─────────────┘   └──────────────┘
```

## File Organization Pattern

```
Each layer follows similar pattern:

internal/
  ├── domain/
  │   ├── user.go        # User domain
  │   ├── message.go     # Message domain
  │   └── esp32.go       # ESP32 domain
  │
  ├── repository/
  │   ├── auth_repository.go      # Auth data access
  │   └── session_repository.go   # Session data access
  │
  ├── usecase/
  │   ├── auth_usecase.go         # Auth business logic
  │   ├── websocket_usecase.go    # WebSocket logic
  │   └── esp32_usecase.go        # ESP32 logic
  │
  └── delivery/
      ├── http/
      │   ├── auth_handler.go     # Auth endpoints
      │   ├── esp32_handler.go    # ESP32 endpoints
      │   └── router.go           # Routes config
      └── websocket/
          └── handler.go          # WebSocket endpoints
```

## Naming Conventions

```
Domain:      user.go, message.go
Repository:  *_repository.go
Use Case:    *_usecase.go
Handler:     *_handler.go
Middleware:  *.go (descriptive name)
Package:     Folder name
```

## Import Pattern

```go
// cmd/server/main.go
import (
    "myfiberapp/internal/delivery/http"
    "myfiberapp/internal/delivery/websocket"
    "myfiberapp/internal/repository"
    "myfiberapp/internal/usecase"
    "myfiberapp/pkg/config"
    "myfiberapp/pkg/mqtt"
)

// internal/delivery/http/auth_handler.go
import (
    "myfiberapp/internal/domain"
    "myfiberapp/internal/usecase"
)

// internal/usecase/auth_usecase.go
import (
    "myfiberapp/internal/domain"
    "myfiberapp/internal/repository"
)

// internal/repository/auth_repository.go
import (
    "myfiberapp/internal/domain"
)
```

## Key Principles

1. **Dependency Inversion**: High-level modules don't depend on low-level modules
2. **Single Responsibility**: Each file/package has one clear purpose
3. **Interface Segregation**: Use interfaces where needed
4. **Separation of Concerns**: Each layer has distinct responsibility
5. **Testability**: Easy to mock and test each component

## Benefits Achieved

✅ **Modularity**: Easy to add/remove features
✅ **Testability**: Each layer can be tested independently
✅ **Maintainability**: Clear structure, easy to navigate
✅ **Scalability**: Easy to scale specific components
✅ **Flexibility**: Easy to swap implementations
✅ **Team Collaboration**: Clear boundaries for team members
