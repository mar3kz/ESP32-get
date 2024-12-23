#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include "cJSON.h"

/*
{} = toto se pocita jako array = cJSON_GetArrayItem
[] = toto se pocita jako array = cJSON_GetArrayItem
*/

#define LED_BUILTIN 2

void blinking(int pinNum);
void connectedToWiFi(int pinNum);

const char *SSID = "SSID";
const char *password = "PASSWORD";
const char *websiteAPI =  "https://api.openweathermap.org/data/2.5/weather?lat={lat}lon={lon}&appid={API_KEY}"; // stranka pro API

// inicializace ESP32 jako HTTP client
HTTPClient client;

void setup() 
{
    Serial.begin(115200); // zacatek komunikace mezi PC a development boardem

    WiFi.mode(WIFI_STA); // nastaveni ESP32 jako WIFI STATION => pripojeni k internetu

    WiFi.begin(SSID, password); // pripojeni k internetu

    while ( WiFi.status() != WL_CONNECTED)
    {
        Serial.println("CONNECTING");
        int time = millis();    // vezme pocet milisekund od zapojeni dev boardu
        
        if ( (6000 - time) <= 0) // 6s timeout
        {
            Serial.println(6000 - time);
            ESP.restart();
        }
        blinking(LED_BUILTIN);
    }
    Serial.println("CONNECTED");
    connectedToWiFi(LED_BUILTIN);

    client.begin(websiteAPI); // vezme si interne jaky je to protokol, url, path apod... => naplni struktur HTTPClient
    int responseCode = client.GET(); // spusti interne dalsi funkci, ktera si vezme informace ze struktury HTTPClient a posle odkaz podle tich interne ulozenych informaci

    //Serial.println(responseCode);

    if (responseCode >= 200 && responseCode < 300) // toto znamena, ze vse proslo OK
    {
        String stringHttpPayload = client.getString(); // vezme http payload z http response a ulozi to do toho pole
        const char *httpPayload = stringHttpPayload.c_str(); // protoze String neni kompatibilni s "C-style" string, proto Arduino.h (Arduino framework) nabizi funkci na prevod
        
        cJSON *object = cJSON_Parse(httpPayload); // supply a JSON, ze ktereho muzeme ziskavat data (interrogate), root pointer = tree

        // child pointery = tree
        cJSON *weather = cJSON_GetObjectItem(object, "weather"); // objekt weather => array
        
        cJSON *main_arr = cJSON_GetObjectItem(object, "main"); // objekt main => array
        cJSON *sys = cJSON_GetObjectItem(object, "sys"); // objekt main => array

        // objekty z array
        cJSON *PointerToArrayFromWeather = cJSON_GetArrayItem(weather, 0);
        cJSON *mainObject = cJSON_GetArrayItem(PointerToArrayFromWeather, 1);
        cJSON *description = cJSON_GetArrayItem(PointerToArrayFromWeather, 2);

        cJSON *tempObject = cJSON_GetArrayItem(main_arr, 0);
        double temp = cJSON_GetNumberValue(tempObject);

        cJSON *country = cJSON_GetArrayItem(sys, 2);

        // neni v array, proto jen cJSON_GetObjectItem
        cJSON *name = cJSON_GetObjectItem(object, "name");

        // prevedeni na char * (C-style string)
        char *ToPrintMainObject = cJSON_Print(mainObject); // prevede to do textu, ale nic se neukaze na vystupu
        char *ToPrint_description = cJSON_Print(description);
        char *ToPrint_country = cJSON_Print(country);
        char *ToPrint_name = cJSON_Print(name);

        Serial.println();

        Serial.print("Stát: ");
        Serial.println(ToPrint_country);

        Serial.print("Místo: ");
        Serial.println(ToPrint_name);

        Serial.print("Počasí: ");
        Serial.println(ToPrintMainObject);

        Serial.print("Počasí - popisek: ");
        Serial.println(ToPrint_description);

        double tempCelsius = temp - 273.15;
        Serial.print("Teplota: ");
        Serial.print(tempCelsius);   
        Serial.println(" °C");
        
        cJSON_free(object);
    }
}

void loop()
{

}

void blinking(int pinNum)
{
    pinMode(pinNum, OUTPUT);
    digitalWrite(pinNum, HIGH);
    delay(500);
    digitalWrite(pinNum, LOW);
    delay(500);
}

void connectedToWiFi(int pinNum)
{
    delay(500); // delay, aby vse fungovalo OK
    pinMode(pinNum, OUTPUT);
    digitalWrite(pinNum, HIGH);
    delay(500);
}

// https://github.com/DaveGamble/cJSON/blob/master/cJSON.h#L103