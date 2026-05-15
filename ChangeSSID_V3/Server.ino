#include "define.h"

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
            content = R"=====(
                <!DOCTYPE html>
                <html lang="ko">
                <head>
                    <meta charset="UTF-8">
                    <meta name="viewport" content="width=device-width, initial-scale=1.0">
                    <title>시계 설정 페이지</title>
                    <style>
                        body { font-family: 'Malgun Gothic', sans-serif; background-color: #f0f2f5; margin: 0; padding: 20px; display: flex; justify-content: center; }
                        .container { background-color: #ffffff; border-radius: 12px; box-shadow: 0 4px 12px rgba(0,0,0,0.1); padding: 25px; width: 100%; max-width: 400px; }
                        h1 { color: #1a73e8; text-align: center; font-size: 22px; margin-bottom: 20px; border-bottom: 2px solid #e8eaed; padding-bottom: 10px; }
                        .section { margin-bottom: 20px; padding: 15px; border: 1px solid #eee; border-radius: 8px; }
                        h2 { font-size: 15px; color: #5f6368; margin: 0 0 12px 0; border-left: 4px solid #1a73e8; padding-left: 8px; }
                        .btn-group { display: flex; gap: 8px; margin-bottom: 10px; }
                        button { flex: 1; padding: 10px; border: none; border-radius: 6px; cursor: pointer; font-size: 14px; font-weight: bold; transition: 0.2s; }
                        
                        .btn-on { background-color: #34a853; color: white; }
                        .btn-off { background-color: #ea4335; color: white; }
                        .btn-submit { background-color: #1a73e8; color: white; width: 80px; flex: none; }
                        .btn-clear { background-color: #80868b; color: white; width: 100%; }
                        
                        .input-row { display: flex; align-items: center; justify-content: space-between; gap: 10px; background: #f8f9fa; padding: 8px; border-radius: 6px; }
                        input[type='number'] { padding: 8px; border: 1px solid #dadce0; border-radius: 4px; width: 50px; text-align: center; }
                        
                        #toast {
                            visibility: hidden; min-width: 200px; background-color: #333; color: #fff; text-align: center;
                            border-radius: 30px; padding: 12px; position: fixed; z-index: 1; left: 50%; bottom: 30px;
                            transform: translateX(-50%); font-size: 14px; opacity: 0; transition: opacity 0.3s, visibility 0.3s;
                        }
                        #toast.show { visibility: visible; opacity: 1; }
                    </style>
                </head>

                <body>

                    <div class="container">
                    <h1>⚙️ 스마트 시계 설정</h1>
                
                    <div class="section">
                        <h2>⏰ 알람 (Alarm)</h2>
                            <div class="btn-group">
                                <button class="btn-on" onclick="sendCmd('/alarmon', '알람 켜기 설정 완료')">켜기</button>
                                <button class="btn-off" onclick="sendCmd('/alarmoff', '알람 끄기 설정 완료')">끄기</button>
                            </div>

                            <div class="input-row">
                                <div style="display:flex; align-items:center; gap:5px;">
                                <input id="alHour" type="number" min="0" max="23" placeholder="시"> :
                                <input id="alMin" type="number" min="0" max="59" placeholder="분">
                            </div>
                            <button class="btn-submit" onclick="setAlarm()">설정</button>
                        </div>
                    </div>

                    <div class="section">
                        <h2>💡 밝기 조절 (1~15)</h2>
                            <div class="input-row">
                                <input id="bright" type="number" min="1" max="15" value="8">
                                <button class="btn-submit" onclick="setBright()">적용</button>
                            </div>
                        </div>

                    <div class="section">
                        <h2>📍 지역 코드 변경</h2>
                        <div class="input-row">
                        <input id="area" type="number" min="0" max="7">
                        <button class="btn-submit" onclick="setArea()">변경</button>
                        </div>
                        <div class="note">0:작전 1:공릉 2:정릉 3:파주 4:아산 5:청라 6:오산 7:고덕</div>
                    </div>

                    <div class="section">
                        <h2>🔔 매 정각 시보</h2>
                        <div class="btn-group">
                        <button class="btn-on" onclick="sendCmd('/timesigon', '시보 켜기 설정 완료')">켜기</button>
                        <button class="btn-off" onclick="sendCmd('/timesigoff', '시보 끄기 설정 완료')">끄기</button>
                        </div>
                    </div>

                    <button class="btn-clear" onclick="sendCmd('/cleareeprom', 'EEPROM 설정 초기화 완료')">💾 설정 초기화</button>
                        </div>

                    <div id="toast"></div>

                    <script>
                        function sendCmd(url, msg) {
                            fetch(url)
                            .then(response => { showToast(msg); })
                            .catch(err => { showToast("통신 오류가 발생했습니다."); });
                        }

                        function setAlarm() {
                            const h = document.getElementById('alHour').value;
                            const m = document.getElementById('alMin').value;
                            if(!h || !m) return showToast("시간을 입력하세요.");
                            sendCmd(`/alarmset?hour=${h}&min=${m}`, `알람이 ${h}시 ${m}분으로 설정되었습니다.`);
                        }

                        function setBright() {
                            const b = document.getElementById('bright').value;
                            sendCmd(`/dotbright?number=${b}`, `밝기가 ${b}단계로 변경되었습니다.`);
                        }

                        function setArea() {
                            const a = document.getElementById('area').value;
                            sendCmd(`/areacode?area=${a}`, `지역 코드가 ${a}번으로 변경되었습니다.`);
                        }

                        function showToast(message) {
                            const toast = document.getElementById("toast");
                            toast.innerText = message;
                            toast.classList.add("show");
                            setTimeout(() => { toast.classList.remove("show"); }, 2500);
                        }
                    </script>
                </body>
                </html>
            )=====";

            server.send(200, "text/html", content);
        });

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
