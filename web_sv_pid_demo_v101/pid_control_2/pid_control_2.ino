#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(D1,D2); //RX, TX
String s,s1;
long data=0, datas=0;
const long interval = 1000;
unsigned long previousMillis = 0;    // will store last time ENCODER was updated

// Set to true to define Relay as Normally Open (NO)
#define RELAY_NO    true

// Set number of relays
#define NUM_RELAYS  2

// Assign each GPIO to a relay
int relayGPIOs[NUM_RELAYS] = {2,14};
const char* PARAM_INPUT_1 = "relay";  
const char* PARAM_INPUT_2 = "state";

// Replace with your network credentials
const char* ssid = "your ssid";
const char* password = "your password";

AsyncWebServer server(80);
const char index_html[] PROGMEM = R"rawliteral(


<!DOCTYPE HTML><html>
    <head>
      <meta charset="UTF-8">
      <script src="https://code.highcharts.com/highcharts.js"></script>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
      <style>
        html {
         font-family: Arial;
         display: inline-block;
         margin: 0px auto;
         text-align: center;
        }
        a{
            font-size: 15px;
            text-align: center;
            font-style: italic;
            color: dodgerblue;
        }
        h2 { font-size: 2.3rem; }
        p { font-size: 2rem; }
        .units { font-size: 1.2rem; }
        .speed-labels{
          font-size: 1.5rem;
          vertical-align:middle;
          padding-bottom: 15px;
        }
    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #e08711; border-radius: 34px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 68px}
    input:checked+.slider {background-color: #059e8a}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
    input[type=text]{
           width: 60%;
           border: 2px rgb(187, 187, 187)3);
           border-radius: 4px;
           margin: 8px;
           outline: none;
           padding: 9px;
           box-sizing: border-box;
           transition: 0.3s;
           
           ;
         }
         input[type=text]:focus{
            border-color: dodgerblue;
            box-shadow: 0 0 8px 0 darkcyan;
         }
         .btn2{
              
              font-family: "Roboto", sans-serif;
              font-weight: bold;
              background: #059e8a;
              width: 100px;
              padding: 10px;
              text-align: center;
              text-decoration: none;
              text-transform: uppercase;
              color: #ffffff;
              border-radius: 5px;
              cursor: pointer;
              box-shadow: 0 0 10px rgba(133, 127, 127, 0.658);
         }
         .btn2:hover, .btn2:focus, .btn2:active{
           box-shadow: 0 0 20px rgba(14, 215, 241, 0.5);
           -webkit-transform: scale(1.1);
           transform: scale(1.1);
         }
      </style>
    </head>
    <body>  
        <h2><i class="fas fa-tachometer-alt" style="color:#059e8a;"></i> 
        <span class="speed_labels" style="color: #e08711;" style="size: 3rem;" >ESP8288+ ARMF4 WEB SERVER </span>
        </h2>
      <p style="color:#059e8a;">
        
        <span class="speed-labels">T???c ????? ?????ng c??: </span> 
        <span id="speed">%SPEED%</span>
        <sup class="units">v??ng/ph??t</sup>
      </p>
      <script>
        setInterval(function ( ) {
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
              document.getElementById("speed").innerHTML = this.responseText;
            }
          };
          xhttp.open("GET", "/speed", true);
          xhttp.send();
        }, 1000 ) ;
        </script>
      <h2>
        <i class="fas fa-keyboard" style="color: #059e8a;"></i>
        <span class="speed_labels" style="color: #e08711;" style="size: 2.5rem;" >H??? th???ng ??i???u khi???n</span>
        </h2>
        <input name="setpoint" type="text" id="sp" placeholder="Set point...">
        <button id="Set" onclick="SET(sp)" class="btn2">Set</button><br>
        <script>
            function SET(data){
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "setpoint/?"+document.getElementById("sp").value, true); 
    xhr.send();
}

        </script>
        %BUTTONPLACEHOLDER% 
        <script>function toggleCheckbox(element) {
            var xhr = new XMLHttpRequest();
            if(element.checked){ xhr.open("GET", "/update?relay="+element.id+"&state=1", true); }
            else { xhr.open("GET", "/update?relay="+element.id+"&state=0", true); }
            xhr.send();
          }</script> 
      <h2>
          <i class="fas fa-chart-line " style="color: #059e8a;"></i>
          <span class="speed_labels" style="color: #e08711;" style="size: 2.5rem;" >
            Bi???u ????? th???ng k?? th???i gian th???c</span>
        </h2>
        <div id="chart-speed" class="container"></div><br><br>
        <script>
            var chartT = new Highcharts.Chart({
              chart:{ renderTo : 'chart-speed' },
              title: { text: '' },
              series: [{
                showInLegend: false,
                data: []
              }],
              plotOptions: {
                line: { animation: false,
                  dataLabels: { enabled: true }
                },
                series: { color: '#059e8a' }
              },
              xAxis: { type: 'datetime',
                dateTimeLabelFormats: { minute: '%H:%M:%S' },
                title: {text:null},
              },
              yAxis: {
                title: { text: 'T???c ?????(rpm)' }
              },
              credits: { enabled: false }
            });
            setInterval(function ( ) {
              var xhttp = new XMLHttpRequest();
              xhttp.onreadystatechange = function() {
                if (this.readyState == 4 && this.status == 
                200) {
                  var x = 'Ng??y '+(new Date()).getDate()+'/'+((new Date()).getMonth()+1)+'/'+(new Date()).getFullYear()
                          +': '+(new Date()).getHours()+' gi??? '+(new Date()).getMinutes()+' ph??t '+(new Date()).getSeconds()+' gi??y ';
                  var y = parseFloat(this.responseText);
                  //console.log(this.responseText);
                  if(chartT.series[0].data.length > 40) {
                    chartT.series[0].addPoint([x, y], true, true, true);
                  } else {
                    chartT.series[0].addPoint([x, y], true, false, true);
                  }
                }
              };
              xhttp.open("GET", "/speed", true);
              xhttp.send();
            }, 1000 ) ;
            </script>
        <a></a><i class="fab fa-facebook" style="color: dodgerblue;"></i></a>    
        <a href="https://www.facebook.com/profile.php?id=100013465690784">https://www.facebook.com/profile.php?id=100013465690784</a>
    </body>
    </html>
)rawliteral";

String relayState(int numRelay){
  if(RELAY_NO){
    if(digitalRead(relayGPIOs[numRelay-1])){
      return "";
    }
    else {
      return "checked";
    }
  }
  else {
    if(digitalRead(relayGPIOs[numRelay-1])){
      return "checked";
    }
    else {
      return "";
    }
  }
  return "";
}

// Replaces placeholder with ENCODER values
String processor(const String& var){
  //Serial.println(var);
  if(var == "SPEED"){
    return String(s);
  }
  if(var == "BUTTONPLACEHOLDER"){
    String buttons ="";
    for(int i=1; i<=NUM_RELAYS; i++){
      String relayStateValue = relayState(i);
      buttons+= "<h4>Relay #" + String(i) + " - GPIO " + relayGPIOs[i-1] + "</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"" + String(i) + "\" "+ relayStateValue +"><span class=\"slider\"></span></label>";
    }
    return buttons;
  }
   return String();
   
}
void setup() {
  
 // Set all relays to off when the program starts - if set to Normally Open (NO), the relay is off when you set the relay to HIGH
  for(int i=1; i<=NUM_RELAYS; i++){
    pinMode(relayGPIOs[i-1], OUTPUT);
    if(RELAY_NO){
      digitalWrite(relayGPIOs[i-1], HIGH);
    }
    else{
      digitalWrite(relayGPIOs[i-1], LOW);
    }
  }
   // Serial port for debugging purposes
  Serial.begin(9600);
  mySerial.begin(9600);
 
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(".");
  }

  // Print ESP8266 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/speed", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(s).c_str());
  });


// Send a GET request to <ESP_IP>/update?relay=<inputMessage>&state=<inputMessage2>
  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    String inputMessage2;
    String inputParam2;
    // GET input1 value on <ESP_IP>/update?relay=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1) & request->hasParam(PARAM_INPUT_2)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
      inputParam2 = PARAM_INPUT_2;
      if(RELAY_NO){
       // Serial.print("NO ");
        digitalWrite(relayGPIOs[inputMessage.toInt()-1], inputMessage2.toInt());
      }
      else{
       // Serial.print("NC ");
        digitalWrite(relayGPIOs[inputMessage.toInt()-1], inputMessage2.toInt());
      }
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
   // Serial.println(inputMessage + inputMessage2);
    request->send(200, "text/plain", "OK");
  });
  server.on("/setpoint", HTTP_GET, [](AsyncWebServerRequest *request){
    int paramsNr = request->params();
    for(int i=0;i<paramsNr;i++){
        AsyncWebParameter* p = request->getParam(i);  
        Serial.println(p->name()); 
    }
     request->send_P(200, "text/html", index_html);
     
  });
  
  // Start server
  server.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  if (mySerial.available())
  {char c = mySerial.read();
  s1 +=c;
  if (c =='\n')
    {
      s=("%d",s1);
      Serial.println(s);
      s1="";
      }
    }
    }
