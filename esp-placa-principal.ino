#include <ESP8266WiFi.h>
#include <FS.h>

#define     MAXSC     6           // MAXIMUM NUMBER OF CLIENTS
#define     INTERVALO_SERIAL 30000 // 30 segundos

const int  porta = 9001;
const char *ssid = "SystemSegPlaca1";
const char *password = "systemseg";

String      Message;              // VARIABLE RECEIVE DATA FROM OTHER CLIENTS
String buff;
int idxPe, idxTp, idxUm, idxPr, idxLu, idxAn, idxBl;
int contador_anem;
float temperatura;
float umidade;
float pressao;
int amonia;
int corrente;
int lux;
float peso;
int balanca;
unsigned long tempoSerial;

IPAddress ip = IPAddress(192, 168, 10, 1);
IPAddress subnet = IPAddress(255, 255, 255, 0);
  
WiFiServer  ESPServer(porta);      // THE SERVER AND THE PORT NUMBER
WiFiClient  ESPClient[MAXSC];     // THE SERVER CLIENTS (Devices)


void SetWifi()
{
  // Stop Any Previous WIFI
  WiFi.disconnect();

  // Setting The Wifi Mode
  WiFi.mode(WIFI_AP_STA);
  //Serial.println("WIFI Mode : AccessPoint Station");
  
  // Starting The Access Point
  //WiFi.softAPConfig(ip, ip, subnet);
  WiFi.softAP(ssid, password);
  
  // Wait For Few Seconds
  delay(500);

  // Getting Server IP
  IPAddress IP = WiFi.softAPIP();
  
  // Printing The Server IP Address
  //Serial.print("AccessPoint IP : ");
  //Serial.println(IP);

  // Printing MAC Address
  //Serial.print("AccessPoint MC : ");
  //Serial.println(String(WiFi.softAPmacAddress()));

  // Starting Server
  ESPServer.begin();
  ESPServer.setNoDelay(true);
  //Serial.println("Server Started");

  tempoSerial = millis() + INTERVALO_SERIAL;
}

void AvailableClients()
{   
  if (ESPServer.hasClient())
  {
    //Serial.println("Chegou cliente!");
    
    for(uint8_t i = 0; i < MAXSC; i++)
    {
      //Serial.print("Verificando cliente: ");
      //Serial.println(i);
      
      //find free/disconnected spot
      if (!ESPClient[i] || !ESPClient[i].connected())
      {
        //Serial.println("Achei um cliente livre!");
        
        // Checks If Previously The Client Is Taken
        if(ESPClient[i])
        {
          //Serial.println("Cliente já utilizado!");
          ESPClient[i].stop();
        }

        // Clients Connected To The Server
        if(ESPClient[i] = ESPServer.available()) {

          //Serial.println("Cliente disponivel conectado!");
        }

        // Continue Scanning
        continue;
      }
    }

    //Serial.println("Terminando conexao de cliente!");
    
    //no free/disconnected spot so reject
    WiFiClient ESPClient = ESPServer.available();
    ESPClient.stop();
  }
}

void AvailableMessage()
{
  //check clients for data
  for(uint8_t i = 0; i < MAXSC; i++)
  {
    //Serial.print("Verificando se chegou mensagem no cliente: ");
    //Serial.println(i);
      
    if (ESPClient[i] && ESPClient[i].connected() && ESPClient[i].available())
    {
      //Serial.println("Achei um cliente conectado e livre!");

      bool achei = false;
        while(ESPClient[i].available())
        {
          //delay(1000);
          //Message += ESPClient[i].readStringUntil('\r');
          //ESPClient[i].flush();         
          int inChar = ESPClient[i].read();
          if (inChar != '\n') {
            Message += (char)inChar;
          }
          else {
            achei = true;
          }
        }

        if (achei) {
          //Serial.println("Client No " + String(i+1) + " - " + Message); 
          handleRoot();
          Message = "";
        } 
        else {
          //Serial.println("Ainda não achei...");
        }
        
    }
  }
}


String formatarValor(int valor)
{
  String entrada = String(valor);
  String saida; 
  if (valor < 10) saida = "0";
  if (valor < 100) saida += "0";
  saida += entrada;
  return saida;
}

void handleRoot() {

  //Serial.println("Tratando handleRoot");
    
  idxPe = 0;
  idxAn = Message.indexOf(",an=");
  idxTp = Message.indexOf(",tp=");
  idxUm = Message.indexOf(",um=");
  idxLu = Message.indexOf(",lu=");
  idxPr = Message.indexOf(",pr=");
  idxBl = Message.indexOf(",bl=");

  buff = Message.substring(idxPe + 3, idxAn);
  peso = buff.toFloat();
  buff = Message.substring(idxAn + 4, idxTp);
  contador_anem = buff.toInt();
  buff = Message.substring(idxTp + 4, idxUm);
  temperatura = buff.toFloat();
  buff = Message.substring(idxUm + 4, idxLu);
  umidade = buff.toFloat();
  buff = Message.substring(idxLu + 4, idxPr);
  lux = buff.toFloat();
  buff = Message.substring(idxPr + 4);
  pressao = buff.toFloat();
  buff = Message.substring(idxBl + 4);
  balanca = buff.toInt();

  // Para uso futuro
  amonia = 0;
  corrente = 0;

  String msg = "{esp01:an=";
  msg += String(formatarValor(contador_anem));
  msg += ",esp01:tp=";
  msg += String(temperatura, 2);
  msg += ",esp01:um=";
  msg += String(umidade, 2);
  msg += ",esp01:am=";
  msg += String(formatarValor(amonia));
  msg += ",esp01:co=";
  msg += String(formatarValor(corrente));
  msg += ",esp01:lu=";
  msg += String(formatarValor(lux));
  msg += ",esp01:pe=";
  msg += String(peso, 3);
  msg += ",esp01:pr=";
  msg += String(pressao, 2);
  msg += ",esp01:bl=";
  msg += String(formatarValor(balanca));
  msg += ",}";

  saveMsgPos(msg.c_str());
}

void enviaSerial() {
  if (qtdeMsgPos()) {
    String msg = readMsgPos();
    Serial.println(msg);
  }
}

void setup() 
{
  Serial.begin(9600);
  delay(500);

  // Setting Up A Wifi Access Point
  SetWifi();

  // Iniciando sistema de arquivos
  SPIFFS.begin();

  // Formatando o sistema de arquivos no primeiro uso
  SPIFFS.format();

  //Serial.println("ESP configurado!");
}

void loop() {
  AvailableClients();     // Checking For Available Clients
  AvailableMessage();     // Checking For Available Client Messages

  // Tratando envio da serial pelo tempo
  if (tempoSerial < millis()) {
    enviaSerial();
    tempoSerial = millis() + INTERVALO_SERIAL;
  }
}
