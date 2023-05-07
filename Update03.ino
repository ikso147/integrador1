
#include <LiquidCrystal.h>
#include <AccelStepper.h>
#include <Ticker.h>
void Timer();
Ticker timer2(Timer, 1000, 0, MILLIS);

//Pinos LCD
#define RS_PIN 26
#define ENABLE_LCD_PIN 27
#define D4 28
#define D5 29
#define D6 30
#define D7 31
LiquidCrystal lcd(RS_PIN, ENABLE_LCD_PIN, D4, D5, D6, D7);

//Pinos Teclado 1x5
const int ROW = 2;
const int COL1 = 3;
const int COL2 = 4;
const int COL3 = 5;
const int COL4 = 6;
const int COL5 = 7;

//Variáveis para os botões
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 400;
char buffer[5];       // buffer para armazenar as teclas pressionadas
int bufferIndex = 0;  // índice atual do buffer

//Pinos Motor 1 (base)
#define DIR_M1_PIN 10
#define STEP_M1_PIN 9
#define EN_M1_PIN 8
#define SLEEP_M1_PIN 11

AccelStepper motor1(AccelStepper::DRIVER, STEP_M1_PIN, DIR_M1_PIN);

//Pinos Motor 2 (topo)
//#define DIR_M2_PIN 4
//#define STEP_M2_PIN 5
///#define EN_M2_PIN 6

//A4988 motor2(DIR_M2_PIN, STEP_M2_PIN, EN_M2_PIN);

// variáveis para armazenar as funções
int modo = 1;
int lavagem = 0;
int cura = 0;
int tela = 1;
int nivel = 0;

//variáveis para temporizador
int currentMillis;
int previousMillis;
int tempoLav;
int tempoCur;
int tempo[2][4] = { { 0, 0, 0, 0 },    //digitos do tempo de lavagem
                    { 0, 0, 0, 0 } };  //digitos do tempo de cura
//indexadores da matriz tempo[][]
int col = 0;
int lin = 0;

char str[20];
char key;

void setup() {
  //Iniciar LCD 20x4
  lcd.begin(20, 4);
  Serial.begin(9600);
  //configurar interrupção para as teclas
  pinMode(ROW, INPUT_PULLUP);
  pinMode(COL1, OUTPUT);
  pinMode(COL2, OUTPUT);
  pinMode(COL3, OUTPUT);
  pinMode(COL4, OUTPUT);
  pinMode(COL5, OUTPUT);
  digitalWrite(COL1, HIGH);
  digitalWrite(COL2, HIGH);
  digitalWrite(COL3, HIGH);
  digitalWrite(COL4, HIGH);
  digitalWrite(COL5, HIGH);

  // Configure the A4988 driver
  pinMode(EN_M1_PIN, OUTPUT);
  digitalWrite(EN_M1_PIN, LOW);  // Enable the driver
  pinMode(SLEEP_M1_PIN, OUTPUT);
  digitalWrite(SLEEP_M1_PIN, HIGH);  // Wake up the driver
  // Set the speed and acceleration of the stepper motor
  motor1.setMaxSpeed(500);  // Maximum speed in steps per second
  motor1.setAcceleration(500);  // Acceleration in steps per second per second



}

void loop() {
  attachInterrupt(digitalPinToInterrupt(ROW), handleInterrupt, RISING);
  Telas(tela);


}  //Fim do loop

void Telas(int x){

  //TELA 1 - seleção do modo de operação
  if (x == 1) {
    lcd.setCursor(0, 0);
    lcd.print("Modo");
    lcd.setCursor(0, 1);
    lcd.print("3 - Cura");
    lcd.setCursor(0, 2);
    lcd.print("2 - Lavagem");
    lcd.setCursor(0, 3);
    lcd.print("1 - Lavagem e cura");

    //Sinalizador do modo selecionado > OK
    lcd.setCursor(0, abs(modo - 4));
    delay(200);
    lcd.print("                    ");
    delay(100);
    key = readKeypad();

    FunctionSelect();

  }  // fim tela 1

  //TELA 2 - configuração dos tempos
  else if (x == 2) {
    if (lavagem == 1) {
      lcd.setCursor(2, 0);
      lcd.print("Tempo de lavagem");
      lcd.setCursor(7, 1);
      sprintf(str, "%d%d:%d%d", tempo[0][0], tempo[0][1], tempo[0][2], tempo[0][3]);
      lcd.print(str);
    }  //lavagem

    if (cura == 1) {
      lcd.setCursor(2, 2);
      lcd.print("Tempo de cura");
      lcd.setCursor(7, 3);
      sprintf(str, "%d%d:%d%d", tempo[1][0], tempo[1][1], tempo[1][2], tempo[1][3]);
      lcd.print(str);
    }  //cura
    key = readKeypad();    
    TimerSelector(); 
  }  // fim tela 2

  //TELA 3 - Timer
  else if (x == 3) {
    if (lavagem == 1)
      Lavagem();
    else if (cura == 1)
      Cura();
  }  // fim tela 3

  else if (x == 4){
    delay(5000);
    tela = 1;
  }
}


void handleInterrupt() {
  static unsigned long lastDebounceTime = 0;
  unsigned long currentTime = millis();
  if (currentTime - lastDebounceTime > debounceDelay) {
    lastDebounceTime = currentTime;
    key = readKeypad();
    if (key != '\0') {
      buffer[bufferIndex] = key;
      bufferIndex = (bufferIndex + 1) % 5;  // ciclo do buffer
    }
  }
}

//checar tecla pressionada
char readKeypad() {
  const int numCols = 5;
  const int numRows = 1;
  const char keys[numRows][numCols] = {{ '1', '2', '3', '4', '5' }};  //1 - SEL; 2- DEC; 3 - INC; 4 - BEFORE; 5 - NEXT
  int colPins[numCols] = { COL1, COL2, COL3, COL4, COL5 };
  int rowPins[numRows] = { ROW };
  int i, j;

  // verificar cada coluna, uma por vez
  for (i = 0; i < numCols; i++) {
    digitalWrite(colPins[i], LOW);
    for (j = 0; j < numRows; j++) {
      if (digitalRead(rowPins[j]) == LOW) {
        delay(10);  // aguardar para estabilizar o sinal
        digitalWrite(colPins[i], HIGH);
        return keys[j][i];
      }
    }
    digitalWrite(colPins[i], HIGH);
  }
  // nenhuma tecla pressionada
  return '\0';
}

void FunctionSelect(){
  //SELECT pressionado
  if (key == '1') {
    lcd.clear();
    tela = 2;
    if (modo == 1) {
      lavagem = 1;
      cura = 1;
    }  //lavagem e cura
    else if (modo == 2) {
      lavagem = 1;
      cura = 0;
    }  //lavagem
    else if (modo == 3) {
      lavagem = 0;
      cura = 1;
      lin = 1;
    }  //cura
    delay(200);
  }
  //DEC pressionado
  else if (key == '2') {
    modo--;
    if (modo < 1)
      modo = 1;
  }
  //INC pressionado
  else if (key == '3') {
    modo++;
    if (modo > 3)
      modo = 3;
  }
}

/////////////// Temporizador ///////////////
void Timer() {
  //n=0 lavagem
  //n=1 cura
  int n = 2;
  if(lavagem == 0 && cura == 1){
    n = 1;
    lcd.setCursor(7, 1);
    lcd.print("CURA");
  //display da contagem
    lcd.setCursor(7, 2);
    sprintf(str, "%d%d:%d%d", tempo[n][0], tempo[n][1], tempo[n][2], tempo[n][3]);
    lcd.print(str);
    if (tempo[n][0] == 0 && tempo[n][1] == 0 && tempo[n][2] == 0 && tempo[n][3] == 0) {
      FimCura();
    }
  }
  else if(lavagem == 1){
    n = 0;
    lcd.setCursor(6, 1);
    lcd.print("LAVAGEM");
      //display da contagem
    lcd.setCursor(7, 2);
    sprintf(str, "%d%d:%d%d", tempo[n][0], tempo[n][1], tempo[n][2], tempo[n][3]);
    lcd.print(str);
    if (tempo[n][0] == 0 && tempo[n][1] == 0 && tempo[n][2] == 0 && tempo[n][3] == 0) {
      FimLavagem();
    } 
  }
  //contagem
  tempo[n][3]--;
  if (tempo[n][3] < 0) {
    tempo[n][3] = 9;
    tempo[n][2]--;
    if (tempo[n][2] < 0) {
      tempo[n][2] = 5;
      tempo[n][1]--;
      if (tempo[n][1] < 0) {
        tempo[n][1] = 9;
        tempo[n][0]--;
        if (tempo[n][0] < 0) {
          tempo[n][1] = 9;
          tempo[n][0] = 0;
        }
      }
    }
  }
}


//Sinalizador de dígito selecionado
void TimerSelector() {
  int i, j;

  if (col < 2)
    i = 7 + col;
  else
    i = 8 + col;

  if (modo == 3)
    j = 3;
  else
    j = 1 + 2 * lin;

  lcd.setCursor(i, j);
  lcd.print(" ");
  delay(100);
  lcd.setCursor(i, j);
  lcd.print(tempo[lin][col]);
  delay(200);

  if (key == '1') {
    //variáveis para verificar se o tempo está maior que o minímo
    tempoLav = (tempo[0][0]*10+tempo[0][1])*60+tempo[0][2]*10+tempo[0][3];
    tempoCur = (tempo[1][0]*10+tempo[1][1])*60+tempo[1][2]*10+tempo[1][3];
    int tempoMin = 60;

    if (modo == 1) {
      //Sinalizar timer menor que mínimo 
      if(tempoLav < tempoMin && tempoCur < tempoMin){
        for(int k = 0; k < 5; k++){
          lcd.setCursor(7,1);
          lcd.print("  :  ");
          lcd.setCursor(7,3);
          lcd.print("  :  ");
          delay(200);
          lcd.setCursor(7,1);
          sprintf(str, "%d%d:%d%d", tempo[0][0], tempo[0][1], tempo[0][2], tempo[0][3]);
          lcd.print(str);
          lcd.setCursor(7,3);
          sprintf(str, "%d%d:%d%d", tempo[1][0], tempo[1][1], tempo[1][2], tempo[1][3]);
          lcd.print(str);
          delay(200);
          col = 0;
          lin = 0;
        }
      }
        else if(tempoLav < tempoMin){
          for(int k = 0; k < 5; k++){
            lcd.setCursor(7,1);
            lcd.print("  :  ");
            delay(200);
            lcd.setCursor(7,1);
            sprintf(str, "%d%d:%d%d", tempo[0][0], tempo[0][1], tempo[0][2], tempo[0][3]);
            lcd.print(str);
            delay(200);
            col = 0;
            lin = 0;
         }
        }
        else if(tempoCur < tempoMin){
          for(int k = 0; k < 5; k++){
            lcd.setCursor(7,3);
            lcd.print("  :  ");
            delay(200);
            lcd.setCursor(7,3);
            sprintf(str, "%d%d:%d%d", tempo[1][0], tempo[1][1], tempo[1][2], tempo[1][3]);
            lcd.print(str);
            delay(200);
            col = 0;
            lin = 1;
          }
        }
        else{
          tela = 3;
          lcd.clear();
          delay(200);
          previousMillis = millis();
        }
      }
      else if(modo == 2){
        if(tempoLav < tempoMin){
          for(int k = 0; k < 5; k++){
            lcd.setCursor(7,1);
            lcd.print("  :  ");
            delay(200);
            lcd.setCursor(7,1);
            sprintf(str, "%d%d:%d%d", tempo[0][0], tempo[0][1], tempo[0][2], tempo[0][3]);
            lcd.print(str);
            delay(200);
            col = 0;
            lin = 0;
          }
        }
        else{
          tela = 3;
          lcd.clear();
          delay(200);
          previousMillis = millis();
        }
      }
      else if(modo == 3){
        if(tempoCur < tempoMin){
          for(int k = 0; k < 5; k++){
            lcd.setCursor(7,3);
            lcd.print("  :  ");
            delay(200);
            lcd.setCursor(7,3);
            sprintf(str, "%d%d:%d%d", tempo[1][0], tempo[1][1], tempo[1][2], tempo[1][3]);
            lcd.print(str);
            delay(200);
            col = 0;
            lin = 1;
          }
        }
        else{
          tela = 3;
          lcd.clear();
          delay(200);
          previousMillis = millis();
        }
      }
    }

  else if (key == '2') {
    tempo[lin][col]--;
    TimerDigits();
  } 
  else if (key == '3') {
    tempo[lin][col]++;
    TimerDigits();
  } 
  else if (key == '4') {
    col--;
    if (col < 0){
      if (modo == 1){
        if (lin == 0){
          col = 0;
        }
        else{
          col = 3;
          lin = 0;
        }
      }
      else
        col = 0;
    }
  } 
  else if (key == '5') {
    col++;
    if (col > 3){
      if (modo == 1){
        if (lin == 0){
          col = 0;
          lin = 1;
        }
        else{
          col = 3;
        }
      }
      else
        col = 3;
    }
  }
}

//Limitador de digitos
void TimerDigits() {
  if (col == 1 || col == 3) {
    if (tempo[lin][col] > 9) {
      tempo[lin][col] = 0;
    } else if (tempo[lin][col] < 0) {
      tempo[lin][col] = 9;
    }
  } 
  else {
    if (tempo[lin][col] > 5) {
      tempo[lin][col] = 0;
    } else if (tempo[lin][col] < 0) {
      tempo[lin][col] = 5;
    }
  } 
}


/////////////// Operação /////////////// em trabalho
void Lavagem() {
  //acender LED
 // if(nivel==0)
  //  EncherTanque();
  //inicia contagem
  
  timer2.start();
  timer2.update();
  Timer();
  LigarMotor1();
  //ligar motor
}

void FimLavagem() {
  lavagem = 0;
  //desliga motor
  //esvaziar tanque
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Lavagem Finalizada");
  motor1.stop();
  //piscaLED

  delay(5000);
  //apagaLED
  lcd.clear();
  if (cura == 0)
    tela = 4;
}

void Cura() {
  //liga torreta
  //acender LED

  //inicia contagem
  currentMillis = millis();

  if (currentMillis - previousMillis >= 1000) {
    previousMillis = currentMillis;
    Timer();
  }  
}

void FimCura() {
  cura = 0;
  //desliga motor

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Cura Finalizada");


  //piscaLED

  delay(5000);
  lcd.clear();
  //apagaLED
  tela = 4;
}

void EncherTanque() {
  lcd.setCursor(0,0);
  lcd.print("Enchendo o tanque");
  delay(15000);  //sensor de nível
  lcd.clear();
  nivel = 1;
}
int flag = 0;
void LigarMotor1() {

  // Send signals to the motor to start rotating
  
  if(flag == 0){
    motor1.moveTo(20000000);
    flag++;
  }
  while(motor1.distanceToGo() != 0) {
    motor1.run();
    currentMillis = millis();
    if (currentMillis - previousMillis >= 1000) {
      previousMillis = currentMillis;
      Timer();
      motor1.run();
    }  
  }
  
   digitalWrite(SLEEP_M1_PIN, LOW);
}
