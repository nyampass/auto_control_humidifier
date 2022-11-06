#include <Ticker.h>
#include <ESP8266WiFi.h>
#define ON_PIN 12
IPAddress ip(192, 168, 3, 220); //ip
IPAddress subnet(255, 255, 255, 0);



long count = 0;
long countAve = 0;
int freq = 50; //50Hz
double analogPinStatus;
double ammeter;
double dt = 0.001;

double ammeterSum = 0;
double realAmmeter = 0;
double voltage = 100.0;
double sum = 0.0;
double ave = 0.0;
long mil = 0;

void getAmmeterValue();

bool last_req_state = false;
Ticker tickerCheck(getAmmeterValue, 1, 0, MILLIS);

const char* ssid = ""; // 初期設定必須
const char* password = ""; // 初期設定必須

WiFiServer server(80);

void setup()
{
    Serial.begin(115200);
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.config(ip, ip, subnet);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    
    server.begin();

    pinMode(ON_PIN,OUTPUT);

//    tickerCheck.attach_ms(1, getAmmeterValue);
}

void loop(){
  tickerCheck.update();
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New Client.");           
    String currentLine = "";               
    while (client.connected()) {           
      if (client.available()) {             
        char c = client.read();             
        Serial.write(c);                  
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            client.print("<h1>Server OK</h1>");
            client.println();
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
        
        if (!currentLine.compareTo("x-access-key: OW3gO9F0ON")){
          last_req_state = true;
          tickerCheck.start();
        }
        if (!currentLine.compareTo("x-access-key: OW3gO9F0OFF")){
          last_req_state = false;
          tickerCheck.start();
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
}

int sumRealAmmeter = 0;
int avg = 0;
void getAmmeterValue()
{
  analogPinStatus = analogRead(0);
//  Serial.println(analogPinStatus);
  ammeter = (analogPinStatus - 680.0) * 0.7368359376 * 0.2;
  ammeterSum = ammeterSum + abs(ammeter) * dt;

  if (count % 1000 == 0){
    realAmmeter = ammeterSum;
    ammeterSum = 0;
    Serial.println(realAmmeter);
    sumRealAmmeter += realAmmeter;
  }
  if(count > 10000){
    tickerCheck.stop();
    Serial.println("ticker stop");
    avg = sumRealAmmeter / 10;
    if(avg * 100 < 400 and last_req_state){
      digitalWrite(ON_PIN,HIGH);
      delay(500);
      digitalWrite(ON_PIN,LOW);
    }else if(avg * 100 > 400 and !last_req_state){
      digitalWrite(ON_PIN,HIGH);
      delay(500);
      digitalWrite(ON_PIN,LOW);
    }
    Serial.println(avg * 100 + ":" + last_req_state);
    sumRealAmmeter = 0;
    count = 0;  
  }

  count++;
}