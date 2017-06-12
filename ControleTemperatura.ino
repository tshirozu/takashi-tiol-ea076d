

/* Definicao dos pinos utilizados
*/
#define PIN_BULB 2					// Pino de acionamento da lâmpada
#define PIN_FAN 6                 	// Pino de acionamento do cooler
#define PIN_SENSOR A1				// Pino ADC do sensor de temperatura

#include "TimerOne.h"
#include "Math.h"


#define MAX_BUFFER_SIZE 10

/*Global Variables */
int lastInput = 25;                		// Ultimo valor registrado no monitor serial; inicialização na temperatura ambiente
int tempSensor = 0;                		// Leitura do sensor
int deltaTemp = 0;                 		// Resultado da realimentacao
int fanPwmValue = 0;                 	// 0-255 => correspondente ao DutyCycle do sinal PWM
int bulbPwmValue = 0;                 	// 0-255 => correspondente ao DutyCycle do sinal PWM


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

  tempSensor = sensor2temp(analogRead(PIN_SENSOR));				//Conversao do valor do ADC para temperatura
  
  
  Serial.println("Temperatura: ");     //Print valor da temperatura
  Serial.println(tempSensor);
  
  
  deltaTemp = lastInput - tempSensor;   //Realimentacao
  Serial.println("Erro: ");
  Serial.println(deltaTemp);   
  Serial.println("------------------------------------");


  if(deltaTemp > 2)							//Limite inferior para reiniciar o aquecimento; só reinicia o aquecimento caso a temperatura esteja 3 graus abaixo do input
  {    
    digitalWrite(PIN_BULB,HIGH);
    digitalWrite(PIN_FAN,LOW);
    
  }
  else if(deltaTemp <=0 )					//Limite superior para desligar o aquecimento
  {   
    digitalWrite(PIN_BULB,LOW);
    digitalWrite(PIN_FAN,HIGH);
  }
    
}



int sensor2temp(int sensorValue)
{
  return (int)(((float)sensorValue-174.0)/(float)2.4)+25; 			//Linearizacao ao redor de 25ºC => 174 un. Slope = 2.4 un/ºC
}

void inputTemp()
{
  Serial.println("Input serial"); 
  int x = 0;
  sscanf(Buffer.data,"%d", &x);
  lastInput = x;                                                //Salva-se a variavel numa variavel global
  buffer_clean();  
  flag_serial_read = 0;
}


void setup() {  
  pinMode(PIN_FAN,OUTPUT);      		//Sinal digital do cooler
  pinMode(PIN_BULB,OUTPUT);     		//Sinal digital da lampada
  pinMode(PIN_SENSOR,INPUT);			//Pino do sensor	
  Serial.begin(9600);           		//Seta comunicacao UART
  
}


void loop() {
  
  Timer1.initialize(5000000);                                       // Chama interrupção periodica a cada 5s
  Timer1.attachInterrupt(ISR_timer);                                // Associa a interrupcao periodica a funcao ISR_timer

  if(flag_serial_read == 1)                                         //Se houve uma leitura no terminal serial
    inputTemp();                                                   
    
}



