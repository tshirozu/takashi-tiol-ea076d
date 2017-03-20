#define PIN_VERMELHO_CARRO 13
#define PIN_AMARELO_CARRO 12
#define PIN_VERDE_CARRO 11

#define PIN_VERDE_PEDESTRE 7
#define PIN_VERMELHO_PEDESTRE 6

#define PIN_SENSOR 2

#define INPUT_BUTTON 5


#include <TimerOne.h>

int estado = 0;
int pedestre_apertou = 0;
int sensor_luz = 1 ; // 1=> COM LUZ ; 0=> SEM LUZ
int espera = 0;
int pisca_flag = 0;
int pisca_flag_noite = 0;
int wait_pisca = 0;
int wait_pisca_noite = 0;

int espera_sensor = 0;
int espera_sensor_noite = 0;

void ISR_timer() {
  // Esta implementacao usa uma estrutura de if-then-else explicita

  
  
  

  switch(estado){
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
    case 1:

      espera = espera + 1 ;

      if (espera==20){
        estado = 2;
        espera = 0;
      }



      break;

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
  // put your setup code here, to run once:
  pinMode(PIN_VERMELHO_CARRO, OUTPUT);
  pinMode(PIN_AMARELO_CARRO, OUTPUT); 
  pinMode(PIN_VERDE_CARRO, OUTPUT);
  pinMode(PIN_VERDE_PEDESTRE, OUTPUT);
  pinMode(PIN_VERMELHO_PEDESTRE, OUTPUT);
  pinMode(INPUT_BUTTON, INPUT);
  pinMode(PIN_SENSOR,INPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
  
  pedestre_apertou = digitalRead(INPUT_BUTTON);
  sensor_luz = digitalRead(PIN_SENSOR);

  
  Timer1.initialize(10000); // Interrupcao a cada 100ms
  Timer1.attachInterrupt(ISR_timer); // Associa a interrupcao periodica a funcao ISR_timer




  //botão foi apertado  
  //digitalWrite(PIN_VERDE_PEDESTRE, HIGH); 
  //digitalWrite(PIN_VERMELHO_PEDESTRE, HIGH);
}
