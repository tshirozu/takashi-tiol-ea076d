/*
 * v1p2
 
 Added functions:
   WRITE: escreve as primeiras 4 paginas da memoria 24C16n
   TESTE: escreve as primeiras 4 paginas da memoria 24C16n
   READ: le as primeiras 4 paginas da memoria 24C16n
 
 
 
 * PINS:
 * A4 SDA
 * A5 SCL
 */


#define PIN_SENSOR A0

 //#########################################################

#include <TimerOne.h> //Biblioteca do timer
#include <stdio.h>
#include <Wire.h>

/* Variaveis globais */

int sensor_luz = 0;
int reading = 0;


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

/* Processo de bufferizacao. Caracteres recebidos sao armazenados em um buffer. Quando um caractere
 *  de fim de linha ('\n') e recebido, todos os caracteres do buffer sao processados simultaneamente.
 */

/* Buffer de dados recebidos */
#define MAX_BUFFER_SIZE 15
typedef struct {
  volatile char data[MAX_BUFFER_SIZE];
  unsigned int tam_buffer;
} serial_buffer;

/* Teremos somente um buffer em nosso programa, O modificador volatile
 *  informa ao compilador que o conteudo de Buffer pode ser modificado a qualquer momento. Isso
 *  restringe algumas otimizacoes que o compilador possa fazer, evitando inconsistencias em
 *  algumas situacoes (por exemplo, evitando que ele possa ser modificado em uma rotina de interrupcao
 *  enquanto esta sendo lido no programa principal).
 */
volatile serial_buffer Buffer;

/* Todas as funcoes a seguir assumem que existe somente um buffer no programa e que ele foi
 *  declarado como Buffer. Esse padrao de design - assumir que so existe uma instancia de uma
 *  determinada estrutura - se chama Singleton (ou: uma adaptacao dele para a programacao
 *  nao-orientada-a-objetos). Ele evita que tenhamos que passar o endereco do
 *  buffer como parametro em todas as operacoes (isso pode economizar algumas instrucoes PUSH/POP
 *  nas chamadas de funcao, mas esse nao eh o nosso motivo principal para utiliza-lo), alem de
 *  garantir um ponto de acesso global a todas as informacoes contidas nele.
 */

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
      Serial.println("0\n");
    } else {
     Serial.println(c);
     buffer_add(c);
    }
  }
}

/* Funcoes internas ao void main() */
const byte DEVADDR = 0x50;

void setup() {
  /* Inicializacao */
  
  
  buffer_clean();
  flag_check_command = 0;
  Wire.begin();
  Serial.begin(9600);

  pinMode(PIN_SENSOR,INPUT);
}


void loop() {
  int x, y;
  char out_buffer[10];
  int flag_write = 0;
  

  
  
  Timer1.initialize(1000000);                       // Chama interrupção periodica
  Timer1.attachInterrupt(serialEvent);              // Associa a interrupcao periodica a funcao ISR_timer

  /* A flag_check_command permite separar a recepcao de caracteres
   *  (vinculada a interrupca) da interpretacao de caracteres. Dessa forma,
   *  mantemos a rotina de interrupcao mais enxuta, enquanto o processo de
   *  interpretacao de comandos - mais lento - nao impede a recepcao de
   *  outros caracteres. Como o processo nao 'prende' a maquina, ele e chamado
   *  de nao-preemptivo.
   */

   
  if (flag_check_command == 1) {
    if (str_cmp(Buffer.data, "PING", 4) ) {
      sprintf(out_buffer, "PONG\n");
      
      flag_write = 1;
    }

    if(str_cmp(Buffer.data,"ERASE",5)){
      buffer_clean();
      Serial.println("Begin erasing process....\n");
      eeprom_erase_page(DEVADDR,0x000);
      eeprom_erase_page(DEVADDR,0x010);
      eeprom_erase_page(DEVADDR,0x020);
      eeprom_erase_page(DEVADDR,0x030);
      Serial.println("Memory erased\n");
    }

    if (str_cmp(Buffer.data, "WRITE", 5) ) {
        buffer_clean();
        Serial.println("Begin writing process....\n");

        byte msg0[] = "This is msg 0";
        byte msg1[] = "Hello World!";
        byte msg2[] = "Howdy!";
        byte msg3[] = "LeeeroyJenkins";
        
        eeprom_write_page(DEVADDR,0x000,msg0, sizeof(msg0));
        eeprom_write_page(DEVADDR,0x010,msg1, sizeof(msg1));
        eeprom_write_page(DEVADDR,0x020,msg2, sizeof(msg2));
        eeprom_write_page(DEVADDR,0x030,msg3, sizeof(msg3));
        
        Serial.println("Memory written\n");    
        
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
        
        for (int itMsg = 0 ; itMsg<4; itMsg++)
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

    if (str_cmp(Buffer.data, "SENSOR", 5) ) {
        sensor_luz = analogRead(PIN_SENSOR);           //leitura do sensor 0V => 0 ; 5V => 1023
        sprintf(out_buffer, "%d", sensor_luz);
        flag_write = 1;
    }
    flag_check_command = 0;


    
    if (str_cmp(Buffer.data,"MEASURE",7))
    {
      Serial.println("Begin MEASURE process... \n");


      
      buffer_clean();
      Serial.println("End MEASURE process\n");
    }
    
    if (str_cmp(Buffer.data,"MEMSTATUS",9))
    {
      Serial.println("Begin MEMSTATUS process... \n");


      
      buffer_clean();
      Serial.println("End MEMSTATUS process\n");
    }
    
    if (str_cmp(Buffer.data,"RESET",5))
    {
      Serial.println("Begin RESET process... \n");


      
      buffer_clean();
      Serial.println("End RESET process\n");
    }
    
    if (str_cmp(Buffer.data,"RECORD",7))
    {
      Serial.println("Begin RECORD process... \n");

      
      buffer_clean();
      Serial.println("End RECORD process\n");
    }
    
    if (str_cmp(Buffer.data,"GET ",4))
    {
      Serial.println("Begin GET process... \n");



      
      buffer_clean();
      Serial.println("End GET process\n");
    }
    
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
