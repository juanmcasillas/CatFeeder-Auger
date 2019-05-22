///////////////////////////////////////////////////////////////////////////////
/// CATFEEDER Adapter /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#ifndef __CATFEEDER_H__
#define __CATFEEDER_H__

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FS.h>
#include <WiFiClient.h>
#include <TimeLib.h>
#include <NtpClientLib.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266mDNS.h>
#include <Ticker.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include <FSWebServerLib.h>
#include <Hash.h>

#include "bot.h"
#include "logger.h"
#include "motorstepper.h"

// TODO: FIX the number of schedules
#define CATFEEDER_PROGRAMS 7

extern MotorStepperClass CATFEEDER_STEPPER_MOTOR;

class CatFeederClass {
    public:
        int offset = 0; // the offset for the calibration process    
        String lastopen = ""; // CONFIG

    protected:
        String CATFEEDER_CONFIG_FILE =  "/catfeeder.json";
        String LOGGER_FILE = "/catfeeder.log";
        
        MotorStepperClass *_motor = &CATFEEDER_STEPPER_MOTOR;
        LoggerClass _logger;
        BotClass _bot;
        String wifissid = "";
        String wifipassword = "";

        String scheduler[CATFEEDER_PROGRAMS];   // CONFIG   
        int PROGRAMS = CATFEEDER_PROGRAMS;
        float SCHEDULE_PERIOD = 1.0f;
        
        FS *_fs;            // ICPFS implementation
        Ticker _ticker;     // Periodic Time Dispatcher (see CheckScheduler)

    public:
        String begin(FS* fs);
        void IsRunning(); // shows the start information
        void DEBUG(String s); // shows a simple trace (to test the log, remove it)
        static void CheckScheduler(void *arg); // Programmed each 1 second. Passed myself as arg.
        int AdvanceSlot(); // advance the feeder in 1 slot
        
        inline int getPROGRAMS() { return this->PROGRAMS; }
        inline String getScheduler(int p) { return this->scheduler[p]; }
        inline void setLastOpen(String s) { this->lastopen = s; }
        inline LoggerClass *getLogger() { return &this->_logger; }

        // exposed WWW methods (REST)
        String Status(AsyncWebServerRequest *request);
        String Calibrate_Auger(AsyncWebServerRequest *request);
        String Test_Auger(AsyncWebServerRequest *request);
       
        String Scheduler_Config(AsyncWebServerRequest *request);
        String Scheduler_Reset(AsyncWebServerRequest *request);
        String Get_Log(AsyncWebServerRequest *request);
        String Reset_Log(AsyncWebServerRequest *request);
        String Bot_Config(AsyncWebServerRequest *request);

        String GetTimeStampNow();
        unsigned long ToTimestamp(String d); 

        // bot management.
        bool RunBot();
        inline bool isBotActive() { return this->_bot.isActive(); }

    // internal methods used to do things
    protected:
        // configuration management

        bool SaveConfig();
        bool LoadConfig();

        // helpers

        // motor comander
        void _motor_feed(float revolutions);
        void _sort(int a[], int size);
};

#endif
