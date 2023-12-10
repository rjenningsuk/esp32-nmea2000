
//we only compile for some boards
#ifdef API_POST

#include "GwApiPostTask.h"

#include <ctime>
#include <iostream>
#include <list>
#include <string>
#include <vector>

#include "ArduinoJson.h"
#include "GWWifi.h"
#include "GwApi.h"
#include "HTTPClient.h"
#include "WiFiClient.h"
#include "WiFiClientSecure.h"

using namespace std;

    /**
     * an init function that ist being called before other initializations from the core
     */
    void apiPostInit(GwApi *api) 
    {
        if (api->getConfig()->getConfigItem("systemName")->asString() == "ESP32NMEA2K") {
            api->getConfig()->setValue(api->getConfig()->systemName, GWAPIPOST_SYSTEM_NAME);
        }

        if (api->getConfig()->getConfigItem("apPassword")->asString() == "esp32nmea2k") {
            api->getConfig()->setValue(api->getConfig()->apPassword, GWAPIPOST_AP_PASSWORD);
        }

        if (api->getConfig()->getConfigItem("adminPassword")->asString() == "esp32admin") {
            api->getConfig()->setValue(api->getConfig()->adminPassword, GWAPIPOST_ADMIN_PASSWORD);
        }

        #ifdef API_POST_ONLY
            api->getConfig()->setValue(api->getConfig()->wifiClient, "true");
            api->getConfig()->setValue(api->getConfig()->sendN2k, "false");
            api->getConfig()->setValue(api->getConfig()->sendUsb, "false");
            api->getConfig()->setValue(api->getConfig()->receiveUsb, "false");
            api->getConfig()->setValue(api->getConfig()->usbToN2k, "false");
            api->getConfig()->setValue(api->getConfig()->usbActisense, "false");
            api->getConfig()->setValue(api->getConfig()->sendTCP, "false");
            api->getConfig()->setValue(api->getConfig()->readTCP, "false");
            api->getConfig()->setValue(api->getConfig()->tcpToN2k, "false");

            api->getConfig()->setValue(api->getConfig()->serialDirection, "receive");
            api->getConfig()->setValue(api->getConfig()->sendSerial, "false");
            api->getConfig()->setValue(api->getConfig()->receiveSerial, "true");
            api->getConfig()->setValue(api->getConfig()->serialBaud, "9600");
            api->getConfig()->setValue(api->getConfig()->serialToN2k, "true");

            api->getConfig()->setValue(api->getConfig()->apiPostEnabled, "true");
            if(GWAPIPOST_API_URL.length()) {
                api->getConfig()->setValue(api->getConfig()->apiTargetUrl, GWAPIPOST_API_URL);
            }
        #endif
    }

    void apiPostTask(GwApi *api)
    {
        GwLog *logger=api->getLogger();
        const char *wifiSSID = api->getConfig()->getConfigItem(api->getConfig()->wifiSSID, "")->asCString();
        const char *wifiPass = api->getConfig()->getConfigItem(api->getConfig()->wifiPass, "")->asCString();
        bool apiPostEnabled = api->getConfig()->getConfigItem(api->getConfig()->apiPostEnabled, "false")->asBoolean();
        String apiTargetUrl = api->getConfig()->getConfigItem(api->getConfig()->apiTargetUrl, "")->asString();
        String apiToken = api->getConfig()->getConfigItem(api->getConfig()->apiToken, "")->asString();

        if(strlen(wifiSSID) == 0 || strlen(wifiPass) == 0) {
            logger->logString("ApiPostTask: Missing wifi details. End Task");
            vTaskDelete(NULL);
            return;
        }

        if (!apiPostEnabled) {
            logger->logString("ApiPostTask: API Disabled. End Task");
            vTaskDelete(NULL);
            return;
        }

        if(!apiTargetUrl.length() || !apiToken.length()) {
            logger->logString("ApiPostTask: Missing API data required. End Task");
            vTaskDelete(NULL);
            return;
        }

        logger->logString("ApiPostTask: Starting task...");

        GwApi::BoatValue *lat = new GwApi::BoatValue(GwBoatData::_LAT);
        GwApi::BoatValue *lon = new GwApi::BoatValue(GwBoatData::_LON);
        GwApi::BoatValue *cog = new GwApi::BoatValue(GwBoatData::_COG);
        GwApi::BoatValue *alt = new GwApi::BoatValue(GwBoatData::_ALT);
        GwApi::BoatValue *aws = new GwApi::BoatValue(GwBoatData::_AWS);
        GwApi::BoatValue *awa = new GwApi::BoatValue(GwBoatData::_AWA);
        GwApi::BoatValue *tws = new GwApi::BoatValue(GwBoatData::_TWS);
        GwApi::BoatValue *twd = new GwApi::BoatValue(GwBoatData::_TWD);
        GwApi::BoatValue *sog = new GwApi::BoatValue(GwBoatData::_SOG);
        GwApi::BoatValue *stw = new GwApi::BoatValue(GwBoatData::_STW);
        GwApi::BoatValue *hdg = new GwApi::BoatValue(GwBoatData::_HDG);
        GwApi::BoatValue *mhdg = new GwApi::BoatValue(GwBoatData::_MHDG);
        GwApi::BoatValue *dbs = new GwApi::BoatValue(GwBoatData::_DBS);
        GwApi::BoatValue *dbt = new GwApi::BoatValue(GwBoatData::_DBT);
        GwApi::BoatValue *wTemp = new GwApi::BoatValue(GwBoatData::_WTemp);
        GwApi::BoatValue *gpsDays = new GwApi::BoatValue(GwBoatData::_GPSD);
        GwApi::BoatValue *gpsTime = new GwApi::BoatValue(GwBoatData::_GPST);
        GwApi::BoatValue *timezone = new GwApi::BoatValue(GwBoatData::_TZ);
        GwApi::BoatValue *valueList[] = {
            lat, lon, cog, alt, aws, awa, tws, twd, sog, stw, hdg, mhdg, dbs, dbt, wTemp, gpsDays, gpsTime, timezone
        };

        HTTPClient httpClient;
        WiFiClientSecure clientSecure;
        clientSecure.setInsecure();  //disable ssl verification. See if we can automate getting the root ca cert somehow?

        long lastPost = millis();
        long lastSave = millis();
        double minTws = 9999;
        double maxTws = 0;
        double avgTws = 0;

        double minAws = 9999;
        double maxAws = 0;
        double avgAws = 0;

        double minSog = 9999;
        double maxSog = 0;
        double avgSog = 0;

        double minDbs = 9999;
        double maxDbs = 0;
        double avgDbs = 0;

        list<String> postQueue;

        while (true) {
            api->getBoatDataValues(18, valueList);

            minTws = (tws->value < minTws) ? tws->value : minTws;
            maxTws = (tws->value > maxTws) ? tws->value : maxTws;
            avgTws = (minTws + maxTws) / 2;

            minAws = (aws->value < minAws) ? aws->value : minAws;
            maxAws = (aws->value > maxAws) ? aws->value : maxAws;
            avgAws = (minAws + maxAws) / 2;

            minSog = (sog->value < minSog) ? sog->value : minSog;
            maxSog = (sog->value > maxSog) ? sog->value : maxSog;
            avgSog = (minSog + maxSog) / 2;

            minDbs = (dbs->value < minDbs) ? dbs->value : minDbs;
            maxDbs = (dbs->value > maxDbs) ? dbs->value : maxDbs;
            avgDbs = (minDbs + maxDbs) / 2;

            DynamicJsonDocument doc(4096);
            doc["apiToken"] = apiToken;
            doc["lat"] = lat->value;
            doc["lon"] = lon->value;
            doc["cog"] = formatCourse(cog->value);
            doc["alt"] = alt->value;
            doc["awa"] = formatWind(awa->value);
            doc["twd"] = formatWind(twd->value);
            doc["minAws"] = formatKnots(minAws);
            doc["maxAws"] = formatKnots(maxAws);
            doc["aws"] = formatKnots(avgAws);
            doc["minTws"] = formatKnots(minTws);
            doc["maxTws"] = formatKnots(maxTws);
            doc["tws"] = formatKnots(avgTws);
            doc["minSog"] = formatKnots(minSog);
            doc["maxSog"] = formatKnots(maxSog);
            doc["sog"] = formatKnots(avgSog);
            doc["stw"] = formatKnots(stw->value);
            doc["hdg"] = formatCourse(hdg->value);
            doc["mhdg"] = formatCourse(mhdg->value);
            doc["minDbs"] = minDbs;
            doc["maxDbs"] = maxDbs;
            doc["dbs"] = avgDbs;
            doc["dbt"] = dbt->value;
            doc["wTemp"] = wTemp->value;
            doc["gpsTime"] = gpsTime->value;
            doc["gpsDays"] = gpsDays->value;
            doc["timezone"] = timezone->value;
            String json;
            serializeJson(doc, json);
            
            long now = millis();
            if(WiFi.isConnected()) {
                // We post every 60s but gather data every 1s to populate max's
                if((now - lastPost) >= 60000) { 

                    //Process queued posts
                    for (auto const &json : postQueue) {
                        String postMessage = "ApiPostTask: Posting queued data to: " + String(apiTargetUrl);
                        logger->logString(postMessage.c_str());
                        logger->logString(json.c_str());
                        httpClient.begin(clientSecure, apiTargetUrl);
                        httpClient.addHeader("Content-Type", "application/json");
                        httpClient.POST(json);
                        logger->logString("ApiPostTask: Response:");
                        logger->logString(httpClient.getString().c_str());
                        httpClient.end();
                        delay(1000); //Avoid overloading api. Should really post all of this in a json array, look into.
                    }
                    postQueue.clear();

                    String postMessage = "ApiPostTask: Posting data to: " + String(apiTargetUrl);
                    logger->logString(postMessage.c_str());
                    logger->logString(json.c_str());
                    httpClient.begin(clientSecure, apiTargetUrl);
                    httpClient.addHeader("Content-Type", "application/json");
                    httpClient.POST(json);
                    logger->logString("ApiPostTask: Response:");
                    logger->logString(httpClient.getString().c_str());
                    httpClient.end();

                    minAws = 9999;
                    maxAws = 0;
                    avgAws = 0;
                    minTws = 9999;
                    maxTws = 0;
                    avgTws = 0;
                    minSog = 9999;
                    maxSog = 0;
                    avgSog = 0;
                    minDbs = 9999;
                    maxDbs = 0;
                    avgDbs = 0;
                    lastPost = millis();
                }
            } else {
                //While offline store data point every hour. This allows us to store around a month worth of offline hourly data.
                if ((now - lastSave) >= 3600000) {

                    // If heap memory is getting low start removing oldest data in queue.
                    int freeHeap = (int)xPortGetFreeHeapSize();
                    if (freeHeap < 20000) {
                        postQueue.pop_front();
                    }
                    postQueue.push_back(json);

                    minAws = 9999;
                    maxAws = 0;
                    avgAws = 0;
                    minTws = 9999;
                    maxTws = 0;
                    avgTws = 0;
                    minSog = 9999;
                    maxSog = 0;
                    avgSog = 0;
                    minDbs = 9999;
                    maxDbs = 0;
                    avgDbs = 0;
                    lastSave = millis();
                }
            }

            delay(1000);
        }

        logger->logString("ApiPostTask: End Task");
        vTaskDelete(NULL);
    }

#endif