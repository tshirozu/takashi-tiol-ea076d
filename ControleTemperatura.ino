

/* Definicao dos pinos utilizados
*/
#define PIN_BULB_PWM 2
#define PIN_FAN_PWM 6                 // Pino de PWM para o driver do motor DC
#define PIN_SENSOR A1

#include "TimerOne.h"
#include "Math.h"


#define MAX_BUFFER_SIZE 10

/*Global Variables */
int lastInput = 25;                // Ultimo valor registrado no monitor serial
int tempSensor = 0;                // Rotacao registrada pelo sensor
int deltaTemp = 0;                 // Resultado da realimentacao
int fanPwmValue = 0;                 // 0-255 => correspondente ao DutyCycle do sinal PWM
int bulbPwmValue = 0;                 // 0-255 => correspondente ao DutyCycle do sinal PWM


//Funcoes para a comunicao serial
int flag_serial_read = 0;

/* Rotina auxiliar para comparacao de strings */
int str_cmp(char *s1, char *s2, int len) {
  /* Compare two strings up to length len. Return 1 if they are
   *  equal, and 0 otherwise.
   */
  int i;
  for (i=0; i<len; i++) {
    if (s1[i] != s2[i]) return 0;
    if (s1[i] == '\0') return 1;
  }
  return 1;
}


/* Buffer de dados recebidos */

typedef struct {
  volatile char data[MAX_BUFFER_SIZE];
  unsigned int tam_buffer;
} serial_buffer;

volatile serial_buffer Buffer;

/* Limpa buffer */
void buffer_clean() {
  Buffer.tam_buffer = 0;
}

/* Adiciona caractere ao buffer */
int buffer_add(char c_in) {
  if (Buffer.tam_buffer < MAX_BUFFER_SIZE) {
    Buffer.data[Buffer.tam_buffer++] = c_in;
    return 1;
  }
  return 0;
}

/*
  Interrupcao periodica
*/

void ISR_timer() {

    //Lê-se monitor serial para inputs do usuario
  char c;
  while (Serial.available()>0) {
    c = Serial.read();

    if (c=='\n') {
      buffer_add('\0'); /* Se recebeu um fim de linha, coloca um terminador de string no buffer */
      flag_serial_read = 1;
    } else {
     buffer_add(c);
    }
  }

  tempSensor = sensor2temp(analogRead(PIN_SENSOR));
  
  
  Serial.println("Temperatura: ");     //Print valor da rotacao
  Serial.println(tempSensor);
  
  
  deltaTemp = lastInput - tempSensor;   //Realimentacao
  Serial.println("Erro: ");
  Serial.println(deltaTemp);   
  Serial.println("------------------------------------");


  if(deltaTemp > 2)
  {
    fanPwmValue = 0;
    bulbPwmValue = 255;
    digitalWrite(PIN_BULB_PWM,HIGH);
    digitalWrite(PIN_FAN_PWM,LOW);
    
  }
  else if(deltaTemp <=0 )
  {
    fanPwmValue = 255;
    bulbPwmValue = 0;
    digitalWrite(PIN_BULB_PWM,LOW);
    digitalWrite(PIN_FAN_PWM,HIGH);
  }
    
  




  

}

/*
  Funcao rpm2pwm: Converte valor de RPM para valor PWM correspondente
*/




void setup() {  
  pinMode(PIN_FAN_PWM,OUTPUT);      //Sinal de PWM como OUTPUT    
  pinMode(PIN_BULB_PWM,OUTPUT);      //Sinal de PWM como OUTPUT    
  pinMode(PIN_SENSOR,INPUT);
  Serial.begin(9600);           //Seta comunicacao UART
  
}

int sensor2temp(int sensorValue)
{
  return (int)(((float)sensorValue-174.0)/(float)2.4)+25; //Linearizacao ao redor de 25ºC => 174 un. Slope = 2.4 un/ºC
}

void loop() {
  
  Timer1.initialize(5000000);                                      // Chama interrupção periodica a cada 5s
  Timer1.attachInterrupt(ISR_timer);                              // Associa a interrupcao periodica a funcao ISR_timer

  if(flag_serial_read == 1)                                       //Se houve uma leitura no terminal serial
    inputTemp();                                                   
    
}


void inputTemp()
{
  Serial.println("Input serial"); 
  int x = 0;
  sscanf(Buffer.data,"%d", &x);
  lastInput = x;                                                //Salva-se a variavel num variavel global
  buffer_clean();  
  flag_serial_read = 0;
}

