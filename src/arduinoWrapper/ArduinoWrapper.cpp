#include "ArduinoWrapper.h"
#include "Arduino.h"
#include "ArduinoConfig.h"
#include "ArduinoDebugLed.h"
#include "ArduinoNetworkInterface.h"
#include "PubSubClient.h"
#include "helperFunctions.h"

//#define ARDUINO_WRAPPER_DEBUG

// private vars:
#define TOPIC_BUFFER_LENGTH 30
#define MESSAGE_BUFFER_LENGTH 50
static char m_topic_buffer[TOPIC_BUFFER_LENGTH];
static char m_message_buffer[MESSAGE_BUFFER_LENGTH];
static char m_will_topic[TOPIC_BUFFER_LENGTH];
uint32_t m_last_reconnect_attempt = 0;
uint32_t m_reconnect_time = 5000; // 5 seconds
uint32_t m_last_time_connected = 0;
static PubSubClient m_mqtt_client;
const char* m_username = nullptr;
const char* m_password = nullptr;

uint32_t m_last_loop_time;
// private function declaration:
static bool wrapperReconnectMqtt();
static void wrapperSubscribeNow();

// function implementation:
void wrapperLoop(bool reconnect)
{
    // wrapper loop will also call network loop
    networkLoop();

    if(!networkIsConnected())
    {
        return;
    }

    long current_time = millis();
    if(!m_mqtt_client.connected() && (current_time - m_last_loop_time > 100))
    {
        m_last_loop_time = current_time;
        // flash led for visual feedback
        DebugLedFlash(1000);
        if(current_time - m_last_reconnect_attempt > m_reconnect_time && reconnect)
        {
            // Attempt to reconnect
            if(wrapperReconnectMqtt())
            {
                m_last_reconnect_attempt = 0;
                DebugLedState(false);
            }
            else
            {
                // reconnectMqtt takes few seconds if it is failing so just read new time
                m_last_reconnect_attempt = millis();
                if(current_time - m_last_time_connected > 300000) // 5 mins
                {
                    // restart
                    networkChipRestart();
                }
            }
        }
    }
    else
    {
        m_last_time_connected = current_time;
        m_mqtt_client.loop();
    }
}

void wrapperSetup()
{
    m_mqtt_client.setClient(networkGetClient());
    strcat(m_will_topic, "d/");
    strcat(m_will_topic, getUniqueId());
    strcat(m_will_topic, "/0/online");
#ifdef ARDUINO_WRAPPER_DEBUG
    Serial.print(F("Will topic: "));
    Serial.println(m_will_topic);
#endif
}

char* wrapperGetTopicBuffer()
{
    return m_topic_buffer;
}

char* wrapperGetMessageBuffer()
{
    return m_message_buffer;
}

void wrapperPublish()
{
#ifdef ARDUINO_WRAPPER_DEBUG
    Serial.print(F("Publishing on topic: "));
    Serial.println(m_topic_buffer);
    Serial.println(m_message_buffer);
#endif

    m_mqtt_client.publish(m_topic_buffer, m_message_buffer);
    wrapperClearTopicBuffer();
    wrapperClearMessageBuffer();
}

void wrapperClearTopicBuffer()
{
    clearBuffer(m_topic_buffer, TOPIC_BUFFER_LENGTH);
}

void wrapperClearMessageBuffer()
{
    clearBuffer(m_message_buffer, MESSAGE_BUFFER_LENGTH);
}

void wrapperSetServer(IPAddress ip)
{
    if(networkIsSecure())
    {
        m_mqtt_client.setServer(ip, 8883);
    }
    else
    {
        m_mqtt_client.setServer(ip, 1883);
    }
}

void wrapperSetServer(uint8_t* ip)
{
    if(networkIsSecure())
    {
        m_mqtt_client.setServer(ip, 8883);
    }
    else
    {
        m_mqtt_client.setServer(ip, 1883);
    }
}

void wrapperSetServer(char* ip)
{
    if(networkIsSecure())
    {
        m_mqtt_client.setServer(ip, 8883);
    }
    else
    {
        m_mqtt_client.setServer(ip, 1883);
    }
}

void wrapperSetCallback(void (*callback)(const char*, const uint8_t*, const unsigned int))
{
    m_mqtt_client.setCallback(callback);
}

bool wrapperIsMqttConnected()
{
    return m_mqtt_client.connected();
}

void wrapperSetUsernamePassword(const char* const username, const char* const password)
{
    m_username = username;
    m_password = password;
}

// private functions:
bool wrapperReconnectMqtt()
{
#ifdef ARDUINO_WRAPPER_DEBUG
    if(m_username == nullptr)
        Serial.println(F("mqtt username is null"));
    if(m_password == nullptr)
        Serial.println(F("mqtt pass is null"));

    Serial.println(F("Trying to reconnect to mqtt broker"));
#endif
    // Attempt to connect
    if(m_mqtt_client.connect(getUniqueId(), m_username, m_password, m_will_topic, MQTTQOS1, true, "offline"))
    {
#ifdef ARDUINO_WRAPPER_DEBUG
        Serial.println(F("Success"));
        Serial.flush();
#endif
        // ... and resubscribe
        wrapperSubscribeNow();
        return true;
    }
    else
    {
#ifdef ARDUINO_WRAPPER_DEBUG
        Serial.print(F("failed, rc="));
        Serial.println(m_mqtt_client.state());
        Serial.flush();
#endif
        return false;
    }
}

void wrapperSubscribeNow()
{
    strcat(m_topic_buffer, getUniqueId());
    strcat(m_topic_buffer, "/#");
#ifdef ARDUINO_WRAPPER_DEBUG
    Serial.println(m_topic_buffer);
#endif

    m_mqtt_client.subscribe(m_topic_buffer, 1);
    m_mqtt_client.subscribe("broadcast", 1);
    clearBuffer(m_topic_buffer, TOPIC_BUFFER_LENGTH);
}
