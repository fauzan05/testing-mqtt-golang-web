# Dockerfile untuk CORE Server
FROM golang:1.24-alpine AS builder

# Install dependencies
RUN apk add --no-cache git

WORKDIR /app

# Copy go mod files
COPY go.mod go.sum ./
RUN go mod download

# Copy source code
COPY . .

# Build application
RUN CGO_ENABLED=0 GOOS=linux go build -a -installsuffix cgo -o server cmd/server/main.go

# Final stage
FROM alpine:latest

RUN apk --no-cache add ca-certificates

WORKDIR /root/

# Copy binary from builder
COPY --from=builder /app/server .

# Copy web assets (jika sudah terorganisir)
# COPY --from=builder /app/web ./web

# Expose port
EXPOSE 8000

# Run
CMD ["./server"]
