
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <Ticker.h>
#include <time.h>
#include <sys/time.h>   // struct timeval
#include <coredecls.h>  // settimeofday_cb()

ESP8266WebServer server(80);

const char *ssid = "ClockAP";
const char *pass = "dotmatrix";
String st;
String content;
int statusCode;
uint16 wCount = 0;
//uint8  bBlk=0;

Ticker blinker;
timeval tv;
timespec tp;
time_t now;


#define OK 1
#define NG 0

#define STX 0x02
#define ETX 0x03
#define MAX_SIRAL_BUFF 20

#define TZ  9      // (utc+) TZ in hours
#define DST_MN 0  // use 60mn for summer time in some countries
#define TZ_MN ((TZ)*60)
#define TZ_SEC ((TZ)*3600)
#define DST_SEC ((DST_MN)*60)

#define EE_ADDR_SSID 0
#define EE_ADDR_SSID_SIZE 32
#define EE_ADDR_PASS 32
#define EE_ADDR_PASS_SIZE 64
#define EE_ADDR_AREACODE (EE_ADDR_PASS + EE_ADDR_PASS_SIZE)

timeval cbtime;  //time set in callback

// Software Timer
enum {
  TMR_ID_TMR,  // Delay Timer
  TMR_ID_BLK,  // Blink Timer
  TMR_ID_WIFISET,
  TMR_ID_COLON,
  TMR_ID_MAX
};
uint16 wTmr[TMR_ID_MAX];

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

String uft8(String input);
//=======================================================================
//=======================================================================
//=======================================================================
//void Timer1_handler(void)
void ICACHE_RAM_ATTR Timer1_handler(void) {
  byte i;

  timer1_write(5000);                 //0.2us * 5000 = 1ms
  for (i = 0; i < TMR_ID_MAX; i++) {  // SW Timer
    if (wTmr[i] != 0) wTmr[i]--;
  }

  /*
    if(wCount++ >= 1000) {
        wCount = 0;
        digitalWrite(LED_BUILTIN, !(digitalRead(LED_BUILTIN)));  //Invert Current State of LED  
    }
    */
}

uint16 GetTimer(uint8 tmid) {
  yield();  // 이 함수를 실행하지 않으면, H/W Watchdog이 걸려 리셋됨.  또는 delay(0); 함수를 실행할 것..
  if (tmid > TMR_ID_MAX) return NG;
  return wTmr[tmid];
}

void SetTimer(uint8 tmid, uint16 tmData) {
  if (tmid < TMR_ID_MAX) {
    wTmr[tmid] = tmData;
  }
}


// for testing purpose:
extern "C" int clock_gettime(clockid_t unused, struct timespec *tp);


//=======================================================================
//=======================================================================
//=======================================================================
void setup() {
  char readEEprom = 0;

  Serial.begin(115200);
  EEPROM.begin(512);
  pinMode(LED_BUILTIN, OUTPUT);
  delay(10);

  //blinker.attach(0.001, Timer1_handler); //Use <strong>attach_ms</strong> if you need time in ms
  timer1_attachInterrupt(Timer1_handler);
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);  // 80MHz / 16 = 5MHz, 0.2us
  timer1_write(5000);                              //0.2us * 5000 = 1ms

  configTime(TZ_SEC, DST_SEC, "pool.ntp.org");

  //Serial.println();
  //Serial.println();
  Serial.println("Startup");

  // read eeprom for ssid and pass
  Serial.println("Reading EEPROM ssid");
  String esid;

  wi.WiFiConnect = false;
  for (int i = EE_ADDR_SSID; i < (EE_ADDR_SSID + EE_ADDR_SSID_SIZE); i++) {
    if (i < (EE_ADDR_SSID + 5)) {
      if (char(EEPROM.read(i)) < 0x20 || char(EEPROM.read(i)) > 0x80) {
        esid = "";
        break;
      }
    }
    esid += char(EEPROM.read(i));
  }
  //esid = "tekbase1111";

  Serial.print("SSID: ");
  Serial.println(esid);
  delay(100);

  Serial.println("Reading EEPROM pass");
  String epass = "";
  for (int i = EE_ADDR_PASS; i < (EE_ADDR_PASS + EE_ADDR_PASS_SIZE); i++) {
    if (i < (EE_ADDR_PASS + 5)) {
      if (char(EEPROM.read(i)) < 0x20 || char(EEPROM.read(i)) > 0x80) {
        epass = "";
        break;
      }
    }
    epass += char(EEPROM.read(i));
  }

  wi.WiFiAreaCode = (byte) char(EEPROM.read(EE_ADDR_AREACODE));
  if (wi.WiFiAreaCode > (AREA_CODE_MAX - 1)) wi.WiFiAreaCode = 0;

  //epass = "tekbase20486";
  Serial.print("PASS: ");
  Serial.println(epass);
  delay(100);

  WiFi.disconnect();

#if 1
  if (esid.length() > 1) {
    WiFi.begin(esid.c_str(), epass.c_str());
    if (ConnectStation()) {
      launchWeb(0);
      wi.WiFiConnect = true;
      return;
    }
  }
#endif
  setupAP();
}

bool ConnectStation(void) {
  int c = 0;
  //Serial.println("Waiting for Wifi to connect");
  while (c++ < 20) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.write(STX);     //STX
      Serial.print("S001");  //connect
      Serial.write(ETX);     //ETX
      return true;
    }
    delay(500);
    Serial.print(".");
  }
  Serial.write(STX);     //STX
  Serial.print("S000");  //Unconnect
  Serial.write(ETX);     //ETX
  return false;
}

void launchWeb(int webtype) {
  Serial.println("");
  Serial.println("WiFi connected");


  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());


  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());

  createWebServer(webtype);

  //Start the server
  server.begin();
  Serial.println("Server started");
}

void setupAP(void) {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; i++) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }

  Serial.println("");
  st = "<ol>";
  for (int i = 0; i < n; ++i) {
    // Print SSID and RSSI for each network found
    st += "<li>";
    st += WiFi.SSID(i);
    st += " (";
    st += WiFi.RSSI(i);
    st += ")";
    st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
    st += "</li>";
  }
  st += "</ol>";

  delay(100);
  WiFi.softAP(ssid, pass, 6);
  Serial.println("softap");
  launchWeb(1);
  Serial.println("over");
}

void loop() {
  static uint8 blk = 0;

  SerialProcess();
  server.handleClient();

  /* ESP8266 장착된 LED 깜빡임 속도 제어 */
  if (GetTimer(TMR_ID_BLK) == 0) {
    if (blk == 1) {
      blk = 0;
      SetTimer(TMR_ID_BLK, 50);
      digitalWrite(LED_BUILTIN, LOW);  //Invert Current State of LED
    } else {
      blk = 1;
      SetTimer(TMR_ID_BLK, 950);
      digitalWrite(LED_BUILTIN, HIGH);  //Invert Current State of LED
    }
  }
}
