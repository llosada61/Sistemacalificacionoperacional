Código software sistema embebido
// LIBRERIAS WIFI
#include "Arduino.h"
#include "Esp.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <EasyBuzzer.h>
WiFiMulti WiFiMulti;                        	// objeto conexion wifi
HTTPClient client;                          	// cliente http
const uint16_t port = 80;                   	// servidor y puerto a donde se va a conectar el dispositivo
const char server[]="direccion api1 aplicacion";
const char server1[]="direccion api2 aplicacion";
DynamicJsonDocument doc(10000);           // objeto json para almacenar información recibida y enviada a servidor  
TaskHandle_t manejadorTareaServidor= NULL;  	// manejador de tarea envío datos a servidor
TaskHandle_t manejadorTareaSensor= NULL; // manejador de tarea sensado
//definiciones
#define LED 2
#define pt 36
#define pres 39
#define buzz 16
#define relay 4
//declaracion de variables
float lectura_pt100=0;
float pt100Media=0;
float pt100=0;
float sensitivity=43.355;
float offset = 3.2244;
const int numReadings=20;
int readingsP[numReadings];
int readingsT[numReadings];
double sumP=0;
double sumT=0;
int CountT=0;
int CountP=0;
float imprimir[256];
float imprimir2[512];
float imprimir3[256];
float imprimir4[512];    	 
float resultadosensor;
void envioDatosServidor(void * parameter){
  for(;;){
	enviarDatosServidor(imprimir,resultadosensor);                               	 
  }
}
void lecturaSensor(void * parameter){

	lecturaSensor();   
  }
}
double funcionTemperatura (double adct)
{
double ecuacion_temperatura;
ecuacion_temperatura= (adct*sensitivity)-offset ;
return ecuacion_temperatura;
}
double FuncionPresion (double adcp)
{
double ecuacion_presion;
ecuacion_presion= ((adcp/vs)+0.04)*250;
return ecuacion_presion;  
}
double lecturaMediaTemperatura()
{
   //Perform average on sensor readings
  double averageT;
  // subtract the last reading:
  sumT = sumT - readingsT[CountT];
  // read the sensor:
  readingsT[CountT] = analogRead(pt);
  // add value to total:
  sumT = sumT + readingsT[CountT];
  // handle count
  CountT = CountT + 1;
  if (CountT >= numReadings)
  {
  	CountT = 0;
 	 
  }
  // calculo promedio
  averageT = sumT / numReadings;
  averageT=averageT/1241.2;
  return averageT;  
}
double lecturaMediaPresion()
{
   //Perform average on sensor readings
  double averageP;
  // subtract the last reading:
  sumP = sumP - readingsP[CountP];
  // read the sensor:
  readingsP[CountP] = analogRead(pres);
  // add value to total:
  sumP = sumP + readingsP[CountP];
  // handle index
  CountP = CountP + 1;
  if (CountP >= numReadings)
  {
  	CountP = 0; 	 
  }
  // calculate the average:
   averageP = sumP / numReadings;
  averageP=averageP/827.272;
  return averageP;
}
void lecturaSensor(){
  int i;
  delay(4);	// Frecuencia de muetreeo = 10 Hz
  if (tomarDatos==true)
	{
	for (i=0;i<511;i++){imprimir2[i]=imprimir2[i+1];}    
	mediaPressure=lecturaMediaPresion();
	pt100Media=lecturaMediaTemperatura();
   pressure = FuncionPresion (mediaPressure);
   pt100= funcionTemperatura (pt100Media);
  // print out the value you read:
	Serial.println(sensorValue);
	imprimir2[511]=sensorValue;
	int j=0;
	for (i=0;i<512;i=i+2)
	{
  	imprimir[j]=imprimir2[i];
  	j++;
	}
    if(pressure>=2.4)
	  {
      / * Crea una secuencia de pitidos con una frecuencia determinada. * /
EasyBuzzer.beep (
  frecuencia,		 // Frecuencia en hercios (HZ). 
  onDuration, 		 // On Duración en milisegundos (ms). 
  offDuration, 		 // Off Duración en milisegundos (ms). 
  pitidos, 		 // El número de pitidos por ciclo. 
  pauseDuration, 	 // Duración de la pausa. 
  ciclos, 		 // El número de ciclo. 
  callback		 // [Opcional] Función para llamar cuando termine. 
);	  

	  }
	else
	  {
       digitalWrite(relay,LOW);
	   digitalWrite(buzzer,LOW);
	  } 


  }
  else{
  	Serial.print("aún no se inicia ejecución de tarea");
  }  
}

void setup(){
	Serial.begin(115200);
	pinMode(led, OUTPUT);
	pinMode(buzzer, OUTPUT);
	EasyBuzzer.setPin(buzzer);
	pinMode(relay, OUTPUT);
	pinMode(15, INPUT);
	// CONFIGURACIÓN CONEXIÓN WIFI
	WiFiMulti.addAP("redinalambrica", "clave"); // poner aquí clave y contraseña wifi
	Serial.print("Esperano conexión WIFI");
	while(WiFiMulti.run() != WL_CONNECTED) {
    	Serial.print(".");
    	delay(500);
	}
	Serial.println("");
	Serial.println("WiFi conectado");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());
// CONFIGURACIÓN DE TAREA ENVÍO DE DATOS A INTERNET
	xTaskCreatePinnedToCore(
    	envioDatosServidor,         	// nombre de la funcion
    	"envio datos servidor",     	// nombre de funcion para depuración
    	10000,                      	// tamaño de stack (bytes)
    	NULL,                       	// parametros pasados
    	1,                          	// prioridad de la tarea
    	&manejadorTareaServidor,    	// manejador de esta tarea
    	1                           	// nucleo en le que se ejecuta esta tarea
	);
	//CONFIGURACIÓN DE TAREA LECTURA DE SENSOR E IMPRESIÓN
	xTaskCreatePinnedToCore(
    	lecturaSensor,              	// nombre de la funcion
    	"lectura sensor",           	// nombre de funcion para depuración
    	10000,                      	// tamaño de stack (bytes)
    	NULL,                       	// parametros pasados
    	7,                          	// prioridad de la tarea
    	&manejadorTareaSensor,      	// manejador de esta tarea
    	0                           	// nucleo en el que se ejecuta esta tarea
	);  
}
void enviarDatosServidor(float SENAL_ENVIAR[256], float temperatura_enviar){
  int httpCode=client.GET();
  if (httpCode > 0) {
	// --------- LEER DATOS DE BASE DE DATOS
	// Get the request response payload
	String payload = client.getString();
	//Serial.println(payload);     	 
	DeserializationError error= deserializeJson(doc, payload); // deserializa y revisa errores
	if (error) {
    	Serial.print(F("deserializeJson() failed with code "));
    	Serial.println(error.c_str());
	}
	client.addHeader("Content-Type", "application/json");
    	for (int j=0;j<256;j++){doc["temperatura"]["senal"][j]=int(SENAL_ENVIAR[j]);}
	doc["temperatura"] = int(temperatura_enviar);
	String message = "";
	serializeJson(doc, message);
	//Serial.print(message);
	httpCode = client.PUT(message);
  }  
}
void loop() {
  digitalWrite(LED,HIGH);
  //REALIZAR LA CONEXION    
  if (!client.begin(server)) {
  	// conexión fallida
  	Serial.println("Connection failed.");
  	while (1){}
  }
  tomarDatos=true; // habilita lectura de sensor
  while (1){}
}
