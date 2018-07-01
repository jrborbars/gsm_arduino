#include <TimeLib.h>
#include <Time.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>

SoftwareSerial Serial_GSM(4,5); //RX,TX
SoftwareSerial Serial_GPS(2,3); //RX,TX
TinyGPSPlus gps;
//Ajuste o timezone de acordo com a regiao
const int UTC_offset = -3;



void setup(){
    Serial.begin(115200);
    Serial_GPS.begin(4800);
    Serial.println("2009_GSM_TEST5");
    delay(300);
    Serial.println("GPS initialized");
    SIM900power();
    Serial_GSM.begin(2400);    // init Serial_GSM for GSM
  
  connectGSM("AT","OK","OK",15000);
  delay(30000);
  connectGSM("ATE0","OK","OK",5000);            // O IS NO ECHO. 1 IS ECHOED EVERY WORDAT+SAPBR
 connectGSM("AT+CSQ?","OK","ERROR",1000);        // SIGNAL QUALITY. BEST IF 2(MIN) TO 31MAX). 99 IS UNKNOWN
 connectGSM("AT+CFUN=0","OK","ERROR",1000);   // 0 IS MIN, 1 IS FULL (MAX)
  connectGSM("AT+CPIN?","OK","ERROR",1000);   // SIM card inserted and unlocked?
connectGSM("AT+CREG?","OK","ERROR",1000);   // Is the SIM card registered?
connectGSM("AT+CGATT? ","OK","ERROR",1000); // attached to GPRS?
 connectGSM("AT+CIICR","OK","ERROR",1000);
 connectGSM("AT+CIFSR","OK","ERROR",1000);
// connectGSM("AT+CIPCSGP=1,\"zap.vivo.com.br\",\"vivo\",\"vivo\"","OK","ERROR",5000);
 connectGSM("AT+CIPCSGP=1,\"java.claro.com.br\",\"claro\",\"claro\"","OK","ERROR",5000);
//connectGSM("AT+CSTT=\"zap.vivo.com.br\",\"vivo\",\"vivo\"","OK","ERROR",5000);
connectGSM("AT+CIPSTART=\"TCP\",\"45.77.115.85\",\"52018\"","OK","ERROR",5000);  //server tcp setting
connectGSM("AT+CIPSTATUS","START","ERROR",5000);
    Serial.println("GSM configured");
    Serial.println("##############  System Ready  ##############");
}


void loop(){
 Serial_GSM.listen();

connectGSM("AT+CIPCSGP?","OK","ERROR",5000);
String dados="blablabla";
    int tam=dados.length();
    Serial_GSM.println(dados+" \r\n");
    Serial_GSM.println(tam);
    delay(2000);                                                                                                  //give the module some thinking time
    connectGSM("AT+CIPSEND","OK","ERROR",5000);   
    Serial_GSM.println("POST http://45.77.115.85:52018 HTTP/1.1\r\n");  
    Serial_GSM.println("Host 45.77.115.85:52018\r\n");  
    
    Serial_GSM.println("User-Agent: Arduino 2009\r\n");  
    Serial_GSM.println("Content-Length:"+ String(tam) +" \r\n");
    Serial_GSM.println("Content-Type: application/x-www-form-urlencoded \r\n");
    Serial_GSM.println(dados+" \r\n");
    Serial_GSM.println(" \r\n");
/*
connectGSM("AT+SAPBR=0,1","OK","ERROR",5000);                                              // disconnect GPRS
connectGSM("AT+SAPBR=3,1,\"Contype\",\"GPRS\"","OK","ERROR",5000);           //
connectGSM("AT+SAPBR=3,1,\"APN\",\"zap.vivo.com.br\"","OK","ERROR",5000);
connectGSM("AT+SAPBR=3,1,\"USER\",\"vivo\"","OK","ERROR",5000);
connectGSM("AT+SAPBR=3,1,\"PWD\",\"vivo\"","OK","ERROR",5000);
connectGSM("AT+SAPBR=1,1","OK","ERROR",5000);                                              // connect GPRS
connectGSM("AT+SAPBR=2,1","OK","ERROR",5000);
*/


/*
  connectGSM("AT+HTTPTERM","OK","ERROR",5000); // terminate HTTP
 connectGSM("AT+HTTPINIT","OK","ERROR",5000);
 connectGSM("AT+HTTPPARA=\"CID\",1","OK","ERROR",5000);
 connectGSM("AT+HTTPPARA=\"URL\",\"http://45.77.115.85:52018\"","OK","ERROR",5000);
 connectGSM("AT+HTTPDATA=10,10000","OK","ERROR",5000);   // 10 is the number of bytes transmitted -> response is "DOWNLOAD"
 connectGSM("0123456789","OK","ERROR",5000);
  connectGSM("AT+HTTPREAD","OK","ERROR",5000); // response is "ACTION"
 connectGSM("AT+HTTPACTION=1","OK","ERROR",5000); // response is "ACTION"
 connectGSM("AT+HTTPTERM","OK","ERROR",5000); // terminate HTTP
 */
    delay(1000);
}


void GPS_Timezone_Adjust(){
  Serial_GPS.listen();
  while (Serial_GPS.available()) {
    if (gps.encode(Serial_GPS.read())) {
      int Year = gps.date.year();
      byte Month = gps.date.month();
      byte Day = gps.date.day();
      byte Hour = gps.time.hour();
      byte Minute = gps.time.minute();
      byte Second = gps.time.second();
      //Ajusta data e hora a partir dos dados do GPS
      setTime(Hour, Minute, Second, Day, Month, Year);
      //Aplica offset para ajustar data e hora
      //de acordo com a timezone
      adjustTime(UTC_offset * SECS_PER_HOUR);
    }
  }
}


void SIM900power(){ // software equivalent of pressing the GSM shield "power" button
  pinMode(9, OUTPUT);
  digitalWrite(9,LOW);
  delay(1000);
  digitalWrite(9,HIGH);
  delay(500);
  digitalWrite(9,LOW);
}



int8_t connectGSM(char* ATcommand, char* expected_answer1, 
        char* expected_answer2, unsigned int timeout){

    uint8_t x=0,  answer=0;
    char response[100];
    unsigned long previous;
    String ans;

    memset(response, '\0', 100);    // Initialize the string

    delay(100);

    while( Serial_GSM.available() > 0) Serial_GSM.read();    // Clean the input buffer

    Serial.println(ATcommand);    // Send the AT command 
    Serial_GSM.println(ATcommand);    // Send the AT command 

    x = 0;
    previous = millis();

    // this loop waits for the answer
    do{ // if there are data in the UART input buffer, reads it and checks for the asnwer
        if(Serial_GSM.available() != 0){    
            response[x] = Serial_GSM.read();
            x++;
            // check if the desired answer 1  is in the response of the module
            if (strstr(response, expected_answer1) != NULL) {
                answer = 1; //1
            } /*else if (strstr(response, expected_answer2) != NULL)  { // check if the desired answer 2 is in the response of the module
                answer = 2;//2
            }*/
        }
    } while((answer == 0) && ((millis() - previous) < timeout));        // Waits for the asnwer with time out
        ans= String(response);
      
    Serial.println(ans);
    return answer;
}
