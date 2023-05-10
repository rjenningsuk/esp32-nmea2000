
//we only compile for some boards
#ifdef API_POST

    #include "GwApiPostTask.h"
    #include "GwApi.h"
    #include "GWWifi.h"
    #include <vector>
    #include "ArduinoJson.h"
    #include "HTTPClient.h"
    #include "WiFiClient.h"
    #include "WiFiClientSecure.h"

    /**
     * an init function that ist being called before other initializations from the core
     */
    void apiPostInit(GwApi *api)
    {        
        //Default certain config values
        if(GWAPIPOST_SYSTEM_NAME.length()) {
            api->getConfig()->setValue(api->getConfig()->systemName, GWAPIPOST_SYSTEM_NAME);
        }
        if(GWAPIPOST_AP_PASSWORD.length()) {
            api->getConfig()->setValue(api->getConfig()->apPassword, GWAPIPOST_AP_PASSWORD);
        }
        if(GWAPIPOST_ADMIN_PASSWORD.length()) {
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


        logger->logString("ApiPostTask: Waiting 30s to allow for initialisation.");
        delay(30000);

        if(!WiFi.isConnected()) {
            logger->logString("ApiPostTask: WiFi not connected after 30s. Check details are correct. End Task");
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
        GwApi::BoatValue *maxAws = new GwApi::BoatValue(GwBoatData::_MaxAws);
        GwApi::BoatValue *tws = new GwApi::BoatValue(GwBoatData::_TWS);
        GwApi::BoatValue *twd = new GwApi::BoatValue(GwBoatData::_TWD);
        GwApi::BoatValue *maxTws = new GwApi::BoatValue(GwBoatData::_MaxTws);
        GwApi::BoatValue *sog = new GwApi::BoatValue(GwBoatData::_SOG);
        GwApi::BoatValue *stw = new GwApi::BoatValue(GwBoatData::_STW);
        GwApi::BoatValue *hdg = new GwApi::BoatValue(GwBoatData::_HDG);
        GwApi::BoatValue *mhdg = new GwApi::BoatValue(GwBoatData::_MHDG);
        GwApi::BoatValue *dbs = new GwApi::BoatValue(GwBoatData::_DBS);
        GwApi::BoatValue *dbt = new GwApi::BoatValue(GwBoatData::_DBT);
        GwApi::BoatValue *wTemp = new GwApi::BoatValue(GwBoatData::_WTemp);
        GwApi::BoatValue *valueList[] = {lat, lon, cog, alt, aws, awa, maxAws, tws, twd, maxTws, sog, stw, hdg, mhdg, dbs, dbt, wTemp};

        HTTPClient httpClient;
        WiFiClientSecure clientSecure;
        clientSecure.setInsecure();  //disable ssl verification. See if we can automate getting the root ca cert somehow?

        while (true) {
            logger->logString("ApiPostTask: Get Boat Data From Internal Api");
            api->getBoatDataValues(17, valueList);

            logger->logString("ApiPostTask: Populate json data object and convert to string");
            DynamicJsonDocument doc(4096);
            doc["apiToken"] = apiToken;
            doc["lat"] = lat->value;
            doc["lon"] = lon->value;
            doc["cog"] = formatCourse(cog->value);
            doc["alt"] = alt->value;
            doc["aws"] = formatKnots(aws->value);
            doc["awa"] = formatWind(awa->value);
            doc["maxAws"] = formatKnots(maxAws->value);
            doc["tws"] = formatKnots(tws->value);
            doc["twd"] = formatWind(twd->value);
            doc["maxTws"] = formatKnots(maxTws->value);
            doc["sog"] = formatKnots(sog->value);
            doc["stw"] = formatKnots(stw->value);
            doc["hdg"] = formatCourse(hdg->value);
            doc["mhdg"] = formatCourse(mhdg->value);
            doc["dbs"] = dbs->value;
            doc["dbt"] = dbt->value;
            doc["wTemp"] = wTemp->value;
            String json;
            serializeJson(doc, json);

            String postMessage = "ApiPostTask: Posting data to: " + String(apiTargetUrl);
            logger->logString(postMessage.c_str());
            logger->logString(json.c_str());
            httpClient.begin(clientSecure, apiTargetUrl);
            httpClient.addHeader("Content-Type", "application/json");
            httpClient.POST(json);
            logger->logString("ApiPostTask: Response:");
            logger->logString(httpClient.getString().c_str());
            httpClient.end();

            delay(10000);
        }

        logger->logString("ApiPostTask: End Task");
        vTaskDelete(NULL);
    }

#endif