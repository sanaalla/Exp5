
#include <ArduinoJson.h> 
#include <SPI.h>

#include <WiFi101.h>
#define PubNub_BASE_CLIENT WiFiClient
#include <PubNub.h>

static char ssid[] = "ocadu-embedded";      //SSID of the wireless network
static char pass[] = "internetofthings";    //password of that network
int status = WL_IDLE_STATUS;                // the Wifi radio's status

const static char pubkey[] = "pub-c-c7e392ef-57af-41df-bf5b-18e583d14c19";  //get this from your PUbNub account
const static char subkey[] = "sub-c-d6f95e84-c881-11e7-9178-bafd478c18bc";  //get this from your PubNub account

const static char pubChannel[] = "sana"; //choose a name for the channel to publish messages to
const static char subChannel[] = "sean"; //choose a name for the channel to publish messages to

int ledPinCoffee = 9;                  //GREEN LED, functions as "do you want coffee" or YES to "do you want coffee"
int ledPinAnswer = 10;                  //RED LED, functions as NO to "do you want coffee"
int togglePinON = 13;                   //the pin the toggle is attached  to
int togglePinOFF = 12;                  //the pin the toggle is attached  to

int myVal2; //my toggle YES/"do you want coffee"
int myVal3; //my toggle NO

int yourVal2; //your toggle "do you want coffee"
int yourVal3; //your toggle NO to "do you want coffee"

void setup() {
  // put your setup code here, to run once:
  pinMode(togglePinON, INPUT_PULLUP);
  pinMode(togglePinOFF, INPUT_PULLUP);
  pinMode(ledPinCoffee, OUTPUT);
   pinMode(ledPinAnswer, OUTPUT);

  Serial.begin(9600);
  connectToServer();
}


void loop() 
{
  
myVal2 = digitalRead(togglePinON);//this is YES or "do you want coffee"
myVal3 = digitalRead(togglePinOFF);//this is NO

publishToPubNub();                    //send your values to PubNub
readFromPubNub();                   //read values from PubNub

  
//IF SOMEONE IS GOING TO GET COFFEE, GREEN LIGHT GOES ON
//IF YOU ARE ANSWERING YES TO COFFEE SIGNAL, YOU TRIGGER YOUR PARTNER'S GREEN LIGHT
  if (yourVal2 == 1 && yourVal3 == 0)
  
  {
  digitalWrite(ledPinCoffee, HIGH);   // turn the LED on (HIGH is the voltage level)    
    }
    
    else {
      digitalWrite(ledPinCoffee, LOW);
    }

//IF ANSWERING NO, RED LIGHT GOES ON TO YOUR PARTNER'S DEVICE
    
  if (yourVal2 == 0)
  
  {
  digitalWrite(ledPinAnswer, HIGH);   // turn the LED on (HIGH is the voltage level)            
    }
    else {
      digitalWrite(ledPinAnswer, LOW);
    }


}




void connectToServer()
{
  WiFi.setPins(8,7,4,2); //This is specific to the feather M0
 
  status = WiFi.begin(ssid, pass);                    //attempt to connect to the network
  Serial.println("***Connecting to WiFi Network***");


 for(int trys = 1; trys<=10; trys++)                    //use a loop to attempt the connection more than once
 { 
    if ( status == WL_CONNECTED)                        //check to see if the connection was successful
    {
      Serial.print("Connected to ");
      Serial.println(ssid);
  
      PubNub.begin(pubkey, subkey);                      //connect to the PubNub Servers
      Serial.println("PubNub Connected"); 
      break;                                             //exit the connection loop     
    } 
    else 
    {
      Serial.print("Could Not Connect - Attempt:");
      Serial.println(trys);

    }

    if(trys==10)
    {
      Serial.println("I don't this this is going to work");
    }
    delay(1000);
 }

  
}


void publishToPubNub()
{
  WiFiClient *client;
  StaticJsonBuffer<800> messageBuffer;                    //create a memory buffer to hold a JSON Object
  JsonObject& pMessage = messageBuffer.createObject();    //create a new JSON object in that buffer
  
 ///the imporant bit where you feed in values
  pMessage["yourVal2"] = myVal2;                      //add a new property and give it a value
  pMessage["yourVal3"] = myVal3;                     //add a new property and give it a value


///                                                       //you can add/remove parameter as you like
  
  //pMessage.prettyPrintTo(Serial);   //uncomment this to see the messages in the serial monitor
  
  
  int mSize = pMessage.measureLength()+1;                     //determine the size of the JSON Message
  char msg[mSize];                                            //create a char array to hold the message 
  pMessage.printTo(msg,mSize);                               //convert the JSON object into simple text (needed for the PN Arduino client)
  
  client = PubNub.publish(pubChannel, msg);                      //publish the message to PubNub

  if (!client)                                                //error check the connection
  {
    Serial.println("client error");
    delay(1000);
    return;
  }
  
  if (PubNub.get_last_http_status_code_class() != PubNub::http_scc_success)  //check that it worked
  {
    Serial.print("Got HTTP status code error from PubNub, class: ");
    Serial.print(PubNub.get_last_http_status_code_class(), DEC);
  }
  
  while (client->available()) 
  {
    Serial.write(client->read());
  }
  client->stop();
  Serial.println("Successful Publish");


  
}


void readFromPubNub()
{
  StaticJsonBuffer<1200> inBuffer;                    //create a memory buffer to hold a JSON Object
  PubSubClient *sClient = PubNub.subscribe(subChannel);

  if (!sClient) {
    Serial.println("message read error");
    delay(1000);
    return;
  }
 

  while (sClient->connected()) 
  {
    while (sClient->connected() && !sClient->available()) ; // wait
    char c = sClient->read();
    JsonObject& sMessage = inBuffer.parse(*sClient);
    
    if(sMessage.success())
    {
      sMessage.prettyPrintTo(Serial); //uncomment to see the JSON message in the serial monitor
      yourVal2 = sMessage["yourVal2"];  //this is the one that connects to the LED
      Serial.print("yourVal2 ");
      Serial.println(yourVal2);
          yourVal3 = sMessage["yourVal3"];  //this is the one that connects to the LED
      Serial.print("yourVal3 ");
      Serial.println(yourVal3);
    }
 
  }
  
  sClient->stop();

}

