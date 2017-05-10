/*
 * v1p6
 
 Todo: Comentar
 
 Added readMatrixKeyboard function;
 
 Added functions:
   WRITE: escreve as primeiras 4 paginas da memoria 24C16n
   TESTE: escreve as primeiras 4 paginas da memoria 24C16n
   READ: le as primeiras 4 paginas da memoria 24C16n
 
 
 
 * PINS:
 * A0 SENSOR
 * A4 SDA azul
 * A5 SCL branco
 * 
 * D11 -> R0
 * D10 -> R1
 * D9  -> R2
 * D8  -> R3
 
 * D5 -> C2
 * D6 -> C1
 * D7 -> C0

 * D2 -> LED
 */

#define PIN_SENSOR A0
#define PIN_R0 11
#define PIN_R1 10
#define PIN_R2 9
#define PIN_R3 8
#define PIN_C0 7
#define PIN_C1 6
#define PIN_C2 5
#define PIN_LED 2

 //#########################################################

#include <TimerOne.h> //Biblioteca do timer
#include <stdio.h>
#include <Wire.h>
#include <stdint.h>

/* Variaveis globais */
  
int flag_RecordAutomatico = 0; 
unsigned long lastRecord = 0;                                         //Timestamp 
int memCount = 0;                                                     //Contador da posicao da memoria
int maxBounceCount = 1;                                               //Contador para debouncer
int sensor_luz = 0;                                                   //Valor do sensor depois do ADC
int flagLastButtonState[12] = {0, 0, 0, 0, 0 ,0 ,0 ,0 ,0 ,0 ,0 ,0};   //Ultimo estado de cada botao do teclado matricial
char matrix_buffer[10];                                               //Buffer para armazenar os comandos do teclado matricial
int matrix_buffer_counter = 0;                                        //Contador do buffer


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
#define MAX_BUFFER_SIZE 15
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


/* Flags globais para controle de processos da interrupcao */
volatile int flag_check_command = 0;

/* Rotinas de interrupcao */

/* Ao receber evento da UART */
void serialEvent() {
  char c;
  
  while (Serial.available()>0) {
    c = Serial.read();

    if (c=='\n') {
      buffer_add('\0'); /* Se recebeu um fim de linha, coloca um terminador de string no buffer */
      flag_check_command = 1;
//      Serial.println("0\n");
    } else {
//     Serial.println(c);
     buffer_add(c);
    }
  }
}

/* Funcoes internas ao void main() */
const byte DEVADDR = 0x50;

void setup() {
  /* Inicializacao */
 
  buffer_clean();                                                   //Limpa o buffer da comunicacao serial
  flag_check_command = 0;                                           //Reset flag command serial
  matrix_buffer_counter = 0;                                        //Reset buffer da matrix
  sprintf(matrix_buffer,"         ");                                   
  Wire.begin();                                                     //Inicia comunicao I2C com memoria
  Serial.begin(9600);

  for(int i = 0; i<16*24 ;i+=16)
    eeprom_erase_page(DEVADDR,i);                                   //Apaga conteudo da memoria
      
  pinMode(PIN_SENSOR,INPUT);                                        //Set os pins
  pinMode(PIN_R0,OUTPUT);
  pinMode(PIN_R1,OUTPUT);
  pinMode(PIN_R2,OUTPUT);
  pinMode(PIN_R3,OUTPUT);
  pinMode(PIN_LED,OUTPUT);

  pinMode(PIN_C0,INPUT);
  pinMode(PIN_C1,INPUT);
  pinMode(PIN_C2,INPUT);
}


void loop() {
  
  char out_buffer[10];
  int flag_write = 0;
  int matrixKeyboard = readMatrixKeyboard();                        //Realiza varredura no teclado matricial, retornando indice

  if(matrixKeyboard != -1)
  {
    matrixKeyboard = convMatrixKeyboard(matrixKeyboard);            //Converte os indices em valores
    if(matrixKeyboard == 11)
    {
      matrix_buffer[matrix_buffer_counter] = 42;                    //Escrita do caracter *
    }
    else if(matrixKeyboard == 10)
    {
      matrix_buffer[matrix_buffer_counter] = 35;                    //Escrita do caracter #
    }
    else
    {
      matrix_buffer[matrix_buffer_counter] = matrixKeyboard + 48;   //Escrita de 0~9
    }
    if(matrix_buffer_counter >= 2)
    {
      matrix_buffer_counter = 0;                                    //Reset matrix buffer se mais que 3 digitos invalidos
    }
    else{
      matrix_buffer_counter++;  
    }
    
    Serial.println(matrix_buffer);    
  }
   
  
  Timer1.initialize(1000000);                                       // Chama interrupção periodica
  Timer1.attachInterrupt(serialEvent);                              // Associa a interrupcao periodica a funcao ISR_timer

  /*
   * COMANDOS DO TECLADO MATRICIAL
   */

   if(str_cmp(matrix_buffer,"#1*",3))                               //Comando #1*
   {
      digitalWrite(PIN_LED,HIGH);                                   //Pisca o LED
      delay(2000);
      digitalWrite(PIN_LED,LOW);
      sprintf(matrix_buffer,"         ");                           //Reset buffer de comando
   }
   if(str_cmp(matrix_buffer,"#2*",3))                               //Comando #2*
   {
      Serial.println("Begin RECORD process... \n");
      sensor_luz = analogRead(PIN_SENSOR);                          //Leitura do sensor 0V => 0 ; 5V => 1023
      sprintf(out_buffer, "%d", sensor_luz);
      flag_write = 1;                                               //Set flag para print do valor do sensor
      eeprom_erase_page(DEVADDR,memCount);                          //Apaga a posicao da memoria
      eeprom_write_page(DEVADDR,memCount,out_buffer, 4);            //Escreve a posicao na posicao da memoria

      if(memCount < 24*(16-1))                                      //Incrementa contador
        memCount+=16;
        
      Serial.println("End RECORD process\n");
      sprintf(matrix_buffer,"         ");                           //Reset buffer de comando
      
   }
   if(str_cmp(matrix_buffer,"#3*",3))                               //Comando #3*
   {
      flag_RecordAutomatico = 1;                                    //Set flag para execucao de RECORD Automatico
      sprintf(matrix_buffer,"         ");                           //Reset buffer de comando
      
   }
   if(str_cmp(matrix_buffer,"#4*",3))
   {
      flag_RecordAutomatico = 0;                                    //Reset flag para execucao de RECORD Automatico
      sprintf(matrix_buffer,"         ");                           //Reset buffer de comando
      
   }

   /* A flag_check_command permite separar a recepcao de caracteres
   *  (vinculada a interrupca) da interpretacao de caracteres. Dessa forma,
   *  mantemos a rotina de interrupcao mais enxuta, enquanto o processo de
   *  interpretacao de comandos - mais lento - nao impede a recepcao de
   *  outros caracteres. Como o processo nao 'prende' a maquina, ele e chamado
   *  de nao-preemptivo.
   */

   
  /*
   * COMANDOS DO TERMINAL SERIAL
   */

   
  if (flag_check_command == 1) {
    if (str_cmp(Buffer.data, "PING", 4) ) {
      sprintf(out_buffer, "PONG\n");
      
      flag_write = 1;
    }
    if (str_cmp(Buffer.data, "ID", 2) ) {
      sprintf(out_buffer, "DATALOGGER DA ZOEIRA \n");
      flag_write = 1;
    }

    if (str_cmp(Buffer.data, "TESTE", 5) ) {
        buffer_clean();
        Serial.println("Begin writing process....\n");

        byte msg0[] = "0123456789";
        byte msg1[] = "9876543210";
        byte msg2[] = "ABCDEFGHIJ";
        byte msg3[] = "KLMNOPQRST";
        
        eeprom_write_page(DEVADDR,0x000,msg0, sizeof(msg0));
        eeprom_write_page(DEVADDR,0x010,msg1, sizeof(msg1));
        eeprom_write_page(DEVADDR,0x020,msg2, sizeof(msg2));
        eeprom_write_page(DEVADDR,0x030,msg3, sizeof(msg3));
        
        Serial.println("Memory written\n");    
        
    }

    
    if (str_cmp(Buffer.data, "READ", 4) ) {
        buffer_clean();
        Serial.println("Begin reading process... \n");

        char readMsg[4][16];
        
        for (int itMsg = 0 ; itMsg<24; itMsg++)
        {
          char outBuffer[16];
          sprintf(outBuffer,"\nMessage %d is:\n",itMsg);
          Serial.println(outBuffer);
          
          for (int i = itMsg*16; i < itMsg*16+16; i++) {
            byte b = eeprom_read_byte(DEVADDR, i);
            readMsg[itMsg][i] = b;
            Serial.print(readMsg[itMsg][i]);
          }
          Serial.println("\n");
        }       
        Serial.println("Memory readed\n");
    }


    
    if (str_cmp(Buffer.data,"MEASURE",7))
    {
      Serial.println("Begin MEASURE process... \n");
      sensor_luz = analogRead(PIN_SENSOR);           //leitura do sensor 0V => 0 ; 5V => 1023
      sprintf(out_buffer, "%d", sensor_luz);
      flag_write = 1;
      
      buffer_clean();
      Serial.println("End MEASURE process\n");
    }
    
    if (str_cmp(Buffer.data,"MEMSTATUS",9))
    {
      Serial.println("Begin MEMSTATUS process... \n");
      sprintf(out_buffer, "%d", memCount/16);
      flag_write = 1;
            
      buffer_clean();
      Serial.println("End MEMSTATUS process\n");
    }
    
    if (str_cmp(Buffer.data,"RESET",5))
    {
      Serial.println("Begin RESET process... \n");

      for(int i = 0; i<16*24 ;i+=16)
        eeprom_erase_page(DEVADDR,i);      

      memCount = 0;
      buffer_clean();
      Serial.println("End RESET process\n");
    }
    
    if (str_cmp(Buffer.data,"RECORD",7))
    {
      Serial.println("Begin RECORD process... \n");
      sensor_luz = analogRead(PIN_SENSOR);           //leitura do sensor 0V => 0 ; 5V => 1023
      sprintf(out_buffer, "%d", sensor_luz);
      flag_write = 1;
      eeprom_erase_page(DEVADDR,memCount);
      eeprom_write_page(DEVADDR,memCount,out_buffer, 4);

      

      if(memCount < 24*(16-1))
        memCount+=16;
        
      
      buffer_clean();
      Serial.println("End RECORD process\n");
    }
    
    if (str_cmp(Buffer.data,"GET ",4))
    {
      Serial.println("Begin GET process... \n");
      int x = 0;
      sscanf(Buffer.data, "%*s %d", &x);
            
      int memPage = x;
      
      int j = 0;
      for (int i = memPage*16; i < memPage*16+4; i++) {
            byte b = eeprom_read_byte(DEVADDR, i);
            char aux = b;
                        
            out_buffer[j] = aux;
            j++;       
            Serial.print(aux);
      }
      Serial.println("\n");     
      flag_write = 1;     
      Serial.println("End GET process\n");
    }
    flag_check_command = 0;
    
  }

  /* Posso construir uma dessas estruturas if(flag) para cada funcionalidade
   *  do sistema. Nesta a seguir, flag_write e habilitada sempre que alguma outra
   *  funcionalidade criou uma requisicao por escrever o conteudo do buffer na
   *  saida UART.
   */
  if (flag_write == 1) {
    Serial.write(out_buffer);
    buffer_clean();
    flag_write = 0;
  }

  if(flag_RecordAutomatico == 1 && (millis()-lastRecord) > 1000)
  {
      Serial.println("Begin RECORD process... \n");
      sensor_luz = analogRead(PIN_SENSOR);           //leitura do sensor 0V => 0 ; 5V => 1023
      sprintf(out_buffer, "%d", sensor_luz);
      flag_write = 1;
      eeprom_erase_page(DEVADDR,memCount);
      eeprom_write_page(DEVADDR,memCount,out_buffer, 4);     
      if(memCount < 24*(16-1))
        memCount+=16;       
      Serial.println("End RECORD process\n");
      lastRecord = millis();
  }
}

int convMatrixKeyboard(int a)
{
  switch(a)
  {
    case 9:
      return 10;
    case 10:
      return 0;
    case 11:
      return 11;
    default:
      return a+1;  
      
  }
}

int readMatrixKeyboard()
{
    
    
    int numCol = 3;
    int numRow = 4;

    int i = 0;
    int j = 0;
    int k = 0;

    int temp = 0;


    for(i=0;i<numRow;i++)
    {

      switch(i)
      {
        case 0:
          digitalWrite(PIN_R0,HIGH);
          digitalWrite(PIN_R1,LOW);
          digitalWrite(PIN_R2,LOW);
          digitalWrite(PIN_R3,LOW);          
          break;
        case 1:          
          digitalWrite(PIN_R0,LOW);
          digitalWrite(PIN_R1,HIGH);
          digitalWrite(PIN_R2,LOW);
          digitalWrite(PIN_R3,LOW);          
          break;
        case 2:
          digitalWrite(PIN_R0,LOW);
          digitalWrite(PIN_R1,LOW);
          digitalWrite(PIN_R2,HIGH);
          digitalWrite(PIN_R3,LOW);          
          break;
        case 3:
          digitalWrite(PIN_R0,LOW);
          digitalWrite(PIN_R1,LOW);
          digitalWrite(PIN_R2,LOW);
          digitalWrite(PIN_R3,HIGH);          
          break;
        }
        delay(100);
      
      for(j=0;j<numCol;j++)
      {
        switch(j)
        {
          case 0:
            
            for(k=0;k<maxBounceCount;k++)
            {
              
              temp = digitalRead(PIN_C0);

              
              if (temp != flagLastButtonState[3*i+j]) {

                j = 0;
                flagLastButtonState[3*i+j] = temp;
                if(temp == 1)                
                  return(3*i+j);               
                else
                  return -1;
              }          
   
            }           
            break;
          case 1:
            
            for(k=0;k<maxBounceCount;k++)
            {
              
              temp = digitalRead(PIN_C1);
              
              
              if (temp != flagLastButtonState[3*i+j]) {

                j = 1;
                flagLastButtonState[3*i+j] = temp;
                if(temp == 1)                
                  return(3*i+j);               
                else
                  return -1;               
              }
            }           
            break;
          case 2:          
           
            for(k=0;k<maxBounceCount;k++)
            {
              
              temp = digitalRead(PIN_C2);
              
              
              if (temp != flagLastButtonState[3*i+j]) {

                j = 2;
                flagLastButtonState[3*i+j] = temp;
                if(temp == 1)                
                  return(3*i+j);               
                else
                  return -1;               
              }                                              
            }
            break;        
        }      
      }
    }
    
    return -1;
}

void eeprom_write_page(byte deviceaddress, unsigned eeaddr,
                      const byte * data, byte length)
{
   // Three lsb of Device address byte are bits 8-10 of eeaddress
   byte devaddr = deviceaddress | ((eeaddr >> 8) & 0x07);
   byte addr    = eeaddr;
   Wire.beginTransmission(devaddr);
   Wire.write(int(addr));
   for (int i = 0; i < length; i++) {
       Wire.write(data[i]);
   }
   Wire.endTransmission();
   delay(10);
}

void eeprom_erase_page(byte deviceaddress,unsigned eeaddr)
{
   byte msgf[16] = {
       0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
       0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
   };
   eeprom_write_page(deviceaddress, eeaddr, msgf, 16);
}



int eeprom_read_byte(byte deviceaddress, unsigned eeaddr)
{
   byte rdata = -1;

   // Three lsb of Device address byte are bits 8-10 of eeaddress
   byte devaddr = deviceaddress | ((eeaddr >> 8) & 0x07);
   byte addr    = eeaddr;

   Wire.beginTransmission(devaddr);
   Wire.write(int(addr));
   Wire.endTransmission();
   Wire.requestFrom(int(devaddr), 1);
   if (Wire.available()) {
       rdata = Wire.read();
   }
   return rdata;
}
