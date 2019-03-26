#include "Endpoint.h"
#include "HomeControlMagic.h"

#include "debugDefines.h"
#include "printWrapper.h"

#ifdef LINUX
using namespace std;
#include <cstring>
#endif

Endpoint::Endpoint(HomeControlMagic* hcm_ptr)
    : m_owner(hcm_ptr)
{
    m_id = new char[2];
    strcpy(m_id, "0");
}

void Endpoint::setId(char* id)
{
    delete m_id;
    m_id = new char[(strlen(id) + 1)];
    strcpy(m_id, id);
}

const char* Endpoint::getId()
{
    return m_id;
}

void Endpoint::setEndpointName(char* name_endpoint)
{
    m_endpoint_name = new char[strlen(name_endpoint)];
    strcpy(m_endpoint_name, name_endpoint);
#ifdef ENDPOINT_DEBUG
    print(F("Setting endpoint id, name: "), m_id, name_endpoint);
#endif
}

char* Endpoint::getEndpointName()
{
    return m_endpoint_name;
}

void Endpoint::sendConfig()
{
#ifdef ENDPOINT_DEBUG
    print(F("Sending config for endpoint: "), m_id);
#endif
    m_owner->sendConfig(m_config, m_endpoint_name, m_id);
}
