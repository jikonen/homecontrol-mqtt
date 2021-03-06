#pragma once

#ifdef ARDUINO
#include "Arduino.h"
#elif LINUX
#include "linuxWrapper/src/LinuxWrapper.hpp"
#include <stdint.h>
#endif

class HomeControlMagic;

class Endpoint
{
public:
    Endpoint(HomeControlMagic* hcm_ptr);
    ~Endpoint();

    void setStatusTime(int status_time);
    void setId(const char* id);
    const char* getId();

    void setEndpointName(const char* name_endpoint);
    char* getEndpointName();

    virtual void sendConfig();

    virtual void sendFeedbackMessage() = 0;

    virtual void incomingMessage(const char* topic, const uint8_t* payload, const unsigned int length) = 0;

protected:
    HomeControlMagic* m_owner;
    char* m_id;
    char* m_endpoint_name = nullptr;
    char* m_config = nullptr;
};
