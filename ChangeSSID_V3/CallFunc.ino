#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

void CallWeather(void) {
  // 1. 메모리 부족을 방지하기 위해 Secure 대신 일반 WiFiClient 사용
  WiFiClient client;
  HTTPClient http;

  String latlon = "";
  if (wi.WiFiAreaCode == AREA_CODE_JACKJEON_DONG) { latlon = "latitude=37.525&longitude=126.722"; }
  else if (wi.WiFiAreaCode == AREA_CODE_GONGNEUNG_DONG) { latlon = "latitude=37.625&longitude=127.082"; }
  else if (wi.WiFiAreaCode == AREA_CODE_JUNGNEUNG2_DONG) { latlon = "latitude=37.605&longitude=127.012"; }
  else if (wi.WiFiAreaCode == AREA_CODE_PAJU_UNJUNG1_DONG) { latlon = "latitude=37.735&longitude=126.755"; }
  else if (wi.WiFiAreaCode == AREA_CODE_ASAN_TANGJUNG_MYUN) { latlon = "latitude=36.804&longitude=127.065"; }
  else if (wi.WiFiAreaCode == AREA_CODE_CHUNGLA) { latlon = "latitude=37.526&longitude=126.634"; }
  else if (wi.WiFiAreaCode == AREA_CODE_OSAN) { latlon = "latitude=37.143&longitude=127.073"; }
  else if (wi.WiFiAreaCode == AREA_CODE_GODEOK) { latlon = "latitude=37.051&longitude=127.053"; } 
  else { latlon = "latitude=37.525&longitude=126.722"; }

  // 2. HTTPS 대신 HTTP 사용 (속도 향상 및 메모리 확보)
  String url = "http://api.open-meteo.com/v1/forecast?" + latlon + "&current=temperature_2m,relative_humidity_2m,weather_code,wind_speed_10m,wind_direction_10m&daily=precipitation_probability_max&timezone=Asia%2FSeoul";

  if (http.begin(client, url)) {
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
      // Stream 대신 String으로 전체를 받아 파싱 오류 방지
      String payload = http.getString();
      DynamicJsonDocument doc(2048);
      DeserializationError error = deserializeJson(doc, payload);
      
      if (!error) {
        String tm = doc["current"]["time"].as<String>();
        String hour = tm.substring(11, 13);
        tm.replace("-", ""); tm.replace("T", ""); tm.replace(":", "");
        
        float temp = doc["current"]["temperature_2m"];
        int wCode = doc["current"]["weather_code"];
        int reh = doc["current"]["relative_humidity_2m"];
        float ws_ms = doc["current"]["wind_speed_10m"].as<float>() / 3.6;
        int wd_deg = doc["current"]["wind_direction_10m"];
        int pop = doc["daily"]["precipitation_probability_max"][0];

        int sky = 1, pty = 0;
        switch (wCode) {
          case 0: sky = 1; pty = 0; break;
          case 1: sky = 2; pty = 0; break;
          case 2: sky = 3; pty = 0; break;
          case 3: sky = 4; pty = 0; break;
          case 45: case 48: sky = 4; pty = 0; break;
          case 51: case 53: case 55: sky = 4; pty = 1; break;
          case 61: case 63: case 65: sky = 4; pty = 1; break;
          case 71: case 73: case 75: case 77: sky = 4; pty = 3; break;
          case 80: case 81: case 82: sky = 4; pty = 4; break;
          case 85: case 86: sky = 4; pty = 3; break;
          case 95: case 96: case 99: sky = 4; pty = 4; break;
        }

        int wd = ((wd_deg + 22) / 45) % 8;

        Serial.write(STX);
        Serial.print("D00,"); // 날씨 프로토콜 D00 
        Serial.print(tm); Serial.print(",");
        Serial.print(hour); Serial.print(",");
        Serial.print(temp, 1); Serial.print(",");
        Serial.print(sky); Serial.print(",");
        Serial.print(pty); Serial.print(",");
        Serial.print(pop); Serial.print(",");
        Serial.print(ws_ms, 1); Serial.print(",");
        Serial.print(wd); Serial.print(",");
        Serial.print(reh); Serial.print(",");
        Serial.print(wi.WiFiAreaCode, DEC);
        Serial.write(ETX);
      }
    }
    http.end();
  }
}

// uft8 함수는 혹시 모를 의존성을 위해 그대로 남겨둡니다.
String uft8(String input) {
  String output;
  for (int i = 0; i < input.length(); i++) {
    output += "%" + String(input[i], HEX);
  }
  return output;
}

void CallDust(void) {
  WiFiClient client;
  HTTPClient http;

  String latlon = "";
  if (wi.WiFiAreaCode == AREA_CODE_JACKJEON_DONG) { latlon = "latitude=37.525&longitude=126.722"; }
  else if (wi.WiFiAreaCode == AREA_CODE_GONGNEUNG_DONG) { latlon = "latitude=37.625&longitude=127.082"; }
  else if (wi.WiFiAreaCode == AREA_CODE_JUNGNEUNG2_DONG) { latlon = "latitude=37.605&longitude=127.012"; }
  else if (wi.WiFiAreaCode == AREA_CODE_PAJU_UNJUNG1_DONG) { latlon = "latitude=37.735&longitude=126.755"; }
  else if (wi.WiFiAreaCode == AREA_CODE_ASAN_TANGJUNG_MYUN) { latlon = "latitude=36.804&longitude=127.065"; }
  else if (wi.WiFiAreaCode == AREA_CODE_CHUNGLA) { latlon = "latitude=37.526&longitude=126.634"; }
  else if (wi.WiFiAreaCode == AREA_CODE_OSAN) { latlon = "latitude=37.143&longitude=127.073"; }
  else if (wi.WiFiAreaCode == AREA_CODE_GODEOK) { latlon = "latitude=37.051&longitude=127.053"; } 
  else { latlon = "latitude=37.525&longitude=126.722"; }

  String url = "http://air-quality-api.open-meteo.com/v1/air-quality?" + latlon + "&current=pm10,pm2_5&timezone=Asia%2FSeoul";

  if (http.begin(client, url)) {
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, payload);
      
      if (!error) {
        String dataTime = doc["current"]["time"].as<String>();
        dataTime.replace("T", " "); // 중간의 T를 공백으로 변환 (ex: 2024-04-18 12:00)
        
        float raw_pm10 = doc["current"]["pm10"];
        float raw_pm25 = doc["current"]["pm2_5"];

        // 1. 실제 수치: 최대 3자리(999)로 제한
        int final_pm10 = (raw_pm10 > 999) ? 999 : (raw_pm10 < 0 ? 0 : (int)raw_pm10);
        int final_pm25 = (raw_pm25 > 999) ? 999 : (raw_pm25 < 0 ? 0 : (int)raw_pm25);

        // 2. 등급 계산 (에어코리아 기준 1:좋음, 2:보통, 3:나쁨, 4:매우나쁨)
        int pm10Grade = (final_pm10 <= 30) ? 1 : (final_pm10 <= 80) ? 2 : (final_pm10 <= 150) ? 3 : 4;
        int pm25Grade = (final_pm25 <= 15) ? 1 : (final_pm25 <= 35) ? 2 : (final_pm25 <= 75) ? 3 : 4;

        // 3. 기존 원본 코드와 완벽히 동일한 구조로 데이터 전송
        Serial.write(STX);
        Serial.print("D02,");
        
        // 데이터와 데이터 사이에 콤마(,)를 넣어서 문자열을 구분합니다. 
        // (원본 코드에서 문자열 자체에 콤마가 포함되어 있었을 확률이 매우 높습니다)
        Serial.print(dataTime); Serial.print(",");
        Serial.print(final_pm10); Serial.print(",");
        Serial.print(final_pm25); Serial.print(",");
        Serial.print(pm10Grade); Serial.print(",");
        Serial.print(pm25Grade); Serial.print(",");
        Serial.print(wi.WiFiAreaCode, DEC);
        
        Serial.write(ETX);
      }
    }
    http.end();
  }
}

void CallTime(void) {
  now = time(nullptr);
  Serial.write(STX);
  Serial.print("D01,");
  Serial.print(ctime(&now));
  Serial.write(ETX);
  //Serial.println();//NewLine
}

void CallApIP(void) {
  Serial.write(STX);
  Serial.print("D03,");
  Serial.print(WiFi.softAPIP());
  Serial.write(ETX);
}

void CallLocalIP(void) {
  Serial.write(STX);
  Serial.print("D04,");
  Serial.print(WiFi.localIP());
  Serial.write(ETX);
}

void SendResponse(byte *pt, bool sta) {
  byte SendBuf[10] = { 0 };
  byte i = 0;

  for (i = 0; i < 4; i++) {
    SendBuf[i] = *(pt + i);
  }
  if (sta) SendBuf[i++] = '1';
  else SendBuf[i++] = '0';
  SendBuf[i++] = ETX;

  Serial.print((char *)SendBuf);  //NewLine
}

void PacketProcess(byte *ptr) {
  char CmdStr, CmdData = 0;
  char Cmdbuf[2] = { 0 };
  byte CmdNum = 0;

  CmdStr = *(ptr + 1);
  Cmdbuf[0] = *(ptr + 2);
  Cmdbuf[1] = *(ptr + 3);
  CmdNum = (byte)atoi((const char *)Cmdbuf);

  switch (CmdStr) {
    case 'D':  //Request Data
      switch (CmdNum) {
        case 0:
          CallWeather();
          break;

        case 1:
          CallTime();
          break;

        case 2:
          CallDust();
          break;

        case 3:
          CallApIP();
          break;

        case 4:
          CallLocalIP();
          break;
      }
      break;

    case 'S':  //Request State
      switch (CmdNum) {
        case 0:
          //SendResponse(ptr, wi.WiFiConnect);
          CmdData = (*(ptr + 4) - '0');
          if (CmdData > (AREA_CODE_MAX - 1)) wi.WiFiAreaCode = 0;
          else wi.WiFiAreaCode = CmdData;
          break;

        case 1:
          SendResponse(ptr, wi.WiFiConnect);
          break;

        case 99:
          delay(100);
          ESP.reset();
          break;
      }
      break;
  }
}

void SerialProcess(void) {
  byte ch;

  if (Serial.available()) {
    Serial.readBytes(&ch, 1);
    switch (ch) {
      case STX:
        wi.SerialFlag = true;
        wi.SerialCnt = 0;
        wi.SerialCmd[wi.SerialCnt++] = ch;
        break;

      case ETX:
        if (wi.SerialFlag == true) {
          wi.SerialCmd[wi.SerialCnt++] = ch;
          PacketProcess(wi.SerialCmd);
          wi.SerialFlag = false;
        }
        break;

      default:
        if (wi.SerialFlag == true) {
          if (wi.SerialCnt >= MAX_SIRAL_BUFF) {
            wi.SerialFlag = false;
            wi.SerialCnt = 0;
          } else {
            wi.SerialCmd[wi.SerialCnt++] = ch;
          }
        }
        break;
    }
  }
}
