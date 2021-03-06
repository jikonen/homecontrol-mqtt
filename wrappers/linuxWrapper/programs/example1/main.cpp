#include "HomeControlMagic.h"
#include "LinuxWrapper.hpp"
#include "arrrgh.hpp"
#include "logger.hpp"
#include "mqtt.hpp"

#include <stdint.h>
#include <unistd.h>

#include "EndpointOnOff.h"

// it is called handle pin just to have some references to examples for arduino
void handlePin(EndpointOnOff& on_off);

int main(int argc, const char* argv[])
{
    arrrgh::parser parser("hc_mqtt_client", "HomeControl MQTT client example");
    const auto& debug_mode = parser.add<bool>("debug", "Turn on debug mode", 'd', arrrgh::Optional, false);

    try
    {
        parser.parse(argc, argv);
    }
    catch(...)
    {
        errorMessage("Parsing failed");
    }

    if(debug_mode.value())
    {
        infoMessage("Debug mode on");
        logger.enableDebugLogging();
    }

    std::string test = "test";

    infoMessage("info {}", test);
    errorMessage("error {}", test);
    debugMessage("debug {}", test);

    // USER VALUES:
    const std::string device_name = "LINUX_CLIENT";
    const std::string gateway_ip = "192.168.5.30";
    // read username and password from the app
    const std::string username = "hc";
    const std::string password = "6287911";
    const bool use_secure = true; // use mqtt with encryption
    //

    wrapperSetup();

    mqtt::Mqtt mqtt(getUniqueId(), gateway_ip, username, password, use_secure);
    linuxSetMqttPtr(&mqtt);

    HomeControlMagic hcm(device_name.c_str());

    EndpointOnOff on_off(&hcm);

    hcm.setup();

    hcm.addEndpoint(&on_off);

    while(true)
    {
        hcm.doMagic();
        handlePin(on_off);
        usleep(100000);
    }

    infoMessage("Exiting");

    return 0;
}

void handlePin(EndpointOnOff& on_off)
{
    static bool old_state = false;
    bool state = on_off.getState();
    if(state != old_state)
    {
        infoMessage("Received new state: {}", state);
        old_state = state;
        // tell system that we received the message
        on_off.sendFeedbackMessage();
    }
}