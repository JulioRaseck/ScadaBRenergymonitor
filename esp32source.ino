/*
  Modbus-Arduino Example - Test Holding Register (Modbus IP ESP32)
  Configure Holding Register (offset 100) with initial value 0xABCD
  You can get or set this holding register
  Copyright by André Sarmento Barbosa
  http://github.com/andresarmento/modbus-arduino
*/

// Bibliotecas utilizadas
#include <PZEM004Tv30.h>
#include <Modbus.h>           
#include <ModbusIP_ESP32.h>

PZEM004Tv30 pzem(&Serial2);   // Criando a comunicação com o PZEM na serial 2 do ESP32

// Modbus Registers Offsets (0-9999)
// Criando as variáveis responsáveis pela comunicação com o ScadaBR, cada uma recebe
// um offset que sera utilizado para a comunicação com o ScadaBR
const int TENSAO = 0;
const int CORRENTE = 1;
const int POTENCIA = 2;
const int ENERGIA = 3;
const int FP = 4;
const int RESET = 5;
const int ALERTA = 6;   // Esta variável eu não utilizo aqui no código fonte, eu criei ela apenas para
// realizar testes com alertas no ScadaBR, basicamente eu criei uma variável virtual para trabalhar com ela no ScadaBR.

// Variáveis utilizadas. Como eu configurei o ScadaBR para receber numeros inteiros e o PZEM retorna numeros
// com vírgulas eu utilizo uma variável float para receber os dados do PZEM e uma int com um fator de multiplicação
// para não perder as casas decimais. No ScadaBR utilizar o inverso do fator de multiplicação para retornar ao valor original.
float tensaoF, correnteF, potenciaF, energiaF, fpF;
int tensaoI, correnteI, potenciaI, energiaI, fpI, resetEnergy = 0;

//ModbusIP object
ModbusIP mb;

void setup() {
  Serial.begin(115200);


  mb.config("nome da rede", "senha");

  // Laço para realizar a conexão Wi-Fi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Configuração da faixa de registro de cada variável.
  // addHreg para as variáveis analógicas, ou seja, são os valores com vírgula.
  // addCoil para as variáveis binárias
  mb.addHreg(TENSAO, 0xABCD);
  mb.addHreg(CORRENTE, 0xABCD);
  mb.addHreg(POTENCIA, 0xABCD);
  mb.addHreg(ENERGIA, 0xABCD);
  mb.addHreg(FP, 0xABCD);
  mb.addCoil(RESET);
  mb.addCoil(ALERTA);
}

void loop() {
  //Call once inside loop() - all magic here
  mb.task();

  
  resetEnergy = mb.Coil(RESET);   // aqui minha variável resetEnergy receber o status da minha variável RESET
  // deste modo eu posso realizar comandos no ScadaBR para alterar o valor desta variável aqui no código fonte,
  // possibilitando realizar comandos como acender um led, ligar e desligar equipamentos, dentre outros.
  // Neste caso, a condição abaixo irá resetar o consumo acumulado sempre que eu enviar um comando a partir do ScadaBR, 
  // o comando nada mais é que setar a variável para 1, também é possível criar uma interface gráfica no ScadaBR para
  // criar um botão virtual responsável por enviar esse comando.
  if (resetEnergy == 1) {
    //Serial.print("Reset Energy");
    pzem.resetEnergy();     // Comando para resetar o consumo de energia acumulado
    mb.Coil(RESET, 0);      // Retorna a variável para zero no ScadaBR
    resetEnergy = 0;        // Retorna a variável para zero no ESP32
  }

  // O restante do código é responsável pela leitura e envio dos dados para o ScadaBR

  tensaoF = pzem.voltage();     // Recebe o valor da tensão lida pelo PZEM
  if (!isnan(tensaoF)) {
    tensaoI = 10 * tensaoF;     // Aplica o fator de multiplicação para criar uma variável inteira
    //Serial.println(tensaoF);
  }
  else {
    tensaoI = 0;
    //Serial.println("Error reading voltage");
  }
  mb.Hreg(TENSAO, tensaoI);     // Atualiza o valor da tensão no ScadaBR

  correnteF = pzem.current();
  if (!isnan(correnteF)) {
    correnteI = 10 * correnteF;
    //Serial.println(correnteF);
  }
  else {
    correnteI = 0;
    //Serial.println("Error reading current");
  }
  mb.Hreg(CORRENTE, correnteI);

  potenciaF = pzem.power();
  if (!isnan(potenciaF)) {
    potenciaI = 10 * potenciaF;
    //Serial.println(potenciaF);
  }
  else {
    potenciaI = 0;
    //Serial.println("Error reading power");
  }
  mb.Hreg(POTENCIA, potenciaI);

  energiaF = pzem.energy();
  if (!isnan(energiaF)) {
    energiaI = 100 * energiaF;
    //Serial.println(energiaF);
  }
  else {
    energiaI = 0;
    //Serial.println("Error reading energy");
  }
  mb.Hreg(ENERGIA, energiaI);

  fpF = pzem.pf();
  if (!isnan(fpF)) {
    fpI = 100 * fpF;
    //Serial.println(fpF);
  }
  else {
    fpI = 0;
    //Serial.println("Error reading power factor");
  }
  mb.Hreg(FP, fpI);
}
