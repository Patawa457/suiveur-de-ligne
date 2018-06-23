#include <QTRSensors.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#define NUM_SENSORS   3
#define TIMEOUT       2500  
#define EMITTER_PIN   QTR_NO_EMITTER_PIN

QTRSensorsRC qtr_Sensor((unsigned char[]) {4, 5, 6},NUM_SENSORS, TIMEOUT, EMITTER_PIN); 
unsigned int sensorValues[NUM_SENSORS];

int DirMotA = 12, FreinMotA = 9 ,vitesseA = 3 ,DirMotB = 13,FreinMotB = 8, vitesseB = 11;
double vitesse = 65;

struct data {
  String conf;
  int etat;
};

/*
 * image prés enregister.
 */
const int nbSituation = 5;
struct data Image_def[nbSituation]=
{{"010", 0},
 {"001", 1},
 {"011", 1},
 {"100", 2},
 {"110", 2}
}; 

/*
 * coeficient pour virage.
 */
double Reaction_moteur[3][2]={
 {1.0 , 1.0},
 {2.4 , 1.0},
 {1.0 , 2.4},
};


void setup()   {                
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)

  display.display();
  delay(2000);
  display.clearDisplay();
  
  pinMode(DirMotA,OUTPUT);
  pinMode(FreinMotA,OUTPUT);
  pinMode(vitesseA,OUTPUT);
  pinMode(DirMotB,OUTPUT);
  pinMode(FreinMotB,OUTPUT);
  pinMode(vitesseB,OUTPUT);
  
digitalWrite(DirMotB, 1); 
digitalWrite(FreinMotB, 0);  
digitalWrite(DirMotA, 0); 
digitalWrite(FreinMotA, 0); 

}

int etat_actuel;
int etat_general;
uint8_t color;
int memoire;

void loop() {
 
while(etat_general == etat_actuel){
  etat_actuel = capteur();
  Affichage_Capteur();
  Affichage_Etat(etat_actuel);
  display.display();
  display.clearDisplay();
  moteur(etat_general);
    Serial.println();
  }
etat_general = etat_actuel;
}

/*
  prend les donnée, tranformation en binaire, stock donné 
 */

int capteur(){
  
  //Partie 1: Reception donné + binarisation  + création et image
  String Sensor= ""; // permet de crée l'image de la route
  int etat = -1;
  qtr_Sensor.read(sensorValues);
  
  for(int i=0; i<NUM_SENSORS ; i++){
    sensorValues[i] = Binaire(sensorValues[i]);
    Sensor += (String) sensorValues[i];        // Création de l'image du capteur avant dans un string
  }

  //Partie 2: Comparaison et memoire de l'état
  for(int i=0; i < nbSituation ; i++){ //permet de trouvé quel situation définis correspond a l'image
    
      if(Sensor == Image_def[i].conf){
      etat = Image_def[i].etat;
      memoire = etat;
      }
      else{ //permet de conservé l'état actuel si l'image capté ne correspond pas aux situations définis 
      etat = memoire; 
      }
      
  }
  return etat;
}


void Affichage_Capteur(){
  
int Y1 = 0;
int X2 = 20;
int Y2 = 10;  
int X1 = (display.width()/2)-(3*X2/2);
int Limite  = X1+3*X2;
int i = 0;

for(;X1< Limite; X1+=20){
  if (sensorValues[i] == 1) color = WHITE;
  else color = BLACK;
  
  display.fillRect( X1+1, Y1+1, X2-1, Y2-1, color);
  display.drawRect(X1, Y1, X2, Y2, WHITE);
  i++;
  }
}

void Affichage_Etat(int etat){
    display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,17);
  display.print(etat);
  display.setTextSize(1);
  display.setCursor(20,17);
  display.print(Reaction_moteur[etat][0]);
  display.print("  ");
  display.print(Reaction_moteur[etat][1]);
  display.println();
  display.print("   ");
  display.print(vitesse * Reaction_moteur[etat][0]);
  display.print(" ");
  display.print(vitesse * Reaction_moteur[etat][1]);
  
}

int Binaire(int val){
  int valeur;
  if(val<1250){
   valeur = 1;
  }else{
   valeur = 0;
  }
  return valeur;
}

/*
 * CAPTEUR -> vitesse
 */
void moteur(int etat){
 analogWrite(vitesseA, vitesse * Reaction_moteur[etat][0]);   
 analogWrite(vitesseB, vitesse * Reaction_moteur[etat][1]);
}

