
void ResetCommand(void)
{
    delay(200);
    Serial.write(STX);
    Serial.print("S98");
    Serial.write(ETX);
}

void createWebServer(int webtype)
{
    if(webtype == 1) {
        server.on("/", []() {
            IPAddress ip = WiFi.softAPIP();
            String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
            content = "<!DOCTYPE HTML>\r\n<html>Hello from ESP8266 at ";
            content += ipStr;
            content += "<p>";
            content += st;
            content += "</p><form method='get' action='setting'><label>SSID: </label><input name='ssid' length=32><label>PASSWORD: </label><input name='pass' length=64><input type='submit'></form>";
            content += "</html>";
            server.send(200, "text/html", content);  
        });
        
        server.on("/setting", []() {
            String qsid = server.arg("ssid");
            String qpass = server.arg("pass");
            if (qsid.length() > 0 && qpass.length() > 0) {
                Serial.println("clearing eeprom");
                for (int i=0;i<100;i++) {
                    EEPROM.write(i, 0);
                }
                Serial.println(qsid);
                Serial.println("");
                Serial.println(qpass);
                Serial.println("");
                
                Serial.println("writing eeprom ssid:");
                for (int i=0;i<qsid.length();i++) {
                    EEPROM.write(i, qsid[i]);
                    Serial.print("Wrote: ");
                    Serial.println(qsid[i]); 
                }
                Serial.println("writing eeprom pass:"); 
                for (int i=0;i<qpass.length();i++) {
                    EEPROM.write(32+i, qpass[i]);
                    Serial.print("Wrote: ");
                    Serial.println(qpass[i]); 
                }
                EEPROM.write(EE_ADDR_AREACODE, wi.WiFiAreaCode);
                EEPROM.commit();
                content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
                statusCode = 200;
                ResetCommand();
            } else {
                content = "{\"Error\":\"404 not found\"}";
                statusCode = 404;
                Serial.println("Sending 404");
            }
            server.send(statusCode, "application/json", content);
        });
    } else {
        server.on("/", []() {
            content  = "<head><title>Clock Setting</title></head>\
                        <h1 style=color:#FF0000>  ====  Clock Setting  ====  </h1>\
                        <p>ALARM \
                        <a href=\"alarmon\"><button>ON</button></a>&nbsp;\
                        <a href=\"alarmoff\"><button>OFF</button></a></p>\
                        <p><form method='get' action='alarmset'><label>Alarm Setting : Hour </label><input name='hour' required minlength=1 maxlength=2 size=2><label> Minute </label><input name='min' required minlength=1 maxlength=2 size=2>&nbsp;&nbsp;<input type='submit'></form></p>\
                        <p><form method='get' action='dotbright'><label>Bright Number(1~15) : Number </label><input name='number' required minlength=1 maxlength=2 size=2>&nbsp;&nbsp;<input type='submit'></form></p>\
                        <p><form method='get' action='areacode'><label>Area Code : Area </label><input name='area' required minlength=1 maxlength=1 size=1>&nbsp;&nbsp;<input type='submit'></form></p>\
                        <p style=color:#00ffFF> (0:JAKJ, 1:GONG, 2:JUNG, 3:PAJU, 4:ASAN, 5:CHNG, 6:OSAN, 7:GODEOK) </p>\
                        <p>TIME SIGNAL \
                        <a href=\"timesigon\"><button>ON</button></a>&nbsp;\
                        <a href=\"timesigoff\"><button>OFF</button></a></p>\
                        <p>MEMORY \
                        <a href=\"cleareeprom\"><button>CLEAR</button></a></p>";

            server.send(200, "text/html", content);
        });

#if 0
            content  = "<head><title>Clock Setting</title></head>\
                        <h1>Wifi Webserver(Clock Setting)</h1>\
                        <p>ALARM \
                        <a href=\"alarmon\"><button>ON</button></a>&nbsp;\
                        <a href=\"alarmoff\"><button>OFF</button></a></p>\
                        <p><form method='get' action='alarmset'><label>Alarm Setting : Hour </label><input name='hour' length=2><label> Minute </label><input name='min' length=2><input type='submit'></form></p>\
                        <p><form method='get' action='dotbright'><label>Bright Number(1~15) : Number </label><input name='number' length=2><input type='submit'></form></p>\
                        <p><form method='get' action='areacode'><label>Area Code : Area </label><input name='area' length=1><input type='submit'></form></p>\
                        <p>TIME SIGNAL \
                        <a href=\"timesigon\"><button>ON</button></a>&nbsp;\
                        <a href=\"timesigoff\"><button>OFF</button></a></p>\
                        <p>MEMORY \
                        <a href=\"cleareeprom\"><button>CLEAR</button></a></p>";
#endif

        
        server.on("/cleareeprom", []() {
            content = "<!DOCTYPE HTML>\r\n<html>";
            content += "<p>Clearing the EEPROM</p></html>";
            server.send(200, "text/html", content);
            Serial.println("clearing eeprom");
            for (int i=0;i<96;i++) { EEPROM.write(i, 0); }
            EEPROM.commit();
            ResetCommand();
        });

        server.on("/dotbright", []() {
            byte num;
            bool NumberCheck = false;
            String inNumber = server.arg("number");

            if(inNumber.length() < 3) {
                for(byte i=0;i<(inNumber.length());i++) {
                    if(isdigit(inNumber.c_str()[i]) == 1) {
                        NumberCheck = true;
                    } else {
                        NumberCheck = false;
                        break;
                    }
                }
            }

            
            if(NumberCheck == true) { 
                num = (byte)inNumber.toInt();
                if(num > 0 && num < 16) {//1~15
                    content = "<!DOCTYPE HTML>\r\n<html>";
                    content += "<p>DotMatrix Bright  ";
                    content += inNumber;
                    content += "</p></html>";
                    server.send(200, "text/html", content);
                    
                    //Serial.print("Bright Number ");
                    //Serial.println(inNumber);
                    Serial.write(STX);
                    Serial.print("S54");//S54 
                    Serial.write(num/10+'0');
                    Serial.write(num%10+'0');
                    Serial.write(ETX);
                    delay(100);
                } else {
                    NumberCheck = false;
                }
            }

            if(NumberCheck == false) {
                content = "<!DOCTYPE HTML>\r\n<html>";
                content += "<p>Invalid DotMatrix Bright Number.(1 ~ 15)</p></html>";
                server.send(200, "text/html", content);

                Serial.write(STX);
                Serial.print("S54");//S54
                Serial.write('N');
                Serial.write(ETX);
            }
        });
        
        server.on("/areacode", []() {
            byte num;
            String isCode = server.arg("area");
            if(atoi(isCode.c_str()) < AREA_CODE_MAX && isdigit(isCode.c_str()[0]) ) { 
                num = (byte)isCode.toInt();
                //if(num > AREA_CODE_GONGNEUNG_DONG) num = AREA_CODE_GONGNEUNG_DONG;
                wi.WiFiAreaCode = num;
            
                content = "<!DOCTYPE HTML>\r\n<html>";
                content += "<p>Area Code Saving the EEPROM</p></html>";
                server.send(200, "text/html", content);
                
                Serial.println("Saving eeprom");
                EEPROM.write(EE_ADDR_AREACODE, 0);
                EEPROM.write(EE_ADDR_AREACODE, num);
                EEPROM.commit();

                Serial.write(STX);
                Serial.print("S53");//S53 
                Serial.write(num+'0');
                Serial.write(ETX);
                delay(100);
            } else {
                content = "<!DOCTYPE HTML>\r\n<html>";
                content += "<p>Invalid Area Code.(0:JAKJ, 1:GONG, 2:JUNG, 3:PAJU, 4:ASAN, 5:CHNG, 6:OSAN, 7:GODEOK)</p></html>";
                server.send(200, "text/html", content);

                Serial.write(STX);
                Serial.print("S53");//S53
                Serial.write('N');
                Serial.write(ETX);
            }
        });
        
        server.on("/alarmon", [](){
            content = "<!DOCTYPE HTML>\r\n<html>";
            content += "<p>Alarm On</p></html>";
            server.send(200, "text/html", content);
            //digitalWrite(LEDPin, HIGH);
            Serial.write(STX);
            Serial.print("S511");//S51, 1(on)
            Serial.write(ETX);
            delay(100);
        });

        server.on("/alarmoff", [](){
            content = "<!DOCTYPE HTML>\r\n<html>";
            content += "<p>Alarm Off</p></html>";
            server.send(200, "text/html", content);
            Serial.write(STX);
            Serial.print("S510");//S51, 0(off)
            Serial.write(ETX);
            delay(100); 
        });

        server.on("/timesigon", [](){
            content = "<!DOCTYPE HTML>\r\n<html>";
            content += "<p>Time Signal On</p></html>";
            server.send(200, "text/html", content);

            Serial.write(STX);
            Serial.print("S551");//S55, 1(on)
            Serial.write(ETX);
            delay(100);
        });

        server.on("/timesigoff", [](){
            content = "<!DOCTYPE HTML>\r\n<html>";
            content += "<p>Time Signal Off</p></html>";
            server.send(200, "text/html", content);

            Serial.write(STX);
            Serial.print("S550");//S55, 0(off)
            Serial.write(ETX);
            delay(100);
        });
        
        server.on("/alarmset", []() {
            bool NumberCheck = false;
            String alHour = server.arg("hour");
            String alMin  = server.arg("min");
            byte hNum, mNum;
            char temp[3] = {0};
      
            if(atoi(alHour.c_str()) >= 0 && isdigit(alHour.c_str()[0]) ) { 
                if(atoi(alMin.c_str()) >= 0 && isdigit(alMin.c_str()[0]) ) {
                    hNum = (byte)alHour.toInt();
                    mNum = (byte)alMin.toInt();
                    if(hNum < 24 && mNum < 60) NumberCheck = true;

                    //문자열 첫문자가 숫자이더라도 뒷 문자가 문자면 에러
                    if(alHour.length() == 2) {
                        if(isdigit(alHour.c_str()[1]) == 0) NumberCheck = false;
                    }

                    if(alMin.length() == 2) {
                        if(isdigit(alMin.c_str()[1]) == 0) NumberCheck = false;
                    }
                }
            }

            content  = "<!DOCTYPE HTML>\r\n<html>";
            content += "<p>Alarm Setting</p>";
            if(NumberCheck == true) {
                content += "<p>hour : ";
                content += alHour;
                content += ", Min : ";
                content += alMin;
                content += "</p></html>";
                server.send(200, "text/html", content);
                Serial.write(STX);
                Serial.print("S521");//S52, 1(OK)
                sprintf((char *)temp, "%02d", hNum);
                Serial.print(temp);
                sprintf((char *)temp, "%02d", mNum);
                Serial.print(temp);
                Serial.write(ETX);
                delay(100);
            } else {
                content += "<p>Value Error, Check your Input Number.</p>";
                content += "</html>";
                server.send(200, "text/html", content);
                Serial.write(STX);
                Serial.print("S520");//S52, 0(NG)
                Serial.write(ETX);
                delay(100);
            }
        });
    }
}
