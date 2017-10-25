/////////////////////////////////////////////////////////////////////////////////
// Acquisition de la mesure d un photometre
// Jeremie Baudet
// 25 septembre 2017
////////////////////////////////////////////////////////////////////////////////
   // This example shows how to read and write data to and from an SD card file   
// The circuit:
// * SD card attached to SPI bus as follows:
// ** MOSI - pin 11   51
// ** MISO - pin 12   50
// ** CLK - pin 13    52
// ** CS - pin 4 53   53

// Inclusions
#include<stdio.h>
#include<math.h>
#include<SD.h>
//#include<SPI.h>
#include<Wire.h>
#include"RTClib.h"

RTC_DS1307 RTC;
const int chipSelect = 53;  // CS
File measuresFile;          // Fichier d ecriture
// RTC Module   Arduino
//    SCL         21
//    SDA         20
//    VCC         5V
//    GND         GND

int writeMeasure(void);
// definitions

// declaration du cablage
// Photometre    Arduino      Function
//    1            22         1 x 10^0 DIGIT
//    2            23         2 x 10^0 DIGIT
//    3            24         4 x 10^0 DIGIT
//    4            25         8 x 10^0 DIGIT
//    5            26         1 x 10^1 DIGIT
//    6            27         2 x 10^1 DIGIT
//    7            28         4 x 10^1 DIGIT
//    8            29         8 x 10^1 DIGIT
//    9            30         1 x 10^2 DIGIT
//    10           31         2 x 10^2 DIGIT
//    11           32         4 x 10^2 DIGIT
//    12           33         8 x 10^2 DIGIT
//    13           34         1 x 10^3 DIGIT
//    14           35         DP 1 188.8
//    15           36         DP 2 18.88
//    16           37         DP 3 1.888
//    17           38         /Print
//    18           39         Hold
//    19           40         Digital Ground
//    20           41         /overrange
//    21           42         Polarity
//    22           43         _____(Blank)
//    23           44         Exponent BIT 1
//    24           45         Exponent BIT 2
//    25           46         Exponent BIT 4

// Pins sur l arduino MEGA 2560
int photoMeterPins[] = {22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41 , 42, 43, 44, 45, 46};

const int pinsNumber = sizeof(photoMeterPins)/sizeof(int);

// Variables globales
int photoMeterValues[pinsNumber];
int digitE0Value[4];
int digitE1Value[4];
int digitE2Value[4];
int digitE3Value = 0;
int digitE0DecimalValue = 0;
int digitE1DecimalValue = 0;
int digitE2DecimalValue = 0;
int digitE3DecimalValue = 0;
int decimalValue = 0;
int decimalPointValues[3];
int exponentValue[3];
int exponentDecimalValue = 0;
int buttonPin = 8;
int ledPin = 4;
int buttonState = 1;
float displayValue = 0.0;
float divisionFactor = 1.0;
float multiplierFactor = 1.0;
int headerFlag=0;
long int id = 0;
char measureBuffer[] = "as";
int Nsignificant = 0;

void setup() {
  // Ouverture du port serie
  Serial.begin(9600);
  
  // Controle du nombre de pins
  //Serial.print("Nombre de pins: ");
  //Serial.println(pinsNumber);
  
  Serial.print("GPIOs Configuration...");
  // Configuration des pins
  for(int i=0; i<13;i++)  // DIGITS
  {
    pinMode(photoMeterPins[i], INPUT);
  }
  pinMode(photoMeterPins[13], INPUT);   // Decimal point
  pinMode(photoMeterPins[14], INPUT);   // Decimal point
  pinMode(photoMeterPins[15], INPUT);   // Decimal point
  pinMode(photoMeterPins[16], OUTPUT);                  // /Print
  //pinMode(photoMeterPins[17], );      // Hold
  //pinMode(photoMeterPins[18], );      // Digital ground
  //pinMode(photoMeterPins[19], );      // /Overrange
  //pinMode(photoMeterPins[20], );      // Polarity
  //pinMode(photoMeterPins[21], );      // ____(Blank)
  pinMode(photoMeterPins[22], INPUT);   // Exponent Bit 1
  pinMode(photoMeterPins[23], INPUT);   // Exponent Bit 2
  pinMode(photoMeterPins[24], INPUT);   // Exponent Bit 4
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  Serial.println(" ...done!");
  Serial.print("Initializing SD card...");
  // On the Ethernet Shield, CS is pin 4. It s set as an output by default.
  // Note that even if it s not used as the CS pin, the hardware SS pin 
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output 
  // or the SD library functions will not work. 
   pinMode(53, OUTPUT);
   pinMode(10, OUTPUT);
      
  if (!SD.begin(chipSelect))
  {
    Serial.println(" ...initialization failed!");
    return;
  }
  Serial.println(" ...initialization done.");
  if (SD.exists("mesures.txt")) {
    Serial.println("mesures.txt exists.");
    // ...
    id=findEndLine();
    readMeasureFile(); 
  }
  
  else {
    Serial.println("mesures.txt doesn t exist.");
    headerFlag = 1;
  }
  
  // Initalisation du module RTC
  Wire.begin();
    RTC.begin();
  // Check to see if the RTC is keeping time.  If it is, load the time from your computer.
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // This will reflect the time that your sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
}


void loop(){
  // Acquisition bouton
  buttonState = digitalRead(buttonPin);
  delay(200);
  if(buttonState == 0)
  {
    if(headerFlag==1)
    {
      writeHeader();
      headerFlag=0;
    }
    getMeasure();
    writeMeasure();
  }
}


// Fonction d acquisition
float getMeasure(void)
{
  // Initialisation des donnees;
  digitE0DecimalValue = 0;
  digitE1DecimalValue = 0;
  digitE2DecimalValue = 0;
  digitE3DecimalValue = 0;
  exponentDecimalValue = 0;
  
  // Acquisition des valeurs du photometre
  for(int i=0; i<4; i++)
  {
    digitE0Value[i] = digitalRead(photoMeterPins[i]);
  }
  for(int i=0; i<4; i++)
  {
    digitE1Value[i] = digitalRead(photoMeterPins[i+4]);
  }
  for(int i=0; i<4; i++)
  {
    digitE2Value[i] = digitalRead(photoMeterPins[i+8]);
  }
  digitE3Value = digitalRead(photoMeterPins[12]);

  for(int i=0; i<3; i++)
  {
    decimalPointValues[i] = digitalRead(photoMeterPins[i+13]);
  }
  
  for(int i=0; i<3; i++)
  {
    exponentValue[i] = digitalRead(photoMeterPins[i+22]);
  }
  
  // Conversion des digits en valeurs decimales
  for(int i=3; i>=0; i--) // Digit E0
  {
    digitE0DecimalValue = (digitE0DecimalValue << 1) + digitE0Value[i];
  }
  for(int i=3; i>=0; i--) // Digit E1
  {
    digitE1DecimalValue = (digitE1DecimalValue << 1) + digitE1Value[i];
  }
  for(int i=3; i>=0; i--) // Digit E2
  {
    digitE2DecimalValue = (digitE2DecimalValue << 1) + digitE2Value[i];
  }
  digitE3DecimalValue = digitE3Value;

  // Calcul de la valeur decimale de l affichage
  decimalValue = digitE0DecimalValue + digitE1DecimalValue*pow(10, 1) + digitE2DecimalValue*pow(10, 2) + digitE3DecimalValue*pow(10, 3);

  // Division de la valeur decimale en fonction du DECIMAL POINT
  
  // Conversion de l exposant en valeur decimale
  for(int i=2; i>=0; i--) // Exponent
  {
    exponentDecimalValue = (exponentDecimalValue << 1) + exponentValue[i];
  }

  switch(exponentDecimalValue)
  {
    case 0:
      multiplierFactor = float(1.0*pow(10.0, 3));
      Nsignificant = 0;
      break;
    case 1:
      multiplierFactor = float(1.0*pow(10.0, 2));
      Nsignificant = 1;
      break;
    case 2:
      multiplierFactor = float(1.0*pow(10.0, 1));
      Nsignificant = 1;
      break;
    case 3:
      multiplierFactor = float(1.0);
      Nsignificant = 2;
      break;
    case 4:
      multiplierFactor = float(1.0*pow(10.0, -1));
      Nsignificant = 3;
      break;
    case 5:
      multiplierFactor = float(1.0*pow(10.0, -2));
      Nsignificant = 4;
      break;
    case 6:
      multiplierFactor = float(1.0*pow(10.0, -3));
      Nsignificant = 5;
      break;
    default:
      multiplierFactor = float(1.0);
      Nsignificant = 3;
      break;
  }

  // Definition du facteur de division en fonction du DP
  if(decimalPointValues[0] == HIGH && decimalPointValues[1] == HIGH && decimalPointValues[2] == LOW) // decimalPointValues[i]
  {
    divisionFactor = float(1000.0);
  }
  else if(decimalPointValues[0] == HIGH && decimalPointValues[1] == LOW && decimalPointValues[2] == HIGH)
  {
    divisionFactor = float(100.0);
  }
  else if(decimalPointValues[0] == LOW && decimalPointValues[1] == HIGH && decimalPointValues[2] == HIGH)
  {
    divisionFactor = float(10.0);
  }
  else
  {
    divisionFactor = float(1);
  }
  
  // Calcul de la valeur a afficher
  displayValue = float(decimalValue*multiplierFactor/divisionFactor);
  Serial.print("Value to print = ");
  Serial.println(displayValue, Nsignificant);
  return(displayValue);
  }

  int writeMeasure(void)
  {
    char buff[2]={12};
    DateTime now = RTC.now(); 
    ////// Tant que pas de SD /////////////////////////////////////
    Serial.print("Mesure: ");
    Serial.print(displayValue, Nsignificant);
    Serial.print(", Date: ");
    sprintf(buff, "%2.2d", now.day());
    Serial.print(buff);
    Serial.print(".");
    sprintf(buff, "%2.2d", now.month());
    Serial.print(buff);
    Serial.print(".");
    sprintf(buff, "%2.2d", now.year());
    Serial.print(buff);
    Serial.print(", Heure: ");
    sprintf(buff, "%2.2d",now.hour());
    Serial.print(buff);
    Serial.print(":");
    sprintf(buff, "%2.2d",now.minute());
    Serial.print(buff);
    Serial.print(":");
    sprintf(buff, "%2.2d",now.second());
    Serial.print (buff);
    Serial.println();
    ///////////////////////////////////////////////////////////////
   // open the file. note that only one file can be open at a time,
   // so you have to close this one before opening another.
   measuresFile = SD.open("mesures.txt", FILE_WRITE);
  // if the file opened okay, write to it:
  if (measuresFile)
  {
    // Incrementation du compteur de mesure
    id++;
    // Allumage de la Led
    digitalWrite(ledPin, HIGH);
    Serial.print("Writing to mesures.txt...");
    measuresFile.print(id);
    measuresFile.print("\t\t");
    sprintf(buff, "%2.2d",now.day());
    measuresFile.print(buff);
    measuresFile.print(".");
    sprintf(buff, "%2.2d",now.month());
    measuresFile.print(buff);
    measuresFile.print(".");
    sprintf(buff, "%2.2d",now.year());
    measuresFile.print(buff);
    measuresFile.print("\t");
    sprintf(buff, "%2.2d",now.hour());
    measuresFile.print(buff);
    measuresFile.print(":");
    sprintf(buff, "%2.2d",now.minute());
    measuresFile.print(buff);
    measuresFile.print(":");
    sprintf(buff, "%2.2d",now.second());
    measuresFile.print (buff);
    measuresFile.print("\t");
    measuresFile.print(displayValue, Nsignificant);
    //measuresFile.print(displayValue);measureBuffer
    measuresFile.println();
     
    // close the file:
    measuresFile.close();
    Serial.println("done.");
    digitalWrite(ledPin, LOW);
  }
  else
  {
    // if the file didn t open, print an error:
    Serial.println("error opening mesures.txt");
  }
  return 0;
  }
int writeHeader(void)
{
  // Ecriture de l en-tete
  // open the file. note that only one file can be open at a time,
   // so you have to close this one before opening another.
   measuresFile = SD.open("mesures.txt", FILE_WRITE);
  // if the file opened okay, write to it:
  if (measuresFile)
  {
    Serial.print("Writing the header to mesures.txt...");
    measuresFile.print("Identifiant");
    measuresFile.print("\t");
    measuresFile.print("Date");
    measuresFile.print("\t\t");
    measuresFile.print("Heure");
    measuresFile.print("\t\t");
    measuresFile.println("Mesure [unit]");
    // close the file:
    measuresFile.close();
    Serial.println(" ...done!");
  }
  else
  {
    // if the file didn t open, print an error:
    Serial.println("Error opening mesures.txt");
  }
}

int readMeasureFile(void)
{
//open the file for reading
  measuresFile = SD.open("mesures.txt", FILE_READ);
  if (measuresFile) {
    Serial.println("mesures.txt:");
  while (measuresFile.available()) {
      Serial.write(measuresFile.read());
    }
    // close the file:
    measuresFile.close();
  }
  else {
    // if the file didn t open, print an error:
    Serial.println("error opening test.txt");
  }
}

int findEndLine(void)
{
  int endLineNo=0;
  char currentChar;
  int currentCharNo;
  int currentLineNo = 0;
  //open the file for reading
  measuresFile = SD.open("mesures.txt", FILE_READ);
  if (measuresFile) {
  while(measuresFile.available()) {
      currentCharNo++;
      currentChar = measuresFile.read();
      if(currentChar==0x0D)
      {
        currentCharNo = 0;
        currentLineNo++;
      }
    }
    endLineNo = currentLineNo;
    // close the file:
    measuresFile.close();
  }
  else {
    // if the file didn t open, print an error:
    Serial.println("error opening test.txt");
  }
  return endLineNo;
}
