///////////////////////////////////////////////////////////////////////////////
/// CATFEEDER Adapter /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "catfeeder.h"

 MotorServoClass CATFEEDER_MOTOR;

// arduino init
String CatFeederClass::begin(FS *fs) {
    
    if (!DBG_OUTPUT_PORT) {
        DBG_OUTPUT_PORT.begin(115200);
	    DBG_OUTPUT_PORT.print("\n\n");
    #ifndef RELEASE
	    DBG_OUTPUT_PORT.setDebugOutput(true);
    #endif
    }

    _fs = fs;
    if (!_fs) { _fs->begin(); }

    // init the logger
    if (!_logger.begin(_fs, this->LOGGER_FILE)) {
        DEBUGLOG("Can't open logger file: %s\n", this->LOGGER_FILE.c_str());
    }

    _logger.INFO("CatFeeder starting");

    // init the refence to the stepper
    _motor->begin();

    // load configuration
    this->LoadConfig();

    // configure the scheduler checker. Do the things each second.
 
    _ticker.attach(this->SCHEDULE_PERIOD, &CatFeederClass::CheckScheduler, static_cast<void*>(this)); 

}

// show some log info about configuration, starts, and so on (e.g. position, dates, schedules)
void CatFeederClass::IsRunning() {
    _logger.INFO("CatFeeder is Running and Handle Requests!");
    _logger.INFO(" CONFIG::lastopen: %s", this->lastopen.c_str());
    _logger.INFO(" CONFIG::feed_turns: %d", this->feed_turns);
    for (int i=0; i< this->PROGRAMS; i++) {
        _logger.INFO(" CONFIG::schedule[%d]: %s", i, this->scheduler[i].c_str());
    }

}

// exercise the logger with some data
void CatFeederClass::DEBUG(String s) {
    _logger.DEBUG(s.c_str());
}

// get the status of the CatFeeder (sys_cal.html)
String CatFeederClass::Calibrate_Auger(AsyncWebServerRequest *request) {


    // TODO
    // fixed for the auger version
   
    //this->_motor_moveto(1,2);

    //
    // continue ...

    this->offset = 0;
    
    StaticJsonDocument<500> root;
    //JsonObject& root = jsonBuffer.createObject();
    root["offset"] = this->offset;
    
    String ret;
    serializeJson(root,ret);
    return(ret);
}


// move the feeder to the given slot CatFeeder (sys_cal.html)
String CatFeederClass::Test_Auger(AsyncWebServerRequest *request) {

    if (!request->hasArg("r")) { 
        request->send(500, "text/plain", "BAD ARGS"); 
        return(""); // not reached, really
    }
    
    // get parameters
    float revolutions = atof(request->arg("r").c_str());

    this->_motor_feed(revolutions);


    // return things
    StaticJsonDocument<500> root;
    //JsonObject& root = jsonBuffer.createObject();
    root["revolutions"] = revolutions;
    String ret;
    serializeJson(root,ret);
    return(ret);
}


// get the status of the CatFeeder (index.html)
String CatFeederClass::Status(AsyncWebServerRequest *request) {
    StaticJsonDocument<500> root;
    //JsonObject& root = jsonBuffer.createObject();
    root["lastopen"] = this->lastopen; 
    String ret;
    serializeJson(root,ret);
    return(ret);
}

// scheduler functions
String CatFeederClass::Scheduler_Config(AsyncWebServerRequest *request) {


	if (request->params() > 0)  // Save Scheduler Settings
	{

        // clear configuration
        this->lastopen = "";
        for (int i=0; i< this->PROGRAMS; i++) this->scheduler[i] = "";

        int ptr = 0;
        this->_logger.INFO("Storing programs");
		for (uint8_t i = 0; i < request->params(); i++) {
            AsyncWebParameter* p = request->getParam(i);
			
            if (p->name().startsWith("sched_") && p->value() != "") { 
                this->scheduler[ptr] = p->value();
                ptr++;
                if (ptr > this->PROGRAMS) break;
                //DEBUGLOG("Arg %d: %s = %s\r\n", i, p->name().c_str(), p->value().c_str());
                this->_logger.INFO("Program #%d: %s", i, p->value().c_str());
            }
		}
        
		this->SaveConfig();
		this->_logger.INFO("Configuration saved");
        //request->redirect("/schedule.html");
        //request->redirect("/"); // to force the reload of the configuration :(
        //return("OK"); // not reached
	}

    // always return the data
    // retrieve data as json

    StaticJsonDocument<500> root;
    //JsonObject& root = jsonBuffer.createObject();
    JsonObject doc = root.to<JsonObject>();
    JsonArray jsonscheduler = doc.createNestedArray("scheduler");
	
    for (int i=0; i< this->PROGRAMS; i++) {
        jsonscheduler.add(this->scheduler[i]);
    }

    String ret;
    serializeJson(root,ret);
    return(ret);
}


// scheduler functions
String CatFeederClass::Scheduler_Reset(AsyncWebServerRequest *request) {


    // clear configuration and position
    this->_logger.INFO("Reset programs. Setting position to initial value (1)");
    for (int i=0; i< this->PROGRAMS; i++) this->scheduler[i] = "";
    this->lastopen = "";
  
   	this->SaveConfig();
	this->_logger.INFO("Configuration saved");

    // always return the data
    // retrieve data as json

    StaticJsonDocument<500> v;
    //JsonObject&  v = jsonBuffer.createObject();
    JsonObject doc = v.to<JsonObject>();
    JsonArray  jsonscheduler = doc.createNestedArray("scheduler");
	
    for (int i=0; i< this->PROGRAMS; i++) {
        jsonscheduler.add(this->scheduler[i]);
    }

    String ret;
    serializeJson(v,ret);
    return(ret);
}


String CatFeederClass::Get_Log(AsyncWebServerRequest *request) {
    File logfd = _fs->open(this->LOGGER_FILE,"r");
    if (!logfd) {
        return ("CAN'T OPEN LOG FILE");
    }

    String ret = "";
    while (logfd.available()){
            ret += char(logfd.read());
    }
    logfd.close();
    return(ret);

}
String CatFeederClass::Reset_Log(AsyncWebServerRequest *request) {
    this->_logger.Truncate();
    return("*log truncated*");  
}


////
/// config

bool CatFeederClass::SaveConfig() {

	StaticJsonDocument<512> json;
	//StaticJsonBuffer<1024> jsonBuffer;
	//JsonObject& json = jsonBuffer.createObject();
    json["lastopen"] = this->lastopen;
    json["feed_turns"] = this->feed_turns;    
    //json["bottoken"] = this->_bot.token;
    JsonObject doc = json.to<JsonObject>();

	JsonArray jsonscheduler = doc.createNestedArray("scheduler");
    for (int i=0; i < this->PROGRAMS; i++) {
	    jsonscheduler.add(this->scheduler[i]);
    }

	File configFile = _fs->open(CATFEEDER_CONFIG_FILE, "w");
	if (!configFile) {
		DEBUGLOG("Failed to open config file %s for writing\r\n", CATFEEDER_CONFIG_FILE.c_str());
		configFile.close();
		return false;
	}




#ifndef RELEASE
	String temp;
	serializeJsonPretty(json, temp);
    DEBUGLOG(temp.c_str());
#endif

	serializeJson(json,configFile);
	configFile.flush();
	configFile.close();
	return true;
}

bool CatFeederClass::LoadConfig() {

	File configFile = _fs->open(CATFEEDER_CONFIG_FILE, "r");
	if (!configFile) {
		DEBUGLOG("CatFeeder: Failed to open config file %s\n", CATFEEDER_CONFIG_FILE.c_str());
		return false;
	}

	size_t size = configFile.size();
	std::unique_ptr<char[]> buf(new char[size]);

	configFile.readBytes(buf.get(), size);
	configFile.close();
	DEBUGLOG("JSON file size: %d bytes\r\n", size);
	StaticJsonDocument<1024> json;
	//JsonObject& json = jsonBuffer.parseObject(buf.get());

    auto error = deserializeJson(json, buf.get());
	if (error) {
		DEBUGLOG("CatFeeder: Failed to parse config file %s\r\n", CATFEEDER_CONFIG_FILE.c_str());
		return false;
	}
    

    if (json.containsKey("lastopen")) {
        this->lastopen = json["lastopen"].as<const char *>();
    }
   
    //if (json.containsKey("bottoken")) {
    //    this->_bot.token = json["bottoken"].as<const char *>();
    //}

        if (json.containsKey("feed_turns")) {
        this->feed_turns = json["feed_turns"].as<int>();
    }

    if (json.containsKey("scheduler")) {
        for (int i=0; i < this->PROGRAMS; i++) {
           this->scheduler[i] = json["scheduler"][i].as<const char *>();
        }
    }
    
#ifndef RELEASE
	String temp;
	serializeJsonPretty(json, temp);
	DEBUGLOG(temp.c_str());
#endif

	DEBUGLOG("CatFeederConfig Config initialized.\r\n");
    DEBUGLOG("lastopen:  %s\n", this->lastopen.c_str());
    DEBUGLOG("feed_turns:  %d\n", this->feed_turns);
    //DEBUGLOG("bot token: %s\n", this->_bot.token.c_str());

    for (int i=0; i < this->PROGRAMS; i++) {
        DEBUGLOG("scheduler[%d]: %s\n", i, this->scheduler[i].c_str());
    }
    DEBUGLOG("\n\n");
	return true;
}

// schedule things

void CatFeederClass::CheckScheduler(void *arg) {
    CatFeederClass *self = reinterpret_cast<CatFeederClass *>(arg);

    // TODO
    //DEBUGLOG("TICK: %s\n", self->GetTimeStampNow().c_str());
    
    int programs = self->getPROGRAMS();
    unsigned long mystamp,prgstamp;
    
    String mytime = self->GetTimeStampNow();
    mystamp = self->ToTimestamp(mytime);

    for (int i=0; i< programs; i++) {
        String sched = self->getScheduler(i);
        if (sched != "") {
            prgstamp = self->ToTimestamp(sched);
            // DEBUGLOG(" TICK_check #%d: %s->%s | %lu|%lu\n", i, mytime.c_str(), sched.c_str(), mystamp, prgstamp);
            if (mystamp == prgstamp) {
                // do things if matches
                DEBUGLOG(" TICK_MATCH! #%d %s %s\n", i, mytime.c_str(), sched.c_str());
                 self->setLastOpen(sched.c_str());
                 self->DoFeed();
            }
        }
    }
}

void CatFeederClass::DoFeed() {
    _motor_feed(this->feed_turns);
    this->SaveConfig();
    this->_logger.INFO("Feed Done");
}

// build the string just like the scheduled.
String CatFeederClass::GetTimeStampNow() {
    time_t t = now(); 
    char *buf = (char *)malloc(1024);
    sprintf(buf, "%02d/%02d/%04d %02d:%02d:%02d", day(t), month(t), year(t), hour(t), minute(t), second(t));
    String ret = buf;
    free(buf);
    return(ret);
}

unsigned long CatFeederClass::ToTimestamp(String d) {
    // EPOCH 01/01/1970 00:00:00
    // 1 hour	3600 seconds
    // 1 day	86400 seconds
    // 1 week	604800 seconds
    // 1 month (30.44 days) 	2629743 seconds
    // 1 year (365.24 days) 	 31556926 seconds


    int day, month, year, hour, minute, second;

    sscanf(d.c_str(),"%02d/%02d/%04d %02d:%02d:%02d",
         &day, &month, &year, &hour, &minute, &second);
    
    unsigned long ret;

    ret = (year-1970 * (unsigned long)31556926) + (month * 2629743) + (day * 86400) +
          (hour * 3600) + (minute * 60) + second;
    
    return(ret);
}


// Save Telegram Bot Token

String CatFeederClass::Bot_Config(AsyncWebServerRequest *request) {

	if (request->params() > 0 && request->hasParam("bottoken")) { // Save Bot Settings 

        // clear configuration
        AsyncWebParameter* p = request->getParam("bottoken",true);
        DEBUGLOG("Bot Token %s = %s\r\n", p->name().c_str(), p->value().c_str());
        
        //this->_bot.Init(p->value());
		this->SaveConfig();

		this->_logger.INFO("Configuration saved");
	}

    // always return the data
    // retrieve data as json

    StaticJsonDocument<500> root;
    //JsonObject& root = jsonBuffer.createObject();
    root["bottoken"] = "this->_bot.token";

    String ret;
    serializeJson(root,ret);
    return(ret);
}


// run the bot loop
bool CatFeederClass::RunBot() {
/*
    if (this->_bot.needsRun()) {
        int numNewMessages = this->_bot.getUpdates();
        while(numNewMessages) {
            for (int i=0; i<numNewMessages; i++) {
                
                String in_msg = this->_bot.getMessage(i);
                String in_id = this->_bot.getId(i);

                // do something with the message here.
                String out_msg = "Meow Meow (" + in_msg + ")";

                this->_bot.sendMessage(in_id, out_msg);
            }
            numNewMessages = this->_bot.getUpdates();
            }

        this->_bot.updateRun();
    }
*/
}




///////////////////////////////////////////////////////////////////////////////
// Internal methods ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// move the motor to the slot pos. See the 
// current position and calculate what is the minimum route

void CatFeederClass::_motor_feed(float revolutions) {
    // haveto: anticlockwise 
    DEBUGLOG("CatFeederClass::_motor_feed (Auger): revolutions: %f\n", revolutions);
    schedule_function(std::bind(MotorServoClass::Feed, revolutions, _motor));
    return;

}

void CatFeederClass::_sort(int a[], int size) {
    for(int i=0; i<(size-1); i++) {
        for(int o=0; o<(size-(i+1)); o++) {
                if(a[o] > a[o+1]) {
                    int t = a[o];
                    a[o] = a[o+1];
                    a[o+1] = t;
                }
        }
    }
}


