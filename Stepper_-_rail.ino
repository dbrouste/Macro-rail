//Board DOIT ESP32 DEVKIT v1

#include <WiFi.h>
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

 //Declare pin functions on RedBoard
#define stp 32
#define dir 33
#define MS1 26
#define MS2 27
#define EN  25

//****Stepper driver Microstep Resolution************
// MS1  MS2 Microstep Resolution
// 0    0   Full Step (2 Phase)
// 1    0   Half Step
// 0    1   Quarter Step
// 1    1   Eigth Step

//Declare variables for functions
// Stepper function
String user_input;
int current_menu;
//int y;
//int state;
float StepperMinDegree = 1.8; // pas mimimum du moteur en degree
int StepperAngleDiv = 1; //1 2 4 ou 8
int Angle;
//int Vitesse;
int thread_size = 700; //in um. M3 = 600 M4 = 700 M5 = 800
int Steps = 20; // in um, lenght between focal plane
int attente = 1000; // Attente avant photo (en ms)
int profondeur = 4; //in mm 
int PasAvance = 5; //in mm
int Coefficient = 5;// Pb dans formule ou je ne sais ou, permet de compenser



// Sony function
volatile int counter;
const char* ssid     = "DIRECT-CeE0:ILCE-7RM2";
const char* password = "9E8EqQDV";     // your WPA2 password
const char* host = "192.168.122.1";   // fixed IP of camera
const int httpPort = 8080;
char JSON_1[] = "{\"version\":\"1.0\",\"id\":1,\"method\":\"getVersions\",\"params\":[]}";
char JSON_2[] = "{\"version\":\"1.0\",\"id\":1,\"method\":\"startRecMode\",\"params\":[]}";
char JSON_3[] = "{\"version\":\"1.0\",\"id\":1,\"method\":\"startBulbShooting\",\"params\":[]}";
char JSON_4[] = "{\"version\":\"1.0\",\"id\":1,\"method\":\"stopBulbShooting\",\"params\":[]}";
char JSON_5[] = "{\"version\":\"1.0\",\"id\":1,\"method\":\"actTakePicture\",\"params\":[]}";
//char JSON_6[] = "{\"version\":\"1.0\",\"id\":1,\"method\":\"actHalfPressShutter\",\"params\":[]}";
//char JSON_7[] = "{\"version\":\"1.0\",\"id\":1,\"method\":\"cancelHalfPressShutter\",\"params\":[]}";
//char JSON_8[] = "{\"version\":\"1.0\",\"id\":1,\"method\":\"setSelfTimer\",\"params\":[2]}";
//char JSON_9[] = "{\"version\":\"1.0\",\"id\":1,\"method\":\"setFNumber\",\"params\":[\"5.6\"]}";
//char JSON_10[] = "{\"version\":\"1.0\",\"id\":1,\"method\":\"setShutterSpeed\",\"params\":[\"1/200\"]}";
//char JSON_11[] = "{\"version\":\"1.0\",\"id\":1,\"method\":\"setIsoSpeedRate\",\"params\":[]}";
//char JSON_6[]="{\"method\":\"getEvent\",\"params\":[true],\"id\":1,\"version\":\"1.0\"}";
//char JSON_3[] = "{\"version\":\"1.0\",\"id\":1,\"method\":\"startLiveview\",\"params\":[]}";
//char JSON_4[] = "{\"version\":\"1.0\",\"id\":1,\"method\":\"stopLiveview\",\"params\":[]}";
WiFiClient client;

unsigned long lastmillis;

void setup() {
  pinMode(stp, OUTPUT);
  pinMode(dir, OUTPUT);
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(EN, OUTPUT);
  resetEDPins(); //Set step, direction, microstep and enable pins to default states
  Serial.begin(115200); //Open Serial connection for debugging
  SerialBT.begin("ESP32test"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
  Serial.println("Open remote control app on your camera!");
  delay(10000);
  current_menu = 1;
//  CameraConnection();
  menu_start();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////                   Menu                                  ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 //Main loop menu
void loop() {
  while(SerialBT.available()){
    user_input = SerialBT.readString(); //Read user input and trigger appropriate function
    switch (current_menu) {
      case 1: 
        //Start menu
        switch (user_input.toInt()) {
          case 1:
            Avance(PasAvance);
            break;
          case 2:
            Recule(PasAvance);
            break;
          case 3:
            BalayageComplet();
            break;
          case 4:
            Configuration();
            break;
          default:
            Error();
            break;
        }
        break;
      case 2:
        //Configuration menu
        switch (user_input.toInt()) {
          case 1:
            SerialBT.print("Valeur du Pas actuelle: ");SerialBT.println(Steps);
            SerialBT.println("Entrer la nouvelle valeur de Pas (en µm):");
            current_menu = 3;
            break;
          case 2:
            SerialBT.print("Valeur de profondeur actuelle: ");SerialBT.println(profondeur);
            SerialBT.println("Entrer la nouvelle profondeur (en mm):");
            current_menu = 4;
            break;
          case 3:
            SerialBT.print("Valeur du pas pour une avance rapide actuelle: ");SerialBT.println(PasAvance);
            SerialBT.println("Entrer pas avance rapide (en mm)");
            current_menu = 5;
            break;
          case 9:
            SerialBT.println("Back selected");
            current_menu = 1;
            break;
          default:
            Error();
            break;
        }
        break;
      case 3:
        //Step menu
        Steps = user_input.toInt();
        SerialBT.print("La nouvelle valeur du Pas est : ");SerialBT.print(Steps);SerialBT.println("µm");
        current_menu = 1;
        break;
      case 4:
        //profondeur menu
        profondeur = user_input.toInt();
        SerialBT.print("La nouvelle valeur de Profondeur est : ");SerialBT.print(profondeur);SerialBT.println("mm");
        current_menu = 1;
        break;
      case 5:
        //avance rapide menu
        PasAvance = user_input.toInt();
        SerialBT.print("La nouvelle valeur de avance rapide est : ");SerialBT.print(PasAvance);SerialBT.println("mm");
        current_menu = 1;
        break;
      default:
        // code to be executed if n doesn't match any cases
        Error();
        break;
    }
    switch (current_menu) {
      case 1:
        SerialBT.println();
        menu_start();
        break;
      case 2:
        SerialBT.println();
        menu_configuration();
        break;
      default:
        break;  
    }
  }
  
  resetEDPins();
}

void menu_start()
{
  SerialBT.println("Enter number for control option:");
  SerialBT.println("1. Avance");
  SerialBT.println("2. Recule");
  SerialBT.println("3. Balayage Complet");
  SerialBT.println("4. Configuration");
  SerialBT.println();
}

void menu_configuration() {
  SerialBT.println("Enter number for control option:");
  SerialBT.println("1. Steps");
  SerialBT.println("2. Profondeur (en mm)");
  SerialBT.println("3. Pas avance rapide");
  SerialBT.println("9. Retour menu");
  SerialBT.println();
}

void Error() {
  SerialBT.println("Aucun des menus sélectionné");
  current_menu = 1;
}

void Configuration()
{
  current_menu = 2;
}

void Avance(int distance)
{
  SerialBT.println("On avance");
  digitalWrite(EN, LOW); //Pull enable pin low to allow motor control
  int PasCalc = distance*1000*360*StepperAngleDiv/(thread_size*StepperMinDegree)*Coefficient; //nb de pas à faire
  AngleForward(PasCalc);
  digitalWrite(EN, HIGH); //Pull enable pin high to disable motor control
}

void Recule(int distance)
{
  SerialBT.println("On recule");
  digitalWrite(EN, LOW); //Pull enable pin low to allow motor control
  int PasCalc = distance*1000*360*StepperAngleDiv/(thread_size*StepperMinDegree)*Coefficient; //nb de pas à faire
  AngleBackward(PasCalc);
  digitalWrite(EN, HIGH); //Pull enable pin high to disable motor control
}

void BalayageComplet()
{
  SerialBT.println("Lancer le scan selected");
  digitalWrite(EN, LOW); //Pull enable pin low to allow motor control
  int y;
  int nbphoto = profondeur*1000/Steps;
  int PasCalc = Steps*360*StepperAngleDiv/(thread_size*StepperMinDegree)*Coefficient; //nb de pas à faire
  for(y=0;y<nbphoto;y++)
  {
    SerialBT.println("On declenche!");
    delay(attente);
    DeclenchementPhoto();
    SerialBT.println("On avance!");
    SerialBT.println(y);
    SerialBT.println(nbphoto);
    AngleForward(PasCalc);
  }
  delay(attente);
  DeclenchementPhoto();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////                   Sony                                  ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CameraConnection()
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {   // wait for WiFi connection
    delay(500);
    SerialBT.print(".");
  }
  SerialBT.println("");
  SerialBT.println("WiFi connected");
  SerialBT.println("IP address: ");
  SerialBT.println(WiFi.localIP());
  httpPost(JSON_1);  // initial connect to camera
  httpPost(JSON_2); // startRecMode
  //httpPost(JSON_3);  //startLiveview  - in this mode change camera settings  (skip to speedup operation)
}

void httpPost(char* jString) {
  SerialBT.print("connecting to ");
  SerialBT.println(host);
  if (!client.connect(host, httpPort)) {
    SerialBT.println("connection failed");
    return;
  }
  else {
    SerialBT.print("connected to ");
    SerialBT.print(host);
    SerialBT.print(":");
    SerialBT.println(httpPort);
  }
  // We now create a URI for the request
  String url = "/sony/camera/";
 
  SerialBT.print("Requesting URL: ");
  SerialBT.println(url);
 
  // This will send the request to the server
  client.print(String("POST " + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n"));
  client.println("Content-Type: application/json");
  client.print("Content-Length: ");
  client.println(strlen(jString));
  // End of headers
  client.println();
  // Request body
  client.println(jString);
  SerialBT.println("wait for data");
  lastmillis = millis();
  while (!client.available() && millis() - lastmillis < 8000) {} // wait 8s max for answer
 
  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    SerialBT.print(line);
  }
  SerialBT.println();
  SerialBT.println("----closing connection----");
  SerialBT.println();
  client.stop();
}

void DeclenchementPhoto()
{
  httpPost(JSON_5);  //actTakePicture
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////                   Moteur                                ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Resolution moteur
void ResolutionMoteur(int Resolution)
{
  if (Resolution == 8) {
    digitalWrite(MS1, HIGH); //Pull MS1, and MS2 high to set logic to 1/8th microstep resolution
    digitalWrite(MS2, HIGH);
    SerialBT.println("Resolution 8");
    }
  else if (Resolution == 4) {
    digitalWrite(MS1, LOW);
    digitalWrite(MS2, HIGH);
    }
  else if (Resolution == 2) {
    digitalWrite(MS1, HIGH);
    digitalWrite(MS2, LOW); 
    } 
  else if (Resolution == 1) {
    digitalWrite(MS1, LOW);
    digitalWrite(MS2, LOW);
    SerialBT.println("Resolution 1");
    }
}

//Default angle mode function
void AngleForward(int Pas2)
{
  SerialBT.println("Moving Angle forward");
  digitalWrite(dir, LOW); //Pull direction pin low to move "forward"
  TournerAngle(Pas2);
}

//Default angle mode function
void AngleBackward(int Pas2)
{
  SerialBT.println("Moving Angle forward");
  digitalWrite(dir, HIGH); //Pull direction pin low to move "backward"
  TournerAngle(Pas2);
  SerialBT.println("Enter new option");
  SerialBT.println();
}

//Default angle mode function
void TournerAngle(int Pas)
{
  ResolutionMoteur(StepperAngleDiv);
  int x;
  
  for(x= 0; x<Pas; x++)  //Loop the forward stepping enough times for motion to be visible
  {
    digitalWrite(stp,HIGH); //Trigger one step forward
    delay(1);
    digitalWrite(stp,LOW); //Pull step pin low so it can be triggered again
    delay(1);
  }
}



//Reset Easy Driver pins to default states
void resetEDPins()
{
  digitalWrite(stp, LOW);
  digitalWrite(dir, LOW);
  digitalWrite(MS1, LOW);
  digitalWrite(MS2, LOW);
  digitalWrite(EN, HIGH);
}
