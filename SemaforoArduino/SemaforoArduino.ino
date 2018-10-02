/*
 Name:		SemaforoArduino.ino
 Created:	02/10/2018 13:32:05
 Author:	paulo
*/
#include <LEDMatrixDriver.hpp>

const uint8_t LEDMATRIX_CS_PIN = 10;

const byte LEDMATRIX_WIDTH = 32;
const byte LEDMATRIX_HEIGHT = 32;
const byte LEDMATRIX_SEGMENTS = 16;

LEDMatrixDriver semafaro(LEDMATRIX_SEGMENTS, LEDMATRIX_CS_PIN);

unsigned long tempoAtual;

unsigned long tempoRecebimentoSerial;
float delayRecebimentoSerial = 10;
String valorRecebidoCompleto;
bool resetado;

byte quantidadeLigacoes = 16;
byte ligacoes[16][2] = {
	{1, 2}, // Base da Espinha com a Espinha do Meio
	{1, 11},
	{1, 14},
	{2, 17},
	{3, 4},
	{3, 17},
	{5, 6},
	{5, 17},
	{6, 7},
	{8, 9},
	{8, 17},
	{9, 10},
	{11, 12},
	{12, 13},
	{14, 15},
	{15, 16}
};

byte quantidadePontos = 17;
float raioCabeca = 2.5;

void setup() {
	Serial.begin(115200);

	Serial.println("Arduino ready");

	semafaro.setEnabled(true);
	// 0 = low, 10 = high
	semafaro.setIntensity(5);

	semafaro.clear();

	semafaro.display();

	resetarValorRecebido();
}

void alterarPixel(byte x, byte y, bool estado) {
	semafaro.setPixel(led_x(x, y), led_y(x, y), estado);
}

void construirValorRecebido() {
	if (Serial.available() > 0)
	{
		resetado = false;

		tempoRecebimentoSerial = tempoAtual;

		while (Serial.available() > 0)
		{
			char valorRecebido = Serial.read();

			valorRecebidoCompleto += valorRecebido;
		}
	}
	else if (!resetado && tempoAtual > tempoRecebimentoSerial + delayRecebimentoSerial)
	{
		processarValorRecebido();

		resetarValorRecebido();
	}
}

void processarValorRecebido() {
	String valores[17];
	int chave = 0;

	char valorRecebidoCompletoArray[150];
	valorRecebidoCompleto.toCharArray(valorRecebidoCompletoArray, 150);

	char * separadoPontoVirgula = strtok(valorRecebidoCompletoArray, ";");

	while (separadoPontoVirgula != NULL)
	{
		String separadoPontoVirgulaString(separadoPontoVirgula);

		valores[chave] = separadoPontoVirgulaString;

		chave++;

		separadoPontoVirgula = strtok(NULL, ";");
	}

	processarPixels(valores);

	processarLinhas(valores);

	processarCabeca(valores);
}

void processarPixels(String valores[]) {
	semafaro.clear();

	for (byte i = 0; i < quantidadePontos; i++)
	{
		int pontoAtual[2];
		pegarPonto(valores[i], pontoAtual);

		alterarPixel(pontoAtual[0], pontoAtual[1], true);
	}
}

void processarLinhas(String valores[]) {
	for (byte i = 0; i < quantidadeLigacoes; i++)
	{
		int ponto1[2];
		pegarPonto(valores[ligacoes[i][0] - 1], ponto1);

		int ponto2[2];
		pegarPonto(valores[ligacoes[i][1] - 1], ponto2);

		linha(
			ponto1[0], ponto1[1],
			ponto2[0], ponto2[1],
			true
		);
	}
}

void processarCabeca(String valores[]) {
	int cabeca[2];
	pegarPonto(valores[3], cabeca);

	circle(cabeca[0], cabeca[1], raioCabeca);
}

void pegarPonto(String pontoString, int ponto[2]) {
	char valorArray[150];
	pontoString.toCharArray(valorArray, 150);

	char * valorSeparado = strtok(valorArray, ",");

	int x = -1;
	int y = -1;

	while (valorSeparado != NULL)
	{
		int numero = String(valorSeparado).toInt();

		if (x == -1)
		{
			x = numero;
		}
		else
		{
			y = numero;
		}

		valorSeparado = strtok(NULL, ",");
	}

	ponto[0] = x;
	ponto[1] = y;
}

byte led_x(byte x, byte y) {
	return x + (y / 8) * 32;
}

byte led_y(byte x, byte y) {
	return y % 8;
}

void linha(byte x1, byte y1, byte x2, byte y2, bool state) {
	for (byte i = 0; i < 32; i++)
	{
		byte pathX = map(i, 0, 32, x1, x2);
		byte pathY = map(i, 0, 32, y1, y2);

		byte led_pathX = led_x(pathX, pathY);
		byte led_pathY = led_y(pathX, pathY);

		semafaro.setPixel(led_pathX, led_pathY, state);
	}
}

void circle(byte cx, byte cy, float radius)
{
	for (float r = 0; r <= radius; r += 0.5)
	{
		for (float angle = 0; angle < TWO_PI; angle += 0.1)
		{
			// ponto da circunferencia
			byte x = ceil(cx + r * cos(angle));
			byte y = ceil(cy + r * sin(angle));

			alterarPixel(x, y, true);
		}
	}
}

void resetarValorRecebido() {
	valorRecebidoCompleto = "";

	resetado = true;
}

void loop() {
	tempoAtual = millis();

	construirValorRecebido();

	semafaro.display();
}
