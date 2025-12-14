// This is the code used to create an API server to receive requests on Arduino Uno R4

#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"
#include "WiFiS3.h"
#include <ArduinoJson.h>

#define LATCH_PIN 4
#define CLK_PIN 7
#define DATA_PIN 8

// 7-segment display hex codes for numbers 0-9 (Common Anode)
const byte SEG_OFF = 0xFF;
const byte ZERO_HEX = 0xC0;
const byte ONE_HEX = 0xF9;
const byte TWO_HEX = 0xA4;
const byte THREE_HEX = 0xB0;
const byte FOUR_HEX = 0x99;
const byte FIVE_HEX = 0x92;
const byte SIX_HEX = 0x82;
const byte SEVEN_HEX = 0xF8;
const byte EIGHT_HEX = 0x80;
const byte NINE_HEX = 0x90;

byte numbers[] = { ZERO_HEX, ONE_HEX, TWO_HEX, THREE_HEX, FOUR_HEX, FIVE_HEX,
                   SIX_HEX, SEVEN_HEX, EIGHT_HEX, NINE_HEX };

const byte SEGMENT_SELECT[] = { 0xF1, 0xF2, 0xF4, 0xF8 };

int led1 = 13;
int led2 = 12;
int led3 = 11;
int led4 = 10;

int buzzer_pin = 3;

byte select_seg1 = 0xF1;
byte select_seg2 = 0xF2;
byte select_seg3 = 0xF4;
byte select_seg4 = 0xF8;

ArduinoLEDMatrix matrix;

const char ssid[] = "Naman_4G";    //Enter your WIFI SSID
const char pass[] = "8287285756";  //Enter your WIFI password
int keyIndex = 0;

String output = "off";
String header;

unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000;

int status = WL_IDLE_STATUS;
WiFiServer server(80);

void setup() {
  Serial.begin(9600);
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(CLK_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
  pinMode(buzzer_pin, OUTPUT);
  analogWrite(buzzer_pin, 255);

  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);

  LedSOFF();
  matrix.begin();

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true)
      ;
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  while (status != WL_CONNECTED) {
    Serial.print("Trying to connect to network named: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(5000);
  }
  server.begin();
  printWifiStatus();
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New Client.");
    String currentLine = "";
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) {
      currentTime = millis();
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        header += c;
        if (c == '\n') {
          if (currentLine.length() == 0) {
            // Check for MCP request
            if (header.indexOf("POST /mcp") >= 0) {
              // Find the start of the JSON body
              int contentLengthIndex = header.indexOf("Content-Length: ");
              if (contentLengthIndex != -1) {
                int contentLength = header.substring(contentLengthIndex + 16).toInt();
                String body = "";
                while (body.length() < contentLength) {
                  if (client.available()) {
                    body += (char)client.read();
                  }
                }
                handleMCPRequest(client, body);
              }
            } else {
              // Regular HTTP response for other requests
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println("Connection: close");
              client.println();
              client.println("<!DOCTYPE html><html>");
              client.println("<head><title>MCP Server</title></head>");
              client.println("<body><h1>MCP Server is running</h1></body>");
              client.println("</html>");
            }
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    header = "";
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}

void handleMCPRequest(WiFiClient client, String body) {
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, body);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    client.println("HTTP/1.1 400 Bad Request");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println();
    client.println("{\"status\": \"error\", \"message\": \"Invalid JSON\"}");
    return;
  }

  const char* toolName = doc["tool_name"];
  String response = "";

  if (strcmp(toolName, "start_led_blinker") == 0) {
    startLedBlinker();
    response = "{\"status\": \"success\", \"message\": \"LED blinker started\"}";
  } else if (strcmp(toolName, "play_sound") == 0) {
    playSound();
    response = "{\"status\": \"success\", \"message\": \"Sound played\"}";
  } else if (strcmp(toolName, "display_countdown") == 0) {
    displayCountdown();
    response = "{\"status\": \"success\", \"message\": \"Countdown displayed\"}";
  } else if (strcmp(toolName, "display_text") == 0) {
    const char* text = doc["text"];
    String textStr = String(text);
    if (textStr.length() > 10) {
      textStr = textStr.substring(0, 10);
    }
    displayText(textStr);
    response = "{\"status\": \"success\", \"message\": \"Text "+ textStr + " displayed\"}";
  } else {
    response = "{\"status\": \"error\", \"message\": \"Invalid tool name\"}";
  }

  // Send MCP response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println();
  client.println(response);
}

void LedSOFF() {
  digitalWrite(led1, HIGH);
  digitalWrite(led2, HIGH);
  digitalWrite(led3, HIGH);
  digitalWrite(led4, HIGH);
}

void startLedBlinker() {
  digitalWrite(led1, LOW);
  delay(200);
  digitalWrite(led1, HIGH);
  delay(200);
  digitalWrite(led2, LOW);
  delay(200);
  digitalWrite(led2, HIGH);
  delay(200);
  digitalWrite(led3, LOW);
  delay(200);
  digitalWrite(led3, HIGH);
  delay(200);
  digitalWrite(led4, LOW);
  delay(200);
  digitalWrite(led4, HIGH);
  delay(200);
}

void playSound() {
  // Generate a simple tone
  for (int i = 0; i < 180; i++) {
    analogWrite(buzzer_pin, 128 + 127 * sin(2 * PI * i / 180));
    delay(2);
  }
  analogWrite(buzzer_pin, 255);
}

void SendDataToSegment(byte Segment_no, byte hexValue) {
  digitalWrite(LATCH_PIN, LOW);
  shiftOut(DATA_PIN, CLK_PIN, MSBFIRST, hexValue);
  shiftOut(DATA_PIN, CLK_PIN, MSBFIRST, Segment_no);
  digitalWrite(LATCH_PIN, HIGH);
}

void displayCountdown() {
  for (int i = 9; i >= 0; i--) {
    SendDataToSegment(select_seg1, numbers[i]);
    delay(100);
  }
  SendDataToSegment(select_seg1, SEG_OFF);
}

void displayText(String text) {
  matrix.beginDraw();
  matrix.stroke(0xFFFFFFFF);
  matrix.textScrollSpeed(150);
  matrix.textFont(Font_5x7);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.println(text);
  matrix.endText(SCROLL_LEFT);
  matrix.endDraw();
}

void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  Serial.print("Now open this URL on your browser --> http://");
  Serial.println(ip);
  displayText(ip.toString());
}
