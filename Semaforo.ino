//########################## PINs ##########################
#define PIN_VERMELHO_CARRO 13
#define PIN_AMARELO_CARRO 12
#define PIN_VERDE_CARRO 11

#define PIN_VERDE_PEDESTRE 7
#define PIN_VERMELHO_PEDESTRE 6

#define BUZZER_PIN 2

#define SENSOR_PIN A5
#define BUTTON_PIN 5
//##########################################################################

#include <TimerOne.h> //Biblioteca do timer

//########################## Declaração variáveis ##########################
int estado = 0;

int pedestre_apertou = 0;     // SE botão apertado = 1 , caso contrário = 0 

int sensor_luz;               // ADC sensor luz: 0V => 0; 5V => 1023

int espera = 0;               //Variável auxiliar de espera para delay entre mudanças de estados

int pisca_flag = 0;           //Flag indicando o último estado do led VERMELHO_PEDESTRE
int pisca_flag_noite = 0;     //Flag indicando o último estado do led AMARELO_CARRO
int wait_pisca = 0;           //Variável auxiliar de espera para pisca do led VERMELHO_PEDESTRE
int wait_pisca_noite = 0;     //Variável auxliar de espera para pisca do led AMARELO_CARRO

int espera_sensor = 0;        //Variável auxiliar de espera para transição modo dia   -> noite
int espera_sensor_noite = 0;  //Variável auxiliar de espera para transição modo noite -> dia


//########################## Declaração de limiares ##########################

int sensor_threshold = 500;       //Limiar para toggle do ADC Sensor de luz: entre 0 ~ 1023

int espera_delayBotao = 100;      //Valor máximo do contador para delay do botão e acionamento do semáforo
int espera_amareloCarro = 300;    //Valor máximo do contador para estado AMARELO para o CARRO
int espera_verdePedestre = 500;  //Valor máximo do contador para estado VERDE para o PEDESTRE

int espera_atencaoPedestre = 300; //Valor máximo do contador para estado VERMELHO PISCANTE PEDESTRE

int espera_pisca_pedestre = 30;    //Período para pisca VERMLEHO PEDESTRE
int espera_pisca_noite = 20;       //Período para pisca AMARELO CARRO

int espera_diaParaNoite = 200;    //Valor máximo do contador para transição dia => noite
int espera_noiteParaDia = 200;    //Valor máximo do contador para transição noite => dia

//############################################################################



void ISR_timer() { 
  
  switch(estado){             //instruções separadas por estado
    
    /* ########################## ESTADO 0 ##################################
    Descrição: 
      -Estado padrão
        1- PIN VERMELHO_PEDESTRE  = HIGH
        2- PIN VERDE_CARRO        = HIGH
        3- Todos outros PINs      = LOW
      
      -Transição para:
        -Estado 1: Caso botão seja apertado
        -Estado 5: Caso sensor indique baixa luz por um dado tempo determinado por "espera_diaParaNoite"
             
    #########################################################################
    */
    case 0:
      
      //Muda de estado if sensor_luz < sensor_threshold
      if(sensor_luz < sensor_threshold){
        espera_sensor = espera_sensor + 1;
    
        if(espera_sensor == espera_diaParaNoite){
          estado = 5;
          espera_sensor = 0;
        }          
      }
      else{
        espera_sensor = 0;
      }
        
      digitalWrite(PIN_VERMELHO_CARRO, LOW);
      digitalWrite(PIN_AMARELO_CARRO, LOW);
      digitalWrite(PIN_VERDE_CARRO, HIGH); 
      digitalWrite(PIN_VERMELHO_PEDESTRE, HIGH);
      digitalWrite(PIN_VERDE_PEDESTRE, LOW);
      digitalWrite(BUZZER_PIN, LOW);

      //Troca de estado
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
      
      //Troca de estado
      if (espera==espera_delayBotao){
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
        -Estado 3 após um determinado tempo determinado pela variável "espera"
             
    #################################################################
    */

    case 2:

      digitalWrite(PIN_VERMELHO_CARRO, LOW);
      digitalWrite(PIN_AMARELO_CARRO, HIGH);
      digitalWrite(PIN_VERDE_CARRO, LOW); 
      digitalWrite(PIN_VERMELHO_PEDESTRE, HIGH);
      digitalWrite(PIN_VERDE_PEDESTRE, LOW);
      digitalWrite(BUZZER_PIN, LOW);

      espera = espera + 1 ;
      //Troca de estado
      if (espera==espera_amareloCarro){
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
        -Estado 4 Após um determinado tempo determinado pela variável "espera_verdePedestre"
             
    #################################################################
    */
    case 3:

      digitalWrite(PIN_VERMELHO_CARRO, HIGH);
      digitalWrite(PIN_AMARELO_CARRO, LOW);
      digitalWrite(PIN_VERDE_CARRO, LOW); 
      digitalWrite(PIN_VERMELHO_PEDESTRE, LOW);
      digitalWrite(PIN_VERDE_PEDESTRE, HIGH);
      digitalWrite(BUZZER_PIN, HIGH);
      espera = espera + 1 ;

      //Troca de estado
      if (espera==espera_verdePedestre){
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
      digitalWrite(BUZZER_PIN, LOW);

      espera = espera + 1 ;
      wait_pisca = wait_pisca + 1;
      

      //Toggle para pisca vermelho
      if (wait_pisca == espera_pisca_pedestre){
        wait_pisca = 0;

        if (pisca_flag==0){
           digitalWrite(PIN_VERMELHO_PEDESTRE, HIGH);
           digitalWrite(BUZZER_PIN, HIGH);
           pisca_flag = 1;
        }
        else if (pisca_flag == 1){
           digitalWrite(PIN_VERMELHO_PEDESTRE, LOW);
           digitalWrite(BUZZER_PIN, LOW);
           pisca_flag = 0;
        }
      }

      //Troca de estado
      if (espera == espera_atencaoPedestre){
        espera = 0;
        estado = 0;
      }
      break;
      
    /* ########################## ESTADO 5 ##########################
    Descrição: 
      -Estado noturno
        1- PIN AMARELO_CARRO       = Pisca
        2- Todos outros PINs       = LOW
      
      -Transição para:
        -Estado 0: Caso sensor indique alta luz por um dado tempo determinado por "espera_sensor"
             
    #################################################################
    */

      case 5:
        
          digitalWrite(PIN_VERMELHO_CARRO, LOW);
          digitalWrite(PIN_VERDE_CARRO, LOW);
    
          digitalWrite(PIN_VERMELHO_PEDESTRE,LOW);
          digitalWrite(PIN_VERDE_PEDESTRE, LOW);
          digitalWrite(BUZZER_PIN, LOW);

          
          wait_pisca_noite = wait_pisca_noite + 1;
          
    
          //Toggle para pisca amarelo
          if (wait_pisca_noite == espera_pisca_noite){
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
          
          //Muda de estado if sensor_luz > sensor_threshold
          if(sensor_luz >= sensor_threshold){
            espera_sensor_noite = espera_sensor_noite + 1;
        
            if(espera_sensor_noite == espera_noiteParaDia){
              estado = 0;
              espera_sensor_noite = 0;
            }
              
          }
          else{
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
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(SENSOR_PIN,INPUT);
  
  Serial.begin(9600);          //  setup serial

}

void loop() { 
  pedestre_apertou = digitalRead(BUTTON_PIN);   //leitura do estado do botão
  sensor_luz = analogRead(SENSOR_PIN);           //leitura do sensor 0V => 0 ; 5V => 1023
  
  Serial.println(sensor_luz);             // debug sensor_luz

  
  Timer1.initialize(10000);                       // Chama interrupção periodica
  Timer1.attachInterrupt(ISR_timer);              // Associa a interrupcao periodica a funcao ISR_timer

}
