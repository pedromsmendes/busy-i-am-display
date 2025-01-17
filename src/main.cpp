#include <WiFi.h>
#include <WiFiMulti.h>
#include <TFT_eSPI.h>
#include <WebSocketsClient.h>

const char *host = WS_HOST;
const int port = WS_PORT;
const char *path = WS_PATH;

const int SCREEN_WIDTH = 320;
const int SCREEN_HEIGHT = 240;

const uint16_t screenBgColor = TFT_BLACK;
const uint16_t statusBgColor = TFT_BLACK;
const uint16_t messageBgColor = TFT_BLACK;

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprMessage = TFT_eSprite(&tft);
TFT_eSprite sprStatus = TFT_eSprite(&tft);

WiFiMulti wifiMulti;
WebSocketsClient webSocket;

int x = 0;
int distance = 100;
int textWidth = 0;
int textHeight = 0;
String theMessage = "";

void SetMessage(String msg)
{
  theMessage = msg;
  textWidth = sprMessage.textWidth(theMessage);
  textHeight = sprMessage.fontHeight();
}

void onEvent(WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:
    sprStatus.fillSprite(statusBgColor);
    sprStatus.setCursor(0, 0);
    sprStatus.setTextColor(TFT_RED, statusBgColor);
    sprStatus.printf("Disconnected from websocket");
    sprStatus.pushSprite(0, 0);
    break;

  case WStype_CONNECTED:
    sprStatus.fillSprite(statusBgColor);
    sprStatus.setCursor(0, 0);
    sprStatus.setTextColor(TFT_GREEN, statusBgColor);
    sprStatus.printf("Connected to websocket");
    sprStatus.pushSprite(0, 0);
    break;

  case WStype_TEXT:
    SetMessage(String((char *)payload));
    break;

  case WStype_ERROR:
    Serial.print("ws error: ");
    Serial.println(String((char *)payload));
    break;

  // Handle binary data
  case WStype_BIN:
    Serial.println(String((char *)payload));
    break;
  }
}

void connectToWifi()
{
  sprStatus.fillSprite(statusBgColor);

  auto wifiString = "Connecting to WiFi";
  auto wifiStringWidth = sprStatus.textWidth(wifiString);

  sprStatus.setCursor(0, 0);
  sprStatus.setTextColor(TFT_YELLOW, statusBgColor);
  sprStatus.printf(wifiString);
  sprStatus.pushSprite(0, 0);

  // WiFi.begin(ssid, password);

  while (wifiMulti.run(3000) != WL_CONNECTED)
  {
    sprStatus.printf(".");
    sprStatus.pushSprite(0, 0);
  }

  sprStatus.fillSprite(statusBgColor);
  sprStatus.setTextColor(TFT_GREEN, statusBgColor);
  sprStatus.setCursor(0, 0);
  sprStatus.printf("Connected to WiFi");
  sprStatus.pushSprite(0, 0);

  delay(2000);

  sprStatus.fillSprite(statusBgColor);
  sprStatus.pushSprite(0, 0);
}

void connectWebSocket()
{
  sprStatus.setCursor(0, 0);
  sprStatus.setTextColor(TFT_YELLOW, statusBgColor);
  sprStatus.printf("Connecting to websocket");
  sprStatus.pushSprite(0, 0);

  webSocket.setReconnectInterval(5000);
  webSocket.beginSSL(host, port, path);
  webSocket.onEvent(onEvent);
}

void setup()
{
  Serial.begin(115200);

  wifiMulti.addAP(WIFI1_SSID, WIFI1_PASSWORD);
  wifiMulti.addAP(WIFI2_SSID, WIFI2_PASSWORD);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(screenBgColor);

  sprMessage.createSprite(SCREEN_WIDTH, 50);
  sprMessage.setTextWrap(false, false);
  sprMessage.setTextColor(TFT_WHITE, messageBgColor);
  sprMessage.setTextSize(6);

  sprStatus.createSprite(SCREEN_WIDTH, 10);
  sprStatus.setTextWrap(false, false);
  sprStatus.setTextColor(TFT_WHITE, statusBgColor);
  sprStatus.setTextSize(1);

  connectToWifi();

  connectWebSocket();
}

auto scrollDelay = 250;
auto sinceLastScroll = scrollDelay;

auto lastTime = millis();
void loop()
{
  auto current = millis();
  auto sinceLast = current - lastTime;
  lastTime = current;

  // Serial.println(sinceLast);

  webSocket.loop();

  if (!theMessage.isEmpty() && webSocket.isConnected() && sinceLastScroll >= scrollDelay)
  {
    sinceLastScroll = 0;

    auto yCursor = sprMessage.height() / 2 - textHeight / 2;

    sprMessage.fillSprite(messageBgColor);
    sprMessage.setCursor(x, yCursor);
    sprMessage.println(theMessage);

    // scroll in second text
    if (x < -distance + SCREEN_WIDTH)
    {
      sprMessage.setCursor(x + (textWidth + distance), yCursor);
      sprMessage.println(theMessage);
    }

    sprMessage.pushSprite(0, SCREEN_HEIGHT / 2 - sprMessage.height() / 2);

    x--;

    if (x < -textWidth)
    {
      // first text is out, move x to second text
      x += (textWidth + distance);
    }
  }

  sinceLastScroll += current;
}