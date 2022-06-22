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
#define led_red 3
#define led_green 5
#define led_blue 6

#define MINUTE_SAMPLES 60
#define SLOW_SAMPLES 8

enum states
{
    IDLE,
    SLOW_SAMP,
    MINUTE_SAMP,
    GAUSS,
    SEND
};

byte dbAcolors[21][4] = {
    {30, 0, 112, 192},
    {35, 0, 128, 168},
    {40, 0, 144, 136},
    {45, 0, 160, 108},
    {50, 0, 176, 80},
    {55, 64, 180, 60},
    {60, 128, 184, 40},
    {65, 192, 188, 20},
    {70, 255, 192, 0},
    {75, 239, 144, 0},
    {80, 223, 96, 0},
    {85, 207, 48, 0},
    {90, 192, 0, 0},
    {95, 202, 0, 0},
    {100, 212, 0, 0},
    {105, 222, 0, 0},
    {110, 232, 0, 0},
    {115, 242, 0, 0},
    {120, 255, 0, 0},
    {125, 255, 0, 0},
    {130, 255, 0, 0},
};

byte msgPackbuffer[32];
float dbA;
int actualState = IDLE, nextState = IDLE, slowSampCnt = 0, sampCnt = 0; 
byte nodeDirection[] = "1Node";
char sonometerID[] = "0001";

RF24 radio(nrf_ce, nrf_csn);
GaussianAverage myAverage(MINUTE_SAMPLES);
GaussianAverage slowAverage(SLOW_SAMPLES);
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
    analogWrite(led_red, 102);
    analogWrite(led_green, 0);
    analogWrite(led_blue, 102);
}

void loop()
{
    switch (actualState)
    {
    case IDLE:
        sampCnt = 0;
        slowSampCnt = 0;
        nextState = SLOW_SAMP;
        break;

    case SLOW_SAMP:
        delay(125);
        dbA = readDBA();
        slowSampCnt++;
        //Serial.print(dbA);
        //Serial.print(",");
        slowAverage.add(dbA);
        if(slowSampCnt < SLOW_SAMPLES)
            nextState = SLOW_SAMP;
        else
            nextState = MINUTE_SAMP;
        break;

    case MINUTE_SAMP:
        sampCnt++;
        slowSampCnt = 0;
        slowAverage.process();
        //Serial.println();
        Serial.print(slowAverage.mean);
        Serial.print(",");
        turnRGB_LED(slowAverage.mean);
        myAverage.add(slowAverage.mean);
        if(sampCnt < MINUTE_SAMPLES)
            nextState = SLOW_SAMP;
        else
            nextState = GAUSS;
        break;

    case GAUSS:
        myAverage.process();
        nextState = SEND;
        break;

    case SEND:
        Serial.println();
        Serial.print("Gauss average: ");
        Serial.println(myAverage.mean);
        msgdata.beginMap();
        msgdata.addString("Id:", sonometerID);
        msgdata.addFloat("dBA", myAverage.mean);
        turnRGB_LED(myAverage.mean);
        msgdata.printRawData();
        radio.write(&msgPackbuffer, 32);
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

int turnRGB_LED(float value)
{
    for (int i = 0; i < 20; i++)
    {
        if (value >= dbAcolors[i][0] && value < dbAcolors[i + 1][0])
        {
            analogWrite(led_red, dbAcolors[i][1]);
            analogWrite(led_green, dbAcolors[i][2]);
            analogWrite(led_blue, dbAcolors[i][3]);
            return 0;
        }
    }
    analogWrite(led_red, dbAcolors[20][1]);
    analogWrite(led_green, dbAcolors[20][2]);
    analogWrite(led_blue, dbAcolors[20][3]);
    return -1;
}