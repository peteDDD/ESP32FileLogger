#ifndef ESP32FILELOGGER_H
#define ESP32FILELOGGER_H

#include <Arduino.h>
#include <LittleFS.h>
#include <esp_log.h>
#include <SD_MMC.h>
#include "SDcardSettings.h"

class ESP32FileLogger
{
public:
    // Constructor
    ESP32FileLogger(const char *logFilePath = "/log.txt");
    ~ESP32FileLogger();

    enum FS
    {
        LITTLEFS,
        SD
    };

    void setFileSystem(FS fs);
    FS getFileSystem();
    // Initialize the filesystem and logging
    bool begin(FS fs = LITTLEFS);
    bool beginSD();
    bool beginLittleFS();
    // Logging control functions
    void enableSerialLogging(bool enable = true);
    void enableFileLogging(bool enable = true);
    void clearLogFile();
    void displayLogFile();

    // Get current logging states
    bool isSerialLoggingEnabled() const { return logToSerial; }
    bool isFileLoggingEnabled() const { return logToFile; }

    // Get the log file path
    const char *getLogFilePath() const { return log_file; }

private:
    bool loggingInitialized = false;
    FS fileSystem;
    const char *log_file;
    bool logToSerial;
    bool logToFile;
    static ESP32FileLogger *instance; // Static instance pointer

    // Custom vprintf function for ESP logging system

    static int my_vprintf(const char *format, va_list args);

    bool initializeLittleFS();
    bool initializeSD();
};

#endif // ESP32FILELOGGER_H