/*
 * v1p1
 * 
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

void setup() {
  /* Inicializacao */
  
  
  buffer_clean();
  flag_check_command = 0;
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

    if (str_cmp(Buffer.data, "WRITE", 5) ) {
        buffer_clean();
        Serial.println("3\n");
        Wire.beginTransmission(80); // transmit to device #80 (0b 0101 0000)
        delay(50);
        Wire.write(0x01);     // endereco dentro da memoria
        delay(50);
        Wire.write(0x02);     
        delay(50);
        Wire.write(0x03);     // dados     
        delay(50);
        Serial.println("@@@\n");

        Serial.println(Wire.endTransmission());
        Serial.println("!!!\n");

     
        
    }
    
    if (str_cmp(Buffer.data, "READ", 4) ) {
        buffer_clean();
        Serial.println("4\n");
        Wire.beginTransmission(80); // transmit to device #80 (0b 101 0000)
        
        Wire.write(0x01);     // endereco dentro da memoria
        Wire.write(0x02);     // endereco dentro da memoria



        
        delay(50);    
        Serial.println("5\n");    
        Wire.requestFrom(80, 2);
        Serial.println("6\n");

        if(Wire.available())
          reading = Wire.read();
        else
          reading = 666;

        Wire.endTransmission();
        
        Serial.println("7\n");
        Serial.println(reading);
        Serial.println("@@@@\n");
        
        
    }

    if (str_cmp(Buffer.data, "SENSOR", 5) ) {
      sensor_luz = analogRead(PIN_SENSOR);           //leitura do sensor 0V => 0 ; 5V => 1023
      sprintf(out_buffer, "%d", sensor_luz);
      flag_write = 1;
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

}
