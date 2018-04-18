#include "EndpointTemperature.h"
#include "HomeControlMagic.h"
#include "helperFunctions.h"

#define ENDPOINT_TEMPERATURE_DEBUG

EndpointTemperature::EndpointTemperature(HomeControlMagic* hcm_ptr)
  : Endpoint(hcm_ptr)
  , m_temperature(0)
{
  m_last_send_time = millis();
}

void EndpointTemperature::setTemperature(double temperature)
{
  m_temperature = temperature;
  m_owner->sendMessage("st", m_temperature, m_id);
}

double EndpointTemperature::getTemperature()
{
  return m_temperature;
}

void EndpointTemperature::sendConfig()
{
  String buff = String("e:temperature;r=") + m_resend_status_time + String(";");
  m_owner->sendMessage("conf", buff, m_id);
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

  if(lineContains(topic, "st"))
  {
    m_owner->sendMessage("st", m_temperature, m_id);
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

      m_owner->sendMessage("st", m_temperature, m_id);
    }
}

void EndpointTemperature::sendFeedback()
{
  #ifdef ENDPOINT_TEMPERATURE_DEBUG
  Serial.println(F("sending feedback message, EndpointTemperature"));
  #endif

  m_owner->sendMessage("st", m_temperature, m_id);
}