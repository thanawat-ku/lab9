/**
 * WiFiManager advanced demo, contains advanced configurartion options
 * Implements TRIGGEN_PIN button press, press for ondemand configportal, hold for 3 seconds for reset settings.
 */
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define TRIGGER_PIN 0

// wifimanager can run in a blocking mode or a non blocking mode
// Be sure to know how to process loops with no delay() if using non blocking
bool wm_nonblocking = false; // change to true to use non blocking

WiFiManager wm; // global wm instance
WiFiManagerParameter custom_field; // global param ( for non blocking w params )

String ApiHost = "http://192.168.1.1";

void setup() {
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP  
  Serial.begin(115200);
  Serial.setDebugOutput(true);  
  delay(3000);
  Serial.println("\n Starting");

  pinMode(TRIGGER_PIN, INPUT);
  

  if(wm_nonblocking) wm.setConfigPortalBlocking(false);

  // custom menu via array or vector
  std::vector<const char *> menu = {"wifi","info","sep","restart","exit"};
  wm.setMenu(menu);

  // set dark theme
  wm.setClass("invert");

  wm.setConfigPortalTimeout(60); // auto close configportal after n seconds

  bool res;
  res = wm.autoConnect("AutoConnectAP","password"); // password protected ap

  if(!res) {
    Serial.println("Failed to connect or hit timeout");
    ESP.restart();
  } 
  else {
    //if you get here you have connected to the WiFi    
    Serial.println("connected...yeey :)");
  }
}

void checkButton(){
  // check for button press
  if ( digitalRead(TRIGGER_PIN) == LOW ) {
    // poor mans debounce/press-hold, code not ideal for production
    delay(50);
    if( digitalRead(TRIGGER_PIN) == LOW ){
      Serial.println("Button Pressed");
      // still holding button for 3000 ms, reset settings, code not ideaa for production
      delay(3000); // reset delay hold
      if( digitalRead(TRIGGER_PIN) == LOW ){
        Serial.println("Button Held");
        Serial.println("Erasing Config, restarting");
        wm.resetSettings();
        //ESP.restart();
      }
      
      // start portal w delay
      Serial.println("Starting config portal");
      wm.setConfigPortalTimeout(120);
      
      if (!wm.startConfigPortal("OnDemandAP","password")) {
        Serial.println("failed to connect or hit timeout");
        delay(3000);
        ESP.restart();
      } else {
        //if you get here you have connected to the WiFi
        Serial.println("connected...yeey :)");
      }
    }
  }
}

void processResponse(int httpCode, HTTPClient& http)
{
	if (httpCode > 0) {
		Serial.printf("Response code: %d\t", httpCode);

		if (httpCode == HTTP_CODE_OK) {
			String payload = http.getString();
			Serial.println(payload);
		}
	}
	else {
		Serial.printf("Request failed, error: %s\n", http.errorToString(httpCode).c_str());
	}
	http.end();
}
void createStudent(String studentCode, String studentName, String gender)
{
	HTTPClient http;
	http.begin(ApiHost + "/student");
	http.addHeader("Content-Type", "application/json");
	
	String message = "";
	StaticJsonDocument<300> jsonDoc;
	jsonDoc["student_code"] = studentCode;
	jsonDoc["student_name"] = studentName;
	jsonDoc["gender"] = gender;
	serializeJson(jsonDoc, message);
	
	int httpCode = http.POST(message);
	processResponse(httpCode, http);
}
void getStudent(String studentCode)
{
	HTTPClient http;
	http.begin(ApiHost + "/student?student_code=" + studentCode);
	int httpCode = http.GET();
	processResponse(httpCode, http);
}
void updateStudent(String studentCode, String studentName, String gender){
    HTTPClient http;
	http.begin(ApiHost + "/student");
	http.addHeader("Content-Type", "application/json");
	
	String message = "";
	StaticJsonDocument<300> jsonDoc;
	jsonDoc["student_code"] = studentCode;
	jsonDoc["student_name"] = studentName;
	jsonDoc["gender"] = gender;
	serializeJson(jsonDoc, message);
	
	int httpCode = http.PUT(message);
	processResponse(httpCode, http);
}
void deleteStudent(String studentCode)
{
	HTTPClient http;
	http.begin(ApiHost + "/student?student_code=" + studentCode);
	int httpCode = http.sendRequest("DELETE");
	processResponse(httpCode, http);
}
void loop() {
  if(wm_nonblocking) wm.process(); // avoid delays() in loop when non-blocking and other long running code  
  checkButton();
  // put your main code here, to run repeatedly:
  createStudent("6540207777","Teerawat","M");
  updateStudent("6540207777","Tanya","F");
  getStudent("6540207777");
  deleteStudent("6540207777");
}
