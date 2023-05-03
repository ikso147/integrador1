#include <LiquidCrystal.h>

//Pinos LCD
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);


//Pinos Teclado 1x5
//const int linPins[5] = (A0, A1, A2, A3 , A4); ////////////////////////////////

//Pinos Motor 1 (topo)

//Pinos Motor 2 (base)

//Pinos do botões
int buttonInc = 18;
int buttonDec = 19;
int buttonLeft = 20;
int buttonRight = 21;
int buttonSelec = 2;

//Variáveis para controlar bouncing dos botões
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 200;

// variáveis para armazenar as funções
int modo = 1;
int lavagem = 0;
int cura = 0;
int tela = 1;

//variáveis para temporizador
int currentMillis;
int previousMillis;

int tempo[2][4] = {{0,0,0,0}, //digitos do tempo de lavagem
                  {0,0,0,0}}; //digitos do tempo de cura

char str[20];

int col = 0;
int lin = 0;

void setup() {
  //Iniciar LCD 20x4
  lcd.begin(20, 4);
}

void loop() {

  Telas(tela);
  //ativar interrupções para as teclas
  attachInterrupt(digitalPinToInterrupt(buttonInc), Increment, RISING);
  attachInterrupt(digitalPinToInterrupt(buttonDec), Decrement, RISING);
  attachInterrupt(digitalPinToInterrupt(buttonLeft), CursorLeft, RISING);
  attachInterrupt(digitalPinToInterrupt(buttonRight), CursorRight, RISING);
  attachInterrupt(digitalPinToInterrupt(buttonSelec), Select, RISING);
}


/////////////// Telas ///////////////
void Telas (int x){
  if (x == 1){ 
    lcd.setCursor(0, 0);
    lcd.print("Modo");
    lcd.setCursor(0, 1);
    lcd.print("3 - Cura");
    lcd.setCursor(0, 2);
    lcd.print("2 - Lavagem");
    lcd.setCursor(0, 3);
    lcd.print("1 - Lavagem e cura");

    FunctionSelector();
  } // seleção do modo de operação

  else if(x == 2){
    
    if(lavagem == 1){
      lcd.setCursor(2,0);
      lcd.print("Tempo de lavagem");
      lcd.setCursor(7,1);
      sprintf(str,"%d%d:%d%d", tempo[0][0],tempo[0][1],tempo[0][2],tempo[0][3]); 
      lcd.print(str);
    } //lavagem

   if(cura == 1){
      lcd.setCursor(2,2);
      lcd.print("Tempo de cura");
      lcd.setCursor(7,3);
      sprintf(str,"%d%d:%d%d", tempo[1][0],tempo[1][1],tempo[1][2],tempo[1][3]); 
      lcd.print(str);
    } //cura
    TimerSelector();
    
  } // configuração dos tempos

  else if(x == 3){
    if (lavagem == 1)
      Lavagem();
    else if(cura == 1)
      Cura();
  }// Timer

}


/////////////// Temporizador ///////////////
void Timer(int n){
  //n=0 lavagem
  //n=1 cura
  

  currentMillis = millis();
  if (currentMillis - previousMillis >= 1000){
    previousMillis = currentMillis;
    tempo[n][3]--;
  }
  if(tempo[n][3]<0){
    tempo[n][3]=9;
    tempo[n][2]--;
  }
  if(tempo[0][2]<0){
    tempo[n][2]=9;
    tempo[n][1]--;
  }
  if(tempo[n][1]<0){
    tempo[n][1]=9;
    tempo[n][0]--;
  }
  if(tempo[n][0]>9){
    tempo[n][0]=9;
  }

  lcd.setCursor(7,2);
  sprintf(str,"%d%d:%d%d", tempo[n][0],tempo[n][1],tempo[n][2],tempo[n][3]); 
  lcd.print(str);
  
  if(n==0){
    lcd.setCursor(6,1);
    lcd.print("LAVAGEM");
    if(tempo[n][0]==0 && tempo[n][1]==0 && tempo[n][2]==0 && tempo[n][3]==0){
      delay(2000);
      lavagem = 0;
      lcd.clear();
    }
  }
  else{
    lcd.setCursor(7,1);
    lcd.print("CURA");
      if(tempo[n][0]==0 && tempo[n][1]==0 && tempo[n][2]==0 && tempo[n][3]==0){
      delay(2000);
      cura=0;
      lcd.clear();
    }
  }

  
  
  if(tempo[n][0]==0 && tempo[n][1]==0 && tempo[n][2]==0 && tempo[n][3]==0)
    delay(3000);
  
}


/////////////// Operação ///////////////
void Lavagem(){
  Timer(0);
}
int Cura(){
  Timer(1);
}



/////////////// Sinalizadores de selecionados no display ///////////////
//Sinalizador do modo selecionado
void FunctionSelector(){
  lcd.setCursor(0,abs(modo-4));
  delay(300);
  lcd.print("                    ");
  delay(200);
}

//Sinalizador de dígito selecionado
void TimerSelector(){
  int i,j;
  
  if(col < 2)
    i = 7 + col;
  else
    i = 8 + col;

  if (modo == 3)
    j = 3;
  else
    j = 1+2*lin;

  lcd.setCursor(i,j);
  delay(300);
  lcd.print(" ");
  delay(200);
}


/////////////// Funções das Teclas ///////////////
//função tecla inc
void Increment(){
  //debounce das teclas
  if ((millis() - lastDebounceTime) > debounceDelay) {
    lastDebounceTime = millis();
    if (tela == 1){
      modo++;
      if (modo>3)
        modo =3;
      
    }
    else if(tela == 2){
    tempo[lin][col]++;
    }
  }
 //limitação dos dígitos no lcd
  if(tempo[lin][col]>9){
    tempo[lin][col]=9;
  }
  else if(tempo[lin][col]<0){
    tempo[lin][col]=0;
  }
}

//funçao tecla dec
void Decrement(){
  //debounce das teclas
  if ((millis() - lastDebounceTime) > debounceDelay) {
    lastDebounceTime = millis();
    if (tela == 1){
      modo--;
      if (modo<1)
        modo = 1;
    }
    else if(tela == 2){
    tempo[lin][col]--;
    }
  }
 //limitação dos dígitos no lcd
  if(tempo[lin][col]>9){
    tempo[lin][col]=9;
  }
  else if(tempo[lin][col]<0){
    tempo[lin][col]=0;
  }
}

//função tecla left
void CursorLeft(){
  //debounce das teclas
  if ((millis() - lastDebounceTime) > debounceDelay) {
    lastDebounceTime = millis();
    if (tela == 1){
    }
    else if(tela == 2){
      col--;
      if(col<0)
        col=0;
    }
  }
}

//função tecla right
void CursorRight(){
  //debounce das teclas
  if ((millis() - lastDebounceTime) > debounceDelay) {
    lastDebounceTime = millis();
    if (tela == 1){
    }
    else if(tela == 2){
      col++;
      if(col>3)
        col=3;
    }
  }
}

//função tecla sel
void Select(){
  //debounce das teclas
  if ((millis() - lastDebounceTime) > debounceDelay) {
  lastDebounceTime = millis();
    if(tela == 1){
      lcd.clear();
      tela = 2;
      if(modo == 1){
        lavagem = 1;
        cura = 1;
      } //lavagem e cura
      else if(modo == 2){
        lavagem = 1;
        cura = 0;
      } //lavagem
      else if(modo == 3){
        lavagem = 0;
        cura = 1;
        lin = 1;
      }//cura
    }
    else if(tela == 2){
      if(tela == 2){
        if(modo == 1){
        lin++;
        col=0;
          if(lin>1){
            tela = 3;
            lcd.clear();
            previousMillis = millis();
          }
        }
        else{
        tela = 3;
        lcd.clear(); 
        previousMillis = millis();
       }
      }
    }
  }
}


