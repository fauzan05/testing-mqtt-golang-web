package mqtt

import (
	"log"
	"time"

	"myfiberapp/internal/domain"
	"myfiberapp/pkg/config"

	mqtt "github.com/eclipse/paho.mqtt.golang"
)

// Client wraps the MQTT client
type Client struct {
	client    mqtt.Client
	config    config.MQTTConfig
	onMessage func(domain.Message)
}

// NewClient creates a new MQTT client
func NewClient(cfg config.MQTTConfig, onMessage func(domain.Message)) *Client {
	c := &Client{
		config:    cfg,
		onMessage: onMessage,
	}

	c.setup()
	return c
}

func (c *Client) setup() {
	log.Printf("Connecting to MQTT broker: %s", c.config.Broker)
	log.Printf("Client ID: %s", c.config.ClientID)
	log.Printf("Subscribing to topic: %s", c.config.Topic)

	opts := mqtt.NewClientOptions()
	opts.AddBroker(c.config.Broker)
	opts.SetClientID(c.config.ClientID)
	opts.SetDefaultPublishHandler(c.messageHandler)
	opts.OnConnect = c.connectHandler
	opts.OnConnectionLost = c.connectionLostHandler
	opts.SetAutoReconnect(true)
	opts.SetConnectRetry(true)
	opts.SetConnectRetryInterval(5 * time.Second)

	c.client = mqtt.NewClient(opts)
	if token := c.client.Connect(); token.Wait() && token.Error() != nil {
		log.Printf("Error connecting to MQTT broker: %v", token.Error())
		log.Println("Server will continue running, waiting for MQTT connection...")
	}

	// Subscribe to topic
	if token := c.client.Subscribe(c.config.Topic, 1, nil); token.Wait() && token.Error() != nil {
		log.Printf("Error subscribing to topic: %v", token.Error())
	} else {
		log.Printf("Successfully subscribed to topic: %s", c.config.Topic)
	}
}

func (c *Client) messageHandler(client mqtt.Client, msg mqtt.Message) {
	message := domain.Message{
		Topic:   msg.Topic(),
		Payload: string(msg.Payload()),
	}

	log.Printf("Received MQTT message - Topic: %s, Payload: %s\n", message.Topic, message.Payload)

	if c.onMessage != nil {
		c.onMessage(message)
	}
}

func (c *Client) connectHandler(client mqtt.Client) {
	log.Println("Connected to MQTT broker")
}

func (c *Client) connectionLostHandler(client mqtt.Client, err error) {
	log.Printf("Connection lost: %v", err)
}

// Publish publishes a message to a topic
func (c *Client) Publish(topic, message string) error {
	if c.client == nil || !c.client.IsConnected() {
		log.Printf("MQTT client not connected, cannot publish")
		return mqtt.ErrNotConnected
	}

	token := c.client.Publish(topic, 0, false, message)
	token.Wait()

	if token.Error() != nil {
		log.Printf("MQTT publish error: %v", token.Error())
		return token.Error()
	}

	log.Printf("MQTT published - Topic: %s, Message: %s", topic, message)
	return nil
}

// IsConnected checks if MQTT client is connected
func (c *Client) IsConnected() bool {
	return c.client != nil && c.client.IsConnected()
}

// Disconnect disconnects from MQTT broker
func (c *Client) Disconnect() {
	if c.client != nil {
		c.client.Disconnect(250)
	}
}
