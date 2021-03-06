#ifdef ESP32
#include "ArduinoEsp32Wrapper.h"
#include "Arduino.h"
#include "ArduinoDebugLed.h"

#define ESP_WRAPPER_DEBUG

#define RESTART_TIME 1800000L // 30 minutes

// private vars
static Client* m_client = nullptr;
static char m_uid[20] = {'0'};
static const char* m_ssid = nullptr;
static const char* m_password = nullptr;
static unsigned long m_last_time_connected = 0;
static bool m_is_secure = false;

// private function declaration:
static bool networkReconnect();

// implementation:
void networkLoop()
{
    if(m_client == nullptr || m_ssid == nullptr || m_password == nullptr)
    {
        Serial.println("networkloop");
#ifdef ESP_WRAPPER_DEBUG
        if(m_client == nullptr)
            Serial.println(F("m_client is null!!!"));
        if(m_ssid == nullptr)
            Serial.println(F("m_ssid is null!!!"));
        if(m_password == nullptr)
            Serial.println(F("m_password is null!!!"));
#endif
        return;
    }

    unsigned long current_time = millis();
    if(WiFi.status() == WL_CONNECTED)
    {
        m_last_time_connected = current_time;
    }
    else
    {
        DebugLedFlash(500);
    }

    if(current_time - m_last_time_connected > RESTART_TIME)
    {
        networkChipRestart();
    }

    // network loop will also keep debugLed running
    DebugLedLoop();
}

void networkSetSecure(bool secure)
{
    m_is_secure = secure;
}

bool networkIsConnected()
{
    return WiFi.status() == WL_CONNECTED;
}

void networkSetup()
{
    DebugLedPinSet(2);
    // if we are using that small module for ESP make sure we killoff debug led

    if(m_is_secure)
    {
        m_client = new WiFiClientSecure();
    }
    else
    {
        m_client = new WiFiClient();
    }
    // TODO: remove sprintf
    sprintf(m_uid, "%lx", ESP.getEfuseMac());
#ifdef ESP_WRAPPER_DEBUG
    Serial.print(F("unique id: "));
    Serial.println(m_uid);
#endif

    // turn off access point
    WiFi.mode(WIFI_STA);

    // Got IP event
    WiFi.onEvent(
        [](WiFiEvent_t event, WiFiEventInfo_t info) {
#ifdef ESP_WRAPPER_DEBUG
            Serial.print("Station connected, IP: ");
            Serial.println(WiFi.localIP());
#endif
            DebugLedState(false);
        },
        SYSTEM_EVENT_STA_GOT_IP);

    // Disconnected event
    WiFi.onEvent(
        [](WiFiEvent_t event, WiFiEventInfo_t info) {
#ifdef ESP_WRAPPER_DEBUG
            Serial.println("Station disconnected");
#endif
            DebugLedFlash(500);
        },
        SYSTEM_EVENT_STA_DISCONNECTED);
}

bool networkReconnect()
{
}

void networkChipRestart()
{
    ESP.restart();
}

void networkStart()
{
    Serial.println("networkstart");
    if(m_client == nullptr || m_ssid == nullptr || m_password == nullptr)
    {
#ifdef ESP_WRAPPER_DEBUG
        Serial.println(F("ESP settings not set!!!"));
#endif
        return;
    }

    WiFi.begin(m_ssid, m_password);
    Serial.println("network start done");
}

void networkSetSsid(const char* const ssid)
{
    m_ssid = ssid;
}

void networkSetPass(const char* const pass)
{
    m_password = pass;
}

bool networkIsSecure()
{
    return m_is_secure;
}

Client& networkGetClient()
{
    return *m_client;
}

char* getUniqueId()
{
    return m_uid;
}

#endif // ESP32
