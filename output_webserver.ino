//Wi-Fi라이브러리 로드
#include <WiFi.h>

// 공유기에 부여된 SSID와 패스워드를 입력
const char* ssid = "";
const char* password = "";

// 웹서버 포트주소를 80
WiFiServer server(80);

// HTTP request를 저장 할 변수
String header;

// 현재의 output상태를 저장할 보조변수
String output45State = "off";
String output46State = "off";

// LED1, LED2 GPIO
const int output41 = 41;
const int output42 = 42;

// 현재시간
unsigned long currentTime = millis();
// 이전시간
unsigned long previousTime = 0; 
// 밀리세컨드로 타임아웃 정의 (예 - 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  // 변수들을 OUTPUT으로 정의
  pinMode(output41, OUTPUT);
  pinMode(output42, OUTPUT);
  // output을 0으로 셑
  digitalWrite(output41, LOW);
  digitalWrite(output42, LOW);

  // Wi-Fi network(SSID and password)연결을 시도
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // 연결되면 IP address출력 and web server 스타트
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop(){
  WiFiClient client = server.available();   // 클라이언트가 있는지 체크
/* 클라이언트로부터 요청이 수신되면 들어오는 데이터를 저장합니다.
다음에 오는 while 루프는 클라이언트가 연결되어 있는 한 실행됩니다 */
  if (client) {                    // 만약 새로운 클라이언트가 있으면
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client."); // 메세지출력
    String currentLine = "";       // 클라이어트로 부터 입력될 스트링준비
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {    // 클라이어트로부터 입력된 데이타가 있으면
        char c = client.read();    // 1바이트를 읽고
        Serial.write(c);           // 다시 프린터에 출력
        header += c;
        if (c == '\n') {           // 만약 newline character이면
          // 만역 현재라인이 공백이면, 행으로 2라인을 읽는다.
          // client HTTP request의 끝이면 답변을 보낸다
          if (currentLine.length() == 0) {
            // HTTP 헤더는 항상 한개의 response code (e.g. HTTP/1.1 200 OK)와
            // 한개의 content-type으로 시작, so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
/* if 및 else 문의 다음 섹션은 웹 페이지에서 어떤 버튼이 눌렸는지 확인하고 
그에 따라 출력을 제어합니다. 이전에 본 것처럼 눌린 버튼에 따라 다른 URL에서
요청을 합니다. */          
            // GPIOs on and off
            if (header.indexOf("GET /41/on") >= 0) {
              Serial.println("GPIO 41 on");
              output41State = "on";
              digitalWrite(output41, HIGH);
            } else if (header.indexOf("GET /41/off") >= 0) {
              Serial.println("GPIO 41 off");
              output41State = "off";
              digitalWrite(output45, LOW);
            } else if (header.indexOf("GET /42/on") >= 0) {
              Serial.println("GPIO 42 on");
              output42State = "on";
              digitalWrite(output42, HIGH);
            } else if (header.indexOf("GET /42/off") >= 0) {
              Serial.println("GPIO 42 off");
              output42State = "off";
              digitalWrite(output42, LOW);
            }
/*  ESP32는 웹 페이지를 구축하기 위해 일부 HTML 코드로 브라우저에 응답을 보냅니다.
client.println() 표현을 사용하여 웹 페이지가 클라이언트로 전송됩니다. 
클라이언트에 보낼 내용을 인수로 입력해야 합니다. */        
            // HTML web page 표시
            client.println("<!DOCTYPE html><html>"); //HTML을 보내고 있음을 나타냄
/* 모든 웹 브라우저에서 웹 페이지가 반응하도록 만듬 */
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
/* favicon에 대한 요청을 방지하기 위해 다음을 사용 */
            client.println("<link rel=\"icon\" href=\"data:,\">");
/* 다음으로 버튼과 웹 페이지 모양의 스타일을 지정하는 CSS 텍스트가 있습니다.
Helvetica 글꼴을 선택하고 콘텐츠를 블록으로 표시하고 중앙에 정렬하도록 정의합니다. */
            // CSS의 on/off 버튼 스타일링 
            // 폰투사이즈, 백라운드칼라 등 변경가능
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
/* 테두리가 없는 #4CAF50 색상, 흰색 텍스트, 패딩: 16px 40px로 버튼의 스타일을 
지정합니다. 또한 텍스트 장식을 없음으로 설정하고 글꼴 크기, 여백 및 포인터에 
대한 커서를 정의합니다. */
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
/* 다른 색상으로 꺼짐 버튼의 스타일정의 */
            client.println(".button2 {background-color: #555555;}</style></head>");         
            // 웹페이지 헤딩
            client.println("<body><h1>ESP32 Web Server</h1>");    
            // 현재상태표시, GPIO 45의 ON/OFF버튼 표시  
            client.println("<p>GPIO 41 - State " + output41State + "</p>");
            // 만약 output45State가 off이면  ON button으로 표시       
            if (output45State=="off") {
              client.println("<p><a href=\"/41/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/41/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
               
            // 현재상태와 ON/OFF buttons(GPIO 46)를 표시한다 
            client.println("<p>GPIO 42 - State " + output42State + "</p>");
            // If the output46State is off, it displays the ON button       
            if (output42State=="off") {
              client.println("<p><a href=\"/42/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/42/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            client.println("</body></html>");
            
            // The HTTP response가 다른 브랭크라인으로 끝난다 
            client.println();
            // 루푸 탈출
            break;
          } else { //  newline이면 현재 라인을 클리어
            currentLine = "";
          }
        } else if (c != '\r') {  // carriage return character가 아닌 다른 글자를 받았다면
          currentLine += c;      // currentLine의 끝부분에 붙인다
        }
      }
    }
    // 헤더변수를 클리어
    header = "";
    // 연결을 끊는다
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
