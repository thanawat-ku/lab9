/**
 * WiFiManager advanced demo, contains advanced configurartion options
 * Implements TRIGGEN_PIN button press, press for ondemand configportal, hold for 3 seconds for reset settings.
 */
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <PubSubClient.h>

#define TRIGGER_PIN 0

// wifimanager can run in a blocking mode or a non blocking mode
// Be sure to know how to process loops with no delay() if using non blocking
bool wm_nonblocking = false; // change to true to use non blocking

WiFiManager wm; // global wm instance
const char* MQTT_BROKER_ADRESS = "mqtt-dashboard.com";
const uint16_t MQTT_PORT = 1883;
const char* MQTT_CLIENT_NAME = "kuse4532";
WiFiClient espClient;
PubSubClient mqttClient(espClient);

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
  InitMqtt();
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

void InitMqtt() 
{
	mqttClient.setServer(MQTT_BROKER_ADRESS, MQTT_PORT);
	SuscribeMqtt();
	mqttClient.setCallback(OnMqttReceived);
}


void ConnectMqtt()
{
	while (!mqttClient.connected())
	{
		Serial.print("Starting MQTT connection...");
		if (mqttClient.connect(MQTT_CLIENT_NAME))
		{
			SuscribeMqtt();
		}
		else
		{
			Serial.print("Failed MQTT connection, rc=");
			Serial.print(mqttClient.state());
			Serial.println(" try again in 5 seconds");

			delay(5000);
		}
	}
}


void HandleMqtt()
{
	if (!mqttClient.connected())
	{
		ConnectMqtt();
	}
	mqttClient.loop();
}

void SuscribeMqtt()
{
	mqttClient.subscribe("kuse/4532");
}

void OnMqttReceived(char* topic, byte* payload, unsigned int length) 
{
  String content = "";
	Serial.print("Received on ");
	Serial.print(topic);
	Serial.print(": ");

	content = "";	
	for (size_t i = 0; i < length; i++) {
		content.concat((char)payload[i]);
	}
	Serial.print(content);
	Serial.println();
}

void loop() {
  if(wm_nonblocking) wm.process(); // avoid delays() in loop when non-blocking and other long running code  
  checkButton();
  // put your main code here, to run repeatedly:

  HandleMqtt();
}
