/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-esp8266-input-data-html-form/

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  https://raw.githubusercontent.com/RuiSantosdotme/Random-Nerd-Tutorials/master/Projects/ESP/ESP_HTML_Form_Input_Data_Save_to_SPIFFS.ino
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  procesor een blok bijmaken
  char* PARAM_wifipaswoord = "wifipaswoord";
  server on  // GET blok
*********/
 

const int handbediening = 4; //gpio d2
const int automatiekbediening = 5;//gpio d1
const int ledhandbediening  = 13; //dpio7 d7
const int leddag = 12 ; //gpio 6 d6  ( dag = 0 // nacht is 1)
const int lednacht = 14 ; //gpio d5   ( dag = 0 // nacht is 1)
const int radio = 10;//
const int nachtlamp = 0 ;//


int var_nachtlampAanOp = 25; // pct
int var_radioAanOp  = 75;

// constants won't change. Used here to set a pin number:
const int ledPin =  2;// gpio2 is de ingebouwde led en pinkt als de pwm iets doet the number of the LED pin
int pwmmosfet = 15; //outputpin gpio15 is pwm https://www.instructables.com/NodeMCU-ESP8266-Details-and-Pinout/

int dutycycle = 0; //pwm%  0-1024 dac
int procentlicht = 0; //0-100 in theorie gesproken
// Variables will change:
int ledState = 0;             // ledState used to set the LED

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;        // will store last time LED was updated
// constants won't change:
long interval = 1000;           // interval at which to blink (milliseconds)
int yourbegrenzer = 100 ; //100 is dat we de dac tot 1024 uitsturen naar de mosfet
#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#include <SPIFFS.h>
#else
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <Hash.h>
#include <FS.h>
#endif
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

// REPLACE WITH YOUR NETWORK CREDENTIALS
char* ssid = "hardcodedssid";
char* password = "hardcodedpass";

const char* ssidap = "Climabreed";
const char* passwordap = "v12345vtm";

const char* PARAM_STRING = "nachtlampAanOp";
const char* PARAM_INT = "tijdsduur";
char* PARAM_FLOAT = "begrenzer";
const char* PARAM_radioAanOp = "radioAanOp";
char* PARAM_tty = "ssidtty";
char* PARAM_wifipaswoord = "wifipaswoord";

// HTML web page to handle 3 input fields (nachtlampAanOp, tijdsduur, begrenzer)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <script>
    function submitMessage() {
      //alert("Saved value to ESP SPIFFS");
    //  setTimeout(function(){ document.location.reload(false); }, 500);   
    }
  </script></head><body>

  <form action="/get" target="hidden-form">
    radioAanOp (current value %radioAanOp%): <input type="text" name="radioAanOp">
    <input type="submit" value="pct Submit" onclick="submitMessage()">
  </form><br>


  
  <form action="/get" target="hidden-form">
    nachtlampAanOp (current value %nachtlampAanOp%): <input type="text" name="nachtlampAanOp">
    <input type="submit" value="pct Submit" onclick="submitMessage()">
  </form><br>
  
  <form action="/get" target="hidden-form">
    tijdsduur  in sec van 0-100pwm (current value %tijdsduur%): <input type="number " name="tijdsduur">
    <input type="submit" value="Seconden Submit" onclick="submitMessage()">
  </form><br>
  
  <form action="/get" target="hidden-form">
    begrenzer (current value %begrenzer%): <input type="number " name="begrenzer">
    <input type="submit" value="pct Submit" onclick="submitMessage()">
  </form><br>


  <form action="/get" target="hidden-form">
    SSIDtty (current value %ssidtty%): <input type="text" name="ssidtty">
    <input type="submit" value="Submit" onclick="submitMessage()">
  </form><br>

    <form action="/get" target="hidden-form">
    wifipaswoord (current value %wifipaswoord%): <input type="text" name="wifipaswoord">
    <input type="submit" value="Submit" onclick="submitMessage()">
  </form><br>

  
  
  <iframe style="display:none" name="hidden-form"></iframe>


 <br> huidige pwmstand  %procentlicht% <br>
 <br> huidige ssid  %ssid% <br>
  <br> huidige ssidpasw  %password% <br>
</body></html>)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

String readFile(fs::FS &fs, const char * path) {
  // Serial.printf("Reading file: %s\r\n", path);
  File file = fs.open(path, "r");
  if (!file || file.isDirectory()) {
    Serial.println("- empty file or failed to open file");
    return String();
  }
  // Serial.println("- read from file:");
  String fileContent;
  while (file.available()) {
    fileContent += String((char)file.read());
  }
  file.close();
  //Serial.println(fileContent);
  return fileContent;
}

void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, "w");
  if (!file) {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
  file.close();
}

// Replaces placeholder with stored values
String processor(const String& var) {
  Serial.println(var);

  ///////////
  if (var == "wifipaswoord") {
    return readFile(SPIFFS, "/wifipaswoord.txt");
  }
  else
    ////////////


    ////////////password
    if (var == "password") {
      return String(password)  ;
    }
    else
      ///////////
      if (var == "ssidtty") {
        return readFile(SPIFFS, "/ssidtty.txt");
      }
      else
        ////////////
        if (var == "procentlicht") {
          return String(procentlicht) + "pct theoretiche lichtopbrengst";
        }
        else
          ////////
          if (var == "radioAanOp") {
            return readFile(SPIFFS, "/radioAanOp.txt");
          }
          else


            if (var == "nachtlampAanOp") {
              return readFile(SPIFFS, "/nachtlampAanOp.txt");
            }
            else if (var == "tijdsduur") {
              return readFile(SPIFFS, "/tijdsduur.txt");
            }
            else if (var == "begrenzer") {
              return readFile(SPIFFS, "/begrenzer.txt");
            }
  return String();
}

void setup() {
  // set the digital pin as output:
  pinMode(ledPin, OUTPUT);
  pinMode(pwmmosfet, OUTPUT);

  pinMode(handbediening, INPUT_PULLUP);
  pinMode(automatiekbediening, INPUT_PULLUP);
  pinMode(ledhandbediening, OUTPUT);
  pinMode(leddag, OUTPUT);
  pinMode(lednacht, OUTPUT);
  pinMode(radio, OUTPUT);
  pinMode(nachtlamp, OUTPUT);

  analogWriteRange(0);
  analogWrite(pwmmosfet, 0); //halvekraccht
  digitalWrite(nachtlamp, LOW);
  digitalWrite(radio, LOW);
  digitalWrite(leddag, LOW);
  digitalWrite(lednacht, LOW);
  digitalWrite(ledhandbediening, LOW);
  delay(1000);


  analogWrite(pwmmosfet, 1024);//halvekraccht
  digitalWrite(nachtlamp, HIGH);
  digitalWrite(radio, HIGH);
  digitalWrite(leddag, HIGH);
  digitalWrite(lednacht, HIGH);
  digitalWrite(ledhandbediening, HIGH);
  delay(1000);


  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println();

  Serial.print ("password hardcoded :");
  Serial.println(password);
#ifdef ESP32
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
#else
  if (!SPIFFS.begin()) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
#endif




  String  yourtty = readFile(SPIFFS, "/ssidtty.txt");
  char ssid_array[yourtty.length() + 1];
  yourtty.toCharArray(ssid_array, yourtty.length() + 1);
  ssid = ssid_array;



  yourtty = readFile(SPIFFS, "/wifipaswoord.txt");
  char paswoord_array[yourtty.length() + 1];
  yourtty.toCharArray(paswoord_array, yourtty.length() + 1);
  password = paswoord_array;

  Serial.print ("ssid van eeprom  die we als client gaan inzetten :");
  Serial.println(ssid);
  Serial.print ("password van eeprom  die we als client gaan inzetten :");
  Serial.println(password);


  ///WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);





  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed! geen gekende ssid gezien");
    //WiFi.disconnect();
    //Serial.print("Configuring access point... paswoord is ; vincentt");
    /* You can remove the password parameter if you want the AP to be open. */
    WiFi.disconnect(true);
    ////accespoint start werkt
    WiFi.softAP(ssidap, passwordap);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    ////accespoint einde werkt


    // return;
  }


  Serial.println();
  Serial.print("Client IP Address: ");
  Serial.println(WiFi.localIP());

  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  // Send a GET request to <ESP_IP>/get?nachtlampAanOp=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest * request) {
    String inputMessage;

    //////////////////
    // GET
    if (request->hasParam(PARAM_wifipaswoord)) {
      inputMessage = request->getParam(PARAM_wifipaswoord)->value();
      writeFile(SPIFFS, "/wifipaswoord.txt", inputMessage.c_str());
    }

    else
      //////////////////////////////


      //////////////////
      // GET
      if (request->hasParam(PARAM_tty)) {
        inputMessage = request->getParam(PARAM_tty)->value();
        writeFile(SPIFFS, "/ssidtty.txt", inputMessage.c_str());
      }

      else
        //////////////////////////////


        //////////////////
        // GET nachtlampAanOp value on <ESP_IP>/get?radioAanOp=<inputMessage>
        if (request->hasParam(PARAM_radioAanOp)) {
          inputMessage = request->getParam(PARAM_radioAanOp)->value();
          writeFile(SPIFFS, "/radioAanOp.txt", inputMessage.c_str());
        }

        else
          //////////////////////////////


          // GET nachtlampAanOp value on <ESP_IP>/get?nachtlampAanOp=<inputMessage>
          if (request->hasParam(PARAM_STRING)) {
            inputMessage = request->getParam(PARAM_STRING)->value();
            writeFile(SPIFFS, "/nachtlampAanOp.txt", inputMessage.c_str());
          }
    // GET tijdsduur value on <ESP_IP>/get?tijdsduur=<inputMessage>
          else if (request->hasParam(PARAM_INT)) {
            inputMessage = request->getParam(PARAM_INT)->value();
            writeFile(SPIFFS, "/tijdsduur.txt", inputMessage.c_str());
          }
    // GET begrenzer value on <ESP_IP>/get?begrenzer=<inputMessage>
          else if (request->hasParam(PARAM_FLOAT)) {
            inputMessage = request->getParam(PARAM_FLOAT)->value();
            writeFile(SPIFFS, "/begrenzer.txt", inputMessage.c_str());
          }
          else {
            inputMessage = "No message sent";
          }
    Serial.println(inputMessage);
    request->send(200, "text/text", inputMessage);
  });
  server.onNotFound(notFound);
  server.begin();


  // To access your stored values on nachtlampAanOp, tijdsduur, begrenzer
  var_nachtlampAanOp = readFile(SPIFFS, "/nachtlampAanOp.txt").toInt();
 Serial.print ("var_nachtlampAanOp:");
  Serial.println(var_nachtlampAanOp);
  
  var_radioAanOp  = readFile(SPIFFS, "/radioAanOp.txt").toInt();

   Serial.print ("var_radioAanOp:");
  Serial.println(var_radioAanOp);

  
  int yourtijdsduur  = readFile(SPIFFS, "/tijdsduur.txt").toInt();
  interval = yourtijdsduur * 10 ;
  yourbegrenzer = readFile(SPIFFS, "/begrenzer.txt").toInt();


  yourtty = readFile(SPIFFS, "/ssidtty.txt");
  ssid_array[yourtty.length() + 1];
  yourtty.toCharArray(ssid_array, yourtty.length() + 1);
  ssid = ssid_array;
  Serial.print ("ssid van eeprom is :");
  Serial.println(ssid);




  yourtty = readFile(SPIFFS, "/wifipaswoord.txt");/////////overwrite van functie
  ssid_array[yourtty.length() + 1];
  yourtty.toCharArray(ssid_array, yourtty.length() + 1);
  password = ssid_array;
  Serial.print ("wifipaswoord van eeprom is :");
  Serial.println(password);
}

void loop() {
  dutycycle = map(procentlicht, 0, 100, 0, 1024 * yourbegrenzer / 100 );

  //pinMode(handbediening, INPUT_PULLUP);
  //pinMode(automatiekbediening, INPUT_PULLUP);
  // pinMode(ledhandbediening, OUTPUT);
  //  pinMode(leddag, OUTPUT);



  if (digitalRead(handbediening ) == LOW)
  {
    digitalWrite(ledhandbediening, HIGH);
    procentlicht = 100 ;
  }
  else
  {
    digitalWrite(ledhandbediening, LOW);
    //   Serial.println(digitalRead(handbediening)) ;//
  }

//automatiek is via de minuterie bepaald om dag en nacht te weten
  if (digitalRead(automatiekbediening ) == LOW)
  {
    //  dag minuterie zegt tis dag
    digitalWrite(leddag, HIGH);
    digitalWrite(lednacht, LOW);
    // digitalWrite(radio, HIGH);
    // digitalWrite(nachtlamp, LOW);

  }
  else
  {
    //  nacht  minuterie zegt tis nacht
    digitalWrite(leddag, LOW);
    digitalWrite(lednacht, HIGH);
    // digitalWrite(radio, LOW);
    //   digitalWrite(nachtlamp, HIGH);

  }




  // var_nachtlampAanOp = readFile(SPIFFS, "/nachtlampAanOp.txt");
  //  var_radioAanOp  = readFile(SPIFFS, "/radioAanOp .txt");



  // the interval at which you want to blink the LED.
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    ///////////////////********************************************************
    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }

    // set the LED with the ledState of the variable:
    digitalWrite(ledPin, ledState);
    analogWrite(pwmmosfet, dutycycle);

    if (procentlicht < 100 &  digitalRead(leddag))
    {
      procentlicht = procentlicht + 1;
    }

    if (procentlicht > 0 &  digitalRead(lednacht))
    {
      procentlicht = procentlicht - 1;
    }

  if (procentlicht > var_radioAanOp  )
  {
    digitalWrite(radio, HIGH);
    Serial.print("radio AAN");
  }

  else
  {
    digitalWrite(radio, LOW);
      Serial.print("radio UIT");
  }


    if (procentlicht < var_nachtlampAanOp  )
  {
    digitalWrite(nachtlamp, HIGH);
      Serial.print("nachtlamp AAN");
  }

  else
  {
    digitalWrite(nachtlamp, LOW);
    Serial.print("nachtlamp UIT");
  }

    Serial.print(procentlicht);
    Serial.print("%");
    Serial.print (dutycycle);//
    Serial.print("  ");
    Serial.print("handbedieningg=");
    Serial.print(!digitalRead(handbediening ) );

    Serial.print("   automatiekbediening (dag=0 nacht=1=");
    Serial.println(digitalRead(automatiekbediening ) );

    ////////////////*******************************************
  }






}
