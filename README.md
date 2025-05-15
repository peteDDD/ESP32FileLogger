This library is used to divert ESP_LOG* output to a logfile and/or serial port.

The log file can be on LittleFS (flash) or on SD card

   SD_MMC library is used to connect to SD card.
    Refer to SDcardSettings.h for pin connections 

HOW TO USE THIS LIBRARY

 See complete example in examples/exampleOfUsage.cpp

 In setup() (and as needed in other places), use the following
 
      logger.enableSerialLogging(true); // false to disable serial logging
      logger.enableFileLogging(true);   // false to disable file logging
      logger.clearLogFile();            // clear the log file
      logger.displayLogFile();          // display the log file contents

  platformio.ini MUST include the following build flags

      -DUSE_ESP_IDF_LOG
      -DCORE_DEBUG_LEVEL=5
      -DTAG="\"ARDUINO\""

  This is tested using the following settings:
 
      platform = espressif32@6.5.0
      board_build.partitions = default_partitions.csv
      framework = arduino
      board_build.filesystem = littlefs

   You must have a partitions csv file (see examples/default_partitions.csv)
   
   Don't forget to create a /data directory for LittleFS,
      build the file system, and upload the file system to your processor
      before uploading you code