#include <SPI.h>
#include <RF24.h>
#include <printf.h>
#include <LinkedList.h>
#include <GaussianAverage.h>
#include <Gaussian.h>
#include <MsgPackMap.h>

#define nrf_ce 9
#define nrf_csn 10
#define sen0232_A0 A0
#define sen12642_gate 7
#define sen12642_env A1
#define sen12642_aud A2
#define MAX_SAMPLES 95

enum states
{
    BEGIN,
    IDLE,
    SAMP,
    GAUSS,
    SEND
};

byte msgPackbuffer[32];
float dbA;
int actualState = BEGIN, nextState = IDLE, sampCnt = 0;
byte nodeDirection[] = "3node";
char sonometerID[] = "0001";

RF24 radio(nrf_ce, nrf_csn);
GaussianAverage myAverage(MAX_SAMPLES);
MsgPackMap msgdata(msgPackbuffer, 32);

void setup()
{
    pinMode(sen12642_gate, INPUT);
    Serial.begin(9600);
    msgdata.setStream(Serial);
    printf_begin();
    radio.begin();
    radio.openReadingPipe(1, nodeDirection);
    radio.openWritingPipe(nodeDirection);
    radio.setRetries(15, 15);
    radio.setPALevel(RF24_PA_LOW);
    radio.printDetails();
}

void loop()
{
    switch (actualState)
    {
    case BEGIN:
        for (int i = 0; i < MAX_SAMPLES; i++)
        {
            dbA = readDBA();
            Serial.print(dbA);
            Serial.print(",");
            myAverage.add(dbA);
        }
        nextState = GAUSS;
        break;

    case IDLE:
        sampCnt = 0;
        nextState = SAMP;
        break;

    case SAMP:
        dbA = readDBA();
        Serial.println(dbA);
        myAverage.add(dbA);      
        nextState = GAUSS;
        break;

    case GAUSS:
        myAverage.process();
        nextState = SEND;
        break;

    case SEND:
        Serial.print("Gauss average: ");
        Serial.println(myAverage.mean);
        msgdata.beginMap();
        msgdata.addString("Id:", sonometerID);
        msgdata.addFloat("dBA", myAverage.mean);
        msgdata.printRawData();
        radio.write(&msgdata, 32);
        nextState = IDLE;
        break;

    default:
        nextState = IDLE;
        break;
    }
    actualState = nextState;
}

float readDBA()
{
    float voltage, dbRead;
    // delay(125);
    voltage = analogRead(sen0232_A0) * (5.0 / 1023.0);
    dbRead = voltage * 50.0;
    return dbRead;
}