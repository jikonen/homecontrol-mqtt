#include "EndpointColor.h"
#include "HomeControlMagic.h"
#include "debugDefines.h"
#include "printWrapper.h"

#ifdef LINUX
#include <cstring>
#endif

static char* const CONFIG = "col";

EndpointColor::EndpointColor(HomeControlMagic* hcm_ptr, double start_level, int r, int g, int b)
    : Endpoint(hcm_ptr)
    , m_rgb({r, g, b})
    , m_state(false)
    , m_level(start_level)
{
    m_config = CONFIG;
}

void EndpointColor::setState(bool state)
{
    m_state = state;
    m_owner->sendMessage("sp", m_state, m_id);
}

bool EndpointColor::getState()
{
    return m_state;
}

void EndpointColor::setLevel(uint16_t level)
{
    m_level = level;
    m_owner->sendMessage("sl", m_level, m_id);
}

uint16_t EndpointColor::getLevel()
{
    return m_level;
}

int EndpointColor::getColorR()
{
    return m_rgb.r;
}

int EndpointColor::getColorG()
{
    return m_rgb.g;
}

int EndpointColor::getColorB()
{
    return m_rgb.b;
}

void EndpointColor::getRGBcharPtr(char* buffer)
{
    char buff[6];

    itoa(m_rgb.r, buff, 10);
    strcat(buffer, buff);
    strcat(buffer, ";");

    itoa(m_rgb.g, buff, 10);
    strcat(buffer, buff);
    strcat(buffer, ";");

    itoa(m_rgb.b, buff, 10);
    strcat(buffer, buff);
}

void EndpointColor::incomingMessage(const char* topic, const uint8_t* payload, const unsigned int length)
{
#ifdef ENDPOINT_COLOR_DEBUG
    print(F("incoming message, EndpointColor"));
#endif

    if(lineContains(topic, "cl", length))
    {
        m_level = extractInteger(payload, length);
    }

    else if(lineContains(topic, "sl", length))
    {
        m_owner->sendMessage("sl", m_level, m_id);
    }

    else if(lineContains(topic, "cp", length))
    {
        m_state = extractBool(payload, length);
    }

    else if(lineContains(topic, "sp", length))
    {
        m_owner->sendMessage("sp", m_state, m_id);
    }

    else if(lineContains(topic, "cc", length))
    {
        m_rgb = extractRGB(payload, length);
    }

    else if(lineContains(topic, "sc", length))
    {
        getRGBcharPtr(m_owner->getMessageBufferPtr());
        m_owner->sendStringMessage("sc", m_id);
    }
}

void EndpointColor::sendFeedbackMessage()
{
#ifdef ENDPOINT_COLOR_DEBUG
    print(F("sending feedback message, EndpointColor"));
#endif

    m_owner->sendMessage("sp", m_state, m_id);
    m_owner->sendMessage("sl", m_level, m_id);
    getRGBcharPtr(m_owner->getMessageBufferPtr());
    m_owner->sendStringMessage("sc", m_id);
}