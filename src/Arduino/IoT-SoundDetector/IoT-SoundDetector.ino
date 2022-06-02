#include <SPI.h>
#include <RF24.h>

#define nrf_ce 10  
#define nrf_csn 11
#define sen0232_A0 A0
#define sen12642_gate 7
#define sen12642_env A1
#define sen12642_aud A2

RF24 radio(nrf_ce, nrf_csn);

float dbA;

void setup()
{
    pinMode(sen12642_env, INPUT);
    Serial.begin(115200);
}

void loop()
{
    Serial.print("dbA: ");
    dbA = readDBA();
    Serial.println(dbA);
    Serial.print("Aud: ");
    Serial.println(analogRead(sen12642_aud));
}

float readDBA() {
    float voltage, dbRead;
    //delay(125);
    voltage = analogRead(sen0232_A0) * (5.0 / 1023.0);
    dbRead = voltage * 50.0;
    return dbRead;
}