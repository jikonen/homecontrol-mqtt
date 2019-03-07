#include "HomeControlMagic.h"
#include "helperFunctions.h"

#include "Endpoints/EndpointZero.h"

#ifdef ARDUINO
#include "arduinoWrapper/ArduinoConfig.h"
#include "arduinoWrapper/ArduinoNetworkInterface.h"
#include "arduinoWrapper/ArduinoWrapper.h"
#elif defined STM
#include "STMWrapper.h"
#endif

#define HCM_DEBUG

static HomeControlMagic* hcm_ptr;

static char* s_topic_buffer_ptr;
static char* s_common_buffer_ptr;

void callback(char* topic, uint8_t* payload, unsigned int length)
{
#ifdef HCM_DEBUG
    Serial.println(F("got in callback"));
    Serial.println(topic);
    for(uint8_t i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println();
#endif
    // check for server announce
    if(lineContains(topic, "broadcast"))
    {
        if(lineContains((char*)payload, "serverannounce"))
        {
            hcm_ptr->announce();
            return;
        }
    }

    // it is not server announce
    uint8_t start_position = lineContains(topic, "/");
    uint8_t end_position = lineContains(topic + start_position, "/") + start_position - 1;
    uint8_t diff = end_position - start_position;

    uint8_t endpoint_id = 0;
    for(int i = 0; i < diff; i++)
    {
        endpoint_id += (topic[start_position + i] - 48) * pow(10, diff - 1 - i);
    }

    Endpoint* end_ptr = hcm_ptr->getEndpoint(endpoint_id);
    if(end_ptr != NULL)
    {
        end_ptr->incomingMessage(topic, payload, length);
    }
}

HomeControlMagic::HomeControlMagic(const char* deviceName)
    : m_number_of_endpoints(0)
    , m_name(deviceName)
    , m_broker_was_connected(false)
{
    // pointer that is used from callback to set messages
    hcm_ptr = this;
    s_topic_buffer_ptr = wrapperGetTopicBuffer();
    s_common_buffer_ptr = wrapperGetMessageBuffer();

    EndpointZero* epZ = new EndpointZero(hcm_ptr);
    epZ->setId("0");
    m_endpoints_pointers[m_number_of_endpoints++] = epZ;
}

void HomeControlMagic::setup()
{
    m_id = getUniqueId();

    strcat(m_base_topic, "d/");
    strcat(m_base_topic, m_id);
    strcat(m_base_topic, "/");

    wrapperSetCallback(callback);
}

void HomeControlMagic::doMagic()
{
    /*
    On arduino this is calling loop for pubsubclient and network. Network is calling debugLed loop
    */
    wrapperLoop();

    if(wrapperIsMqttConnected())
    {
        if(!m_broker_was_connected)
        {
            m_broker_was_connected = true;
            announce();
        }
    }
    else
    {
        m_broker_was_connected = false;
    }
}

void HomeControlMagic::setTopic(char* topic, char* endpoint_id)
{
    strcat(s_topic_buffer_ptr, m_base_topic);
    strcat(s_topic_buffer_ptr, endpoint_id);
    strcat(s_topic_buffer_ptr, "/");
    strcat(s_topic_buffer_ptr, topic);
}

/*
 * Use with s_common_buffer_ptr. Get it by calling getMessageBufferPtr()
 */
void HomeControlMagic::sendStringMessage(char* topic, char* endpoint_id)
{
    setTopic(topic, endpoint_id);

#ifdef HCM_DEBUG
    Serial.println(s_topic_buffer_ptr);
    Serial.println(s_common_buffer_ptr);
#endif

    wrapperPublish();
}

void HomeControlMagic::sendMessage(char* topic, bool message, char* endpoint_id)
{
    setTopic(topic, endpoint_id);

#ifdef HCM_DEBUG
    Serial.println(s_topic_buffer_ptr);
#endif

    if(message)
    {
        s_common_buffer_ptr[0] = '1';
    }
    else
    {
        s_common_buffer_ptr[0] = '0';
    }

#ifdef HCM_DEBUG
    Serial.println(s_common_buffer_ptr);
#endif

    wrapperPublish();
}

void HomeControlMagic::sendMessage(char* topic, uint16_t message, char* endpoint_id)
{
    setTopic(topic, endpoint_id);

#ifdef HCM_DEBUG
    Serial.println(s_topic_buffer_ptr);
#endif

    itoa(message, s_common_buffer_ptr, 10);

#ifdef HCM_DEBUG
    Serial.println(s_common_buffer_ptr);
#endif

    wrapperPublish();
}

void HomeControlMagic::sendMessage(char* topic, double message, char* endpoint_id)
{
    setTopic(topic, endpoint_id);

#ifdef HCM_DEBUG
    Serial.println(s_topic_buffer_ptr);
#endif

    dtostrf(message, 4, 2, s_common_buffer_ptr);

#ifdef HCM_DEBUG
    Serial.println(s_common_buffer_ptr);
#endif

    wrapperPublish();
}

void HomeControlMagic::sendConfig(char* config, char* endpoint_name, char* endpoint_id)
{
    setTopic("conf", endpoint_id);

    strcat(s_common_buffer_ptr, "e=");
    strcat(s_common_buffer_ptr, config);

    if(endpoint_name != nullptr)
    {
        // TODO: replace name with just n
        strcat(s_common_buffer_ptr, ";name=");
        strcat(s_common_buffer_ptr, endpoint_name);
    }

    strcat(s_common_buffer_ptr, ";");

#ifdef HCM_DEBUG
    Serial.println(s_topic_buffer_ptr);
    Serial.println(s_common_buffer_ptr);
#endif
    wrapperPublish();
}

void HomeControlMagic::announce()
{
    strcat(s_common_buffer_ptr, m_name);
    sendStringMessage("announce", "0");
    strcat(s_common_buffer_ptr, "online");
    sendStringMessage("online", "0");

    sendFeedback();
}

Endpoint* HomeControlMagic::getEndpoint(uint8_t number)
{
    if(number >= m_number_of_endpoints)
    {
        return NULL;
    }

    return m_endpoints_pointers[number];
}

char* HomeControlMagic::getId()
{
    return m_id;
}

uint8_t HomeControlMagic::getNumberOfEndpoints()
{
    return m_number_of_endpoints;
}

void HomeControlMagic::addEndpoint(Endpoint* endpoint_ptr)
{
    m_endpoints_pointers[m_number_of_endpoints++] = endpoint_ptr;
    itoa(m_number_of_endpoints - 1, s_common_buffer_ptr, 10);
#ifdef HCM_DEBUG
    Serial.print(F("Id to set: "));
    Serial.println(s_common_buffer_ptr);
#endif
    endpoint_ptr->setId(s_common_buffer_ptr);
    wrapperClearMessageBuffer();
}

void HomeControlMagic::addEndpoint(Endpoint* endpoint_ptr, char* endpoint_id)
{
    m_endpoints_pointers[m_number_of_endpoints++] = endpoint_ptr;
#ifdef HCM_DEBUG
    Serial.print(F("Id to set: "));
    Serial.println(endpoint_id);
#endif
    endpoint_ptr->setId(endpoint_id);
}

void HomeControlMagic::sendConfigs()
{
    for(uint8_t i = 0; i < m_number_of_endpoints; i++)
    {
        m_endpoints_pointers[i]->sendConfig();
    }
}

void HomeControlMagic::sendFeedback()
{
    for(uint8_t i = 0; i < m_number_of_endpoints; i++)
    {
        m_endpoints_pointers[i]->sendFeedbackMessage();
    }
}

char* HomeControlMagic::getMessageBufferPtr()
{
    return s_common_buffer_ptr;
}
