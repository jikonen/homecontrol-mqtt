#include "EndpointTemperature.h"
#include "HomeControlMagic.h"
#include "helperFunctions.h"

//#define ENDPOINT_TEMPERATURE_DEBUG

EndpointTemperature::EndpointTemperature(HomeControlMagic* hcm_ptr)
  : Endpoint(hcm_ptr)
  , m_temperature(0)
{
  m_last_send_time = millis();
  m_resend_status_time = 60;
}

void EndpointTemperature::setTemperature(double temperature)
{
  m_temperature = temperature;
  m_owner->sendMessage(F("st"), m_temperature, m_id);
}

double EndpointTemperature::getTemperature()
{
  return m_temperature;
}

void EndpointTemperature::sendConfig()
{
  if(m_endpoint_name != nullptr)
  {
    sprintf(m_buff, F("e:color;r=%d;name=%s"), m_resend_status_time, m_endpoint_name);
  }
  else
  {
    sprintf(m_buff, F("e:color;r=%d;"), m_resend_status_time);
  }

  m_owner->sendMessage(F("conf"), m_buff, m_id);
}

void EndpointTemperature::incomingMessage(char* topic, byte* payload, unsigned int length)
{
  #ifdef ENDPOINT_TEMPERATURE_DEBUG
  Serial.println(F("incoming message, EndpointTemperature"));

  for(int i=0; i< length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  #endif

  if(lineContains(topic, F("st")))
  {
    m_owner->sendMessage(F("st"), m_temperature, m_id);
  }
}

void EndpointTemperature::sendStatusMessage()
{
    if (millis() - m_last_send_time > m_resend_status_time * 1000)
    {
      m_last_send_time = millis();
      #ifdef ENDPOINT_TEMPERATURE_DEBUG
        Serial.println(F("sending status message, EndpointTemperature"));
      #endif

      m_owner->sendMessage(F("st"), m_temperature, m_id);
    }
}

void EndpointTemperature::sendFeedbackMessage()
{
  #ifdef ENDPOINT_TEMPERATURE_DEBUG
  Serial.println(F("sending feedback message, EndpointTemperature"));
  #endif

  m_owner->sendMessage(F("st"), m_temperature, m_id);
}