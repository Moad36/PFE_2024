#include <LiquidCrystal.h>
#include <DHT11.h>
#include <IRremote.hpp>
#define DHT11PIN A0
#define claima 7
#define ventilation 6
#define CO2_Sensor A1
#define UP 10
#define DOWN 9
#define MODE 8
#define IR_recepteur_pin A4
#define IR_emetteur_pin A5

int T = 20;
int temperature = 0;
int humidity = 0;
bool modstate = false;
bool upstate = false;
bool downstate = false;
String Mods[2] = {"Relais","Infrarouge"};
int currentMode = 0;
int mode = 1;
unsigned long up_button_signal, down_button_signal, ON_OFF_button_signal;

bool state = true;
bool state1 = true;
bool claima_status_on = true;
bool claima_status_off = true;

DHT11 dht11(DHT11PIN);

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

IRrecv irrecv(IR_recepteur_pin);
IRsend sender(IR_emetteur_pin);

decode_results results;

void setup() {
  lcd.begin(16, 2);
  pinMode(UP, INPUT_PULLUP);
  pinMode(DOWN, INPUT_PULLUP);
  pinMode(MODE, INPUT_PULLUP);
  pinMode(claima, OUTPUT);
  pinMode(ventilation, OUTPUT);
  pinMode(CO2_Sensor, INPUT);
  pinMode(DHT11PIN, INPUT);
  pinMode(IR_recepteur_pin, INPUT);
  pinMode(IR_emetteur_pin, OUTPUT);
  lcd.print("Demarrage...");
  irrecv.enableIRIn();
  Serial.begin(9600);
  delay(100);
}

void loop() {
  state = true;
  state1 = true;
  if (digitalRead(MODE) == HIGH) {
    mode++;
    if (mode > 3) mode = 1; // Nombre de modes disponibles
    delay(500);
  }
  int result = dht11.readTemperatureHumidity(temperature, humidity);
  delay(500);
  float AIR = analogRead(CO2_Sensor);
  switch (mode) {
    case 1: // Affichage par défaut
      lcd.clear();
      lcd.print("T: "); lcd.print(temperature); lcd.print((char)223); lcd.print("C");
      lcd.print(" | H: "); lcd.print(humidity); lcd.print("% ");
      lcd.setCursor(0, 1);
      lcd.print("CO2: ");lcd.print(AIR);
      if (AIR > 250.00) { digitalWrite(ventilation, LOW);}
      else if (AIR < 250.00) { digitalWrite(ventilation, HIGH);}
      if (temperature > T) {
        if ((Mods[currentMode] == "Infrarouge") && (claima_status_on)) {
          sender.sendNEC(ON_OFF_button_signal, 32);
          claima_status_on = false;
          claima_status_off = true;
        }
        else if (Mods[currentMode] == "Relais") {
          digitalWrite(claima, LOW);
        }
      }
      else if (temperature < T) {
        if ((Mods[currentMode] == "Infrarouge") && (claima_status_off)) {
          sender.sendNEC(ON_OFF_button_signal, 32);
          claima_status_off = false;
          claima_status_on = true;
        }
        else if (Mods[currentMode] == "Relais") {
          digitalWrite(claima, HIGH);
        }
      }
      delay(500);
      break;
    case 2: // Mode d'ajustement de la température
      lcd.clear();
      lcd.print("température T :");
      lcd.setCursor(0, 1);
      lcd.print("->"); lcd.print(T);
      while (state) {
        if (digitalRead(UP) == HIGH) {
          T++;
          if(up_button_signal != 0){sender.sendNEC(up_button_signal, 32);}
        }
        else if (digitalRead(DOWN) == HIGH) {
          T--;
          if(up_button_signal != 0){sender.sendNEC(down_button_signal, 32);}
        }
        lcd.setCursor(0, 1);
        lcd.print("->"); lcd.print(T);
        delay(250);
        if (digitalRead(MODE) == HIGH) { delay(500); state = false; mode++; }
      }
      delay(500);
      break;
    case 3: // Mode de sélection du mode
      lcd.clear();
      lcd.print("choisir mode :");
      lcd.setCursor(0, 1);
      lcd.print("->"); lcd.print(Mods[currentMode]);
      delay(500);
      while (state1) {
        if (digitalRead(UP) == HIGH) {
          currentMode++;
          if (currentMode == 2) { currentMode = 0; }
          lcd.clear();
          lcd.print("choisir mode :");
          lcd.setCursor(0, 1);
          lcd.print("->"); lcd.print(Mods[currentMode]);
          delay(500);
        }
        if ((digitalRead(MODE) == HIGH) && (Mods[currentMode] == "Infrarouge")) {
          bool saved = false;
          int state2 = true;
          while (state2) {
            delay(500);
            lcd.clear();
            lcd.print("Bouton HAUT:");
            lcd.setCursor(0, 1);
            lcd.print("->"); lcd.print(up_button_signal, HEX);
            if (IrReceiver.decode()) {
              auto value = IrReceiver.decodedIRData.decodedRawData;
              up_button_signal = value;
              IrReceiver.resume(); // Recevoir la valeur suivante
            }
            if (digitalRead(MODE) == HIGH) {
              while (true) {
                lcd.clear();
                lcd.print("Bouton BAS:");
                lcd.setCursor(0, 1);
                lcd.print("->"); lcd.print(down_button_signal, HEX);
                if (IrReceiver.decode()) {
                  auto value = IrReceiver.decodedIRData.decodedRawData;
                  down_button_signal = value;
                  IrReceiver.resume(); // Recevoir la valeur suivante
                }
                delay(500);
                if (digitalRead(MODE) == HIGH) {
                  while (true) {
                    lcd.clear();
                    lcd.print("Bouton ON/OFF:");
                    lcd.setCursor(0, 1);
                    lcd.print("->"); lcd.print(ON_OFF_button_signal, HEX);
                    if (IrReceiver.decode()) {
                      auto value = IrReceiver.decodedIRData.decodedRawData;
                      ON_OFF_button_signal = value;
                      IrReceiver.resume(); // Recevoir la valeur suivante
                    }
                    delay(500);
                    if (digitalRead(MODE) == HIGH) { state2 = false; state1 = false; mode = 1; break; saved=true;delay(500);}
                  }
                }
              }
              if(saved){break; lcd.setCursor(0, 1);lcd.print("->breaked");}
            }
            delay(500);
          }
        }
        if ((digitalRead(MODE) == HIGH) && (Mods[currentMode] == "Relais")) { delay(500); state1 = false; mode = 1; break; }
      }
      break;
  }
}
