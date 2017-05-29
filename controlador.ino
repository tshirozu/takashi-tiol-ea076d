
#define PIN_PWM 3
#define PIN_INTERRUPT 2

#include "TimerOne.h"
#include "Math.h"


#define MAX_BUFFER_SIZE 10

/*Global Variables */
int lastInput = 0;
int rpmSensor = 0;
int deltaRpm = 0;
int pwmValue = 0;
int sensorCounter = 0;

float controlGain = 1;
float rpm2pwm_constant = 255.0/2000.0;

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

void ISR_timer() {
  
  rpmSensor = sensorCounter*60/2; //2 helices, amostragem 2 segundos
  
  Serial.println("Sensor RPM: ");
  Serial.println(rpmSensor);
  sensorCounter = 0;
  
  deltaRpm = lastInput - rpmSensor;
  Serial.println("Erro: ");
  Serial.println(deltaRpm);
  if (pwmValue + rpm2pwm(deltaRpm*controlGain) < 255)
  {    
    pwmValue += rpm2pwm(deltaRpm*controlGain);    
  }
  else
  {    
    pwmValue = 255;
  }

  if(pwmValue < 0)
    pwmValue = 0;

  Serial.println("pwmValue: ");
  Serial.println(pwmValue);
  analogWrite(PIN_PWM,pwmValue);


  //Lê-se monitor serial

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

int rpm2pwm(float rpmValue)
{

  return int(rpmValue*rpm2pwm_constant);
}

float pwm2rpm(int pwmValue)
{
  return (float)pwmValue/rpm2pwm_constant;
}

void incInterruptCounter()
{
  sensorCounter += 1;  
}

void setup() {
  // put your setup code here, to run once:
  pinMode(PIN_PWM,OUTPUT);
  
  attachInterrupt(digitalPinToInterrupt(PIN_INTERRUPT),incInterruptCounter,RISING);
  Serial.begin(9600);
  
}

void loop() {
  // put your main code here, to run repeatedly:

  
  Timer1.initialize(1000000);                                       // Chama interrupção periodica
  Timer1.attachInterrupt(ISR_timer);                              // Associa a interrupcao periodica a funcao ISR_timer

  if(flag_serial_read == 1)  
    inputRpm();
    
}


void inputRpm()
{
  Serial.println("Input serial"); 
  int x = 0;
  sscanf(Buffer.data,"%d", &x);
  lastInput = x;
  buffer_clean();
  
  flag_serial_read = 0;
}

