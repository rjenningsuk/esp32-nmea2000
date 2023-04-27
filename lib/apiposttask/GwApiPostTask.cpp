
//we only compile for some boards
#ifdef API_POST

    #include "GwApiPostTask.h"
    #include "GwApi.h"
    #include "GwCounter.h"
    #include <vector>
    #include "ArduinoJson.h"
    #include "HTTPClient.h"
    #include "WiFi.h"
    #include "WiFiClient.h"
    #include "WiFiClientSecure.h"


    /**
     * an init function that ist being called before other initializations from the core
     */
    void apiPostInit(GwApi *api){
        
        if(GWAPIPOST_SYSTEM_NAME.length()) {
            api->getConfig()->setValue(api->getConfig()->systemName, GWAPIPOST_SYSTEM_NAME);
        }
        if(GWAPIPOST_AP_PASSWORD.length()) {
            api->getConfig()->setValue(api->getConfig()->apPassword, GWAPIPOST_AP_PASSWORD);
        }

        #ifdef API_POST_ONLY
            api->getConfig()->setValue(api->getConfig()->wifiClient, "true");
            api->getConfig()->setValue(api->getConfig()->sendN2k, "false");
            api->getConfig()->setValue(api->getConfig()->sendUsb, "false");
            api->getConfig()->setValue(api->getConfig()->receiveUsb, "false");
            api->getConfig()->setValue(api->getConfig()->usbToN2k, "false");
            api->getConfig()->setValue(api->getConfig()->usbActisense, "false");
            api->getConfig()->setValue(api->getConfig()->serialDirection, "off");
            api->getConfig()->setValue(api->getConfig()->sendSerial, "false");
            api->getConfig()->setValue(api->getConfig()->receiveSerial, "false");
            api->getConfig()->setValue(api->getConfig()->serialToN2k, "false");
            api->getConfig()->setValue(api->getConfig()->sendTCP, "false");
            api->getConfig()->setValue(api->getConfig()->readTCP, "false");
            api->getConfig()->setValue(api->getConfig()->tcpToN2k, "false");
        #endif
    }

    void apiPostTask(GwApi *api){
        GwLog *logger=api->getLogger();
        const char *wifiSSID = api->getConfig()->getConfigItem(api->getConfig()->wifiSSID, "")->asCString();
        const char *wifiPass = api->getConfig()->getConfigItem(api->getConfig()->wifiPass, "")->asCString();
        bool apiPostEnabled = api->getConfig()->getConfigItem(api->getConfig()->apiPostEnabled, false)->asBoolean();
        String apiTargetUrl = api->getConfig()->getConfigItem(api->getConfig()->apiTargetUrl, "")->asString();
        String apiToken = api->getConfig()->getConfigItem(api->getConfig()->apiToken, "")->asString();

        if(strlen(wifiSSID) == 0 || strlen(wifiPass) == 0) {
            logger->logString("Missing wifi details. End Task");
            vTaskDelete(NULL);
            return;
        }

        if (!apiPostEnabled) {
            logger->logString("API Disabled. End Task");
            vTaskDelete(NULL);
            return;
        }

        if(!apiTargetUrl.length() || !apiToken.length()) {
            logger->logString("Missing API data required. End Task");
            vTaskDelete(NULL);
            return;
        }

        logger->logString("Starting API Post");

        //There must be a way to use the current wifi connection in main.cpp but not sure so this will do for now
        //as that could just be for access point stuffs.
        WiFi.begin(wifiSSID, wifiPass);
        String connecting = "Connecting WIFI... SSID: " + String(wifiSSID) + " PASS: " + String(wifiPass);
        logger->logString(connecting.c_str());
        while (WiFi.status() != WL_CONNECTED) {
            logger->logString("Connecting...");
            delay(500);
        }
        logger->logString("Connected to WiFi");

        GwApi::BoatValue *lat = new GwApi::BoatValue(GwBoatData::_LAT);
        GwApi::BoatValue *lon = new GwApi::BoatValue(GwBoatData::_LON);
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
        GwApi::BoatValue *valueList[] = {lat, lon, alt, aws, awa, maxAws, tws, twd, maxTws, sog, stw, hdg, mhdg, dbs, dbt, wTemp};
        GwApi::Status status;

        HTTPClient httpClient;
        WiFiClientSecure clientSecure;
        clientSecure.setInsecure();  //disable ssl verification. See if we can automate getting the root ca cert somehow?

        api->getStatus(status);

        while (true) {
            logger->logString("Get Boat Data From Internal Api");
            api->getBoatDataValues(5, valueList);

            logger->logString("Populate json data object and convert to string");
            DynamicJsonDocument doc(1024);
            doc["apiToken"] = apiToken;
            doc["lat"] = lat->value;
            doc["lon"] = lon->value;
            doc["alt"] = alt->value;
            doc["aws"] = aws->value;
            doc["awa"] = awa->value;
            doc["maxAws"] = maxAws->value;
            doc["tws"] = tws->value;
            doc["twd"] = twd->value;
            doc["maxTws"] = maxTws->value;
            doc["sog"] = sog->value;
            doc["stw"] = stw->value;
            doc["hdg"] = hdg->value;
            doc["mhdg"] = mhdg->value;
            doc["dbs"] = dbs->value;
            doc["dbt"] = dbt->value;
            doc["wTemp"] = wTemp->value;
            String json;
            serializeJson(doc, json);

            String postMessage = "Posting data to: " + String(apiTargetUrl);
            logger->logString(postMessage.c_str());
            logger->logString(json.c_str());
            httpClient.begin(clientSecure, apiTargetUrl);
            httpClient.addHeader("Content-Type", "application/json");
            httpClient.POST(json);
            logger->logString("Response:");
            logger->logString(httpClient.getString().c_str());
            httpClient.end();

            //Need to work out how to get this onto count page. main.cpp uses counters and looks like it overrides getStatus() function.
            //However we don't have access to the api in there to be able to grab this value.
            status.apiTx++;
            api->getStatus(status);

            delay(10000);
        }

        logger->logString("End Task");
        vTaskDelete(NULL);
    }

#endif