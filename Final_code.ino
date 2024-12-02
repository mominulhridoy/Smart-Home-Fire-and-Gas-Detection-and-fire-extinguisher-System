#include <WiFi.h>
#include <ESPAsyncWebServer.h>

#define GAS_SENSOR_PIN 34      
#define FLAME_SENSOR_PIN 35    
#define BUZZER_PIN 26          
#define RELAY_PIN 27           
#define WIFI_SSID "Your SSID"    
#define WIFI_PASSWORD "Your Password" 

AsyncWebServer server(80);

const int gasThreshold = 400;  
bool isGasDetected = false;
bool isFireDetected = false;
bool overrideMode = false;  // Flag for override mode

// Declare global variables for sensor readings
float gasLevel = 0;           
bool flameStatus = false;     
bool relayOverride = false;   
bool buzzerOverride = false;  

const float R0 = 10.0; // Baseline resistance of MQ-135 in clean air (in kOhms)
const float exponent = 1.6;  // Exponent value for MQ-135

// Function to convert ADC reading to PPM
float convertToPPM(int gasReading) {
  // Calculate the sensor's voltage
  float voltage = gasReading * (3.3 / 1023.0); // Assuming 3.3V reference and 10-bit ADC
  
  // Convert voltage to resistance (simplified model)
  float resistance = (3.3 - voltage) / voltage;  // Rough calculation for MQ sensors
  
  // Apply conversion formula to get PPM
  float ppm = pow((resistance / R0), -exponent) * 10;
  
  return ppm;  
}

void setup() {
  // Pin configuration
  pinMode(GAS_SENSOR_PIN, INPUT);
  pinMode(FLAME_SENSOR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  
  
  digitalWrite(BUZZER_PIN, LOW);  
  digitalWrite(RELAY_PIN, HIGH);  // Relay OFF (active-low)
  
  // Initialize serial communication
  Serial.begin(115200);
  delay(2000);  
  
  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());  
  
  // Serve the web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = R"rawliteral(
      <!DOCTYPE html>
      <html>
      <head>
        <title>Smart Home Control</title>
        <style>
          body {
            font-family: Arial, sans-serif;
            text-align: center;
            margin: 0;
            padding: 0;
            background-color: #f4f4f9;
          }
          .container {
            margin-top: 50px;
          }
          h1 {
            color: #333;
          }
          .button {
            display: inline-block;
            margin: 10px;
            padding: 15px 30px;
            font-size: 16px;
            color: #fff;
            background-color: #007bff;
            border: none;
            border-radius: 5px;
            text-decoration: none;
            cursor: pointer;
          }
          .button:hover {
            background-color: #0056b3;
          }
          .button.off {
            background-color: #dc3545;
          }
          .button.off:hover {
            background-color: #c82333;
          }
          .status-box {
            margin-top: 20px;
            font-size: 18px;
            padding: 10px;
            border: 1px solid #ddd;
            background-color: #fff;
          }
        </style>
        <script>
          function sendRequest(action) {
            fetch(action)
              .then(response => response.text())
              .then(data => {
                console.log(data); // For debugging
              })
              .catch(error => {
                console.error('Error:', error);
              });
          }

          function updateStatus() {
            fetch('/status')
              .then(response => response.json())
              .then(data => {
                document.getElementById('gasLevel').innerText = data.gasLevel;
                document.getElementById('flameStatus').innerText = data.flameStatus ? 'Fire Detected' : 'No Fire';
                document.getElementById('relayStatus').innerText = data.relayStatus ? 'ON' : 'OFF';
                document.getElementById('buzzerStatus').innerText = data.buzzerStatus ? 'ON' : 'OFF';
              })
              .catch(error => {
                console.error('Error:', error);
              });
          }

          setInterval(updateStatus, 1000); // Update status every second
        </script>
      </head>
      <body>
        <div class="container">
          <h1>Smart Home Control Panel</h1>
          <div>
            <button class="button" onclick="sendRequest('/relay/on')">Turn Relay ON</button>
            <button class="button off" onclick="sendRequest('/relay/off')">Turn Relay OFF</button>
          </div>
          <div>
            <button class="button" onclick="sendRequest('/buzzer/on')">Turn Buzzer ON</button>
            <button class="button off" onclick="sendRequest('/buzzer/off')">Turn Buzzer OFF</button>
          </div>
          <div class="status-box">
            <p>Gas Level (PPM): <span id="gasLevel">Loading...</span></p>
            <p>Flame Status: <span id="flameStatus">Loading...</span></p>
            <p>Relay Status: <span id="relayStatus">Loading...</span></p>
            <p>Buzzer Status: <span id="buzzerStatus">Loading...</span></p>
          </div>
        </div>
      </body>
      </html>
    )rawliteral";
    request->send(200, "text/html", html);
  });

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    String json = "{";
    json += "\"gasLevel\":" + String(gasLevel) + ","; // Actual gas sensor reading (PPM)
    json += "\"flameStatus\":" + String(flameStatus) + ","; // Actual flame status
    json += "\"relayStatus\":" + String(digitalRead(RELAY_PIN) == LOW) + ","; // Relay status
    json += "\"buzzerStatus\":" + String(digitalRead(BUZZER_PIN) == HIGH); // Buzzer status
    json += "}";
    request->send(200, "application/json", json);
  });

  server.on("/relay/on", HTTP_GET, [](AsyncWebServerRequest *request) {
    digitalWrite(RELAY_PIN, LOW); 
    relayOverride = true;         
    request->send(200, "text/plain", "Relay is ON");
  });

  server.on("/relay/off", HTTP_GET, [](AsyncWebServerRequest *request) {
    digitalWrite(RELAY_PIN, HIGH); 
    relayOverride = false;         
    request->send(200, "text/plain", "Relay is OFF");
  });

  server.on("/buzzer/on", HTTP_GET, [](AsyncWebServerRequest *request) {
    digitalWrite(BUZZER_PIN, HIGH); 
    buzzerOverride = true;          
    request->send(200, "text/plain", "Buzzer is ON");
  });

  server.on("/buzzer/off", HTTP_GET, [](AsyncWebServerRequest *request) {
    digitalWrite(BUZZER_PIN, LOW); 
    buzzerOverride = false;       
    request->send(200, "text/plain", "Buzzer is OFF");
  });

  // Start the server
  server.begin();
}

void loop() {
  
  int gasReading = analogRead(GAS_SENSOR_PIN);  

  
  gasLevel = convertToPPM(gasReading);  

  
  if (gasLevel > gasThreshold && !isGasDetected) {
    isGasDetected = true;
    digitalWrite(BUZZER_PIN, HIGH); 
  } else if (gasLevel <= gasThreshold && isGasDetected) {
    isGasDetected = false;
    digitalWrite(BUZZER_PIN, LOW);  
  }

  // Detect flame and handle buzzer and relay triggers
  flameStatus = digitalRead(FLAME_SENSOR_PIN) == LOW;  

  if (flameStatus && !isFireDetected) {
    isFireDetected = true;
    digitalWrite(BUZZER_PIN, HIGH); 
    digitalWrite(RELAY_PIN, LOW);   
  } else if (!flameStatus && isFireDetected) {
    isFireDetected = false;
    digitalWrite(BUZZER_PIN, LOW);  
    digitalWrite(RELAY_PIN, HIGH);  
  }


  Serial.print("Gas Level (PPM): ");
  Serial.println(gasLevel);
  Serial.println(flameStatus ? "Fire Detected" : "No Fire");
  delay(1000); 
}
