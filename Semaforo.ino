//########################## PINs ##########################
#define PIN_VERMELHO_CARRO 13
#define PIN_AMARELO_CARRO 12
#define PIN_VERDE_CARRO 11

#define PIN_VERDE_PEDESTRE 7
#define PIN_VERMELHO_PEDESTRE 6

#define PIN_SENSOR 2
#define INPUT_BUTTON 5
//#########################################################


#include <TimerOne.h> //Biblioteca do timer


int estado = 0;

int pedestre_apertou = 0;     // SE botão apertado = 1 , caso contrário = 0 
int sensor_luz = 1 ;          // 1=> COM LUZ ; 0=> SEM LUZ

int espera = 0;               //Variável auxiliar de espera para delay entre mudanças de estados

int pisca_flag = 0;           //Flag indicando o último estado do led VERMELHO_PEDESTRE
int pisca_flag_noite = 0;     //Flag indicando o último estado do led AMARELO_CARRO
int wait_pisca = 0;           //Variável auxiliar de espera para pisca do led VERMELHO_PEDESTRE
int wait_pisca_noite = 0;     //Variável auxliar de espera para pisca do led AMARELO_CARRO

int espera_sensor = 0;        //Variável auxiliar de espera para transição modo dia   -> noite
int espera_sensor_noite = 0;  //Variável auxiliar de espera para transição modo noite -> dia

void ISR_timer() { 
  
  switch(estado){             //instruções separadas por estado
    
    /* ########################## ESTADO 0 ##########################
    Descrição: 
      -Estado padrão
        1- PIN VERMELHO_PEDESTRE  = HIGH
        2- PIN VERDE_CARRO        = HIGH
        3- Todos outros PINs      = LOW
      
      -Transição para:
        -Estado 1: Caso botão seja apertado
        -Estado 5: Caso sensor indique baixa luz por um dado tempo determinado por "espera_sensor"
             
    #################################################################
    */
    case 0:
      if(sensor_luz == 0){
        espera_sensor = espera_sensor + 1;
    
        if(espera_sensor == 200){
          estado = 5;
          espera_sensor = 0;
        }
          
      }
      else if (sensor_luz == 1){
        espera_sensor = 0;
      }
        
      digitalWrite(PIN_VERMELHO_CARRO, LOW);
      digitalWrite(PIN_AMARELO_CARRO, LOW);
      digitalWrite(PIN_VERDE_CARRO, HIGH); 
      digitalWrite(PIN_VERMELHO_PEDESTRE, HIGH);
      digitalWrite(PIN_VERDE_PEDESTRE, LOW);
 
      if(pedestre_apertou == 1)
         estado = 1;     
      
      break;
      
      
    /* ########################## ESTADO 1 ##########################
    Descrição: 
      -Introduz delay após o apertar do botão
      
      -Transição para:
        -Estado 2: Após um determinado tempo determinado pela variável "espera"
             
    #################################################################
    */
    case 1:

      espera = espera + 1 ;

      if (espera==20){
        estado = 2;
        espera = 0;
      }



      break;
      
    /* ########################## ESTADO 2 ##########################
    Descrição: 
      -Estado de alerta para os veículos
        1- PIN VERMELHO_PEDESTRE  = HIGH
        2- PIN AMARELO_CARRO      = HIGH
        3- Todos outros PINs      = LOW
      
      -Transição para:
        -Estado 3 Após um determinado tempo determinado pela variável "espera"
             
    #################################################################
    */

    case 2:

      digitalWrite(PIN_VERMELHO_CARRO, LOW);
      digitalWrite(PIN_AMARELO_CARRO, HIGH);
      digitalWrite(PIN_VERDE_CARRO, LOW); 
      digitalWrite(PIN_VERMELHO_PEDESTRE, HIGH);
      digitalWrite(PIN_VERDE_PEDESTRE, LOW);

      espera = espera + 1 ;

      if (espera==250){
        estado = 3;
        espera = 0;
      }

     
      break;
      
    /* ########################## ESTADO 3 ##########################
    Descrição: 
      -Estado de liberado para pedestre
        1- PIN VERDE_PEDESTRE      = HIGH
        2- PIN VERMELHO_CARRO      = HIGH
        3- Todos outros PINs       = LOW
      
      -Transição para:
        -Estado 4 Após um determinado tempo determinado pela variável "espera"
             
    #################################################################
    */
    case 3:

      digitalWrite(PIN_VERMELHO_CARRO, HIGH);
      digitalWrite(PIN_AMARELO_CARRO, LOW);
      digitalWrite(PIN_VERDE_CARRO, LOW); 
      digitalWrite(PIN_VERMELHO_PEDESTRE, LOW);
      digitalWrite(PIN_VERDE_PEDESTRE, HIGH);

      espera = espera + 1 ;

      if (espera==600){
        estado = 4;
        espera = 0;
      }
      break;
      
    /* ########################## ESTADO 4 ##########################
    Descrição: 
      -Estado de atenção para o pedestre
        1- PIN VERMELHO_PEDESCRE   = Pisca
        2- PIN VERMELHO_CARRO      = HIGH
        3- Todos outros PINs       = LOW
      
      -Transição para:
        -Estado 0 Após um determinado tempo determinado pela variável "espera"
             
    #################################################################
    */

    case 4:

      digitalWrite(PIN_VERMELHO_CARRO, HIGH);
      digitalWrite(PIN_AMARELO_CARRO, LOW);
      digitalWrite(PIN_VERDE_CARRO, LOW); 

      digitalWrite(PIN_VERDE_PEDESTRE, LOW);

      espera = espera + 1 ;
      wait_pisca = wait_pisca + 1;
      

      
      if (wait_pisca == 20){
        wait_pisca = 0;

        if (pisca_flag==0){
           digitalWrite(PIN_VERMELHO_PEDESTRE, HIGH);
           pisca_flag = 1;
        }
        else if (pisca_flag == 1){
           digitalWrite(PIN_VERMELHO_PEDESTRE, LOW);
           pisca_flag = 0;
        }
      }

      if (espera == 300){
        espera = 0;
        estado = 0;
      }
      break;
      
    /* ########################## ESTADO 4 ##########################
    Descrição: 
      -Estado de atenção para o pedestre
        1- PIN VERMELHO_PEDESCRE   = Pisca
        2- PIN VERMELHO_CARRO      = HIGH
        3- Todos outros PINs       = LOW
      
      -Transição para:
        -Estado 0 Após um determinado tempo determinado pela variável "espera"
             
    #################################################################
    */

      case 5:
        
          digitalWrite(PIN_VERMELHO_CARRO, LOW);
          digitalWrite(PIN_VERDE_CARRO, LOW);
    
          digitalWrite(PIN_VERMELHO_PEDESTRE,LOW);
          digitalWrite(PIN_VERDE_PEDESTRE, LOW);
    
          
          wait_pisca_noite = wait_pisca_noite + 1;
          
    
          
          if (wait_pisca_noite == 20){
            wait_pisca_noite = 0;
    
            if (pisca_flag_noite==0){
               digitalWrite(PIN_AMARELO_CARRO, HIGH);
               pisca_flag_noite = 1;
            }
            else if (pisca_flag_noite == 1){
               digitalWrite(PIN_AMARELO_CARRO, LOW);
               pisca_flag_noite = 0;
            }
          }
          
          if(sensor_luz == 1){
            espera_sensor_noite = espera_sensor_noite + 1;
        
            if(espera_sensor_noite == 10){
              estado = 0;
              espera_sensor_noite = 0;
            }
              
          }
          else if (sensor_luz == 0){
            espera_sensor_noite = 0;
          }  
        break;
      
  }
}


void setup() {
  //Setup dos PINs utilizados
  pinMode(PIN_VERMELHO_CARRO, OUTPUT); 
  pinMode(PIN_AMARELO_CARRO, OUTPUT); 
  pinMode(PIN_VERDE_CARRO, OUTPUT);
  pinMode(PIN_VERDE_PEDESTRE, OUTPUT);
  pinMode(PIN_VERMELHO_PEDESTRE, OUTPUT);
  pinMode(INPUT_BUTTON, INPUT);
  pinMode(PIN_SENSOR,INPUT);

}

void loop() { 
  pedestre_apertou = digitalRead(INPUT_BUTTON);   //leitura do estado do botão
  sensor_luz = digitalRead(PIN_SENSOR);           //leitura do sensor

  
  Timer1.initialize(10000);                       // Chama interrupção a cada 100ms
  Timer1.attachInterrupt(ISR_timer);              // Associa a interrupcao periodica a funcao ISR_timer

}
