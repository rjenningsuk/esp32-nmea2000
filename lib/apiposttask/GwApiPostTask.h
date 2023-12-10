#pragma once
#include "GwApi.h"
#include "GwApiPostTaskParams.h"

//we only compile for some boards
#ifdef API_POST

    void apiPostTask(GwApi *param);
    void apiPostInit(GwApi *param);
    DECLARE_USERTASK_PARAM(apiPostTask, 8000);
    DECLARE_INITFUNCTION(apiPostInit);
    DECLARE_CAPABILITY(api_post, true); 

    #ifdef API_POST_ONLY

        DECLARE_CAPABILITY(api_post_only, true);
        DECLARE_STRING_CAPABILITY(HELP_URL,GWAPIPOST_HELP_URL);

        // DECLARE_CAPABILITY(HIDEsystemName, true);
        DECLARE_CAPABILITY(HIDEtalkerId, true);
        DECLARE_CAPABILITY(HIDEstopApTime, true);
        // DECLARE_CAPABILITY(HIDEapPassword, true);
        DECLARE_CAPABILITY(HIDEuseAdminPass, true);
        // DECLARE_CAPABILITY(HIDEadminPassword, true);

        DECLARE_CAPABILITY(HIDEapiPostEnabled, true);
        DECLARE_CAPABILITY(HIDEapiTargetUrl, true);

        DECLARE_CAPABILITY(HIDEshowInvalidData, true);
        DECLARE_CAPABILITY(HIDElogLevel, true);
        DECLARE_CAPABILITY(HIDEminXdrInterval, true);
        DECLARE_CAPABILITY(HIDEmin2KInterval, true);
        DECLARE_CAPABILITY(HIDEsendN2k, true);
        DECLARE_CAPABILITY(HIDEusbActisense, true);
        DECLARE_CAPABILITY(HIDEusbBaud, true);
        DECLARE_CAPABILITY(HIDEsendUsb, true);
        DECLARE_CAPABILITY(HIDEreceiveUsb, true);
        DECLARE_CAPABILITY(HIDEusbToN2k, true);
        DECLARE_CAPABILITY(HIDEusbReadFilter, true);
        DECLARE_CAPABILITY(HIDEusbWriteFilter, true);
        DECLARE_CAPABILITY(HIDEusbActSend, true);
        DECLARE_CAPABILITY(HIDEserialDirection, true);
        DECLARE_CAPABILITY(HIDEserialBaud, true);
        DECLARE_CAPABILITY(HIDEsendSerial, true);
        DECLARE_CAPABILITY(HIDEreceiveSerial, true);
        DECLARE_CAPABILITY(HIDEserialToN2k, true);
        DECLARE_CAPABILITY(HIDEserialReadF, true);
        DECLARE_CAPABILITY(HIDEserialWriteF, true);
        DECLARE_CAPABILITY(HIDEserverPort, true);
        DECLARE_CAPABILITY(HIDEmaxClients, true);
        DECLARE_CAPABILITY(HIDEsendTCP, true);
        DECLARE_CAPABILITY(HIDEreadTCP, true);
        DECLARE_CAPABILITY(HIDEtcpToN2k, true);
        DECLARE_CAPABILITY(HIDEtcpReadFilter, true);
        DECLARE_CAPABILITY(HIDEtcpWriteFilter, true);
        DECLARE_CAPABILITY(HIDEsendSeasmart, true);
        DECLARE_CAPABILITY(HIDEtclEnabled, true);
        DECLARE_CAPABILITY(HIDEremotePort, true);
        DECLARE_CAPABILITY(HIDEremoteAddress, true);
        DECLARE_CAPABILITY(HIDEsendTCL, true);
        DECLARE_CAPABILITY(HIDEreadTCL, true);
        DECLARE_CAPABILITY(HIDEtclToN2k, true);
        DECLARE_CAPABILITY(HIDEtclReadFilter, true);
        DECLARE_CAPABILITY(HIDEtclWriteFilter, true);
        DECLARE_CAPABILITY(HIDEtclSeasmart, true);
        DECLARE_CAPABILITY(HIDEwifiClient, true);

        DECLARE_CAPABILITY(HIDEXDR1, true);
        DECLARE_CAPABILITY(HIDEXDR2, true);
        DECLARE_CAPABILITY(HIDEXDR3, true);
        DECLARE_CAPABILITY(HIDEXDR4, true);
        DECLARE_CAPABILITY(HIDEXDR5, true);
        DECLARE_CAPABILITY(HIDEXDR6, true);
        DECLARE_CAPABILITY(HIDEXDR8, true);
        DECLARE_CAPABILITY(HIDEXDR9, true);
        DECLARE_CAPABILITY(HIDEXDR10, true);
        DECLARE_CAPABILITY(HIDEXDR11, true);
        DECLARE_CAPABILITY(HIDEXDR12, true);
        DECLARE_CAPABILITY(HIDEXDR13, true);
        DECLARE_CAPABILITY(HIDEXDR14, true);
        DECLARE_CAPABILITY(HIDEXDR15, true);
        DECLARE_CAPABILITY(HIDEXDR16, true);
        DECLARE_CAPABILITY(HIDEXDR17, true);
        DECLARE_CAPABILITY(HIDEXDR18, true);
        DECLARE_CAPABILITY(HIDEXDR19, true);
        DECLARE_CAPABILITY(HIDEXDR20, true);
        DECLARE_CAPABILITY(HIDEXDR21, true);
        DECLARE_CAPABILITY(HIDEXDR22, true);
        DECLARE_CAPABILITY(HIDEXDR23, true);
        DECLARE_CAPABILITY(HIDEXDR24, true);
        DECLARE_CAPABILITY(HIDEXDR25, true);
        DECLARE_CAPABILITY(HIDEXDR26, true);
        DECLARE_CAPABILITY(HIDEXDR27, true);
        DECLARE_CAPABILITY(HIDEXDR28, true);
        DECLARE_CAPABILITY(HIDEXDR29, true);
        DECLARE_CAPABILITY(HIDEXDR30, true);
    #endif

#endif