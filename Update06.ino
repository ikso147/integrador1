#include <LiquidCrystal.h>
#include <AccelStepper.h>
#include <Ticker.h>

/////////////////////////////////////// PINOS //////////////////////////////////////
//Sensor da tampa

//Pinos Teclado 1x5
#define ROW 2 // precisar ser pino com interrupção (verificar datasheet do arduino)
#define COL1 3
#define COL2 4
#define COL3 5
#define COL4 6
#define COL5 7

//MotorBottom
#define STEP_M1_PIN 9
#define DIR_M1_PIN 10

//MotorTop
#define STEP_M2_PIN 12
#define DIR_M2_PIN 13

//LED UV 30 ~ 38?

//Solenoid
#define SOLENOID_PIN 20

//Bomba d'água
#define PUMP_PIN 21

//LCD 
#define RS_PIN 26
#define ENABLE_LCD_PIN 27
#define D4 28
#define D5 29
#define D6 30
#define D7 31

///////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////// /////// /////////////////////////////////////////

LiquidCrystal lcd(RS_PIN, ENABLE_LCD_PIN, D4, D5, D6, D7);

AccelStepper motorBottom(AccelStepper::DRIVER, STEP_M1_PIN, DIR_M1_PIN);
AccelStepper motorTop(AccelStepper::DRIVER, STEP_M2_PIN, DIR_M2_PIN);

///////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////// Variáveis globais //////////////////////////////////
//Variáveis para os botões
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 400;
char buffer[5];       // buffer para armazenar as teclas pressionadas
int bufferIndex = 0;  // índice atual do buffer

// variáveis para armazenar as funções
int tela = 1;
int linha = 1;
int lavagem = 0;
int cura = 0;
int setNivel = 0;
int volume = 0;// 1 - Baixo; 2 - Medio; 3 - Alto
int aux = 0;
int RPM = 0;

//variáveis para temporizador
unsigned long currentMillis;
int previousMillis;
unsigned long tempoLav;
unsigned long tempoCur;
int tempo[2][4] = { { 0, 0, 0, 0 },    //digitos do tempo de lavagem
                    { 0, 0, 0, 0 } };  //digitos do tempo de cura
//indexadores da matriz tempo[][]
int col = 0;
int lin = 0;

char str[20];
char key;

///////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////Setup e Loop//////////////////////////////////////////
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

  pinMode(PUMP_PIN, OUTPUT); //configurar bomba d'agua
  pinMode(SOLENOID_PIN, OUTPUT); //configurar solenoid

  //pinMode(LEDS_UV, OUTPUT); // configurar LEDS


  // Configuração do motor do topo
  motorTop.setMaxSpeed(500);  // Velocidade máxima em passos por segundo
  motorTop.setAcceleration(100);  // Aceleração em passos por segundo


}

void loop() {
  attachInterrupt(digitalPinToInterrupt(ROW), handleInterrupt, RISING);
  Telas(tela);


}  //Fim do loop

///////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////// TELAS ////////////////////////////////////////////
void Telas(int x){

  //TELA 1 - seleção do modo de operação
  if (x == 1) {
    
    if (aux == 0){
      lcd.setCursor(0, 0);
      lcd.print("modo");
      lcd.setCursor(0, 1);
      lcd.print("3 - Cura");
      lcd.setCursor(0, 2);
      lcd.print("2 - Lavagem");
      lcd.setCursor(0, 3);
      lcd.print("1 - Lavagem e cura");
      //Sinalizador do modo selecionado
      lcd.setCursor(0, abs(linha - 4));
      delay(200);
      lcd.print("                    ");
      key = readKeypad();
      FunctionSelect();
      delay(200);
    }
    else if (aux == 1){
      lcd.setCursor(0, 0);
      lcd.print("Nivel do tanque");
      lcd.setCursor(0, 1);
      lcd.print("3 - Alto");
      lcd.setCursor(0, 2);
      lcd.print("2 - Medio");
      lcd.setCursor(0, 3);
      lcd.print("1 - Baixo");
      //Sinalizador do linha selecionado
      lcd.setCursor(0, abs(linha - 4));
      delay(200);
      lcd.print("                    ");
      key = readKeypad();
      VolumeSelect();
      delay(200);
    }
    else if (aux == 2){
      lcd.setCursor(0, 0);
      lcd.print("RPM");
      lcd.setCursor(0, 1);
      lcd.print("3 - Alto");
      lcd.setCursor(0, 2);
      lcd.print("2 - Medio");
      lcd.setCursor(0, 3);
      lcd.print("1 - Baixo");
      //Sinalizador do linha selecionado
      lcd.setCursor(0, abs(linha - 4));
      delay(200);
      lcd.print("                    ");
      key = readKeypad();
      MotorSpeedSelect();
      delay(200);
    }
    else
      tela = 2;
  }
  // fim tela 1

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
    if (lavagem == 1){
      Lavagem();
    }
    else if (cura == 1)
      Cura();
  }  // fim tela 3

  else if (x == 4){
    delay(5000);
    tela = 1;
  }
}
///////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////// Teclado /////////////////////////////////////////
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

//Selecionador da função - função das teclas na tela 1
void FunctionSelect(){
  //SELECT pressionado
  if (key == '1') {
    lcd.clear();
    aux = 1;
    if (linha == 1) {
      lavagem = 1;
      cura = 1;
    }  //lavagem e cura
    else if (linha == 2) {
      lavagem = 1;
      cura = 0;
    }  //lavagem
    else if (linha == 3) {
      lavagem = 0;
      cura = 1;
      lin = 1;
      aux = 3;
    }  //cura
    linha = 1;
    delay(200);
  }
  //DEC pressionado
  else if (key == '2') {
    linha--;
    if (linha < 1)
      linha = 1;
  }
  //INC pressionado
  else if (key == '3') {
    linha++;
    if (linha > 3)
      linha = 3;
  }
}

void VolumeSelect(){
  //SELECT pressionado
  if (key == '1') {
    lcd.clear();
    aux = 2;
    if (linha == 1) 
      setNivel = 500; //baixo
    else if (linha == 2)
      setNivel = 1000; // medio
    else if (linha == 3)
      setNivel = 1500; //alto
    linha = 1;
    delay(200);
  }
  //DEC pressionado
  else if (key == '2') {
    linha--;
    if (linha < 1)
      linha = 1;
  }
  //INC pressionado
  else if (key == '3') {
    linha++;
    if (linha > 3)
      linha = 3;
  }
}

void MotorSpeedSelect(){
  //SELECT pressionado
  if (key == '1') {
    lcd.clear();
    aux = 3;
    if (linha == 1)
      RPM = 500;
    else if (linha == 2) 
      RPM = 800;
    else if (linha == 3)
      RPM = 1000;
    linha = 1;
    delay(200);
  }
  //DEC pressionado
  else if (key == '2') {
    linha--;
    if (linha < 1)
      linha = 1;
  }
  //INC pressionado
  else if (key == '3') {
    linha++;
    if (linha > 3)
      linha = 3;
  }
}

//Sinalizador de dígito selecionado - função das teclas na tela 2
void TimerSelector() {
  int i, j;

  if (col < 2)
    i = 7 + col;
  else
    i = 8 + col;

  if (cura == 1 && lavagem == 0)
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

    if (lavagem == 1 && cura == 1) {
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
      else if(lavagem == 1 && cura == 0){
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
      else if(lavagem == 0 && cura == 1){
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
      if (lavagem == 1 && cura == 1){
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
      if (lavagem == 1 && cura == 1){
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
///////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////// Temporizador /////////////////////////////////////
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

Ticker timerTicker(Timer, 1000, 0, MILLIS); //ticker para chamar a função Timer() a cada 1s


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


////////////////////////////////////// Operação ////////////////////////////////////////
void Lavagem() {
  
  EncherTanque(); //encher o tanque até nível desejado
  Timer(); //display do timer
  motorBottom.setMaxSpeed(RPM);  // Maximum speed in steps per second
  motorBottom.setAcceleration(500);  // Acceleration in steps per second per second
  //ligar motor
  motorBottom.move(tempoLav*2000); //valor grande o suficiente para não terminar os passos antes do timer
  timerTicker.start();
  while(motorBottom.distanceToGo() != 0) {
    motorBottom.run();
    timerTicker.update();//atualiza display
  }
  motorBottom.stop();
}

void FimLavagem() {
  lavagem = 0;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Esvaziando tanque");
  //esvaziar tanque
  /*
  digitalWrite(SOLENOID_PIN, HIGH); // abrir valvula da solenoide
  while(volume != 0)
  {
  lcd.setCursor(0, 0);
  lcd.print("Esvaziando Tanque");
  delay(200);
  lcd.clear();
  delay(200);
  }
  digitalWrite(SOLENOID_PIN, LOW); // fechar valvula da solenoide
  */
  delay(5*setNivel);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Lavagem Finalizada");
  delay(5000);
  lcd.clear();
  if (cura == 0)
    tela = 4;
}

void Cura() {

  /*
  sensor da tampa
  //liga torreta
  //acender LED
  */

  Timer();//display do timer

  motorTop.move(tempoCur*2000); //valor grande o suficiente para não terminar os passos antes do timer
  timerTicker.start();
  while(motorTop.distanceToGo() != 0) {
    motorTop.run();
    timerTicker.update(); //atualiza display
  }
  motorTop.stop();
}


void FimCura() {
  cura = 0;
  //desliga motor
  //desliga torreta

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

/*while(nivel != setNivel){
  digitalWrite(PUMP_PIN, HIGH); //ativa bomba
}
  digitalWrite(PUMP_PIN, LOW); //desativa bomba
*/
  delay(setNivel*3);  //sensor de nível
  lcd.clear();

}


