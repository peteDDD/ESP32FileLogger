#include <Arduino.h>
#include "ESP32FileLogger.h"

//* Use one of the following to initialize the logger
//ESP32FileLogger logger("/logg.txt");  // specifying the log file path and name
ESP32FileLogger logger;  // using the default log file path and name "/log.txt"

void setup()
{
    Serial.begin(115200);

    //* Use one of the following to initialize the logger
    // if (!logger.begin(ESP32FileLogger::SD)) // will initialize logging to SD
    // if (!logger.begin())  // will initialize logging to LittleFS 
    // if (!logger.beginSD()) // will initialize logging to SD
    if (!logger.beginLittleFS()) // will initialize logging to LittleFS
    {
        Serial.println("Failed to initialize logger!");
        Serial.println("Using standard ESP_LOG Serial Logging");
    }

    
    //* You can select either, or both, or neither of the following two states
    //* You can also suspend the logging and resume logging during your program using these functions
    //* Here, we will test logging with both methods enabled
    logger.enableSerialLogging(true); 
    logger.enableFileLogging(true);

    //* clears the log file.  Use this to start fresh.
    //* if you don't call this, the log file will append to the end of the existing logfile
    logger.clearLogFile();  

    // Test with different log levels
    ESP_LOGE("MYAPP", "Error log test");
    ESP_LOGW("MYAPP", "Warning log test");
    ESP_LOGI("MYAPP", "Info log test: Value: %d", 42);
    ESP_LOGD("MYAPP", "Debug log test");
    ESP_LOGV("MYAPP", "Verbose log test");

    // Disable file logging
    logger.enableFileLogging(false);

    // Display the log file contents
    logger.displayLogFile();
}

void loop()
{
    // Your code here
}