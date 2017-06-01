

/* Definicao dos pinos utilizados
*/

#define PIN_PWM 3                 // Pino de PWM para o driver do motor DC
#define PIN_INTERRUPT 2           // Pino de interrupcao do sensor IR

#include "TimerOne.h"
#include "Math.h"


#define MAX_BUFFER_SIZE 10

/*Global Variables */
int lastInput = 0;                // Ultimo valor registrado no monitor serial
int rpmSensor = 0;                // Rotacao registrada pelo sensor
int deltaRpm = 0;                 // Resultado da realimentacao
int pwmValue = 0;                 // 0-255 => correspondente ao DutyCycle do sinal PWM
int sensorCounter = 0;            // Contador do numero de interrupcoes chamadas pelo sensor

float controlGain = 0.4;                    //Ganho proporcional do controlador
float rpm2pwm_constant = 255.0/5400.0;      //Constante de conversao 
                                            //Através de osciloscópio: Duty Cycle = 1  <==>  Rotacao = 5400 rpm

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
  
  rpmSensor = sensorCounter*120/2;    //Conversao do contador para RPM (2 helices, Tamostragem = 0.5s)
  sensorCounter = 0;                  //Reset contador
  Serial.println("Sensor RPM: ");     //Print valor da rotacao
  Serial.println(rpmSensor);
  
  
  deltaRpm = lastInput - rpmSensor;   //Realimentacao
  Serial.println("Erro: ");
  Serial.println(deltaRpm);   

  Serial.println("------------------------------------");
  if (pwmValue + rpm2pwm(deltaRpm*controlGain) < 255)   
  {    
    pwmValue += rpm2pwm(deltaRpm*controlGain);    //Incrementa-se a variavel de Duty Cycle
  }
  else
  {    
    pwmValue = 255;                               //Limitante superior da variavel
  }
  if(pwmValue < 0)                                //Limitante inferior da variavel
    pwmValue = 0;

  analogWrite(PIN_PWM,pwmValue);                  //Atualiza o Duty Cycle do sinal


  
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
}

/*
  Funcao rpm2pwm: Converte valor de RPM para valor PWM correspondente
*/

int rpm2pwm(float rpmValue)
{
  return int(rpmValue*rpm2pwm_constant);
}

/*
Funcao pwm2rpm: Converte valor de PWM para o valor em RPM correspondente
*/
float pwm2rpm(int pwmValue)
{
  return (float)pwmValue/rpm2pwm_constant;
}

/*
Interrupcao chamada pelo senor que incrementa um contador
*/
void incInterruptCounter()
{
  sensorCounter += 1;  
}

void setup() {  
  pinMode(PIN_PWM,OUTPUT);      //Sinal de PWM como OUTPUT  
  attachInterrupt(digitalPinToInterrupt(PIN_INTERRUPT),incInterruptCounter,RISING); //Sinal do sensor causa interrupcao na borda de SUBIDA
  Serial.begin(9600);           //Seta comunicacao UART
  
}

void loop() {
  
  Timer1.initialize(500000);                                      // Chama interrupção periodica a cada 0.5s
  Timer1.attachInterrupt(ISR_timer);                              // Associa a interrupcao periodica a funcao ISR_timer

  if(flag_serial_read == 1)                                       //Se houve uma leitura no terminal serial
    inputRpm();                                                   
    
}


void inputRpm()
{
  Serial.println("Input serial"); 
  int x = 0;
  sscanf(Buffer.data,"%d", &x);
  lastInput = x;                                                //Salva-se a variavel num variavel global
  buffer_clean();  
  flag_serial_read = 0;
}

