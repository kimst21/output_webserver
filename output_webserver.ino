//Wi-Fi 라이브러리 로드
#include <WiFi.h>  // ESP32의 Wi-Fi 기능을 사용하기 위한 라이브러리 포함

// 공유기에 부여된 SSID와 패스워드를 입력
const char* ssid = "";  // Wi-Fi 네트워크 이름 (사용자가 입력해야 함)
const char* password = "";  // Wi-Fi 비밀번호 (사용자가 입력해야 함)

// 웹서버 포트 주소를 80으로 설정
WiFiServer server(80);  // ESP32에서 실행할 웹 서버 객체 생성

// HTTP request를 저장할 변수
String header;  // 클라이언트로부터 받은 HTTP 요청을 저장할 문자열

// 현재의 output 상태를 저장할 변수
String output45State = "off";  // GPIO 41의 상태 저장 변수
String output46State = "off";  // GPIO 42의 상태 저장 변수

// LED1, LED2 GPIO 핀 번호 설정
const int output41 = 41;  // LED1 제어 핀
const int output42 = 42;  // LED2 제어 핀

// 현재 시간 변수
unsigned long currentTime = millis();  // 현재 밀리초 단위의 시간 저장
// 이전 시간 변수
unsigned long previousTime = 0; 
// 타임아웃 시간 설정 (2000ms = 2초)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);  // 시리얼 통신 초기화 (디버깅 용도)
  
  // LED 핀을 출력 모드로 설정
  pinMode(output41, OUTPUT);
  pinMode(output42, OUTPUT);
  // 초기 상태를 LOW로 설정하여 LED를 끔
  digitalWrite(output41, LOW);
  digitalWrite(output42, LOW);

  // Wi-Fi 네트워크에 연결 시도
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {  // 연결될 때까지 대기
    delay(500);
    Serial.print(".");
  }

  // 연결되면 IP 주소 출력 및 웹 서버 시작
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());  // ESP32의 IP 주소 출력
  server.begin();  // 웹 서버 시작
}

void loop(){
  WiFiClient client = server.available();  // 클라이언트가 접속했는지 확인

  if (client) {  // 클라이언트가 연결되었을 경우
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");  // 새로운 클라이언트가 연결됨을 출력
    String currentLine = "";  // 클라이언트 요청을 저장할 문자열

    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // 클라이언트가 연결된 동안 반복 실행
      currentTime = millis();
      if (client.available()) {  // 클라이언트로부터 데이터가 들어오면 실행
        char c = client.read();  // 1바이트씩 읽기
        Serial.write(c);  // 읽은 데이터를 시리얼 모니터에 출력
        header += c;  // HTTP 요청 데이터를 저장

        if (c == '\n') {  // 개행 문자를 감지하면 실행
          if (currentLine.length() == 0) {  // 줄 바꿈이 2번 연속 발생하면 요청 종료
            // HTTP 응답 헤더 전송
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // GPIO on/off 제어
            if (header.indexOf("GET /41/on") >= 0) {
              Serial.println("GPIO 41 on");
              output45State = "on";
              digitalWrite(output41, HIGH);
            } else if (header.indexOf("GET /41/off") >= 0) {
              Serial.println("GPIO 41 off");
              output45State = "off";
              digitalWrite(output41, LOW);
            } else if (header.indexOf("GET /42/on") >= 0) {
              Serial.println("GPIO 42 on");
              output46State = "on";
              digitalWrite(output42, HIGH);
            } else if (header.indexOf("GET /42/off") >= 0) {
              Serial.println("GPIO 42 off");
              output46State = "off";
              digitalWrite(output42, LOW);
            }

            // HTML 웹 페이지 생성 및 전송
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>html { font-family: Helvetica; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");

            // 웹 페이지 제목 출력
            client.println("<body><h1>ESP32 Web Server</h1>");

            // GPIO 41 상태 표시 및 버튼 추가
            client.println("<p>GPIO 41 - State " + output45State + "</p>");
            if (output45State == "off") {
              client.println("<p><a href=\"/41/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/41/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 

            // GPIO 42 상태 표시 및 버튼 추가
            client.println("<p>GPIO 42 - State " + output46State + "</p>");
            if (output46State == "off") {
              client.println("<p><a href=\"/42/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/42/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            client.println("</body></html>");

            // HTTP 응답 종료
            client.println();
            break;  // 루프 종료
          } else { 
            currentLine = "";  // 현재 줄 초기화
          }
        } else if (c != '\r') {  // 개행 문자가 아닌 경우
          currentLine += c;  // 현재 줄에 문자 추가
        }
      }
    }
    header = "";  // HTTP 요청 데이터 초기화
    client.stop();  // 클라이언트와 연결 종료
    Serial.println("Client disconnected.");
  }
}
