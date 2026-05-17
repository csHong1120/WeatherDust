#ifndef CONFIG_H
#define CONFIG_H

    #include <ESP8266WiFi.h>
    #include <ESP8266WebServer.h>
    #include <ESP8266HTTPClient.h>
    #include <Ticker.h>
    #include <ArduinoJson.h>
    #include <EEPROM.h>
    #include <time.h>
    #include <sys/time.h>   // struct timeval
    #include <coredecls.h>  // settimeofday_cb()

    #define VERSION             100001      // 1.00.001  
    #define OK                  1
    #define NG                  0

    #define STX                 0x02
    #define ETX                 0x03
    #define MAX_SIRAL_BUFF      20

    #define TZ                  9      // (utc+) TZ in hours
    #define DST_MN              0  // use 60mn for summer time in some countries
    #define TZ_MN               ((TZ)*60)
    #define TZ_SEC              ((TZ)*3600)
    #define DST_SEC             ((DST_MN)*60)

    #define EE_ADDR_SSID        0
    #define EE_ADDR_SSID_SIZE   32
    #define EE_ADDR_PASS        32
    #define EE_ADDR_PASS_SIZE   64
    #define EE_ADDR_AREACODE    (EE_ADDR_PASS + EE_ADDR_PASS_SIZE)

    // Software Timer
    enum {
        TMR_ID_TMR,  // Delay Timer
        TMR_ID_BLK,  // Blink Timer
        TMR_ID_WIFISET,
        TMR_ID_COLON,
        TMR_ID_MAX
    };
    
    enum {
        AREA_CODE_JACKJEON_DONG,       //0
        AREA_CODE_GONGNEUNG_DONG,      //1
        AREA_CODE_JUNGNEUNG2_DONG,     //2
        AREA_CODE_PAJU_UNJUNG1_DONG,   //3
        AREA_CODE_ASAN_TANGJUNG_MYUN,  //4
        AREA_CODE_CHUNGLA,             //5
        AREA_CODE_OSAN,                //6
        AREA_CODE_GODEOK,              //7
        AREA_CODE_MAX
    };

    typedef struct {
        bool WiFiConnect;
        byte WiFiAreaCode;  //지역구분코드
        byte SerialCnt;
        byte SerialCmd[MAX_SIRAL_BUFF];
        bool SerialFlag;
    } TWifiValue;
    TWifiValue wi;

    extern uint16 wTmr[];
    extern TWifiValue wi;

#endif