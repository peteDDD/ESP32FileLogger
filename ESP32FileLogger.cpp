#include "ESP32FileLogger.h"

//*********************************************************************************************
//** This library is used to divert ESP_LOG* output to a logfile and/or serial port.
//**
//** The log file can be on LittleFS (flash) or on SD card
//**  /
//**  SD_MMC library is used to connect to SD card
//**    refer to SDcardSettings.h for pin connections 
//**
//** HOW TO USE THIS LIBRARY
//**
//**  See complete example in examples/exampleOfUsage.cpp
//**
//**
//**
//**  In setup() (and as needed in other places), use the following
//**      logger.enableSerialLogging(true); // false to disable serial logging
//**      logger.enableFileLogging(true);   // false to disable file logging
//**      logger.clearLogFile();            // clear the log file
//**      logger.displayLogFile();          // display the log file contents
//**
//**  platformio.ini MUST include the following build flags
//**
//**      -DUSE_ESP_IDF_LOG
//**      -DCORE_DEBUG_LEVEL=5
//**      -DTAG="\"ARDUINO\""
//**
//**      This is tested using the following settings:
//** 
//**      platform = espressif32@6.5.0
//**      board_build.partitions = default_partitions.csv
//**      framework = arduino
//**      board_build.filesystem = littlefs

//**   You must have a partitions csv file (see examples/partitions.csv)
//**   
//**   Don't forget to create a /data directory for LittleFS,
//**      build the file system, and upload the file system to your processor
//**      before uploading you code
//**
//*********************************************************************************************

// Static member initialization
ESP32FileLogger *ESP32FileLogger::instance = nullptr;

ESP32FileLogger::ESP32FileLogger(const char *logFilePath)
{
    log_file = logFilePath;
    logToSerial = true;
    logToFile = true;
    instance = this; // Set the instance pointer
}

ESP32FileLogger::~ESP32FileLogger()
{
    if (instance == this)
    {
        instance = nullptr;
    }
}

bool ESP32FileLogger::beginSD()
{
    return begin(SD);
}

bool ESP32FileLogger::beginLittleFS()
{
    return begin(LITTLEFS);
}

bool ESP32FileLogger::begin(FS fs)
{
    // Set up logging
    esp_log_level_set("*", ESP_LOG_VERBOSE);

    fileSystem = fs;
    if (fs == SD)
    {
        if (!initializeSD())
        {
            return false;
        }
    }
    else
    {
        if (!initializeLittleFS())
        {
            return false;
        }
    }
    Serial.printf("Logging to: %s\n", fileSystem == SD ? "SD" : "LittleFS");

    esp_log_set_vprintf(my_vprintf);
    loggingInitialized = true;
    return true;
}

bool ESP32FileLogger::initializeSD()
{
    if (SD_MMC.cardType() == CARD_NONE)
    {
        Serial.println("SD card not mounted, attempting to mount...");

        // Configure pins with proper drive strength and pull-ups
        if (!SD_MMC.setPins(SD_SDMMC_CLK_PIN, SD_SDMMC_CMD_PIN, SD_SDMMC_D0_PIN,
                            SD_SDMMC_D1_PIN, SD_SDMMC_D2_PIN, SD_SDMMC_D3_PIN))
        {
            Serial.println("SD_MMC.setPins failed");
            return false;
        }

        // Mount SD card with proper configuration
        if (!SD_MMC.begin("/sdcard", true, false))
        {
            Serial.println("SD_MMC.begin failed");
            return false;
        }
    }

    // Check card type and size
    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE)
    {
        Serial.println("SD_MMC.cardType = NONE");
        return false;
    }

    if (!SD_MMC.exists(log_file))
    {
        File f = SD_MMC.open(log_file, FILE_WRITE);
        if (!f)
        {
            Serial.println("Failed to create log file on SD card!");
            return false;
        }
        f.close();
        Serial.printf("Created new log file %s on SD card\n", log_file);
    }
    Serial.println("SD card mounted successfully");
    Serial.printf("Log file: %s\n", log_file);
    return true;
}

bool ESP32FileLogger::initializeLittleFS()
{
    if (!LittleFS.begin(true)) // true parameter formats the filesystem if mount fails
    {
        Serial.println("LittleFS mount failed! Formatting...");
        if (!LittleFS.format())
        {
            Serial.println("LittleFS format failed!");
            return false;
        }
        if (!LittleFS.begin())
        {
            Serial.println("LittleFS mount failed after formatting!");
            return false;
        }
    }
    Serial.println("LittleFS mounted successfully");

    // Create an empty log file if it doesn't exist
    if (!LittleFS.exists(log_file))
    {
        File f = LittleFS.open(log_file, "w");
        if (!f)
        {
            Serial.println("Failed to create log file!");
            return false;
        }
        f.close();
        Serial.printf("Created new littleFS log file %s\n", log_file);
    }
    Serial.printf("Log file: %s\n", log_file);
    return true;
}

int ESP32FileLogger::my_vprintf(const char *format, va_list args)
{
    if (!instance)
        return 0; // No instance available

    static File logFile;
    // Open or create the log file
    if (!logFile || !logFile.size())
    {
        if (instance->fileSystem == SD)
        {
            logFile = SD_MMC.open(instance->log_file, "a");
            if (!logFile)
            {
                logFile = SD_MMC.open(instance->log_file, "w"); // Create if not exists
                if (!logFile)
                {
                    Serial.println("Failed to open/create log file");
                    return 0; // Indicate failure
                }
            }
        }
        else
        {
            logFile = LittleFS.open(instance->log_file, "a");
            if (!logFile)
            {
                logFile = LittleFS.open(instance->log_file, "w"); // Create if not exists
                if (!logFile)
                {
                    Serial.println("Failed to open/create log file");
                    return 0; // Indicate failure
                }
            }
        }
    }
    // Write to the log file
    char buffer[512];
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    if (len > 0)
    {
        if (instance->logToSerial)
        {
            Serial.print(buffer);
        }

        if (instance->logToFile)
        {
            logFile.print(buffer);
            logFile.flush(); // Ensure data is written to the disk
        }
    }
    return len;
}

void ESP32FileLogger::enableSerialLogging(bool enable)
{
    logToSerial = enable;
    Serial.printf("Serial logging %s %s\n", enable ? "enabled" : "disabled", logToSerial && !loggingInitialized ? ", however, logging not initialized - using standard ESP_LOG" : "");
}

void ESP32FileLogger::enableFileLogging(bool enable)
{
    logToFile = enable;
    Serial.printf("File logging %s %s\n", enable ? "enabled" : "disabled", logToFile && !loggingInitialized ? ", however, logging not initialized - will not log to file" : "");
}

void ESP32FileLogger::clearLogFile()
{
    if (!loggingInitialized)
    {
        Serial.println("Logging not initialized.  clearLogFile() ignored)");
        return;
    }

    File f;
    if (fileSystem == SD)
    {
        if (SD_MMC.exists(log_file))
        {
            f = SD_MMC.open(log_file, "w");
        }
    }
    else
    {
        if (LittleFS.exists(log_file))
        {
            f = LittleFS.open(log_file, "w");
        }
    }

    if (f)
    {
        f.close();
        Serial.println("Log file cleared successfully");
    }
    else
    {
        Serial.println("Failed to clear log file");
    }
}

void ESP32FileLogger::displayLogFile()
{
    if (!loggingInitialized)
    {
        Serial.println("Logging not initialized.  displayLogFile() ignored)");
        return;
    }

    File f;
    // Open file from appropriate filesystem
    if (fileSystem == SD)
    {
        if (!SD_MMC.exists(log_file))
        {
            Serial.println("No log file exists!");
            return;
        }
        f = SD_MMC.open(log_file, "r");
    }
    else
    {
        if (!LittleFS.exists(log_file))
        {
            Serial.println("No log file exists!");
            return;
        }
        f = LittleFS.open(log_file, "r");
    }

    if (!f)
    {
        Serial.println("Failed to open log file for reading!");
        return;
    }

    Serial.println("\n=== Log File Contents ===");
    while (f.available())
    {
        Serial.write(f.read());
    }
    Serial.println("\n=== End of Log File ===");

    f.close();
}
