#include <Arduino.h>
#include <TFT_eSPI.h>
#include <ezButton.h>
#include <ModbusMaster.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>

// ------------------- TFT Setup -------------------
TFT_eSPI tft = TFT_eSPI();  // ST7789 240x240

// ------------------- Tombol -------------------
ezButton keyUp(26);
// ezButton keyDown(35);

// ------------------- JSY1050 Setup -------------------
#define RXD_JSY 16
#define TXD_JSY 17
HardwareSerial SerialJSY(2);
ModbusMaster node;

// ------------------- LED & Relay -------------------
#define LED_RUNTIME 13
#define LED_STOP    27
#define LED_RUN     14
#define RELAY_PIN   32
// Rotary Encoder Pinout pada ESP32
#define CLK_PIN 21   // Pilih GPIO 21
#define DT_PIN  19   // Pilih GPIO 19
#define SW_PIN  5    // Pilih GPIO 5
#define WEB_SERVER_SWITCH 22  // Web server mode switch (active low)

// DAC Pin
#define DAC_PIN 25   // GPIO 25 untuk DAC output

 // ------------------- WiFi & Web Server -------------------
const char* ap_ssid = "CORE Test";      // Access Point SSID
const char* ap_password = "12345678";     // Access Point Password (8+ chars)
WebServer server(80);
bool webServerMode = false;

// DNS server untuk memetakan nama domain lokal (mis. core.local) ke IP AP (192.168.4.1)
const byte DNS_PORT = 53;
DNSServer dnsServer;

// WiFi Configuration to avoid interference
bool dualWiFiMode = true;  // Flag for dual WiFi operation (AP + Station)
IPAddress localIP(192, 168, 4, 1);  // Fixed IP for AP mode
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

// WiFi Settings
String wifiSSID = "PDKB_INTERNET_G";
String wifiPassword = "uptpulogadung";
String cloudServerAddress = "https://api.example.com/submit-data";

// MQTT Settings
String mqttHost = "vps.domain.com";
int mqttPort = 1883;
String mqttUser = "esp1";
String mqttPass = "password";
String mqttClientId = "esp32_01";
String mqttTopic = "sensor/esp32";

// WiFi Connection Status
bool wifiConnected = false;
bool wifiStationMode = false;

// ---------------- STARFIELD CONFIG ----------------
#define NSTARS 1024
uint8_t sx[NSTARS], sy[NSTARS], sz[NSTARS];
uint8_t za, zb, zc, zx;
inline uint8_t rng8() {
  zx++;
  za = (za ^ zc ^ zx);
  zb = (zb + za);
  zc = ((zc + (zb >> 1)) ^ za);
  return zc;
}

// ---------------- UI COLORS & Layout ----------------
float ampValue = 0.0;        // amplitude awal 0%
float stepSize = 0.01;       // langkah 1%
int lastCLK = HIGH;
#define BG_COLOR TFT_BLACK // black background
#define COLOR_V TFT_RED // red for voltage
#define COLOR_I TFT_GREEN // green for current
#define COLOR_R TFT_CYAN // sky blue for resistance
#define STATUS_RUN TFT_GREEN // green for run
#define STATUS_READY TFT_CYAN // blue sky for ready
#define STATUS_HOLD tft.color565(128, 128, 128) // gray for hold
#define OUTLINE TFT_WHITE // white outline
const int WIDTH = 240;
const int HEIGHT = 240;
int lastSecond = -1;
// ------------------- Variabel -------------------
float voltage = 0.0f, currentA = 0.0f, resistanceVal = 0.0f;
unsigned long lastRead = 0;
const unsigned long READ_INTERVAL = 500; // ms

enum State { READY, RUN, STOPPED };
State systemState = STOPPED;  // posisi awal STOPPED

enum MenuItem { MENU_RUNTIME = 0, MENU_RUN = 1, MENU_STOP = 2 };
MenuItem currentMenu = MENU_STOP;  // posisi awal di STOP
const int MENU_COUNT = 3;

unsigned long countdownStart = 0;
unsigned long countdownDuration = 0;
bool countdownActive = false;

 // 200mA Injection Variables
bool autoInjectionMode = false;
bool targetReached = false;
// JSY current is in Amperes, so 200mA = 0.2A
float targetCurrent = 0.2f; // 200mA
float autoIncrementStep = 0.01f; // 1% increment (in amplitude units)
unsigned long autoIncrementDelay = 5000; // 5 seconds between increments
unsigned long lastAutoIncrement = 0; // Track last auto increment time
// User-configurable countdown for special test mode (milliseconds)
unsigned long userCountdownDuration = 2 * 60 * 1000UL; // default 120s

// Time-based lamp variables
bool lampTimeBased = false;
unsigned long lampStartTime = 0;
unsigned long lampDuration = 0; // in milliseconds
bool lampState = false;
unsigned long lastLampToggle = 0;
const unsigned long LAMP_BLINK_INTERVAL = 1000; // Blink every 1 second

// Data submission form variables
String uptValue = "";
String namaValue = "";
String kodeConductiveValue = "";
bool dataSubmitted = false;
String lastSubmissionStatus = "";

// ------------------- Layout -------------------
#define X_LABEL  20
#define X_VALUE  180
#define Y_V      55
#define Y_I      95
void handleRotaryEncoder();
void updateDAC();
#define Y_R      140
#define Y_TIME   185
#define Y_STATUS 215

// ------------------- Prototypes -------------------
void drawHeader();
void drawFrame();
void updateValues(float v, float i, float r);
void updateTime();
void updateStatus();
void executeMenu(MenuItem menu);
void readJSY1050();
void handleButtons();
void updateLEDsAndRelay();
void starfieldIntro();
void handleWebServer();
void handleRoot();
void handleSetAmplitude();
void handleGetStatus();
void checkWebServerSwitch();
void displayWebServerMode();
void refreshWebServerDisplay();
void handleSettings();
void handleMainMenu();
void handleInjection();
void handleInjection();
void handleDataSubmission();
void connectToWiFi();
void handleWiFiSettings();
void handleCloudSettings();
void handleWiFiReset();
void handleAutoInjection();
void sendDataToCloud();
void handleInjectAPI();
void handleStopAPI();
void loadSettingsFromMemory();
void saveSettingsToMemory(const String& ssid, const String& password, const String& cloudServer);
void resetWiFiSettings();
void stopAutoInjection();
void handleWiFiInterference();
void updateTimeBasedLamp();
bool sendDataToCloudServer(const String& jsonData, String& response);
void handleInjection();

// ------------------- STARFIELD INTRO -------------------
void starfieldIntro() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextFont(2);
  tft.drawCentreString("CORE Test", 120, 100, 4);
  for (int k = 0; k < 160; k++) {
    uint8_t spawn = 255 - (k % 200);
    for (int i = 0; i < NSTARS; i++) {
      if (sz[i] <= 1) {
        sx[i] = (120 - 80) + rng8();
        sy[i] = rng8(); // 0..255
        sz[i] = spawn--;
      } else {
        int oldx = ((int)sx[i] - 120) * 256 / (sz[i] < 1 ? 1 : sz[i]) + 120;
        int oldy = ((int)sy[i] - 120) * 256 / (sz[i] < 1 ? 1 : sz[i]) + 120;
        if (oldx >= 0 && oldy >= 0 && oldx < WIDTH && oldy < HEIGHT)
          tft.drawPixel(oldx, oldy, TFT_BLACK);
        sz[i] -= 2;
        if (sz[i] > 1) {
          int x = ((int)sx[i] - 120) * 256 / (sz[i] < 1 ? 1 : sz[i]) + 120;
          int y = ((int)sy[i] - 120) * 256 / (sz[i] < 1 ? 1 : sz[i]) + 120;
          if (x >= 0 && y >= 0 && x < WIDTH && y < HEIGHT) {
            uint8_t c = 255 - sz[i];
            tft.drawPixel(x, y, tft.color565(c, c, c));
          } else sz[i] = 0;
        }
      }
    }
    delay(12);
  }
  delay(200);
  tft.fillScreen(BG_COLOR);
}

void checkWebServerSwitch() {
  static bool lastSwitchState = HIGH;
  static unsigned long lastDebounceTime = 0;
  static unsigned long lastDebugTime = 0;
  static bool switchHandled = false; // Track if switch press has been handled
  
  bool currentSwitchState = digitalRead(WEB_SERVER_SWITCH);
  
  // Debounce switch (ignore rapid state changes)
  if (currentSwitchState != lastSwitchState) {
    lastDebounceTime = millis();
    switchHandled = false; // Reset when state changes
  }
  
  // Debug output every 2 seconds
  if (millis() - lastDebugTime > 2000) {
    Serial.print("Switch pin 22 state: ");
    Serial.println(currentSwitchState ? "HIGH" : "LOW");
    Serial.print("Web server mode: ");
    Serial.println(webServerMode ? "ON" : "OFF");
    lastDebugTime = millis();
  }
  
  // Only process state change after debounce period (50ms)
  if ((millis() - lastDebounceTime) > 50 && !switchHandled) {
    // Handle switch press (LOW state)
    if (currentSwitchState == LOW) {
      Serial.println("Switch pressed! Activating web server mode...");
      webServerMode = true;
      Serial.print("Web server mode: ");
      Serial.println(webServerMode ? "ON" : "OFF");
      
      // Reset WiFi settings to default PDKB_INTERNET_G
      Serial.println("Resetting WiFi to PDKB_INTERNET_G...");
      resetWiFiSettings();
      
      // Enter web server mode - Create Access Point
      Serial.println("Starting Access Point...");
      
      // Configure WiFi with dual mode (AP + STA) - support both connections
      WiFi.mode(WIFI_AP_STA);
      WiFi.softAPConfig(localIP, gateway, subnet);
      WiFi.softAP(ap_ssid, ap_password);

      // Mulai DNS server: arahkan semua domain (termasuk core.local) ke IP AP 192.168.4.1
      dnsServer.start(DNS_PORT, "*", localIP);
      
      IPAddress IP = WiFi.softAPIP();
      Serial.print("✅ Access Point started! AP IP: ");
      Serial.println(IP);
      
      // Connect to router WiFi (Station mode) if credentials available
      if (dualWiFiMode && wifiSSID.length() > 0) {
        Serial.println("🌐 Connecting to router WiFi: " + wifiSSID);
        Serial.print("   SSID: ");
        Serial.println(wifiSSID);
        Serial.print("   Password: ");
        Serial.println(wifiPassword);
        
        // Disconnect first to ensure clean state
        WiFi.disconnect();
        delay(100);
        
        // Begin connection
        WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
        
        // Wait up to 20 seconds for connection (increased timeout)
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 40) {
          delay(500);
          Serial.print(".");
          attempts++;
          
          // Show WiFi status for debugging
          if (attempts % 10 == 0) {
            Serial.println();
            Serial.print("   WiFi Status: ");
            Serial.println(WiFi.status());
          }
        }
        
        if (WiFi.status() == WL_CONNECTED) {
          wifiConnected = true;
          wifiStationMode = true;
          IPAddress stationIP = WiFi.localIP();
          Serial.println();
          Serial.print("✅ Connected to router! Station IP: ");
          Serial.println(stationIP);
          Serial.println("📡 ESP32 now accessible from 2 networks:");
          Serial.println("   1. AP Mode: " + IP.toString() + " (Direct WiFi: CORE Test)");
          Serial.println("   2. Station Mode: " + stationIP.toString() + " (Router: " + wifiSSID + ")");
        } else {
          wifiConnected = false;
          Serial.println();
          Serial.println("⚠️ Failed to connect to router WiFi");
          Serial.print("   Final WiFi Status: ");
          Serial.println(WiFi.status());
          Serial.println("   Possible reasons:");
          Serial.println("   - Wrong password");
          Serial.println("   - Router too far / weak signal");
          Serial.println("   - Router MAC filtering enabled");
          Serial.println("   AP Mode still active: " + IP.toString());
        }
      }
      
      // Start web server
      server.on("/", handleRoot);
      server.on("/main", handleMainMenu);
      server.on("/settings", handleSettings);
      server.on("/api/wifi", handleWiFiSettings);
      server.on("/api/wifi-reset", handleWiFiReset);
      server.on("/api/cloud", handleCloudSettings);
      server.on("/api/auto-injection", handleAutoInjection);
      server.on("/api/submit-data", sendDataToCloud);
      server.on("/status", handleGetStatus);
      server.on("/set_amplitude", handleSetAmplitude);
      server.on("/api/inject", HTTP_POST, handleInjectAPI);
      server.on("/api/stop", HTTP_POST, handleStopAPI);
      server.begin();
      Serial.println("Web server started");
      
      displayWebServerMode();
      switchHandled = true;
    }
    // Handle switch release (HIGH state)
    else if (currentSwitchState == HIGH && webServerMode) {
      Serial.println("Switch released! Deactivating web server mode...");
      webServerMode = false;
      Serial.print("Web server mode: ");
      Serial.println(webServerMode ? "ON" : "OFF");
      
      // Exit web server mode
      Serial.println("Stopping web server...");
      server.stop();
      dnsServer.stop();
      WiFi.softAPdisconnect(true);
      Serial.println("Web server stopped");
      
      // Redraw normal interface
      drawFrame();
      updateValues(voltage, currentA, resistanceVal);
      updateTime();
      updateStatus();
      switchHandled = true;
    }
  }
  
  lastSwitchState = currentSwitchState;
}

void handleWebServer() {
  if (webServerMode) {
    dnsServer.processNextRequest();
    server.handleClient();
  }
}

void handleRoot() {
  // Stop auto injection when returning to injection dashboard
  if (autoInjectionMode) {
    stopAutoInjection();
  }

  String html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>CORE Test - Conductive Suite Resistance Evaluator</title>
    <meta charset='UTF-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        
        @keyframes slideIn {
            from { opacity: 0; transform: translateY(20px); }
            to { opacity: 1; transform: translateY(0); }
        }
        
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            background: linear-gradient(135deg, #06b6d4 0%, #14b8a6 100%);
            min-height: 100vh;
            padding: 20px;
            color: #1a202c;
        }
        
        .container { max-width: 1100px; margin: 0 auto; animation: slideIn 0.5s; }
        
        .header {
            background: white;
            border-radius: 20px;
            padding: 24px;
            margin-bottom: 20px;
            display: flex;
            justify-content: space-between;
            align-items: center;
            box-shadow: 0 4px 20px rgba(0,0,0,0.1);
        }
        
        .header h1 {
            font-size: 28px;
            font-weight: 700;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
        }
        
        .settings-btn {
            padding: 10px 20px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            border: none;
            border-radius: 10px;
            color: white;
            text-decoration: none;
            font-weight: 600;
            transition: transform 0.2s;
        }
        
        .settings-btn:hover { transform: translateY(-2px); }
        
        .wifi-status {
            display: inline-flex;
            align-items: center;
            gap: 8px;
            padding: 10px 18px;
            border-radius: 10px;
            font-size: 13px;
            font-weight: 600;
            margin-bottom: 20px;
        }
        
        .wifi-connected { background: #10b981; color: white; }
        .wifi-disconnected { background: #ef4444; color: white; }
        
        .gauges-grid {
            display: grid;
            grid-template-columns: repeat(4, 1fr);
            gap: 20px;
            margin-bottom: 20px;
            max-width: 1000px;
            margin-left: auto;
            margin-right: auto;
        }
        
        .gauge-card {
            background: white;
            border-radius: 16px;
            padding: 20px;
            text-align: center;
            box-shadow: 0 4px 15px rgba(0,0,0,0.08);
            transition: transform 0.2s;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
        }
        
        .gauge-card:hover { transform: translateY(-4px); box-shadow: 0 8px 25px rgba(0,0,0,0.12); }
        
        .gauge {
            width: 120px;
            height: 120px;
            margin: 0 auto 12px;
            position: relative;
            display: flex;
            align-items: center;
            justify-content: center;
        }
        
        .gauge svg { transform: rotate(-90deg); }
        
        .gauge-bg {
            fill: none;
            stroke: #e5e7eb;
            stroke-width: 10;
        }
        
        .gauge-fill {
            fill: none;
            stroke-width: 10;
            stroke-linecap: round;
            transition: stroke-dashoffset 0.5s ease;
        }
        
        .gauge-value {
            position: absolute;
            top: 50%;
            left: 50%;
            transform: translate(-50%, -50%);
            font-size: 18px;
            font-weight: 700;
            color: #1a202c;
        }
        
        .gauge-label {
            font-size: 12px;
            color: #6b7280;
            font-weight: 600;
            text-transform: uppercase;
            letter-spacing: 0.5px;
        }
        
        .control-section {
            background: white;
            border-radius: 16px;
            padding: 20px;
            margin-bottom: 16px;
            box-shadow: 0 4px 15px rgba(0,0,0,0.08);
            max-width: 1000px;
            margin-left: auto;
            margin-right: auto;
        }
        
        .section-title {
            font-size: 17px;
            font-weight: 700;
            margin-bottom: 16px;
            color: #1a202c;
        }
        
        .form-group { margin-bottom: 16px; }
        
        label {
            display: block;
            margin-bottom: 6px;
            font-size: 13px;
            font-weight: 600;
            color: #374151;
        }
        
        select, input {
            width: 100%;
            padding: 12px;
            background: #f9fafb;
            border: 2px solid #e5e7eb;
            border-radius: 10px;
            color: #1a202c;
            font-size: 14px;
            transition: all 0.2s;
        }
        
        select:focus, input:focus {
            outline: none;
            border-color: #667eea;
            background: white;
        }
        
        .control-card {
            background: #f9fafb;
            border-radius: 12px;
            padding: 20px;
            margin: 16px 0;
        }
        
        .numeric-control {
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 12px;
            margin: 16px 0;
        }
        
        .numeric-btn {
            width: 48px;
            height: 48px;
            border-radius: 12px;
            border: none;
            font-size: 22px;
            font-weight: 700;
            color: white;
            cursor: pointer;
            transition: all 0.2s;
        }
        
        .numeric-btn.dec { background: #ef4444; }
        .numeric-btn.inc { background: #3b82f6; }
        .numeric-btn:hover { transform: scale(1.05); }
        .numeric-btn:active { transform: scale(0.95); }
        
        .numeric-value {
            width: 90px;
            text-align: center;
            padding: 12px;
            font-size: 22px;
            font-weight: 700;
            background: white;
            border: 2px solid #667eea;
            border-radius: 10px;
            color: #667eea;
        }
        
        .slider-container { margin: 16px 0; }
        
        input[type="range"] {
            width: 100%;
            height: 6px;
            background: #e5e7eb;
            border-radius: 3px;
            outline: none;
        }
        
        input[type="range"]::-webkit-slider-thumb {
            -webkit-appearance: none;
            width: 20px;
            height: 20px;
            background: linear-gradient(135deg, #667eea, #764ba2);
            border-radius: 50%;
            cursor: pointer;
        }
        
        .button-group {
            display: flex;
            gap: 12px;
            justify-content: center;
            margin: 20px 0;
        }
        
        .btn {
            padding: 14px 30px;
            border: none;
            border-radius: 10px;
            font-size: 15px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.2s;
            color: white;
        }
        
        .btn-primary { background: linear-gradient(135deg, #667eea, #764ba2); }
        .btn-stop { background: #ef4444; }
        .btn-success { background: #10b981; }
        
        .btn:hover { transform: translateY(-2px); box-shadow: 0 4px 12px rgba(0,0,0,0.15); }
        .btn:active { transform: translateY(0); }
        
        .status-message {
            padding: 20px 24px;
            border-radius: 16px;
            margin: 20px 0;
            font-size: 14px;
            border: none;
            box-shadow: 0 4px 16px rgba(0,0,0,0.08);
        }
        
        .status-info { 
            background: linear-gradient(135deg, rgba(219, 234, 254, 0.8) 0%, rgba(147, 197, 253, 0.6) 100%);
            backdrop-filter: blur(10px);
            color: #1e40af;
        }
        .status-success { 
            background: linear-gradient(135deg, rgba(209, 250, 229, 0.8) 0%, rgba(134, 239, 172, 0.6) 100%);
            backdrop-filter: blur(10px);
            color: #065f46;
        }
        .status-error { 
            background: linear-gradient(135deg, rgba(254, 226, 226, 0.8) 0%, rgba(252, 165, 165, 0.6) 100%);
            backdrop-filter: blur(10px);
            color: #991b1b;
        }
        
        .form-section {
            margin-top: 20px;
            padding-top: 20px;
            border-top: 2px solid #e5e7eb;
        }
        
        @media (max-width: 768px) {
            .header { flex-direction: column; gap: 12px; text-align: center; }
            .gauges-grid { grid-template-columns: 1fr; }
            .button-group { flex-direction: column; }
            .btn { width: 100%; }
        }
    </style>
</head>
<body>
    <div class='container'>
        <div class='header'>
            <h1>⚡ CORE Test</h1>
            <small>Conductive Suite Resistance Evaluator</small>
            <a href='/settings' class='settings-btn'>⚙️ Settings</a>
        </div>
        
        <div id='wifiStatus' class='wifi-status wifi-disconnected'>
            <span>📡</span>
            <span id='wifiText'>Disconnected</span>
        </div>
        
        <!-- User Selection -->
        <div class='control-section' style='margin-bottom: 20px;'>
            <div style='display: grid; grid-template-columns: 2fr 3fr 2.5fr; gap: 20px; align-items: end;'>
                <div class='form-group' style='margin-bottom: 0;'>
                    <label for='uptSelect'>UPT:</label>
                    <select id='uptSelect' onchange='updateNameList()'>
                        <option value=''>-- Pilih UPT --</option>
                        <option value='Pulogadung'>UPT Pulogadung</option>
                        <option value='Cawang'>UPT Cawang</option>
                        <option value='Durikosambi'>UPT Durikosambi</option>
                        <option value='Cilegon'>UPT Cilegon</option>
                        <option value='Gandul'>UPT Gandul</option>
                        <option value='Cikupa'>UPT Cikupa</option>
                    </select>
                </div>
                <div class='form-group' style='margin-bottom: 0;'>
                    <label for='namaInput'>Nama:</label>
                    <input type='text' id='namaInput' placeholder='Tulis Nama Lengkap' disabled onchange='updateRList()' style='width: 100%; padding: 12px; border: 2px solid #e5e7eb; border-radius: 12px; font-size: 14px; transition: all 0.3s;' onfocus='this.style.borderColor="#667eea"' onblur='this.style.borderColor="#e5e7eb"'>
                </div>
                <div class='form-group' style='margin-bottom: 0;'>
                    <label for='rSelect'>Titik Ukur:</label>
                    <select id='rSelect' disabled onchange='updateRekapTable()'>
                        <option value=''>-- Pilih Titik Uji --</option>
                    </select>
                </div>
            </div>
        </div>
        
        <!-- Real-time Gauge Meters -->
        <div class='gauges-grid'>
            <div class='gauge-card'>
                <div class='gauge'>
                    <svg width='120' height='120'>
                        <circle class='gauge-bg' cx='60' cy='60' r='52' />
                        <circle id='voltageGauge' class='gauge-fill' cx='60' cy='60' r='52' 
                                stroke='#ef4444' stroke-dasharray='326.73' stroke-dashoffset='326.73' />
                    </svg>
                    <div class='gauge-value' id='voltageValue'>0V</div>
                </div>
                <div class='gauge-label'>Voltage</div>
            </div>
            
            <div class='gauge-card'>
                <div class='gauge'>
                    <svg width='120' height='120'>
                        <circle class='gauge-bg' cx='60' cy='60' r='52' />
                        <circle id='currentGauge' class='gauge-fill' cx='60' cy='60' r='52' 
                                stroke='#10b981' stroke-dasharray='326.73' stroke-dashoffset='326.73' />
                    </svg>
                    <div class='gauge-value' id='currentValue'>0mA</div>
                </div>
                <div class='gauge-label'>Current</div>
            </div>
            
            <div class='gauge-card'>
                <div class='gauge'>
                    <svg width='120' height='120'>
                        <circle class='gauge-bg' cx='60' cy='60' r='52' />
                        <circle id='resistanceGauge' class='gauge-fill' cx='60' cy='60' r='52' 
                                stroke='#3b82f6' stroke-dasharray='326.73' stroke-dashoffset='326.73' />
                    </svg>
                    <div class='gauge-value' id='resistanceValue'>0Ω</div>
                </div>
                <div class='gauge-label'>Resistance</div>
            </div>
            
            <div class='gauge-card'>
                <div class='gauge'>
                    <svg width='120' height='120'>
                        <circle class='gauge-bg' cx='60' cy='60' r='52' />
                        <circle id='amplitudeGauge' class='gauge-fill' cx='60' cy='60' r='52' 
                                stroke='#a78bfa' stroke-dasharray='326.73' stroke-dashoffset='326.73' />
                    </svg>
                    <div class='gauge-value' id='amplitudeValue'>0%</div>
                </div>
                <div class='gauge-label'>Amplitude</div>
            </div>
        </div>

        <div class='control-section'>
            <div class='section-title'>Test Mode Selection</div>
            <div class='form-group'>
                <select id='testMode' onchange='updateModeDisplay()'>
                    <option value='quick'>Manual Mode</option>
                    <option value='special'>Automatic Mode</option>
                </select>
            </div>
            
            <div id='modeDisplay' class='status-message status-info'>
                <div style='font-size: 18px; font-weight: 700; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); -webkit-background-clip: text; -webkit-text-fill-color: transparent;'><span id='currentModeText'>Manual Mode</span></div>
                <div style='margin-top: 10px; font-size: 14px; color: #6b7280;' id='modeDescription'>Tes cepat dengan kontrol amplitudo manual</div>
            </div>
            
            <div class='control-card' id='quickControl'>
                <div class='control-title'>Manual Mode Amplitude</div>
                <div class='numeric-control'>
                    <button type='button' class='numeric-btn dec' onclick='changeQuickAmplitude(-1)'>-</button>
                    <input type='number' id='quickAmplitude' class='numeric-value' min='0' max='100' step='1' value='0' onchange='updateQuickAmplitude(this.value)'>
                    <button type='button' class='numeric-btn inc' onclick='changeQuickAmplitude(1)'>+</button>
                </div>
                <div class='slider-container'>
                    <input type='range' id='quickAmplitudeSlider' class='slider-inline' min='0' max='100' step='1' value='0' oninput='updateQuickFromSlider(this.value)'>
                </div>
                <div class='control-subtitle'>Atur besar injeksi (%) dengan tombol - / + atau geser slide.</div>
            </div>
            <div class='control-card' id='specialControl' style='display: none;'>
            </div>
            
            <!-- Shared Timer Display (for both Manual and Automatic modes) -->
            <div id='timerDisplay' style='display: none; background: linear-gradient(135deg, #10b981 0%, #059669 100%); padding: 28px; border-radius: 16px; text-align: center; margin-bottom: 16px; box-shadow: 0 10px 40px rgba(16, 185, 129, 0.3);'>
                <div style='font-size: 15px; color: rgba(255,255,255,0.95); margin-bottom: 12px; font-weight: 600; letter-spacing: 1px;'>🕒 RECORDING...</div>
                <div style='font-size: 48px; font-weight: 700; color: white; text-shadow: 0 4px 8px rgba(0,0,0,0.2);' id='countdownTimer'>02:00</div>
            </div>
            
            <div class='button-group'>
                <button class='btn btn-primary' id='startBtn' onclick='startTest()'>Start Test</button>
                <button class='btn btn-primary' id='injectBtn' onclick='injectCurrent()' style='display: none;'>⚡ Inject 200mA</button>
                <button class='btn btn-success' id='recordBtn' onclick='startRecord()' style='display: none;'>🔴 Record (2 min)</button>
                <button class='btn btn-stop' id='stopBtn' onclick='stopTest()' style='display: none;'>Stop</button>
            </div>
            
        </div>
        
        <!-- Rekap Hasil Uji -->
        <div class='control-section'>
            <div class='section-title'>Hasil Pengujian</div>
            <div style='overflow-x: auto;'>
                <table style='width: 100%; border-collapse: collapse; font-size: 14px;'>
                    <thead>
                        <tr style='background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white;'>
                            <th style='padding: 12px; border: 1px solid #e5e7eb; text-align: left;'>Titik Pengujian</th>
                            <th style='padding: 12px; border: 1px solid #e5e7eb; text-align: center;'>Hasil (Ohm)</th>
                            <th style='padding: 12px; border: 1px solid #e5e7eb; text-align: center;'>Status</th>
                        </tr>
                    </thead>
                    <tbody id='rekapTable'>
                        <tr>
                            <td colspan='3' style='padding: 20px; text-align: center; color: #9ca3af;'>Belum ada data</td>
                        </tr>
                    </tbody>
                </table>
            </div>
            <div style='text-align: center; margin-top: 20px;'>
                <button class='btn btn-success' onclick='submitAllData()' style='padding: 15px 40px; font-size: 16px;'>Submit Hasil Pengujian</button>
            </div>
            <div id='submissionStatus' class='status-panel' style='display: none;'></div>
        </div>
    </div>

    <script>
        let currentTestMode = 'quick';
        let testActive = false;
        let recordingActive = false;
        let countdownInterval = null;

        // Data nama per UPT
        const namaPerUPT = {
            'Pulogadung': [
                'Hamzah Muntoha',
                'Dimas Harry Laksana Fajar',
                'Okky Andreas Triono',
                'Muhammad Wahyudi',
                'Andi Arjuna Caesarea',
                'Anggi Yusuf',
                'Awang Ibrahim',
                'Julianus Pellupessy',
                'Fridom Tusano Hadi',
                'Afdhal',
                'Aji Nugraha Yusuf'
            ],
            'Cawang': [
                'Munirul Anam',
                'Wahyu Ngainur Rofik',
                'Nur Ali',
                'Pahala Kemala Simanjuntak'
            ],
            'Durikosambi': [
                'Aris Fardila',
                'Arif Al Khairi',
                'Sayiful Elmi'
            ],
            'Cilegon': [
                'Muhammad Asyhari',
                'Muhammad Abu Rizal',
                'Pradhitya Wastu Nurjito'
            ],
            'Gandul': [
                'Soni Sofyan',
                'Muhammad Aditia'
            ],
            'Cikupa': [
                'Zulfikar Hidayat',
                'Kresna Bayu',
                'Prisma Panji Pratama'
            ]
        };
        // Note: namaPerUPT is kept for potential future use but nama is now free text input
        
        // Data titik ukur R1-R8
        const titikUkurList = [
            'R1 - Sarung tangan & tali celana',
            'R2 - Tali celana & tudung',
            'R3 - Tali celana & tudung',
            'R4 - Sarung tangan & tali celana',
            'R5 - Kaos kaki & tali celana',
            'R6 - Kaos kaki & tali celana',
            'R7 - Tali bonding & baju konduktif',
            'R8 - Tali bonding & baju konduktif'
        ];
        
        // Storage untuk hasil uji
        let hasilUji = {};
        
        function updateNameList() {
            // Nama is now text input, just enable R dropdown if UPT is selected
            const uptSelect = document.getElementById('uptSelect');
            const namaInput = document.getElementById('namaInput');
            const rSelect = document.getElementById('rSelect');
            const selectedUPT = uptSelect.value;
            
            if (selectedUPT) {
                namaInput.disabled = false;
            } else {
                namaInput.disabled = true;
                namaInput.value = '';
            }
            
            // Reset R dropdown
            rSelect.innerHTML = '<option value="">-- Tulis Nama dulu --</option>';
            rSelect.disabled = true;
        }
        
        function updateRList() {
            const namaInput = document.getElementById('namaInput');
            const rSelect = document.getElementById('rSelect');
            const selectedNama = namaInput.value.trim();
            
            if (!selectedNama) {
                rSelect.disabled = true;
                rSelect.innerHTML = '<option value="">-- Tulis Nama dulu --</option>';
                return;
            }
            
            rSelect.disabled = false;
            rSelect.innerHTML = '<option value="">-- Pilih Titik Ukur --</option>';
            
            titikUkurList.forEach((titik, index) => {
                const option = document.createElement('option');
                const rKey = 'R' + (index + 1);
                option.value = rKey;
                option.textContent = titik;
                
                // Tandai jika sudah diisi
                if (hasilUji[selectedNama] && hasilUji[selectedNama][rKey]) {
                    option.textContent += ' ✓';
                    option.style.color = '#10b981';
                }
                
                rSelect.appendChild(option);
            });
            
            // Auto-select R pertama yang belum diisi
            if (hasilUji[selectedNama]) {
                for (let i = 1; i <= 8; i++) {
                    const rKey = 'R' + i;
                    if (!hasilUji[selectedNama][rKey]) {
                        rSelect.value = rKey;
                        break;
                    }
                }
            } else {
                rSelect.value = 'R1'; // Default R1
            }
            
            updateRekapTable();
        }

        function updateModeDisplay() {
            const mode = document.getElementById('testMode').value;
            const display = document.getElementById('modeDisplay');
            const currentModeText = document.getElementById('currentModeText');
            const modeDescription = document.getElementById('modeDescription');
            const quickControl = document.getElementById('quickControl');
            const specialControl = document.getElementById('specialControl');
            
            currentTestMode = mode;
            
            if (mode === 'quick') {
                currentModeText.textContent = 'Manual Mode';
                modeDescription.textContent = '';
                display.className = 'status-panel status-info';
                display.style.display = 'none';
                quickControl.style.display = 'block';
                specialControl.style.display = 'none';
                // Reset buttons
                document.getElementById('startBtn').style.display = 'inline-block';
                document.getElementById('injectBtn').style.display = 'none';
                document.getElementById('recordBtn').style.display = 'none';
                document.getElementById('stopBtn').style.display = 'none';
            } else if (mode === 'special') {
                currentModeText.textContent = 'Automatic Mode';
                modeDescription.textContent = '';
                display.className = 'status-panel status-info';
                display.style.display = 'none';
                quickControl.style.display = 'none';
                specialControl.style.display = 'block';
                // Show inject button
                document.getElementById('startBtn').style.display = 'none';
                document.getElementById('injectBtn').style.display = 'inline-block';
                document.getElementById('recordBtn').style.display = 'none';
                document.getElementById('stopBtn').style.display = 'none';
            }
        }
        
        function injectCurrent() {
            // Validasi UPT, Nama, dan R harus dipilih
            if (!validateSelection()) {
                return;
            }
            
            // Start injecting 200mA in 3 seconds
            updateStatusMessage('⚡ Injecting 200mA... (3 seconds)');
            document.getElementById('injectBtn').disabled = true;
            
            // Simulate 3 second injection (in real implementation, ESP32 will ramp up)
            fetch('/api/auto-injection?action=inject&target=200', {method: 'POST'})
                .then(response => response.json())
                .then(data => {
                    setTimeout(() => {
                        document.getElementById('injectBtn').style.display = 'none';
                        document.getElementById('recordBtn').style.display = 'inline-block';
                        document.getElementById('stopBtn').style.display = 'inline-block';
                        updateStatusMessage('✅ 200mA tercapai! Klik Record untuk mulai timer.');
                    }, 3000);
                });
        }
        
        function startRecord() {
            // Start 2 minute recording
            recordingActive = true;
            document.getElementById('recordBtn').style.display = 'none';
            document.getElementById('timerDisplay').style.display = 'block';
            updateStatusMessage('🔴 Recording... Timer berjalan 2 menit.');
            
            // Panggil API record ke ESP32 (works for both manual and automatic mode)
            if (currentTestMode === 'special') {
                fetch('/api/auto-injection?action=record', {method: 'POST'})
                    .then(response => response.json())
                    .then(data => {
                        console.log('Recording started on ESP32 (Automatic Mode)');
                    });
            } else {
                // Manual mode - just start timer, injection already running
                console.log('Recording started (Manual Mode)');
            }
            
            // Start countdown timer
            let timeLeft = 120; // 2 minutes in seconds
            countdownInterval = setInterval(() => {
                timeLeft--;
                const minutes = Math.floor(timeLeft / 60);
                const seconds = timeLeft % 60;
                document.getElementById('countdownTimer').textContent = 
                    String(minutes).padStart(2, '0') + ':' + String(seconds).padStart(2, '0');
                
                if (timeLeft <= 0) {
                    clearInterval(countdownInterval);
                    recordingActive = false;
                    document.getElementById('timerDisplay').style.display = 'none';
                    updateStatusMessage('✅ Recording selesai! Menyimpan data...');
                    
                    // Auto-submit data setelah recording selesai
                    autoSaveAndNext();
                    
                    // Stop injection after timer ends
                    if (currentTestMode === 'quick') {
                        // Manual Mode - stop and reset amplitude to 0
                        fetch('/set_amplitude?state=STOP')
                            .then(response => response.text())
                            .then(() => {
                                const ampEl = document.getElementById('quickAmplitude');
                                const slider = document.getElementById('quickAmplitudeSlider');
                                if (ampEl) {
                                    ampEl.value = 0;
                                    ampEl.readOnly = false;
                                }
                                if (slider) {
                                    slider.value = 0;
                                    slider.disabled = false;
                                }
                            });
                    } else {
                        // Automatic Mode - stop auto injection
                        fetch('/api/auto-injection?action=stop', {method: 'POST'});
                    }
                }
            }, 1000);
        }
        
        function autoSaveAndNext() {
            const namaInput = document.getElementById('namaInput');
            const rSelect = document.getElementById('rSelect');
            const selectedNama = namaInput.value.trim();
            const selectedR = rSelect.value;
            
            // Ambil nilai resistance dari sensor
            fetch('/status')
                .then(response => response.json())
                .then(statusData => {
                    const resistance = statusData.resistance.toFixed(2);
                    
                    // Simpan ke hasil uji
                    if (!hasilUji[selectedNama]) {
                        hasilUji[selectedNama] = {};
                    }
                    hasilUji[selectedNama][selectedR] = resistance;
                    
                    // Update tabel rekap
                    updateRekapTable();
                    
                    // Cari R berikutnya yang belum diisi
                    let nextR = null;
                    for (let i = 1; i <= 8; i++) {
                        const rKey = 'R' + i;
                        if (!hasilUji[selectedNama][rKey]) {
                            nextR = rKey;
                            break;
                        }
                    }
                    
                    // Reset buttons
                    document.getElementById('injectBtn').style.display = 'inline-block';
                    document.getElementById('injectBtn').disabled = false;
                    document.getElementById('recordBtn').style.display = 'none';
                    document.getElementById('stopBtn').style.display = 'none';
                    
                    // Pindah ke R berikutnya atau selesai
                    if (nextR) {
                        rSelect.value = nextR;
                        updateStatusMessage('✅ ' + selectedR + ' tersimpan: ' + resistance + 'Ω. Lanjut ke ' + nextR + '.');
                    } else {
                        updateStatusMessage('🎉 Semua titik ukur (R1-R8) sudah selesai!');
                    }
                    
                    // Refresh R list untuk update tanda centang
                    updateRList();
                });
        }

        function updateQuickAmplitude(value) {
            if (currentTestMode !== 'quick') return;
            const v = Math.max(0, Math.min(100, parseInt(value || '0')));
            const input = document.getElementById('quickAmplitude');
            const slider = document.getElementById('quickAmplitudeSlider');
            const normalized = isNaN(v) ? 0 : v;

            if (input) {
                input.value = normalized;
            }
            if (slider) {
                slider.value = normalized;
            }

            if (testActive) {
                fetch('/set_amplitude?value=' + normalized)
                    .then(response => response.text())
                    .then(data => console.log('Quick amplitude updated:', normalized));
            }
        }

        function updateQuickFromSlider(value) {
            if (currentTestMode !== 'quick') return;
            const v = Math.max(0, Math.min(100, parseInt(value || '0')));
            const slider = document.getElementById('quickAmplitudeSlider');
            const normalized = isNaN(v) ? 0 : v;

            if (slider) {
                slider.value = normalized;
            }
            // Delegasikan ke updateQuickAmplitude supaya input angka ikut sinkron dan,
            // kalau testActive, nilai juga terkirim ke alat.
            updateQuickAmplitude(normalized);
        }

        function changeQuickAmplitude(delta) {
            const input = document.getElementById('quickAmplitude');
            if (!input) return;
            const current = parseInt(input.value || '0');
            let next = isNaN(current) ? 0 : current + delta;
            if (next < 0) next = 0;
            if (next > 100) next = 100;
            // Hanya ganti lewat fungsi umum supaya slider dan backend ikut sinkron
            updateQuickAmplitude(next);
        }

        function changeSpecialDuration(delta) {
            const input = document.getElementById('specialDuration');
            if (!input) return;
            const current = parseInt(input.value || '0');
            let next = isNaN(current) ? 0 : current + delta;
            if (next < 1) next = 1;
            if (next > 3600) next = 3600;
            input.value = next;
        }

        function validateSelection() {
            const uptSelect = document.getElementById('uptSelect');
            const namaInput = document.getElementById('namaInput');
            const rSelect = document.getElementById('rSelect');
            
            if (!uptSelect.value) {
                alert('⚠️ Harap pilih UPT terlebih dahulu!');
                return false;
            }
            
            if (!namaInput.value.trim()) {
                alert('⚠️ Harap tulis Nama Lengkap terlebih dahulu!');
                return false;
            }
            
            if (!rSelect.value) {
                alert('⚠️ Harap pilih Titik Ukur (R1-R8) terlebih dahulu!');
                return false;
            }
            
            return true;
        }
        
        function startTest() {
            // Validasi UPT, Nama, dan R harus dipilih
            if (!validateSelection()) {
                return;
            }
            
            if (currentTestMode === 'special') {
                // Should not reach here - special mode uses injectCurrent()
                return;
            } else { // Manual Mode - start injection
                const ampInput = document.getElementById('quickAmplitude');
                const amp = Math.max(0, Math.min(100, parseInt(ampInput.value || '0')));
                ampInput.value = amp;
                
                updateStatusMessage('⚡ Injecting current with ' + amp + '% amplitude...');
                document.getElementById('startBtn').disabled = true;
                
                fetch('/set_amplitude?value=' + amp)
                    .then(response => response.text())
                    .then(() => {
                        return fetch('/set_amplitude?state=RUN');
                    })
                    .then(response => response.text())
                    .then(() => {
                        testActive = true;
                        document.getElementById('startBtn').style.display = 'none';
                        document.getElementById('recordBtn').style.display = 'inline-block';
                        document.getElementById('stopBtn').style.display = 'inline-block';
                        // Lock amplitude input and slider
                        const ampEl = document.getElementById('quickAmplitude');
                        const slider = document.getElementById('quickAmplitudeSlider');
                        if (ampEl) ampEl.readOnly = true;
                        if (slider) slider.disabled = true;
                        updateStatusMessage('✅ Injection aktif dengan ' + amp + '% amplitude. Klik Record untuk mulai timer.');
                    });
            }
        }

        function stopTest() {
            if (currentTestMode === 'special') {
                if (countdownInterval) {
                    clearInterval(countdownInterval);
                    countdownInterval = null;
                }
                fetch('/api/auto-injection?action=stop', {method: 'POST'})
                    .then(response => response.json())
                    .then(data => {
                        testActive = false;
                        recordingActive = false;
                        document.getElementById('timerDisplay').style.display = 'none';
                        document.getElementById('injectBtn').style.display = 'inline-block';
                        document.getElementById('injectBtn').disabled = false;
                        document.getElementById('recordBtn').style.display = 'none';
                        document.getElementById('stopBtn').style.display = 'none';
                        updateStatusMessage('Automatic Mode dihentikan.');
                    });
            } else { // Manual Mode
                if (countdownInterval) {
                    clearInterval(countdownInterval);
                    countdownInterval = null;
                }
                fetch('/set_amplitude?state=STOP')
                    .then(response => response.text())
                    .then(data => {
                        testActive = false;
                        recordingActive = false;
                        document.getElementById('timerDisplay').style.display = 'none';
                        document.getElementById('startBtn').style.display = 'inline-block';
                        document.getElementById('startBtn').disabled = false;
                        document.getElementById('recordBtn').style.display = 'none';
                        document.getElementById('stopBtn').style.display = 'none';
                        // Unlock amplitude controls
                        const ampEl = document.getElementById('quickAmplitude');
                        const slider = document.getElementById('quickAmplitudeSlider');
                        if (ampEl) {
                            ampEl.readOnly = false;
                        }
                        if (slider) {
                            slider.disabled = false;
                        }
                        updateStatusMessage('Manual Mode dihentikan.');
                    });
            }
        }

        function resetButtons() {
            document.getElementById('startBtn').style.display = 'inline-block';
            document.getElementById('stopBtn').style.display = 'none';
        }

        function updateStatusMessage(message) {
            const display = document.getElementById('modeDisplay');
            display.className = 'status-panel status-success';
            document.getElementById('modeDescription').textContent = message;
        }
        
        function updateRekapTable() {
            const namaInput = document.getElementById('namaInput');
            const selectedNama = namaInput.value.trim();
            const tbody = document.getElementById('rekapTable');
            
            if (!selectedNama || !hasilUji[selectedNama] || Object.keys(hasilUji[selectedNama]).length === 0) {
                tbody.innerHTML = '<tr><td colspan="3" style="padding: 20px; text-align: center; color: #9ca3af;">Belum ada data</td></tr>';
                return;
            }
            
            tbody.innerHTML = '';
            for (let i = 1; i <= 8; i++) {
                const rKey = 'R' + i;
                const row = document.createElement('tr');
                
                const cellTitik = document.createElement('td');
                cellTitik.style.cssText = 'padding: 10px; border: 1px solid #e5e7eb;';
                cellTitik.innerHTML = '<strong>' + rKey + '</strong><br><small style="color: #6b7280;">' + titikUkurList[i - 1] + '</small>';
                
                const cellR = document.createElement('td');
                cellR.style.cssText = 'padding: 10px; border: 1px solid #e5e7eb; font-weight: 600; text-align: center;';
                
                const cellStatus = document.createElement('td');
                cellStatus.style.cssText = 'padding: 10px; border: 1px solid #e5e7eb; text-align: center;';
                
                if (hasilUji[selectedNama][rKey]) {
                    const resistance = parseFloat(hasilUji[selectedNama][rKey]);
                    cellR.textContent = resistance.toFixed(2);
                    
                    // Status: Good jika ≤ 200 Ω, Bad jika > 200 Ω
                    if (resistance <= 200) {
                        cellStatus.innerHTML = '<span style="display: inline-block; padding: 6px 16px; background: #10b981; color: white; border-radius: 6px; font-weight: 600; font-size: 13px;">Good</span>';
                    } else {
                        cellStatus.innerHTML = '<span style="display: inline-block; padding: 6px 16px; background: #ef4444; color: white; border-radius: 6px; font-weight: 600; font-size: 13px;">Bad</span>';
                    }
                } else {
                    cellR.innerHTML = '<span style="color: #9ca3af;">-</span>';
                    cellStatus.innerHTML = '<span style="color: #9ca3af; font-size: 13px;">Belum diuji</span>';
                }
                
                row.appendChild(cellTitik);
                row.appendChild(cellR);
                row.appendChild(cellStatus);
                tbody.appendChild(row);
            }
        }

        function submitAllData() {
            const uptSelect = document.getElementById('uptSelect');
            const namaInput = document.getElementById('namaInput');
            const selectedUPT = uptSelect.value;
            const selectedNama = namaInput.value.trim();
            
            // Validasi UPT dan Nama
            if (!selectedUPT || !selectedNama) {
                const status = document.getElementById('submissionStatus');
                status.className = 'status-panel status-error';
                status.textContent = '⚠️ Harap pilih UPT dan Nama terlebih dahulu!';
                status.style.display = 'block';
                return;
            }
            
            // Validasi ada data yang sudah diuji
            if (!hasilUji[selectedNama] || Object.keys(hasilUji[selectedNama]).length === 0) {
                const status = document.getElementById('submissionStatus');
                status.className = 'status-panel status-error';
                status.textContent = '⚠️ Belum ada data pengujian yang tersimpan!';
                status.style.display = 'block';
                return;
            }
            
            // Siapkan data untuk submit
            const data = {
                UPT: 'UPT ' + selectedUPT,
                NIP: selectedNama,
                HASIL_UJI: hasilUji[selectedNama],
                timestamp: new Date().toISOString()
            };
            
            // Submit ke server
            fetch('/api/submit-data', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify(data)
            })
            .then(response => response.json())
            .then(result => {
                const status = document.getElementById('submissionStatus');
                if (result.success) {
                    status.className = 'status-panel status-success';
                    status.textContent = '✅ Data berhasil dikirim ke server!';
                    status.style.display = 'block';
                } else {
                    status.className = 'status-panel status-error';
                    status.textContent = '❌ Gagal mengirim data: ' + result.message;
                    status.style.display = 'block';
                }
            })
            .catch(error => {
                const status = document.getElementById('submissionStatus');
                status.className = 'status-panel status-error';
                status.textContent = '❌ Error: ' + error.message;
                status.style.display = 'block';
            });
        }
        
        function showForm() {
            document.getElementById('dataForm').style.display = 'block';
        }

        function submitToCloud(event) {
            event.preventDefault();

            const uptSelect = document.getElementById('uptSelect');
            const namaSelect = document.getElementById('namaSelect');
            const formData = new FormData(document.getElementById('dataForm'));
            
            // Validasi UPT dan Nama harus dipilih
            if (!uptSelect.value || !namaSelect.value) {
                const status = document.getElementById('submissionStatus');
                status.className = 'status-panel status-error';
                status.textContent = 'Harap pilih UPT dan Nama terlebih dahulu!';
                status.style.display = 'block';
                return;
            }
            
            // Hanya kirim 4 item ke cloud: UPT, NIP (Nama), KODE_SUIT, R
            const data = {
                UPT: 'UPT ' + uptSelect.value,
                NIP: namaSelect.value,
                KODE_SUIT: formData.get('kodeSuit'),
                R: 0
            };

            // Ambil nilai R (resistance) terakhir dari alat
            fetch('/status')
                .then(response => response.json())
                .then(statusData => {
                    data.R = statusData.resistance;

                    const rField = document.getElementById('hasilR');
                    if (rField) {
                        rField.value = statusData.resistance.toFixed(2);
                    }

                    return fetch('/api/submit-data', {
                        method: 'POST',
                        headers: {'Content-Type': 'application/json'},
                        body: JSON.stringify(data)
                    });
                })
                .then(response => response.json())
                .then(result => {
                    const status = document.getElementById('submissionStatus');
                    if (result.success) {
                        status.className = 'status-panel status-success';
                        status.textContent = 'Data submitted successfully! Reference ID: ' + result.referenceId;
                        document.getElementById('dataForm').reset();
                    } else {
                        status.className = 'status-panel status-error';
                        status.textContent = 'Failed to submit data: ' + result.message;
                    }
                    status.style.display = 'block';
                })
                .catch(error => {
                    const status = document.getElementById('submissionStatus');
                    status.className = 'status-panel status-error';
                    status.textContent = 'Error submitting data: ' + error.message;
                    status.style.display = 'block';
                });
        }

        // Update gauge animation
        function updateGauge(gaugeId, value, maxValue) {
            const gauge = document.getElementById(gaugeId);
            const circumference = 326.73;
            const percent = Math.min(value / maxValue, 1);
            const offset = circumference * (1 - percent);
            gauge.style.strokeDashoffset = offset;
        }
        
        function updateReadings() {
            fetch('/status')
                .then(response => response.json())
                .then(data => {
                    // Update gauge meters with animation
                    document.getElementById('voltageValue').textContent = data.voltage.toFixed(1) + 'V';
                    updateGauge('voltageGauge', data.voltage, 250); // Max 250V
                    
                    const currentMA = (data.current * 1000).toFixed(0);
                    document.getElementById('currentValue').textContent = currentMA + 'mA';
                    updateGauge('currentGauge', data.current * 1000, 1000); // Max 1000mA
                    
                    document.getElementById('resistanceValue').textContent = data.resistance.toFixed(1) + 'Ω';
                    updateGauge('resistanceGauge', data.resistance, 1000); // Max 1000Ω
                    
                    const ampPercent = (data.amplitude * 100).toFixed(0);
                    document.getElementById('amplitudeValue').textContent = ampPercent + '%';
                    updateGauge('amplitudeGauge', ampPercent, 100); // Max 100%
                    
                    const quickInput = document.getElementById('quickAmplitude');
                    if (quickInput && testActive && currentTestMode === 'quick') {
                        quickInput.value = ampPercent;
                    }

                    const rField = document.getElementById('hasilR');
                    if (rField) {
                        rField.value = data.resistance.toFixed(2);
                    }

                    if (data.wifiConnected) {
                        document.getElementById('wifiStatus').className = 'wifi-status wifi-connected';
                        document.getElementById('wifiText').textContent = 'Connected to ' + data.wifiSSID;
                    } else {
                        document.getElementById('wifiStatus').className = 'wifi-status wifi-disconnected';
                        document.getElementById('wifiText').textContent = 'Disconnected';
                    }
                })
                .catch(error => {
                    console.error('Error fetching status:', error);
                });
        }

        updateReadings();
        setInterval(updateReadings, 1000);
        updateModeDisplay();
    </script>
</body>
</html>
  )";

  server.send(200, "text/html", html);
}

void handleSettings() {
  // Always load latest saved settings from NVS
  loadSettingsFromMemory();

  String html =
"<!DOCTYPE html>\n"
"<html>\n"
"<head>\n"
"    <title>CORE Test - Settings</title>\n"
"    <meta charset='UTF-8'>\n"
"    <meta name='viewport' content='width=device-width, initial-scale=1.0'>\n"
"    <style>\n"
"        * { margin: 0; padding: 0; box-sizing: border-box; }\n"
"        \n"
"        body {\n"
"            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;\n"
"            background: linear-gradient(135deg, #06b6d4 0%, #14b8a6 100%);\n"
"            min-height: 100vh;\n"
"            padding: 20px;\n"
"            color: #1a202c;\n"
"        }\n"
"        \n"
"        .container {\n"
"            max-width: 640px;\n"
"            margin: 24px auto;\n"
"            background: white;\n"
"            padding: 32px;\n"
"            border-radius: 20px;\n"
"            box-shadow: 0 4px 20px rgba(0,0,0,0.1);\n"
"        }\n"
"        \n"
"        h1 {\n"
"            margin-top: 0;\n"
"            margin-bottom: 24px;\n"
"            font-size: 28px;\n"
"            font-weight: 700;\n"
"            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);\n"
"            -webkit-background-clip: text;\n"
"            -webkit-text-fill-color: transparent;\n"
"            text-align: center;\n"
"        }\n"
"        \n"
"        h3 {\n"
"            font-size: 18px;\n"
"            font-weight: 700;\n"
"            margin-bottom: 16px;\n"
"            color: #1a202c;\n"
"        }\n"
"        \n"
"        .form-group { margin: 16px 0; }\n"
"        \n"
"        label {\n"
"            display: block;\n"
"            margin-bottom: 6px;\n"
"            font-weight: 600;\n"
"            font-size: 13px;\n"
"            color: #374151;\n"
"        }\n"
"        \n"
"        input[type='text'], input[type='password'] {\n"
"            width: 100%;\n"
"            padding: 12px;\n"
"            background: #f9fafb;\n"
"            border: 2px solid #e5e7eb;\n"
"            border-radius: 10px;\n"
"            color: #1a202c;\n"
"            font-size: 14px;\n"
"            transition: all 0.2s;\n"
"        }\n"
"        \n"
"        input[type='text']:focus, input[type='password']:focus {\n"
"            outline: none;\n"
"            border-color: #667eea;\n"
"            background: white;\n"
"        }\n"
"        \n"
"        .btn {\n"
"            padding: 12px 24px;\n"
"            margin: 8px 6px 8px 0;\n"
"            border: none;\n"
"            border-radius: 10px;\n"
"            cursor: pointer;\n"
"            font-size: 14px;\n"
"            font-weight: 600;\n"
"            transition: all 0.2s;\n"
"            color: white;\n"
"        }\n"
"        \n"
"        .btn-primary {\n"
"            background: linear-gradient(135deg, #667eea, #764ba2);\n"
"        }\n"
"        \n"
"        .btn-primary:hover {\n"
"            transform: translateY(-2px);\n"
"            box-shadow: 0 4px 12px rgba(102, 126, 234, 0.4);\n"
"        }\n"
"        \n"
"        .btn-secondary {\n"
"            background: #6b7280;\n"
"            color: white;\n"
"        }\n"
"        \n"
"        .btn-secondary:hover {\n"
"            background: #4b5563;\n"
"            transform: translateY(-2px);\n"
"        }\n"
"        \n"
"        .btn-success {\n"
"            background: linear-gradient(135deg, #10b981, #059669);\n"
"        }\n"
"        \n"
"        .btn-success:hover {\n"
"            transform: translateY(-2px);\n"
"            box-shadow: 0 4px 12px rgba(16, 185, 129, 0.4);\n"
"        }\n"
"        \n"
"        .section {\n"
"            background: #f9fafb;\n"
"            padding: 24px;\n"
"            border-radius: 12px;\n"
"            margin: 20px 0;\n"
"            border: 2px solid #e5e7eb;\n"
"        }\n"
"        \n"
"        .status {\n"
"            padding: 12px;\n"
"            border-radius: 8px;\n"
"            margin: 12px 0;\n"
"            font-size: 13px;\n"
"            font-weight: 600;\n"
"        }\n"
"        \n"
"        .success {\n"
"            background: #d1fae5;\n"
"            color: #065f46;\n"
"            border: 2px solid #10b981;\n"
"        }\n"
"        \n"
"        .error {\n"
"            background: #fee2e2;\n"
"            color: #991b1b;\n"
"            border: 2px solid #ef4444;\n"
"        }\n"
"    </style>\n"
"</head>\n"
"<body>\n"
"    <div class='container'>\n"
"        <h1>CORE Test Settings</h1>\n"
"\n"
"        <div class='section'>\n"
"            <h3>WiFi Settings</h3>\n"
"            <div class='form-group'>\n"
"                <label for='wifiSSID'>WiFi SSID:</label>\n"
"                <input type='text' id='wifiSSID' value='" + wifiSSID + "' placeholder='Enter WiFi network name'>\n"
"            </div>\n"
"            <div class='form-group'>\n"
"                <label for='wifiPassword'>WiFi Password:</label>\n"
"                <input type='password' id='wifiPassword' value='" + wifiPassword + "' placeholder='Enter WiFi password'>\n"
"            </div>\n"
"            <button class='btn btn-primary' onclick='saveWiFiSettings()'>Connect to WiFi</button>\n"
"            <button class='btn btn-secondary' onclick='resetWiFi()' style='background: #dc3545; border-color: #dc3545;'>Reset to PDKB_INTERNET_G</button>\n"
"            <div id='wifiStatus'></div>\n"
"        </div>\n"
"\n"
"        <div class='section'>\n"
"            <h3>Cloud Server Settings (MQTT)</h3>\n"
"            <div class='form-group'>\n"
"                <label for='mqttHost'>MQTT Host:</label>\n"
"                <input type='text' id='mqttHost' value='" + mqttHost + "' placeholder='vps.domain.com'>\n"
"            </div>\n"
"            <div class='form-group'>\n"
"                <label for='mqttPort'>MQTT Port:</label>\n"
"                <input type='text' id='mqttPort' value='" + String(mqttPort) + "' placeholder='1883'>\n"
"            </div>\n"
"            <div class='form-group'>\n"
"                <label for='mqttUser'>MQTT Username:</label>\n"
"                <input type='text' id='mqttUser' value='" + mqttUser + "' placeholder='esp1'>\n"
"            </div>\n"
"            <div class='form-group'>\n"
"                <label for='mqttPass'>MQTT Password:</label>\n"
"                <input type='password' id='mqttPass' value='" + mqttPass + "' placeholder='password'>\n"
"            </div>\n"
"            <div class='form-group'>\n"
"                <label for='mqttClientId'>MQTT Client ID:</label>\n"
"                <input type='text' id='mqttClientId' value='" + mqttClientId + "' placeholder='esp32_01'>\n"
"            </div>\n"
"            <div class='form-group'>\n"
"                <label for='mqttTopic'>MQTT Topic (publish):</label>\n"
"                <input type='text' id='mqttTopic' value='" + mqttTopic + "' placeholder='sensor/esp32'>\n"
"            </div>\n"
"            <button class='btn btn-success' onclick='saveCloudSettings()'>Save MQTT Settings</button>\n"
"            <div id='cloudStatus'></div>\n"
"        </div>\n"
"\n"
"        <div style='text-align: center; margin-top: 30px;'>\n"
"            <button class='btn btn-secondary' onclick=\"window.location.href='/'\">Back to Main Menu</button>\n"
"        </div>\n"
"    </div>\n"
"\n"
"    <script>\n"
"        function saveWiFiSettings() {\n"
"            const ssid = document.getElementById('wifiSSID').value;\n"
"            const password = document.getElementById('wifiPassword').value;\n"
"            if (!ssid) {\n"
"                showStatus('wifiStatus', 'Please enter WiFi SSID', 'error');\n"
"                return;\n"
"            }\n"
"            fetch('/api/wifi', {\n"
"                method: 'POST',\n"
"                headers: {'Content-Type': 'application/x-www-form-urlencoded'},\n"
"                body: 'ssid=' + encodeURIComponent(ssid) + '&password=' + encodeURIComponent(password)\n"
"            })\n"
"            .then(response => response.json())\n"
"            .then(data => {\n"
"                if (data.success) {\n"
"                    showStatus('wifiStatus', 'WiFi connection successful!', 'success');\n"
"                } else {\n"
"                    showStatus('wifiStatus', 'WiFi connection failed: ' + data.message, 'error');\n"
"                }\n"
"            })\n"
"            .catch(error => {\n"
"                showStatus('wifiStatus', 'Error: ' + error.message, 'error');\n"
"            });\n"
"        }\n"
"\n"
"        function resetWiFi() {\n"
"            if (!confirm('Reset WiFi ke PDKB_INTERNET_G? ESP32 akan restart.')) {\n"
"                return;\n"
"            }\n"
"            fetch('/api/wifi-reset', {\n"
"                method: 'POST'\n"
"            })\n"
"            .then(response => response.json())\n"
"            .then(data => {\n"
"                if (data.success) {\n"
"                    showStatus('wifiStatus', 'WiFi reset! ESP32 akan restart...', 'success');\n"
"                    setTimeout(() => { location.reload(); }, 3000);\n"
"                } else {\n"
"                    showStatus('wifiStatus', 'Failed to reset: ' + data.message, 'error');\n"
"                }\n"
"            })\n"
"            .catch(error => {\n"
"                showStatus('wifiStatus', 'Error: ' + error.message, 'error');\n"
"            });\n"
"        }\n"
"\n"
"\n"
"        function saveCloudSettings() {\n"
"            const host = document.getElementById('mqttHost').value;\n"
"            const port = document.getElementById('mqttPort').value;\n"
"            const user = document.getElementById('mqttUser').value;\n"
"            const pass = document.getElementById('mqttPass').value;\n"
"            const clientId = document.getElementById('mqttClientId').value;\n"
"            const topic = document.getElementById('mqttTopic').value;\n"
"            \n"
"            if (!host) {\n"
"                showStatus('cloudStatus', 'Please enter MQTT host', 'error');\n"
"                return;\n"
"            }\n"
"            \n"
"            const params = 'host=' + encodeURIComponent(host) + \n"
"                          '&port=' + encodeURIComponent(port) + \n"
"                          '&user=' + encodeURIComponent(user) + \n"
"                          '&pass=' + encodeURIComponent(pass) + \n"
"                          '&clientId=' + encodeURIComponent(clientId) + \n"
"                          '&topic=' + encodeURIComponent(topic);\n"
"            \n"
"            fetch('/api/cloud', {\n"
"                method: 'POST',\n"
"                headers: {'Content-Type': 'application/x-www-form-urlencoded'},\n"
"                body: params\n"
"            })\n"
"            .then(response => response.json())\n"
"            .then(data => {\n"
"                if (data.success) {\n"
"                    showStatus('cloudStatus', 'MQTT settings saved successfully!', 'success');\n"
"                } else {\n"
"                    showStatus('cloudStatus', 'Failed to save settings: ' + data.message, 'error');\n"
"                }\n"
"            })\n"
"            .catch(error => {\n"
"                showStatus('cloudStatus', 'Error: ' + error.message, 'error');\n"
"            });\n"
"        }\n"
"\n"
"        function showStatus(elementId, message, type) {\n"
"            const element = document.getElementById(elementId);\n"
"            element.className = 'status ' + type;\n"
"            element.textContent = message;\n"
"            element.style.display = 'block';\n"
"        }\n"
"    </script>\n"
"</body>\n"
"</html>\n";

  server.send(200, "text/html", html);
}

void loadSettingsFromMemory() {
  Preferences preferences;
  preferences.begin("core-settings", false);
  
  // Load saved settings or use defaults
  wifiSSID = preferences.getString("wifiSSID", "PDKB_INTERNET_G");
  wifiPassword = preferences.getString("wifiPassword", "uptpulogadung");
  cloudServerAddress = preferences.getString("cloudServer", "https://api.example.com/submit-data");
  
  // Load MQTT settings
  mqttHost = preferences.getString("mqttHost", "vps.domain.com");
  mqttPort = preferences.getInt("mqttPort", 1883);
  mqttUser = preferences.getString("mqttUser", "esp1");
  mqttPass = preferences.getString("mqttPass", "password");
  mqttClientId = preferences.getString("mqttClientId", "esp32_01");
  mqttTopic = preferences.getString("mqttTopic", "sensor/esp32");
  
  preferences.end();
  
  Serial.println("Settings loaded from memory:");
  Serial.println("WiFi SSID: " + wifiSSID);
  Serial.println("MQTT Host: " + mqttHost);
  Serial.println("MQTT Port: " + String(mqttPort));
}

void resetWiFiSettings() {
  Preferences preferences;
  preferences.begin("core-settings", false);
  
  // Clear all saved settings
  preferences.clear();
  
  preferences.end();
  
  // Reset to default values
  wifiSSID = "PDKB_INTERNET_G";
  wifiPassword = "uptpulogadung";
  cloudServerAddress = "https://api.example.com/submit-data";
  
  Serial.println("WiFi settings reset to default:");
  Serial.println("WiFi SSID: " + wifiSSID);
  Serial.println("WiFi Password: " + wifiPassword);
}

void saveSettingsToMemory(const String& ssid, const String& password, const String& cloudServer) {
  Preferences preferences;
  preferences.begin("core-settings", false);
  
  preferences.putString("wifiSSID", ssid);
  preferences.putString("wifiPassword", password);
  preferences.putString("cloudServer", cloudServer);
  
  preferences.end();
  
  Serial.println("Settings saved to memory:");
  Serial.println("WiFi SSID: " + ssid);
  Serial.println("Cloud Server: " + cloudServer);
}

void handleMainMenu() {
  // Redirect to root for now
  server.send(302, "Location", "/");
}

void handleInjection() {
  String html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>CORE Test</title>
    <meta charset='UTF-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <style>
        body { font-family: Arial; margin: 20px; background: #f0f0f0; }
        .container { max-width: 600px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; }
        .form-group { margin: 20px 0; }
        label { display: block; margin-bottom: 5px; font-weight: bold; }
        select, input[type='text'] { width: 100%; padding: 10px; border: 1px solid #ddd; border-radius: 5px; box-sizing: border-box; }
        .btn { padding: 15px 30px; margin: 10px 5px; border: none; border-radius: 5px; cursor: pointer; font-size: 16px; }
        .btn-primary { background: #007bff; color: white; }
        .btn-primary:hover { background: #0056b3; }
        .btn-secondary { background: #6c757d; color: white; }
        .btn-secondary:hover { background: #545b62; }
        .btn-success { background: #28a745; color: white; }
        .btn-success:hover { background: #1e7e34; }
        .btn-stop { background: #dc3545; color: white; }
        .btn-stop:hover { background: #c82333; }
        .readings { display: grid; grid-template-columns: 1fr 1fr 1fr; gap: 15px; margin: 20px 0; }
        .reading { background: #e9ecef; padding: 15px; border-radius: 5px; text-align: center; }
        .value { font-size: 24px; font-weight: bold; color: #495057; }
        .label { font-size: 14px; color: #6c757d; margin-bottom: 5px; }
        .status-panel { padding: 15px; border-radius: 5px; margin: 20px 0; }
        .status-success { background: #d4edda; border: 1px solid #c3e6cb; color: #155724; }
        .status-error { background: #f8d7da; border: 1px solid #f5c6cb; color: #721c24; }
        .status-info { background: #d1ecf1; border: 1px solid #bee5eb; color: #0c5460; }
    </style>
</head>
<body>
    <div class='container'>
        <h1>CORE Test</h1>
        
        <div class='readings'>
            <div class='reading'>
                <div class='label'>Voltage</div>
                <div class='value' id='voltageValue'>0.00 V</div>
            </div>
            <div class='reading'>
                <div class='label'>Current</div>
                <div class='value' id='currentValue'>0.00 A</div>
            </div>
            <div class='reading'>
                <div class='label'>Resistance</div>
                <div class='value' id='resistanceValue'>0.00 Ω</div>
            </div>
        </div>
        
        <div class='form-group'>
            <label for='testMode'>Test Mode:</label>
            <select id='testMode' onchange='updateModeDisplay()'>
                <option value='manual'>Manual Injection</option>
                <option value='200ma'>200mA Auto Injection</option>
                <option value='runtime'>Runtime Mode (2 min)</option>
            </select>
        </div>
        
        <div id='modeDisplay' class='status-panel status-info'>
            <strong>Current Mode:</strong> <span id='currentModeText'>Manual Injection</span>
            <p id='modeDescription'>Adjustable amplitude with manual control</p>
        </div>
        
        <div class='form-group' id='amplitudeControl' style='display: none;'>
            <label for='amplitudeSlider'>Amplitude: <span id='amplitudeDisplay'>0%</span></label>
            <input type='range' id='amplitudeSlider' min='0' max='100' value='0' onchange='updateAmplitude(this.value)'>
        </div>
        
        <div class='button-group'>
            <button class='btn btn-primary' id='startBtn' onclick='startTest()'>Start Test</button>
            <button class='btn btn-stop' id='stopBtn' onclick='stopTest()' style='display: none;'>Stop</button>
            <button class='btn btn-success' id='submitBtn' onclick='showForm()'>Submit to Cloud</button>
        </div>
        
        <form id='dataForm' onsubmit='submitToCloud(event)'>
            <div class='form-group'>
                <label for='upt'>UPT:</label>
                <input type='text' id='upt' name='upt' required>
            </div>
            <div class='form-group'>
                <label for='nip'>NIP:</label>
                <input type='text' id='nip' name='nip' required>
            </div>
            <div class='form-group'>
                <label for='kodeSuit'>KODE_SUIT:</label>
                <input type='text' id='kodeSuit' name='kodeSuit' required>
            </div>
            <div class='form-group'>
                <label for='hasilR'>R (hasil uji):</label>
                <input type='text' id='hasilR' name='hasilR' readonly>
            </div>
            <button type='submit' class='btn btn-success'>Submit Test Results</button>
        </form>
        
        <div id='submissionStatus' class='status-panel' style='display: none;'></div>
        
        <div style='text-align: center; margin-top: 30px;'>
            <button class='btn btn-secondary' onclick="window.location.href='/'">Back to Main Menu</button>
        </div>
    </div>

    <script>
        let currentTestMode = 'manual';
        let testActive = false;
        
        function updateModeDisplay() {
            const mode = document.getElementById('testMode').value;
            const display = document.getElementById('modeDisplay');
            const currentModeText = document.getElementById('currentModeText');
            const modeDescription = document.getElementById('modeDescription');
            const amplitudeControl = document.getElementById('amplitudeControl');
            
            currentTestMode = mode;
            
            if (mode === 'manual') {
                currentModeText.textContent = 'Manual Injection';
                modeDescription.textContent = 'Adjustable amplitude with manual control';
                display.className = 'status-panel status-info';
                amplitudeControl.style.display = 'block';
            } else if (mode === '200ma') {
                currentModeText.textContent = '200mA Auto Injection';
                modeDescription.textContent = 'Automatic injection to 200mA with 2-minute countdown';
                display.className = 'status-panel status-info';
                amplitudeControl.style.display = 'none';
            } else if (mode === 'runtime') {
                currentModeText.textContent = 'Runtime Mode';
                modeDescription.textContent = 'Fixed amplitude with 2-minute timer';
                display.className = 'status-panel status-info';
                amplitudeControl.style.display = 'none';
            }
        }
        
        function updateAmplitude(value) {
            document.getElementById('amplitudeDisplay').textContent = value + '%';
            if (testActive) {
                fetch('/set_amplitude?value=' + value)
                    .then(response => response.text())
                    .then(data => console.log('Amplitude updated:', value));
            }
        }
        
        function startTest() {
            if (currentTestMode === '200ma') {
                fetch('/api/auto-injection?action=start', {method: 'POST'})
                    .then(response => response.json())
                    .then(data => {
                        if (data.success) {
                            testActive = true;
                            document.getElementById('startBtn').style.display = 'none';
                            document.getElementById('stopBtn').style.display = 'inline-block';
                            document.getElementById('submitBtn').style.display = 'inline-block';
                            updateStatus('Test started - 200mA Auto Injection');
                        }
                    });
            } else {
                fetch('/set_amplitude?state=RUN')
                    .then(response => response.text())
                    .then(data => {
                        testActive = true;
                        document.getElementById('startBtn').style.display = 'none';
                        document.getElementById('stopBtn').style.display = 'inline-block';
                        document.getElementById('submitBtn').style.display = 'inline-block';
                        updateStatus('Test started - ' + currentTestMode);
                    });
            }
        }
        
        function stopTest() {
            if (currentTestMode === '200ma') {
                fetch('/api/auto-injection?action=stop', {method: 'POST'})
                    .then(response => response.json())
                    .then(data => {
                        testActive = false;
                        resetButtons();
                        updateStatus('Test stopped');
                    });
            } else {
                fetch('/set_amplitude?state=STOP')
                    .then(response => response.text())
                    .then(data => {
                        testActive = false;
                        resetButtons();
                        updateStatus('Test stopped');
                    });
            }
        }
        
        function resetButtons() {
            document.getElementById('startBtn').style.display = 'inline-block';
            document.getElementById('stopBtn').style.display = 'none';
            document.getElementById('submitBtn').style.display = 'inline-block';
        }
        
        function updateStatus(message) {
            const display = document.getElementById('modeDisplay');
            display.className = 'status-panel status-success';
            document.getElementById('modeDescription').textContent = message;
        }
        
        function showForm() {
            document.getElementById('dataForm').style.display = 'block';
        }
        
        function submitToCloud(event) {
            event.preventDefault();
            
            const formData = new FormData(document.getElementById('dataForm'));
            const data = {
                UPT: formData.get('upt'),
                NIP: formData.get('nip'),
                KODE_SUIT: formData.get('kodeSuit'),
                R: 0,
                testMode: currentTestMode,
                voltage: 0,
                current: 0,
                resistance: 0,
                timestamp: new Date().toISOString()
            };
            
            // Get current readings
            fetch('/status')
                .then(response => response.json())
                .then(statusData => {
                    data.voltage = statusData.voltage;
                    data.current = statusData.current;
                    data.resistance = statusData.resistance;
                    data.R = statusData.resistance;

                    const rField = document.getElementById('hasilR');
                    if (rField) {
                        rField.value = statusData.resistance.toFixed(2);
                    }
                    
                    // Submit data
                    fetch('/api/submit-data', {
                        method: 'POST',
                        headers: {'Content-Type': 'application/json'},
                        body: JSON.stringify(data)
                    })
                    .then(response => response.json())
                    .then(result => {
                        const status = document.getElementById('submissionStatus');
                        if (result.success) {
                            status.className = 'status-panel status-success';
                            status.textContent = 'Data submitted successfully! Reference ID: ' + result.referenceId;
                            document.getElementById('dataForm').reset();
                        } else {
                            status.className = 'status-panel status-error';
                            status.textContent = 'Failed to submit data: ' + result.message;
                        }
                        status.style.display = 'block';
                    })
                    .catch(error => {
                        const status = document.getElementById('submissionStatus');
                        status.className = 'status-panel status-error';
                        status.textContent = 'Error submitting data: ' + error.message;
                        status.style.display = 'block';
                    });
                });
        }
        
        function updateReadings() {
            fetch('/status')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('voltageValue').textContent = data.voltage.toFixed(2) + ' V';
                    document.getElementById('currentValue').textContent = data.current.toFixed(2) + ' A';
                    document.getElementById('resistanceValue').textContent = data.resistance.toFixed(2) + ' Ω';

                    const rField = document.getElementById('hasilR');
                    if (rField) {
                        rField.value = data.resistance.toFixed(2);
                    }
                });
        }
        
        // Update readings every second
        updateReadings();
        setInterval(updateReadings, 1000);
        updateModeDisplay(); // Initialize display
    </script>
</body>
</html>
  )";
  
  server.send(200, "text/html", html);
}

void handleNormalInjection() {
  String html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>CORE Test - Normal Injection</title>
    <meta charset='UTF-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <style>
        body { font-family: Arial; margin: 20px; background: #f0f0f0; }
        .container { max-width: 600px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; }
        .control-panel { background: #f8f9fa; padding: 20px; border-radius: 8px; margin: 20px 0; }
        .slider-container { margin: 20px 0; }
        .slider { width: 100%; margin: 10px 0; }
        .button-group { text-align: center; margin: 20px 0; }
        .btn { padding: 15px 30px; margin: 10px; border: none; border-radius: 5px; cursor: pointer; font-size: 16px; }
        .btn-start { background: #28a745; color: white; }
        .btn-stop { background: #dc3545; color: white; }
        .btn-back { background: #6c757d; color: white; }
        .readings { display: grid; grid-template-columns: 1fr 1fr 1fr; gap: 15px; margin: 20px 0; }
        .reading { background: #e9ecef; padding: 15px; border-radius: 5px; text-align: center; }
        .value { font-size: 24px; font-weight: bold; color: #495057; }
        .label { font-size: 14px; color: #6c757d; margin-bottom: 5px; }
    </style>
</head>
<body>
    <div class='container'>
        <h1>Normal Injection Mode</h1>
        <p>Manual injection control with adjustable amplitude</p>
        
        <div class='readings'>
            <div class='reading'>
                <div class='label'>Voltage</div>
                <div class='value' id='voltageValue'>0.00 V</div>
            </div>
            <div class='reading'>
                <div class='label'>Current</div>
                <div class='value' id='currentValue'>0.00 A</div>
            </div>
            <div class='reading'>
                <div class='label'>Resistance</div>
                <div class='value' id='resistanceValue'>0.00 Ω</div>
            </div>
        </div>
        
        <div class='control-panel'>
            <h3>Amplitude Control</h3>
            <div class='slider-container'>
                <label for='amplitudeSlider'>Amplitude: <span id='amplitudeDisplay'>0%</span></label>
                <input type='range' id='amplitudeSlider' class='slider' min='0' max='100' value='0' onchange='updateAmplitude(this.value)'>
            </div>
            
            <div class='button-group'>
                <button class='btn btn-start' onclick='startInjection()'>Start Injection</button>
                <button class='btn btn-stop' onclick='stopInjection()'>Stop Injection</button>
            </div>
        </div>
        
        <div style='text-align: center; margin-top: 30px;'>
            <button class='btn btn-back' onclick="window.location.href='/'">Back to Main Menu</button>
        </div>
    </div>

    <script>
        function updateAmplitude(value) {
            document.getElementById('amplitudeDisplay').textContent = value + '%';
            fetch('/set_amplitude?value=' + value)
                .then(response => response.text())
                .then(data => console.log('Amplitude updated:', value));
        }
        
        function startInjection() {
            fetch('/set_amplitude?state=RUN')
                .then(response => response.text())
                .then(data => {
                    console.log('Injection started');
                });
        }
        
        function stopInjection() {
            fetch('/set_amplitude?state=STOP')
                .then(response => response.text())
                .then(data => {
                    console.log('Injection stopped');
                    document.getElementById('amplitudeSlider').value = 0;
                    document.getElementById('amplitudeDisplay').textContent = '0%';
                });
        }
        
        function updateReadings() {
            fetch('/status')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('voltageValue').textContent = data.voltage.toFixed(2) + ' V';
                    document.getElementById('currentValue').textContent = data.current.toFixed(2) + ' A';
                    document.getElementById('resistanceValue').textContent = data.resistance.toFixed(2) + ' Ω';
                    document.getElementById('amplitudeSlider').value = (data.amplitude * 100).toFixed(0);
                    document.getElementById('amplitudeDisplay').textContent = (data.amplitude * 100).toFixed(0) + '%';
                });
        }
        
        // Update readings every second
        updateReadings();
        setInterval(updateReadings, 1000);
    </script>
</body>
</html>
  )";
  
  server.send(200, "text/html", html);
}

void handle200mAInjection() {
  String html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>CORE Test - 200mA Injection</title>
    <meta charset='UTF-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <style>
        body { font-family: Arial; margin: 20px; background: #f0f0f0; }
        .container { max-width: 600px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; }
        .status-panel { background: #fff3cd; padding: 20px; border-radius: 8px; margin: 20px 0; border: 1px solid #ffeaa7; }
        .success-panel { background: #d4edda; border: 1px solid #c3e6cb; }
        .timer-display { font-size: 48px; font-weight: bold; text-align: center; color: #dc3545; margin: 20px 0; }
        .progress-bar { width: 100%; height: 20px; background: #f8f9fa; border-radius: 10px; overflow: hidden; margin: 20px 0; }
        .progress-fill { height: 100%; background: #dc3545; transition: width 0.5s ease; }
        .button-group { text-align: center; margin: 20px 0; }
        .btn { padding: 15px 30px; margin: 10px; border: none; border-radius: 5px; cursor: pointer; font-size: 16px; }
        .btn-start { background: #28a745; color: white; }
        .btn-stop { background: #dc3545; color: white; }
        .btn-back { background: #6c757d; color: white; }
        .readings { display: grid; grid-template-columns: 1fr 1fr 1fr; gap: 15px; margin: 20px 0; }
        .reading { background: #e9ecef; padding: 15px; border-radius: 5px; text-align: center; }
        .value { font-size: 24px; font-weight: bold; color: #495057; }
        .label { font-size: 14px; color: #6c757d; margin-bottom: 5px; }
    </style>
</head>
<body>
    <div class='container'>
        <h1>200mA Auto Injection</h1>
        <p>Automatically inject current until 200mA is reached, then start 2-minute countdown</p>
        
        <div class='readings'>
            <div class='reading'>
                <div class='label'>Voltage</div>
                <div class='value' id='voltageValue'>0.00 V</div>
            </div>
            <div class='reading'>
                <div class='label'>Current</div>
                <div class='value' id='currentValue'>0.00 A</div>
            </div>
            <div class='reading'>
                <div class='label'>Resistance</div>
                <div class='value' id='resistanceValue'>0.00 Ω</div>
            </div>
        </div>
        
        <div class='status-panel' id='statusPanel'>
            <h3>Status: <span id='statusText'>Ready to start</span></h3>
            <p id='statusDescription'>Click start to begin automatic injection to 200mA</p>
        </div>
        
        <div class='timer-display' id='timerDisplay' style='display: none;'>02:00</div>
        <div class='progress-bar' id='progressBar' style='display: none;'>
            <div class='progress-fill' id='progressFill' style='width: 100%;'></div>
        </div>
        
        <div class='button-group'>
            <button class='btn btn-start' id='startBtn' onclick='startAutoInjection()'>Start Auto Injection</button>
            <button class='btn btn-stop' id='stopBtn' onclick='stopInjection()' style='display: inline-block;'>Stop</button>
            <button class='btn btn-back' onclick="window.location.href='/'">Back to Main Menu</button>
        </div>
    </div>

    <script>
        let autoInjectionActive = false;
        let countdownActive = false;
        let targetTime = 0;
        let totalDuration = 120000; // 2 minutes in milliseconds
        
        function startAutoInjection() {
            fetch('/api/auto-injection?action=start', {method: 'POST'})
                .then(response => response.json())
                .then(data => {
                    if (data.success) {
                        autoInjectionActive = true;
                        document.getElementById('startBtn').style.display = 'none';
                        document.getElementById('stopBtn').style.display = 'inline-block';
                        updateStatus('auto_injection', 'Automatically increasing current to 200mA...');
                    }
                });
        }
        
        function stopInjection() {
            fetch('/api/auto-injection?action=stop', {method: 'POST'})
                .then(response => response.json())
                .then(data => {
                    autoInjectionActive = false;
                    countdownActive = false;
                    document.getElementById('startBtn').style.display = 'inline-block';
                    document.getElementById('stopBtn').style.display = 'none';
                    document.getElementById('timerDisplay').style.display = 'none';
                    document.getElementById('progressBar').style.display = 'none';
                    updateStatus('stopped', 'Injection stopped');
                });
        }
        
        function updateStatus(status, description) {
            const statusText = document.getElementById('statusText');
            const statusDescription = document.getElementById('statusDescription');
            const statusPanel = document.getElementById('statusPanel');
            
            statusText.textContent = status;
            statusDescription.textContent = description;
            
            if (status === 'countdown') {
                statusPanel.className = 'status-panel success-panel';
            }
        }
        
        function updateReadings() {
            fetch('/status')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('voltageValue').textContent = data.voltage.toFixed(2) + ' V';
                    document.getElementById('currentValue').textContent = data.current.toFixed(2) + ' A';
                    document.getElementById('resistanceValue').textContent = data.resistance.toFixed(2) + ' Ω';
                    
                    // Check auto injection status
                    if (data.autoInjectionActive && !countdownActive) {
                        updateStatus('injecting', 'Current: ' + data.current.toFixed(2) + ' A (Target: 200mA)');
                    } else if (data.countdownActive) {
                        countdownActive = true;
                        document.getElementById('timerDisplay').style.display = 'block';
                        document.getElementById('progressBar').style.display = 'block';
                        updateStatus('countdown', '200mA reached! Countdown in progress...');
                    }
                    
                    // Update countdown if active
                    if (countdownActive && data.countdownActive) {
                        const now = Date.now();
                        const remaining = data.countdownEndTime - now;
                        
                        if (remaining > 0) {
                            const minutes = Math.floor(remaining / 60000);
                            const seconds = Math.floor((remaining % 60000) / 1000);
                            document.getElementById('timerDisplay').textContent =
                                String(minutes).padStart(2, '0') + ':' + String(seconds).padStart(2, '0');
                            
                            const progress = (remaining / totalDuration) * 100;
                            document.getElementById('progressFill').style.width = progress + '%';
                        }
                    }
                });
        }
        
        // Update readings every second
        updateReadings();
        setInterval(updateReadings, 1000);
    </script>
</body>
</html>
  )";
  
  server.send(200, "text/html", html);
}

void handleDataSubmission() {
  String html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>CORE Test - Data Submission</title>
    <meta charset='UTF-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <style>
        body { font-family: Arial; margin: 20px; background: #f0f0f0; }
        .container { max-width: 600px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; }
        .form-group { margin: 20px 0; }
        label { display: block; margin-bottom: 5px; font-weight: bold; }
        input[type='text'] { width: 100%; padding: 10px; border: 1px solid #ddd; border-radius: 5px; box-sizing: border-box; }
        .btn { padding: 15px 30px; margin: 10px 5px; border: none; border-radius: 5px; cursor: pointer; font-size: 16px; }
        .btn-submit { background: #28a745; color: white; }
        .btn-submit:hover { background: #1e7e34; }
        .btn-back { background: #6c757d; color: white; }
        .status { padding: 15px; border-radius: 5px; margin: 20px 0; display: none; }
        .success { background: #d4edda; color: #155724; border: 1px solid #c3e6cb; }
        .error { background: #f8d7da; color: #721c24; border: 1px solid #f5c6cb; }
        .info { background: #d1ecf1; color: #0c5460; border: 1px solid #bee5eb; }
    </style>
</head>
<body>
    <div class='container'>
        <h1>Data Submission</h1>
        <p>Submit test results to cloud server</p>
        
        <form id='dataForm' onsubmit='submitData(event)'>
            <div class='form-group'>
                <label for='upt'>UPT:</label>
                <input type='text' id='upt' name='upt' required>
            </div>
            
            <div class='form-group'>
                <label for='nip'>NIP:</label>
                <input type='text' id='nip' name='nip' required>
            </div>
            
            <div class='form-group'>
                <label for='kodeSuit'>KODE_SUIT:</label>
                <input type='text' id='kodeSuit' name='kodeSuit' required>
            </div>

            <div class='form-group'>
                <label for='hasilR'>R (hasil uji):</label>
                <input type='text' id='hasilR' name='hasilR' readonly>
            </div>
            
            <div style='text-align: center; margin: 30px 0;'>
                <button type='submit' class='btn btn-submit'>Submit Data</button>
                <button type='button' class='btn btn-back' onclick="window.location.href='/'">Back to Main Menu</button>
            </div>
        </form>
        
        <div id='submissionStatus' class='status'></div>
    </div>

    <script>
        function submitData(event) {
            event.preventDefault();
            
            const formData = new FormData(document.getElementById('dataForm'));
            const data = {
                UPT: formData.get('upt'),
                NIP: formData.get('nip'),
                KODE_SUIT: formData.get('kodeSuit'),
                R: 0,
                voltage: 0,
                current: 0,
                resistance: 0,
                timestamp: new Date().toISOString()
            };
            
            // Get current readings
            fetch('/status')
                .then(response => response.json())
                .then(statusData => {
                    data.voltage = statusData.voltage;
                    data.current = statusData.current;
                    data.resistance = statusData.resistance;
                    data.R = statusData.resistance;

                    const rField = document.getElementById('hasilR');
                    if (rField) {
                        rField.value = statusData.resistance.toFixed(2);
                    }
                    
                    // Submit data
                    fetch('/api/submit-data', {
                        method: 'POST',
                        headers: {'Content-Type': 'application/json'},
                        body: JSON.stringify(data)
                    })
                    .then(response => response.json())
                    .then(result => {
                        if (result.success) {
                            showStatus('Data submitted successfully! Reference ID: ' + result.referenceId, 'success');
                            document.getElementById('dataForm').reset();
                        } else {
                            showStatus('Failed to submit data: ' + result.message, 'error');
                        }
                    })
                    .catch(error => {
                        showStatus('Error submitting data: ' + error.message, 'error');
                    });
                })
                .catch(error => {
                    showStatus('Error getting current readings: ' + error.message, 'error');
                });
        }
        
        function showStatus(message, type) {
            const status = document.getElementById('submissionStatus');
            status.textContent = message;
            status.className = 'status ' + type;
            status.style.display = 'block';
        }
    </script>
</body>
</html>
  )";
  
  server.send(200, "text/html", html);
}

void displayWebServerMode() {
  // Completely clear the screen and fill with black background
  tft.fillScreen(TFT_BLACK);
  
  // Draw header
  drawHeader();
  
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("Mode: Web Server", 120, 70, 2);
  
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("Controlled via", 120, 95, 2);
  
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Local Web", 120, 115, 2);
  
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString("SSID: CORE Test", 120, 160, 2);
  tft.drawString("IP: 192.168.4.1", 120, 175, 2);
  
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.drawString("Switch to Exit", 120, 195, 2);
}

void refreshWebServerDisplay() {
  // Static display - no updates needed
  // TFT display remains unchanged during web server mode
}

// ------------------- Header -------------------
void drawHeader() {
  tft.setTextDatum(MC_DATUM);

  // Efek shadow halus (abu-abu gelap di belakang tulisan)
  tft.setTextColor(tft.color565(30, 30, 30), TFT_BLACK);
  tft.drawString("CORE Test", 122, 14, 4);

  // Teks utama — biru langit cerah
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("CORE Test", 120, 12, 4);

  // Garis pemisah bawah (biru langit)
  tft.fillRect(0, 30, 240, 2, TFT_CYAN);
}

// ------------------- Frame -------------------
void drawFrame() {
  tft.fillScreen(TFT_BLACK);
  drawHeader();
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  tft.drawString("V:", X_LABEL, Y_V, 4);
  tft.drawString("I:", X_LABEL, Y_I, 4);
  tft.drawString("R:", X_LABEL, Y_R, 4);
  
  // garis biru langit horizontal di bawah R:
  tft.fillRect(0, 175, 240, 2, TFT_CYAN);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("TIME:", X_LABEL, Y_TIME, 2);
  tft.drawString("STATUS:", X_LABEL, Y_STATUS, 2);
}

// ------------------- Update Nilai -------------------
void updateValues(float v, float i, float r) {
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.fillRect(X_LABEL + 60, Y_V - 5, 150, 40, TFT_BLACK);
  tft.fillRect(X_LABEL + 60, Y_I - 5, 150, 40, TFT_BLACK);
  tft.fillRect(X_LABEL + 60, Y_R - 5, 150, 40, TFT_BLACK);

  tft.drawFloat(v, 2, X_VALUE, Y_V, 4);
  tft.drawFloat(i, 2, X_VALUE, Y_I, 4);
  tft.drawFloat(r, 2, X_VALUE, Y_R, 4);
}

// ------------------- Update Countdown -------------------
// ------------------- Update Countdown (dengan progress bar) -------------------
// ------------------- Update Countdown (Progress Bar Lebar) -------------------
// ------------------- Update Countdown (Progress Bar Lebar) -------------------
void updateTime() {
  // Don't update TFT display if in web server mode
  if (webServerMode) return;
  
  static int lastSecond = -1;
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);

  int barX = 120;
  int barY = Y_TIME + 3;
  int barW = 100;
  int barH = 10;

  String timeStr = "--:--";

  if (countdownActive) {
    unsigned long elapsed = millis() - countdownStart;
    unsigned long remaining = (countdownDuration > elapsed) ? countdownDuration - elapsed : 0;
    unsigned int secs = remaining / 1000;
    unsigned int mins = secs / 60;
    secs = secs % 60;

    float progress = (float)remaining / (float)countdownDuration;
    if (progress < 0) progress = 0;

    if ((int)secs != lastSecond) {
      lastSecond = secs;

      // Bersihkan area waktu & bar
      tft.fillRect(X_LABEL + 45, Y_TIME - 2, 180, 25, TFT_BLACK);

      // Tampilkan waktu
      char buf[10];
      sprintf(buf, "%02d:%02d", mins, secs);
      timeStr = buf;
      tft.drawString(timeStr, X_LABEL + 45, Y_TIME, 2);

      // Gambar progress bar
      int filled = barW * progress;
      tft.drawRoundRect(barX, barY, barW, barH, 3, TFT_CYAN);
      if (filled > 2)
        tft.fillRoundRect(barX + 1, barY + 1, filled - 2, barH - 2, 3, TFT_CYAN);
    }

    if (remaining == 0) {
      // When countdown finishes during auto injection, fully stop and reset
      if (autoInjectionMode) {
        stopAutoInjection();
        updateStatus();
      } else {
        countdownActive = false;
        lastSecond = -1;
        systemState = STOPPED;
        currentMenu = MENU_STOP;
        updateStatus();
        updateLEDsAndRelay();
      }
      
      // Refresh web server display if in web server mode
      if (webServerMode) {
        refreshWebServerDisplay();
      }
    }
    return;
  }

  // ---- Mode non-timer ----
  lastSecond = -1;
  tft.fillRect(X_LABEL + 45, Y_TIME - 2, 180, 25, TFT_BLACK);

  if (systemState == RUN) {
    // Mode RUN tanpa timer → hanya tampilkan "∞"
    timeStr = "∞";
    tft.drawString(timeStr, X_LABEL + 45, Y_TIME, 2);
    // Tidak ada progress bar di sini
  }
  else if (systemState == STOPPED) {
    timeStr = "00:00";
    tft.drawString(timeStr, X_LABEL + 45, Y_TIME, 2);
    // Tampilkan outline kosong untuk STOP
    tft.drawRoundRect(barX, barY, barW, barH, 3, TFT_CYAN);
  }
}





// ------------------- Update Status -------------------
void updateStatus() {
  tft.setTextDatum(TR_DATUM);
  tft.fillRect(X_LABEL + 80, Y_STATUS - 2, 140, 25, TFT_BLACK);

  switch (systemState) {
    case READY:
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.drawString("READY", X_VALUE, Y_STATUS, 2);
      break;
    case RUN:
      tft.setTextColor(TFT_CYAN, TFT_BLACK);
      tft.drawString("RUN", X_VALUE, Y_STATUS, 2);
      break;
    case STOPPED:
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.drawString("STOP", X_VALUE, Y_STATUS, 2);
      break;
  }
}

// ------------------- LED + RELAY -------------------
void updateLEDsAndRelay() {
  digitalWrite(LED_RUNTIME, currentMenu == MENU_RUNTIME ? HIGH : LOW);
  digitalWrite(LED_STOP,    currentMenu == MENU_STOP ? HIGH : LOW);
  digitalWrite(LED_RUN,     currentMenu == MENU_RUN ? HIGH : LOW);

  digitalWrite(RELAY_PIN, (systemState == RUN) ? LOW : HIGH);
}

// ------------------- Eksekusi Menu -------------------
void executeMenu(MenuItem menu) {
  switch (menu) {
    case MENU_RUNTIME:
      countdownDuration = 2 * 60 * 1000UL; // 2 menit
      countdownStart = millis();
      countdownActive = true;
      systemState = RUN;
      Serial.println("▶️ RUN TIME");
      break;

    case MENU_RUN:
      systemState = RUN;
      countdownActive = false;
      Serial.println("▶️ RUN (NO TIMER)");
      break;

    case MENU_STOP:
      systemState = STOPPED;
      countdownActive = false;
      ampValue = 0.0;  // Reset amplitude ke 0 saat STOP
      updateDAC();
      Serial.println("⏹ STOP");
      break;
  }

  updateStatus();
  updateTime();
  updateLEDsAndRelay();
  
  // Refresh web server display if in web server mode
  if (webServerMode) {
    refreshWebServerDisplay();
  }
}

// ------------------- Baca JSY1050 -------------------
void readJSY1050() {
  uint8_t result = node.readHoldingRegisters(0x0048, 10);
  if (result == node.ku8MBSuccess) {
    voltage = node.getResponseBuffer(0) / 100.0f;
    currentA = node.getResponseBuffer(1) / 100.0f;
    resistanceVal = (currentA > 0.01f) ? voltage / currentA : 0.0f;
  }
}

// ------------------- Tombol -------------------
void handleButtons() {
  keyUp.loop();
  // keyDown.loop();

  if (keyUp.isPressed()) {
    currentMenu = (MenuItem)((int)currentMenu - 1);
    if ((int)currentMenu < 0) currentMenu = (MenuItem)(MENU_COUNT - 1);
    executeMenu(currentMenu);
  }

  // if (keyDown.isPressed()) {
  //   currentMenu = (MenuItem)((int)currentMenu + 1);
  //   if ((int)currentMenu >= MENU_COUNT) currentMenu = (MenuItem)0;
  //   executeMenu(currentMenu);
  // }
}

// ------------------- Rotary Encoder & Serial to Nano -------------------
void handleRotaryEncoder() {
  // Hanya izinkan pengaturan amplitude saat RUN atau RUN TIME
  if (systemState != RUN) return;

  int currentCLK = digitalRead(CLK_PIN);

  if (currentCLK != lastCLK && currentCLK == LOW) {
    if (digitalRead(DT_PIN) == HIGH) {
      ampValue += stepSize;        // putar kanan → naik
    } else {
      ampValue -= stepSize;        // putar kiri → turun
    }

    ampValue = constrain(ampValue, 0.0, 1.0);
    updateDAC();
  }

  lastCLK = currentCLK;

  // -------- Tombol tekan (reset amplitude) --------
  if (digitalRead(SW_PIN) == LOW) {
    ampValue = 0.0;
    updateDAC();
    delay(200);
  }
}

void updateDAC() {
  int dacValue = constrain(ampValue * 255, 0, 255);  // Konversi ke 0-255
  dacWrite(DAC_PIN, dacValue);
  Serial.print("DAC = ");
  Serial.println(dacValue);
}

void handleWiFiSettings() {
  if (server.method() == HTTP_POST) {
    String ssid = server.arg("ssid");
    String password = server.arg("password");
    String cloudServer = server.arg("cloudServer");
    
    if (ssid.length() > 0) {
      // Save to both RAM and NVS memory
      wifiSSID = ssid;
      wifiPassword = password;
      
      // Save to NVS memory for persistence
      saveSettingsToMemory(ssid, password, cloudServer);
      cloudServerAddress = cloudServer;
      
      Serial.println("Settings updated in RAM:");
      Serial.println("WiFi SSID: " + wifiSSID);
      Serial.println("Cloud Server: " + cloudServerAddress);
      
      // Switch to dual mode and try to connect while maintaining AP
      WiFi.mode(WIFI_AP_STA);
      Serial.println("Attempting to connect to WiFi...");
      WiFi.begin(ssid.c_str(), password.c_str());
      
      // Wait for connection with shorter timeout
      int attempts = 0;
      bool connected = false;
      while (attempts < 10 && WiFi.status() != WL_CONNECTED) {
        delay(500);
        attempts++;
        Serial.print(".");
      }
      
      if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        wifiStationMode = true;
        
        Serial.println("");
        Serial.println("WiFi connected successfully!");
        Serial.println("IP: " + WiFi.localIP().toString());
        Serial.println("AP still active at 192.168.4.1");
        
        String response = "{\"success\": true, \"message\": \"Connected to WiFi, AP still active\", \"ip\": \"" + WiFi.localIP().toString() + "\"}";
        server.send(200, "application/json", response);
      } else {
        wifiConnected = false;
        Serial.println("");
        Serial.println("WiFi connection failed, AP mode maintained");
        
        String response = "{\"success\": false, \"message\": \"Failed to connect, AP mode maintained\", \"ip\": \"192.168.4.1\"}";
        server.send(200, "application/json", response);
      }
    } else {
      server.send(400, "application/json", "{\"success\": false, \"message\": \"SSID is required\"}");
    }
  } else {
    server.send(405, "application/json", "{\"success\": false, \"message\": \"Method not allowed\"}");
  }
}

void handleCloudSettings() {
  if (server.method() == HTTP_POST) {
    String host = server.arg("host");
    String port = server.arg("port");
    String user = server.arg("user");
    String pass = server.arg("pass");
    String clientId = server.arg("clientId");
    String topic = server.arg("topic");
    
    if (host.length() > 0) {
      // Update RAM values
      mqttHost = host;
      mqttPort = port.toInt();
      mqttUser = user;
      mqttPass = pass;
      mqttClientId = clientId;
      mqttTopic = topic;
      
      // Persist to NVS
      Preferences preferences;
      preferences.begin("core-settings", false);
      preferences.putString("mqttHost", mqttHost);
      preferences.putInt("mqttPort", mqttPort);
      preferences.putString("mqttUser", mqttUser);
      preferences.putString("mqttPass", mqttPass);
      preferences.putString("mqttClientId", mqttClientId);
      preferences.putString("mqttTopic", mqttTopic);
      preferences.end();
      
      Serial.println("MQTT settings saved:");
      Serial.println("Host: " + mqttHost);
      Serial.println("Port: " + String(mqttPort));
      Serial.println("User: " + mqttUser);
      Serial.println("Client ID: " + mqttClientId);
      Serial.println("Topic: " + mqttTopic);
      
      server.send(200, "application/json", "{\"success\": true, \"message\": \"MQTT settings saved\"}");
    } else {
      server.send(400, "application/json", "{\"success\": false, \"message\": \"MQTT host is required\"}");
    }
  } else {
    server.send(405, "application/json", "{\"success\": false, \"message\": \"Method not allowed\"}");
  }
}

void handleWiFiReset() {
  if (server.method() == HTTP_POST) {
    Serial.println("WiFi reset requested - clearing saved settings...");
    
    // Clear all saved settings and reset to default
    resetWiFiSettings();
    
    server.send(200, "application/json", "{\"success\": true, \"message\": \"WiFi reset to PDKB_INTERNET_G\"}");
    
    // Restart ESP32 after 2 seconds
    delay(2000);
    ESP.restart();
  } else {
    server.send(405, "application/json", "{\"success\": false, \"message\": \"Method not allowed\"}");
  }
}

void stopAutoInjection() {
  autoInjectionMode = false;
  targetReached = false;
  lampTimeBased = false;
  systemState = STOPPED;
  countdownActive = false;
  ampValue = 0.0;
  updateDAC();
  // Saat auto injection (Spesial Conduct Suit Test) berhenti,
  // LED kembali menunjukkan STOP.
  currentMenu = MENU_STOP;
  updateLEDsAndRelay();
  
  Serial.println("Auto injection stopped");
}

void handleWiFiInterference() {
  // Switch to dual mode: AP + STA (Access Point + Station)
  // This allows ESP32 to be both a WiFi client (internet) and access point (local web server)
  Serial.println("Configuring WiFi for dual mode (AP + STA)...");
  WiFi.mode(WIFI_AP_STA);
  
  // Configure Access Point
  WiFi.softAPConfig(localIP, gateway, subnet);
  WiFi.softAP(ap_ssid, ap_password);
  
  // Keep existing STA connection if connected to internet WiFi
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Already connected to internet WiFi: " + WiFi.SSID());
    Serial.println("Local IP: " + WiFi.localIP().toString());
  } else {
    Serial.println("No internet WiFi connection found");
  }
  
  Serial.println("AP IP: " + WiFi.softAPIP().toString());
}

void updateTimeBasedLamp() {
  if (lampTimeBased && autoInjectionMode) {
    unsigned long currentTime = millis();
    
    // Check if lamp duration exceeded
    if (currentTime - lampStartTime >= lampDuration) {
      lampTimeBased = false;
      lampState = false;
      digitalWrite(LED_RUNTIME, LOW);  // Turn off LED_RUNTIME for start_time mode
      Serial.println("Time-based lamp duration exceeded, turning off lamp");
      return;
    }
    
    // Blink lamp every second
    if (currentTime - lastLampToggle >= LAMP_BLINK_INTERVAL) {
      lampState = !lampState;
      digitalWrite(LED_RUNTIME, lampState ? HIGH : LOW);  // Use LED_RUNTIME for start_time mode
      lastLampToggle = currentTime;
      
      Serial.print("Time-based lamp: ");
      Serial.println(lampState ? "ON" : "OFF");
    }
  }
}

void handleAutoInjection() {
  if (server.method() == HTTP_POST) {
    String action = server.arg("action");
    
    if (action == "inject") {
      // Inject 200mA in 3 seconds
      if (!autoInjectionMode) {
        autoInjectionMode = true;
        targetReached = false;
        ampValue = 0.0;
        updateDAC();
        systemState = RUN;
        currentMenu = MENU_RUN;
        updateLEDsAndRelay();
        
        // Calculate steps: 100% in 3 seconds = 33.33% per second
        // Increment every 100ms = 3.33% per step, 30 steps total
        
        server.send(200, "application/json", "{\"success\":true,\"message\":\"Injecting 200mA\",\"duration\":3}");
        Serial.println("Inject 200mA started (3 seconds)");
      } else {
        server.send(200, "application/json", "{\"success\":false,\"message\":\"Already injecting\"}");
      }
    } else if (action == "record") {
      // Start 2 minute recording
      if (targetReached && !countdownActive) {
        countdownActive = true;
        countdownStart = millis();
        countdownDuration = 2 * 60 * 1000UL; // 2 minutes
        
        server.send(200, "application/json", "{\"success\":true,\"message\":\"Recording started\",\"duration\":120}");
        Serial.println("Recording started (2 minutes)");
      } else {
        server.send(200, "application/json", "{\"success\":false,\"message\":\"Cannot start recording\"}");
      }
    } else if (action == "stop") {
      stopAutoInjection();
      countdownActive = false;
      server.send(200, "application/json", "{\"success\":true,\"message\":\"Stopped\"}");
    } else {
      server.send(400, "application/json", "{\"success\":false,\"message\":\"Invalid action\"}");
    }
  } else {
    server.send(405, "application/json", "{\"success\":false,\"message\":\"Method not allowed\"}");
  }
}

void sendDataToCloud() {
  if (server.method() == HTTP_POST) {
    String jsonData = server.arg("plain");
    
    String response;
    bool success = sendDataToCloudServer(jsonData, response);
    
    if (success) {
      dataSubmitted = true;
      lastSubmissionStatus = "Success";
    } else {
      dataSubmitted = false;
      lastSubmissionStatus = "Failed";
    }
    
    server.send(200, "application/json", response);
  } else {
    server.send(405, "application/json", "{\"success\": false, \"message\": \"Method not allowed\"}");
  }
}

bool sendDataToCloudServer(const String& jsonData, String& response) {
  Serial.println("=== Sending Data to Cloud Server ===");
  Serial.println("Cloud Server Address: " + cloudServerAddress);
  Serial.println("WiFi Status: " + String(WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected"));
  Serial.println("WiFi SSID: " + WiFi.SSID());
  Serial.println("Signal Strength: " + String(WiFi.RSSI()) + " dBm");
  
  if (WiFi.status() != WL_CONNECTED) {
    response = "{\"success\": false, \"message\": \"WiFi not connected\"}";
    return false;
  }
  
  HTTPClient http;
  WiFiClient client;
  
  // Parse incoming JSON and add ESP32 specific data
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, jsonData);
  
  if (error) {
    response = "{\"success\": false, \"message\": \"Invalid JSON data\"}";
    return false;
  }
  
  // Add ESP32 specific metadata
  doc["device_id"] = "CORE_ESP32_" + String((uint32_t)ESP.getEfuseMac(), HEX);
  doc["firmware_version"] = "1.0.0";
  doc["submission_timestamp"] = String(millis());
  doc["network_status"] = WiFi.SSID();
  doc["signal_strength"] = WiFi.RSSI();
  doc["local_ip"] = WiFi.localIP().toString();
  
  // Serialize updated JSON
  String finalJson;
  serializeJson(doc, finalJson);
  
  Serial.println("Final JSON to send: " + finalJson);
  
  // Configure HTTP request
  http.begin(client, cloudServerAddress);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("User-Agent", "CORE-ESP32/1.0.0");
  
  // Send POST request
  int httpResponseCode = http.POST(finalJson);
  
  if (httpResponseCode > 0) {
    String responseBody = http.getString();
    Serial.println("HTTP Response Code: " + String(httpResponseCode));
    Serial.println("Response Body: " + responseBody);
    
    if (httpResponseCode == 200 || httpResponseCode == 201) {
      response = "{\"success\": true, \"message\": \"Data submitted successfully\", \"http_code\": " +
                String(httpResponseCode) + ", \"response\": " + responseBody + "}";
      return true;
    } else {
      response = "{\"success\": false, \"message\": \"Server error\", \"http_code\": " +
                String(httpResponseCode) + ", \"response\": " + responseBody + "}";
      return false;
    }
  } else {
    String errorMsg = "HTTP request failed: " + http.errorToString(httpResponseCode);
    response = "{\"success\": false, \"message\": \"" + errorMsg + "\"}";
    return false;
  }
  
  http.end();
}

void handleInjectAPI() {
  // Parse JSON body from pengujian.html
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, body);
    
    if (error) {
      server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
      return;
    }
    
    String mode = doc["mode"] | "quick";
    int amplitudePercent = doc["amplitude"] | 0; // Amplitude dari slider (0-100%)
    
    if (mode == "special") {
      // Special mode: auto-increment sampai 200mA (100%) dengan countdown
      int duration = doc["duration"] | 15; // Default 15 seconds
      userCountdownDuration = duration * 1000UL;
      autoInjectionMode = true; // Pakai auto-increment dari 0% ke 100%
      targetReached = false;
      systemState = RUN;
      ampValue = 0.0; // Mulai dari 0%, auto-increment akan naikkan perlahan
      countdownActive = false; // Countdown belum aktif (aktif setelah reach 200mA)
      lastAutoIncrement = millis(); // Reset timer auto-increment
      
      updateDAC();
      digitalWrite(RELAY_PIN, HIGH);
      digitalWrite(LED_RUN, HIGH);
      digitalWrite(LED_STOP, LOW);
      
      Serial.println("=== SPECIAL MODE: AUTO-INCREMENT TO 200mA ===");
      Serial.print("Starting from: 0%");
      Serial.print(", Target: 200mA (100%)");
      Serial.print(", Countdown after reach: ");
      Serial.print(duration);
      Serial.println(" seconds");
      
      server.send(200, "application/json", "{\"success\":true,\"message\":\"Special: Auto-increment to 200mA\"}");
    } else {
      // Quick mode: set amplitude SESUAI SLIDER (0-100%)
      ampValue = amplitudePercent / 100.0; // Convert percent to 0.0-1.0
      systemState = RUN;
      autoInjectionMode = false;
      countdownActive = false; // Tidak ada countdown otomatis di Quick mode
      
      // Update DAC sesuai amplitude
      updateDAC();
      digitalWrite(RELAY_PIN, HIGH);
      digitalWrite(LED_RUN, HIGH);
      digitalWrite(LED_STOP, LOW);
      
      Serial.println("=== QUICK MODE: MANUAL CONTROL ===");
      Serial.print("Amplitude set to: ");
      Serial.print(amplitudePercent);
      Serial.print("% (ampValue: ");
      Serial.print(ampValue);
      Serial.print(", DAC: ");
      Serial.print((int)(ampValue * 255));
      Serial.println(")");
      
      String msg = "{\"success\":true,\"message\":\"Quick: " + String(amplitudePercent) + "% set\"}";
      server.send(200, "application/json", msg);
    }
    
    updateStatus();
  } else {
    server.send(400, "application/json", "{\"error\":\"No data\"}");
  }
}

void handleStopAPI() {
  // Stop injection - called from pengujian.html
  systemState = STOPPED;
  ampValue = 0.0;
  autoInjectionMode = false;
  targetReached = false;
  countdownActive = false;
  updateDAC();
  updateStatus();
  updateLEDsAndRelay();
  
  server.send(200, "application/json", "{\"success\":true,\"message\":\"Stopped\"}");
}

void handleGetStatus() {
  // Debug: print current sensor values
  Serial.print("Status Request - V:");
  Serial.print(voltage);
  Serial.print("V, I:");
  Serial.print(currentA * 1000);
  Serial.print("mA, R:");
  Serial.print(resistanceVal);
  Serial.println("Ω");
  
  String json = "{";
  json += "\"voltage\":" + String(voltage, 2) + ",";
  json += "\"current\":" + String(currentA, 3) + ",";
  json += "\"resistance\":" + String(resistanceVal, 2) + ",";
  json += "\"state\":\"" + String(systemState == RUN ? "RUN" : (systemState == READY ? "READY" : "STOP")) + "\",";
  json += "\"amplitude\":" + String(ampValue, 3) + ",";
  json += "\"countdownActive\":" + String(countdownActive ? "true" : "false") + ",";
  json += "\"autoInjectionActive\":" + String(autoInjectionMode ? "true" : "false") + ",";
  json += "\"targetReached\":" + String(targetReached ? "true" : "false") + ",";
  json += "\"wifiConnected\":" + String(wifiConnected ? "true" : "false") + ",";
  json += "\"wifiSSID\":\"" + wifiSSID + "\",";
  
  // Add countdown time if active
  if (countdownActive) {
    unsigned long elapsed = millis() - countdownStart;
    unsigned long remaining = (countdownDuration > elapsed) ? countdownDuration - elapsed : 0;
    unsigned int secs = remaining / 1000;
    unsigned int mins = secs / 60;
    secs = secs % 60;
    
    char timeBuf[10];
    sprintf(timeBuf, "%02d:%02d", mins, secs);
    json += "\"countdownTime\":\"" + String(timeBuf) + "\",";
    json += "\"countdownEndTime\":" + String(countdownStart + countdownDuration) + ",";
  }
  
  json += "\"menu\":\"" + String(currentMenu == MENU_RUNTIME ? "RUNTIME" : (currentMenu == MENU_RUN ? "RUN" : "STOP")) + "\"";
  json += "}";
  
  server.send(200, "application/json", json);
}

void handleSetAmplitude() {
  if (server.hasArg("value")) {
    float newAmplitude = server.arg("value").toFloat() / 100.0;
    ampValue = constrain(newAmplitude, 0.0, 1.0);
    updateDAC();
    server.send(200, "text/plain", "OK");
  } else if (server.hasArg("state")) {
    String state = server.arg("state");
    if (state == "RUN") {
      systemState = RUN;
      currentMenu = MENU_RUN;
      countdownActive = false;
    } else if (state == "RUNTIME") {
      systemState = RUN;
      currentMenu = MENU_RUNTIME;
      countdownDuration = 2 * 60 * 1000UL; // 2 menit
      countdownStart = millis();
      countdownActive = true;
    } else if (state == "STOP") {
      systemState = STOPPED;
      currentMenu = MENU_STOP;
      countdownActive = false;
      ampValue = 0.0;
      updateDAC();
    }
    updateStatus();
    updateTime();
    updateLEDsAndRelay();
    
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Bad Request");
  }
}

// ------------------- Setup -------------------
void setup() {
 Serial.begin(115200);
 Serial.println("Starting JSY1050 Dashboard...");

 // Load saved WiFi & cloud settings from NVS on boot
 loadSettingsFromMemory();

 tft.init();
  tft.setRotation(0);
  // starfieldIntro();
  drawFrame();
  updateValues(0, 0, 0);
  updateTime();
  updateStatus();

  keyUp.setDebounceTime(50);
  pinMode(CLK_PIN, INPUT_PULLUP);
  pinMode(DT_PIN, INPUT_PULLUP);
  pinMode(SW_PIN, INPUT_PULLUP);
  lastCLK = digitalRead(CLK_PIN);
  // keyDown.setDebounceTime(50);

  pinMode(LED_RUNTIME, OUTPUT);
  pinMode(LED_STOP, OUTPUT);
  pinMode(LED_RUN, OUTPUT);
  updateDAC();  // Set DAC awal ke 0
  pinMode(RELAY_PIN, OUTPUT);
  updateLEDsAndRelay();

  SerialJSY.begin(9600, SERIAL_8N1, RXD_JSY, TXD_JSY);
  node.begin(1, SerialJSY);
  Serial.println("Modbus JSY1050 initialized.");
  
  // Configure web server switch pin
  pinMode(WEB_SERVER_SWITCH, INPUT_PULLUP);
  Serial.print("Web server switch configured on pin ");
  Serial.println(WEB_SERVER_SWITCH);
  Serial.println("Waiting for switch press on GPIO 22 (active low)...");
}

void loop() {
  checkWebServerSwitch();
  
  if (!webServerMode) {
    // Only allow local control when not in web server mode
    handleButtons();
    handleRotaryEncoder();
  } else {
    // Handle web server requests
    handleWebServer();
  }

  // Auto injection logic - Inject 200mA with precise amplitude control
  if (autoInjectionMode) {
    // Update every 50ms for faster response
    if (millis() - lastAutoIncrement >= 50) {
      lastAutoIncrement = millis();
      
      float currentMA = currentA * 1000.0f; // Convert to mA
      float targetMA = targetCurrent * 1000.0f; // 200mA
      float error = targetMA - currentMA;
      
      // Tolerance: ±5mA (very tight)
      if (abs(error) <= 5.0f) {
        // Within tolerance - target reached! Keep running at 200mA
        if (!targetReached) {
          targetReached = true;
          Serial.println("✅ 200mA target reached! Ready for RECORD.");
          Serial.print("   Current: ");
          Serial.print(currentMA, 1);
          Serial.print("mA, Amplitude: ");
          Serial.print(ampValue * 100, 1);
          Serial.println("%");
        }
        // Keep amplitude stable, don't adjust
      } else if (currentMA < targetMA) {
        // Current too low - increase amplitude
        if (error > 50) {
          ampValue += 0.01f; // Large error: +1%
        } else if (error > 20) {
          ampValue += 0.005f; // Medium error: +0.5%
        } else {
          ampValue += 0.001f; // Small error: +0.1%
        }
        
        if (ampValue > 1.0f) ampValue = 1.0f;
        updateDAC();
        
        Serial.print("📈 UP: ");
        Serial.print(currentMA, 1);
        Serial.print("mA → ");
        Serial.print(targetMA, 0);
        Serial.print("mA (");
        Serial.print(ampValue * 100, 1);
        Serial.println("%)");
      } else {
        // Current too high - decrease amplitude
        if (error < -50) {
          ampValue -= 0.01f; // Large overshoot: -1%
        } else if (error < -20) {
          ampValue -= 0.005f; // Medium overshoot: -0.5%
        } else {
          ampValue -= 0.001f; // Small overshoot: -0.1%
        }
        
        if (ampValue < 0.0f) ampValue = 0.0f;
        updateDAC();
        
        Serial.print("📉 DOWN: ");
        Serial.print(currentMA, 1);
        Serial.print("mA → ");
        Serial.print(targetMA, 0);
        Serial.print("mA (");
        Serial.print(ampValue * 100, 1);
        Serial.println("%)");
      }
    }
  }
  
  // Timer countdown logic - stops injection after 2 minutes
  if (countdownActive && autoInjectionMode) {
    unsigned long elapsed = millis() - countdownStart;
    if (elapsed >= countdownDuration) {
      // 2 minutes elapsed - stop injection completely
      Serial.println("⏱️ Timer 2 menit habis! Stopping injection...");
      stopAutoInjection();
    }
  }
  
  // Update time-based lamp functionality
  updateTimeBasedLamp();
  
  // Always update LEDs regardless of mode
  updateLEDsAndRelay();

  if (millis() - lastRead >= READ_INTERVAL) {
    lastRead = millis();
    readJSY1050();
    // Only update V,I,R display when NOT in web server mode
    if (systemState == RUN && !webServerMode) {
      updateValues(voltage, currentA, resistanceVal);
    }
  }

  if (countdownActive && !webServerMode) {
    updateTime();
  }
}
