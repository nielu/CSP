#pragma mark - Depend ESP8266Audio and ESP8266_Spiram libraries
/*
cd ~/Arduino/libraries
git clone https://github.com/earlephilhower/ESP8266Audio
git clone https://github.com/Gianbacchio/ESP8266_Spiram

Use the "Tools->ESP32 Sketch Data Upload" menu to write the MP3 to SPIFFS
Then upload the sketch normally.
https://github.com/me-no-dev/arduino-esp32fs-plugin
*/

#include <M5Core2.h>
#include <WiFi.h>
#include "AudioFileSourceSPIFFS.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"

#include <Math.h>
#include "FS.h"
#include "SPIFFS.h"

#define FORMAT_SPIFFS_IF_FAILED true

#define CONFIG_I2S_BCK_PIN 12
#define CONFIG_I2S_LRCK_PIN 0
#define CONFIG_I2S_DATA_PIN 2
#define CONFIG_I2S_DATA_IN_PIN 34

#define Speak_I2S_NUMBER I2S_NUM_0

#define MODE_MIC 0
#define MODE_SPK 1

#define OUTPUT_GAIN 10

const char *filenames[] = {
    "/a_co_zmieniac.mp3",
    "/a_jak_wszyscy_chlopcy.mp3",
    "/a_to_ja_nie_wiem.mp3",
    "/badzmy_lagodni.mp3",
    "/bardzo.mp3",
    "/cuo.mp3",
    "/dosc.mp3",
    "/e_co.mp3",
    "/he_he_he.mp3",
    "/he_he_he_evil.mp3",
    "/jak_mi_dadza_to_jem.mp3",
    "/jak_pan_jezus_powiedzial.mp3",
    "/jest_mozliwe.mp3",
    "/jeszcze_jak.mp3",
    "/moze_tak_kiedys.mp3",
    "/moze_z_dziewczynkami.mp3",
    "/mozna_jak_najbardziej.mp3",
    "/nie_jak_dotad_nie.mp3",
    "/nie_wiem.mp3",
    "/nie_wiem_o_takiej_gumie.mp3",
    "/okrutnik_no.mp3",
    "/poco_wybierac.mp3",
    "/taak.mp3",
    "/takiego_prawdziwego_to_nie.mp3",
    "/tak_czasem_bili.mp3",
    "/z_warkoczykami.mp3"};

#define filenames_length (sizeof(filenames) / sizeof(const char *))

AudioGeneratorMP3 *mp3;
AudioOutputI2S *out;
AudioFileSourceSPIFFS *file = NULL;

float accX = 0.0f;
float accY = 0.0f;
float accZ = 0.0f;
float totalAcc = 0.0f;

void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if (!root)
    {
        Serial.println("− failed to open directory");
        return;
    }
    if (!root.isDirectory())
    {
        Serial.println(" − not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if (levels)
            {
                listDir(fs, file.name(), levels - 1);
            }
        }
        else
        {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void playShortPapaj()
{
    if (mp3->isRunning())
        return;

    int fileNo = random(filenames_length);

    M5.Lcd.printf("Playing %d\n", fileNo);
    Serial.printf("Playing %d\n", fileNo);

    mp3 = new AudioGeneratorMP3();
    file = new AudioFileSourceSPIFFS(filenames[fileNo]);
    // delay(5000);
    file->open(filenames[fileNo]);
    mp3->begin(file, out);
}

void setup()
{
    M5.begin();
    M5.IMU.Init();
    WiFi.mode(WIFI_OFF);
    M5.Axp.SetSpkEnable(true);

    delay(500);

    M5.Lcd.setTextFont(2);
    M5.Lcd.printf("Sample MP3 playback begins...\n");
    Serial.printf("Sample MP3 playback begins...\n");

    out = new AudioOutputI2S(0, 0); // Output to builtInDAC
    mp3 = new AudioGeneratorMP3();

    out->SetPinout(CONFIG_I2S_BCK_PIN, CONFIG_I2S_LRCK_PIN, CONFIG_I2S_DATA_PIN);
    out->SetOutputModeMono(true);
    out->SetGain(3.9f);

    if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED))
    {
        Serial.printf("SPIFFS Mount failed");
        M5.Lcd.printf("SPIFFS Mount failed");
        return;
    }

    listDir(SPIFFS, "/", 0);

    file = new AudioFileSourceSPIFFS("/mozna_jak_najbardziej.mp3");
    mp3->begin(file, out);
}

void loop()
{
    if (mp3->isRunning())
    {
        if (!mp3->loop())
        {
            // M5.Axp.SetSpkEnable(false);
            mp3->stop();
        }
    }
    else
    {
        M5.IMU.getAccelData(&accX, &accY, &accZ);
        totalAcc = sqrt(accX * accX + accY * accY + accZ * accZ);
        if (totalAcc > 1.5f)
        {
            M5.Lcd.printf("Acceleration: %.2f\n", totalAcc);
            playShortPapaj();
        }
        delay(250);
    }
}
