#include <Arduino.h>
#include "coap.h"
#include "arduino_secrets.h"
#include <Ethernet.h>
#include <EthernetUdp.h>

char     pinnumber[]     = SECRET_PINNUMBER;
char     gprs_apn[]      = SECRET_GPRS_APN;
uint32_t coap_ip         = SECRET_COAP_IP;
char     coap_endpoint[] = SECRET_COAP_ENDPOINT;
uint32_t coap_port       = SECRET_COAP_PORT;
uint32_t rat             = SECRET_RAT;
uint32_t cops            = SECRET_COPS;
bool     debug           = true;


EthernetUDP     Udp;
Coap      coap(Udp);

IPAddress iotgw_ip(coap_ip);

void setup() {
  Serial.begin(115200);
  while(!Serial){ 
    ; //wait for serial. 
  }
  
  Serial1.begin(115200);
  while(!Serial1){ 
    ; //wait for serial.
  }
  
  delay(2000);
  
  Serial.println("-- Initialized --");
  Serial.println("-- Setting up simcom device --");
  
  modem_command("ATZ", 1000);
  modem_command("AT+CFUN=0", 1000);
  modem_command("AT*MCGDEFCONT=\"IP\",\"telenor.iotgw\"", 1000);
  modem_command("AT+CFUN=1", 1000);
  modem_command("AT+CGREG?", 1000);
  modem_command("AT+CGCONTRDP", 10000);
  Serial.println("-- Setup complete --");
  coap.start();
}


void loop() {
  while (Serial.available() == 0);
  while (Serial.available() > 0) {
    Serial1.write(Serial.read());
  }
  while (Serial1.available() > 0){
    Serial.write(Serial1.read());
  }
}


void modem_command(String command, int commandtimout){
  Serial1.println(command);
  while (Serial1.available() == 0);
  unsigned long lastRead = millis();
  while (millis() - lastRead < commandtimout){   
    while (Serial1.available() > 0){
      Serial.write(Serial1.read());
      lastRead = millis();
    }
  }
}

//function from telenor git coap repo.
uint16_t sendPacket () {
  // Generate random simulated data
  float tmp = 20 + random(0, 9);
  float hum = 60 + random(0, 9);

  /**
   * Create a JSON payload.
   * You would typically compress the payload as much as possible here,
   * and then use the "uplink transform" in the Thing Type to unpack it later
   * once the packet has been received in Telenor MIC.
   */
  char buffer[100];
  uint32_t buf_size = snprintf(buffer, 100, "{\"tmp\":\"%.2f\",\"hum\":\"%.2f\",\"latlng\":\"59.898812,10.627212\"}", tmp, hum);

  // Send a CoAP POST message to Telenor IoT Gateway
  uint16_t msgid = coap.send(
    iotgw_ip,           // IP address
    coap_port,          // Port
    coap_endpoint,      // CoAP endpoint
    COAP_CON,           // CoAP packet type (con, noncon, ack, reset)
    COAP_POST,          // CoAP method
    NULL,               // CoAP token
    0,                  // CoAP token length
    (uint8_t *) buffer, // Message payload
    buf_size            // Message payload length
  );
  Serial.write(msgid);
  return msgid;
}
