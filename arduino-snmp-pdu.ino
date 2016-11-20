/**
* Agentuino SNMP Agent Library Prototyping...
*
* Copyright 2010 Eric C. Gionet <lavco_eg@hotmail.com>
*
*/
#include <Streaming.h>         // Include the Streaming library
#include <EtherCard.h>          // Include the Ethernet library
//#include <SPI.h>
#include <MemoryFree.h>
#include <Agentuino.h> 
//#include <Flash.h>

#include <EEPROM.h>

#define DEBUG
//
static byte mymac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

#define STATIC 1  // set to 1 to disable DHCP (adjust myip/gwip values below)

#if STATIC
static byte myip[] = { 192, 168, 1, 200 };
static byte gwip[] = { 192, 168, 1, 1 };
#endif

//static byte subnet[] = { 255, 255, 255, 0 };
//
// tkmib - linux mib browser
//
// RFC1213-MIB OIDs
// .iso (.1)
// .iso.org (.1.3)
// .iso.org.dod (.1.3.6)
// .iso.org.dod.internet (.1.3.6.1)
// .iso.org.dod.internet.mgmt (.1.3.6.1.2)
// .iso.org.dod.internet.mgmt.mib-2 (.1.3.6.1.2.1)
static const char mib2[] PROGMEM          = "1.3.6.1.2.1";
// .iso.org.dod.internet.mgmt.mib-2.system (.1.3.6.1.2.1.1)
// .iso.org.dod.internet.mgmt.mib-2.system.sysDescr (.1.3.6.1.2.1.1.1)
static const char sysDescr[] PROGMEM      = "1.3.6.1.2.1.1.1";  // read-only  (DisplayString)
// .iso.org.dod.internet.mgmt.mib-2.system.sysObjectID (.1.3.6.1.2.1.1.2)
static const char sysObjectID[] PROGMEM   = "1.3.6.1.2.1.1.2";  // read-only  (ObjectIdentifier)
// .iso.org.dod.internet.mgmt.mib-2.system.sysUpTime (.1.3.6.1.2.1.1.3)
static const char sysUpTime[] PROGMEM     = "1.3.6.1.2.1.1.3";  // read-only  (TimeTicks)
// .iso.org.dod.internet.mgmt.mib-2.system.sysContact (.1.3.6.1.2.1.1.4)
static const char sysContact[] PROGMEM    = "1.3.6.1.2.1.1.4";  // read-write (DisplayString)
// .iso.org.dod.internet.mgmt.mib-2.system.sysName (.1.3.6.1.2.1.1.5)
static const char sysName[] PROGMEM       = "1.3.6.1.2.1.1.5";  // read-write (DisplayString)
// .iso.org.dod.internet.mgmt.mib-2.system.sysLocation (.1.3.6.1.2.1.1.6)
static const char sysLocation[] PROGMEM   = "1.3.6.1.2.1.1.6";  // read-write (DisplayString)
// .iso.org.dod.internet.mgmt.mib-2.system.sysServices (.1.3.6.1.2.1.1.7)
static const char sysServices[] PROGMEM   = "1.3.6.1.2.1.1.7";  // read-only  (Integer)
//
static const char enterprises[] PROGMEM = "1.3.6.1.4.1";
static const char outlets[] PROGMEM   = "1.3.6.1.4.1.318.1.1.4.4.2.1.3.";
static const char outlets1[] PROGMEM   = "1.3.6.1.4.1.318.1.1.4.4.2.1.3.1";
static const char outlets2[] PROGMEM   = "1.3.6.1.4.1.318.1.1.4.4.2.1.3.2";
static const char outlets3[] PROGMEM   = "1.3.6.1.4.1.318.1.1.4.4.2.1.3.3";
static const char outlets4[] PROGMEM   = "1.3.6.1.4.1.318.1.1.4.4.2.1.3.4";
static const char outlets5[] PROGMEM   = "1.3.6.1.4.1.318.1.1.4.4.2.1.3.5";
static const char outlets6[] PROGMEM   = "1.3.6.1.4.1.318.1.1.4.4.2.1.3.6";
static const char outlets7[] PROGMEM   = "1.3.6.1.4.1.318.1.1.4.4.2.1.3.7";
static const char outlets8[] PROGMEM   = "1.3.6.1.4.1.318.1.1.4.4.2.1.3.8";

// Arduino defined OIDs
// .iso.org.dod.internet.private (.1.3.6.1.4)
// .iso.org.dod.internet.private.enterprises (.1.3.6.1.4.1)
// .iso.org.dod.internet.private.enterprises.arduino (.1.3.6.1.4.1.36582)
//
//
// RFC1213 local values
static char locDescr[]              = "Agentuino, a light-weight SNMP Agent.";  // read-only (static)
static char locObjectID[]           = "1.3.6.1.3.2009.0";                       // read-only (static)
static uint32_t locUpTime           = 0;                                        // read-only (static)
static char locContact[20]          = "Eric Gionet";                            // should be stored/read from EEPROM - read/write (not done for simplicity)
static char locName[20]             = "Agentuino";                              // should be stored/read from EEPROM - read/write (not done for simplicity)
static char locLocation[20]         = "Nova Scotia, CA";                        // should be stored/read from EEPROM - read/write (not done for simplicity)
static int32_t locServices          = 7;                                        // read-only (static)

uint32_t prevMillis = millis();
char oid[SNMP_MAX_OID_LEN];
SNMP_API_STAT_CODES api_status;
SNMP_ERR_CODES status;
unsigned char outlet_values;

byte Ethernet::buffer[500]; // tcp/ip send and receive buffer

void udpSerialPrint(uint16_t dest_port, uint8_t src_ip[IP_LEN], uint16_t src_port, const char *data, uint16_t len){
  /*Serial.println("packet!");
  Serial.print("dest_port: ");
  Serial.println(dest_port);
  Serial.print("src_port: ");
  Serial.println(src_port);
  
  
  Serial.print("src_ip: ");
  ether.printIp(src_ip);
  Serial.println();
  Serial.println("data: ");
  Serial.println(data);*/
  
  Agentuino.parsePacket(dest_port, src_ip, src_port, data, len);
}

void pduReceived()
{
  SNMP_PDU pdu;
  int8_t outlet_command;
  //
  #ifdef DEBUG
    Serial << F("UDP Packet Received Start..") << F(" RAM:") << freeMemory() << endl;
  #endif
  //
  api_status = Agentuino.requestPdu(&pdu);
  //Serial.println(api_status);
  
  //
  //Serial.println(pdu.type, HEX);
  if ( (pdu.type == SNMP_PDU_GET || pdu.type == SNMP_PDU_GET_NEXT || pdu.type == SNMP_PDU_SET)
    && pdu.error == SNMP_ERR_NO_ERROR && api_status == SNMP_API_STAT_SUCCESS ) {

    pdu.OID.toString(oid);
    Serial.println(oid);
    
    if (pdu.type == SNMP_PDU_GET_NEXT) {
      if (strcmp_P(oid, mib2) == 0) {
        strcpy_P(oid, sysDescr);
        pdu.OID.fromString(oid);
      } else if (strcmp_P(oid, sysDescr) == 0) {
        strcpy_P(oid, sysObjectID);
        pdu.OID.fromString(oid);
      } else if (strcmp_P(oid, sysObjectID) == 0) {
        strcpy_P(oid, sysUpTime);
        pdu.OID.fromString(oid);
      } else if (strcmp_P(oid, sysUpTime) == 0) {
        strcpy_P(oid, sysContact);
        pdu.OID.fromString(oid);
      } else if (strcmp_P(oid, sysContact) == 0) {
        strcpy_P(oid, sysName);
        pdu.OID.fromString(oid);
      } else if (strcmp_P(oid, sysName) == 0) {
        strcpy_P(oid, sysLocation);
        pdu.OID.fromString(oid);
      } else if (strcmp_P(oid, sysLocation) == 0) {
        strcpy_P(oid, sysServices);
        pdu.OID.fromString(oid);
      } else if (strcmp_P(oid, enterprises) == 0) {
        strcpy_P(oid, outlets1);
        pdu.OID.fromString(oid);
      } else if (strcmp_P(oid, outlets1) == 0) {
        strcpy_P(oid, outlets2);
        pdu.OID.fromString(oid);
      } else if (strcmp_P(oid, outlets2) == 0) {
        strcpy_P(oid, outlets3);
        pdu.OID.fromString(oid);
      } else if (strcmp_P(oid, outlets3) == 0) {
        strcpy_P(oid, outlets4);
        pdu.OID.fromString(oid);
      } else if (strcmp_P(oid, outlets4) == 0) {
        strcpy_P(oid, outlets5);
        pdu.OID.fromString(oid);
      } else if (strcmp_P(oid, outlets5) == 0) {
        strcpy_P(oid, outlets6);
        pdu.OID.fromString(oid);
      } else if (strcmp_P(oid, outlets6) == 0) {
        strcpy_P(oid, outlets7);
        pdu.OID.fromString(oid);
      } else if (strcmp_P(oid, outlets7) == 0) {
        strcpy_P(oid, outlets8);
        pdu.OID.fromString(oid);
      } else {
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = SNMP_ERR_NO_SUCH_NAME;
        Agentuino.responsePdu(&pdu);
        Agentuino.freePdu(&pdu);
        return;
      }

      pdu.type = SNMP_PDU_GET;
    }
    
    pdu.OID.toString(oid);
    Serial.println(oid);
    //
    //Serial << "OID: " << oid << endl;
    //
    if ( strcmp_P(oid, sysDescr ) == 0 ) {
      // handle sysDescr (set/get) requests
      if ( pdu.type == SNMP_PDU_SET ) {
        // response packet from set-request - object is read-only
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = SNMP_ERR_READ_ONLY;
      } else {
        // response packet from get-request - locDescr
        status = pdu.VALUE.encode(SNMP_SYNTAX_OCTETS, locDescr);
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = status;
      }
      //
      #ifdef DEBUG
        Serial << F("sysDescr...") << locDescr << F(" ") << pdu.VALUE.size << endl;
      #endif
    } else if ( strcmp_P(oid, sysObjectID ) == 0 ) {
      // handle sysObjectID (set/get) requests
      if ( pdu.type == SNMP_PDU_SET ) {
        // response packet from set-request - object is read-only
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = SNMP_ERR_READ_ONLY;
      } else {
        // response packet from get-request - locDescr
        status = pdu.VALUE.encode(SNMP_SYNTAX_OCTETS, locObjectID);
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = status;
      }
      //
      #ifdef DEBUG
        Serial << F("sysObjectID...") << locObjectID << F(" ") << pdu.VALUE.size << endl;
      #endif
    } else if ( strcmp_P(oid, sysUpTime ) == 0 ) {
      // handle sysName (set/get) requests
      if ( pdu.type == SNMP_PDU_SET ) {
        // response packet from set-request - object is read-only
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = SNMP_ERR_READ_ONLY;
      } else {
        // response packet from get-request - locUpTime
        status = pdu.VALUE.encode(SNMP_SYNTAX_TIME_TICKS, locUpTime);
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = status;
      }
      //
      #ifdef DEBUG
        Serial << F("sysUpTime...") << locUpTime << F(" ") << pdu.VALUE.size << endl;
      #endif
    } else if ( strcmp_P(oid, sysName ) == 0 ) {
      // handle sysName (set/get) requests
      if ( pdu.type == SNMP_PDU_SET ) {
        // response packet from set-request - object is read/write
        status = pdu.VALUE.decode(locName, strlen(locName)); 
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = status;
      } else {
        // response packet from get-request - locName
        status = pdu.VALUE.encode(SNMP_SYNTAX_OCTETS, locName);
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = status;
      }
      //
      #ifdef DEBUG
        Serial << F("sysName...") << locName << F(" ") << pdu.VALUE.size << endl;
      #endif
    } else if ( strcmp_P(oid, sysContact ) == 0 ) {
      // handle sysContact (set/get) requests
      if ( pdu.type == SNMP_PDU_SET ) {
        // response packet from set-request - object is read/write
        status = pdu.VALUE.decode(locContact, strlen(locContact)); 
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = status;
      } else {
        // response packet from get-request - locContact
        status = pdu.VALUE.encode(SNMP_SYNTAX_OCTETS, locContact);
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = status;
      }
      //
      #ifdef DEBUG
        Serial << F("sysContact...") << locContact << F(" ") << pdu.VALUE.size << endl;
      #endif
    } else if ( strcmp_P(oid, sysLocation ) == 0 ) {
      // handle sysLocation (set/get) requests
      if ( pdu.type == SNMP_PDU_SET ) {
        // response packet from set-request - object is read/write
        status = pdu.VALUE.decode(locLocation, strlen(locLocation)); 
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = status;
      } else {
        // response packet from get-request - locLocation
        status = pdu.VALUE.encode(SNMP_SYNTAX_OCTETS, locLocation);
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = status;
      }
      //
      #ifdef DEBUG
        Serial << F("sysLocation...") << locLocation << F(" ") << pdu.VALUE.size << endl;
      #endif
    } else if ( strcmp_P(oid, sysServices) == 0 ) {
      // handle sysServices (set/get) requests
      if ( pdu.type == SNMP_PDU_SET ) {
        // response packet from set-request - object is read-only
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = SNMP_ERR_READ_ONLY;
      } else {
        // response packet from get-request - locServices
        status = pdu.VALUE.encode(SNMP_SYNTAX_INT, locServices);
        pdu.type = SNMP_PDU_RESPONSE;
        pdu.error = status;
      }
      //
      #ifdef DEBUG
        Serial << F("locServices...") << locServices << F(" ") << pdu.VALUE.size << endl;
      #endif
    } else if ( strncmp_P(oid, outlets, strlen_P(outlets)) == 0 ) {
      if (pdu.OID.size != 15) {
        pdu.error = SNMP_ERR_NO_SUCH_NAME;
      }
      if (pdu.OID.data[14] < 1 || pdu.OID.data[14] > 8) {
        pdu.error = SNMP_ERR_NO_SUCH_NAME;
      }
      if ( pdu.type == SNMP_PDU_SET ) {
        status = pdu.VALUE.decode(&outlet_command);
        if (outlet_command == 1) {
          setBit(pdu.OID.data[14] - 1);
          updateBits();
        } else if (outlet_command == 2) {
          resetBit(pdu.OID.data[14] - 1);
          updateBits();
        }
      } else {
        if (outlet_values & (1 << (pdu.OID.data[14] - 1)))
          outlet_command = 1;
        else
          outlet_command = 2;
        status = pdu.VALUE.encode(SNMP_SYNTAX_INT, outlet_command);
      }
      pdu.type = SNMP_PDU_RESPONSE;
      pdu.error = status;
      //
      #ifdef DEBUG
        Serial << F("outlets1...") << endl;
      #endif
    } else {
      // oid does not exist
      //
      // response packet - object not found
      pdu.type = SNMP_PDU_RESPONSE;
      pdu.error = SNMP_ERR_NO_SUCH_NAME;
    }
    //
    //Serial.println("END");
    Agentuino.responsePdu(&pdu);
  }
  //Serial.println("NOK");
  //  while(1);
  //
  Agentuino.freePdu(&pdu);
  //
  //Serial << "UDP Packet Received End.." << " RAM:" << freeMemory() << endl;
}

void setup()
{
  Serial.begin(57600);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);  
  outlet_values = swap(0);
  swap(outlet_values);
  
  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0)
    Serial.println(F("Failed to access Ethernet controller"));
  //Serial.println("start2");
#if STATIC
  ether.staticSetup(myip, gwip);
#else
  if (!ether.dhcpSetup())
    Serial.println(F("DHCP failed"));
#endif
  //Serial.println("start3");
  
  ether.printIp("IP:  ", ether.myip);
  ether.printIp("GW:  ", ether.gwip);
  //ether.printIp("DNS: ", ether.dnsip);

  //Ethernet.begin(mac, ip);
  //
  api_status = Agentuino.begin();
  //
  if ( api_status == SNMP_API_STAT_SUCCESS ) {
    //
    Agentuino.onPduReceive(pduReceived);
    ether.udpServerListenOnPort(&udpSerialPrint, SNMP_DEFAULT_PORT);
    //
    delay(10);
    //
    Serial << F("SNMP Agent Initalized...") << endl;
    //
    return;
  }
  //
  delay(10);
  //
  Serial << F("SNMP Agent Initalization Problem...") << status << endl;
}

void loop()
{
  // listen/handle for incoming SNMP requests
  Agentuino.listen();
  //ether.packetLoop(ether.packetReceive());
  //
  // sysUpTime - The time (in hundredths of a second) since
  // the network management portion of the system was last
  // re-initialized.
  if ( millis() - prevMillis > 1000 ) {
    // increment previous milliseconds
    prevMillis += 1000;
    //
    // increment up-time counter
    locUpTime += 100;
  }
}

void setBit(unsigned char n) {
  //outlet_values = swap(0);
  outlet_values |= (1 << n);
  swap(outlet_values);
}

void resetBit(unsigned char n) {
  //outlet_values = swap(0);
  outlet_values &= ~(1 << n);
  swap(outlet_values);
}

void toggleBit(unsigned char n) {
  //outlet_values = swap(0);
  outlet_values ^= (1 << n);
  swap(outlet_values);
}

void updateBits() {
  digitalWrite(7, HIGH);
  delayMicroseconds(5);
  digitalWrite(7, LOW);
}

unsigned char swap(unsigned char inbyte) {
  unsigned char outbyte;
  for (unsigned char i = 0; i < 8; i++) {
    if (inbyte & 0x80) {
      digitalWrite(6, HIGH);
    } else {
      digitalWrite(6, LOW);
    }
    outbyte <<= 1;
    outbyte |= digitalRead(4);
    digitalWrite(5, HIGH);
    delayMicroseconds(5);
    digitalWrite(5, LOW);
    inbyte <<= 1;
  }
  return outbyte;
}

