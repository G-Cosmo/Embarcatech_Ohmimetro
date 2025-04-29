#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "pico/bootrom.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define ADC_PIN 28 // GPIO para o voltímetro
#define Botao_A 5  // GPIO para botão A
#define Botao_B 6

int volatile R_conhecido = 10000;   // Resistor de 10k ohm
int R_conhecido1 = 10000;   // primeiro valor de referencia
int R_conhecido2 = 8200;    // segundo valor de referencia
float R_x = 0.0;           // Resistor desconhecido
int ADC_RESOLUTION = 4095; // Resolução do ADC (12 bits)
float R_aproximado = 0.0; //  Resistencia aproximada
char faixa1[10] = ""; //primeira faixa de cor
char faixa2[10]= "";  //segunda faixa de cor
char faixa3[10] = ""; //terceira faixa de cor
uint32_t last_time = 0;


// Trecho para modo BOOTSEL com botão B


void gpio_irq_handler(uint gpio, uint32_t events)
{
    uint32_t current_time = to_ms_since_boot(get_absolute_time());

  if(current_time - last_time > 200)
  {
    if(gpio == Botao_B){
      reset_usb_boot(0, 0);
    }
    else if(gpio == Botao_A)
    {
      if(R_conhecido == R_conhecido1)
        {R_conhecido = R_conhecido2;}
      else if (R_conhecido == R_conhecido2)
        {R_conhecido = R_conhecido1;}
    }

    last_time = current_time;
  }

}

float aproximar_valor(float resistencia) {

  float e24[] = {10, 11, 12, 13, 15, 16, 18, 20, 22, 24, 27, 30,
    33, 36, 39, 43, 47, 51, 56, 62, 68, 75, 82, 91};

  float melhor_valor = 0.0f;  // 
  float menor_diferenca = 1e9; // valor muito grande
  float fator = 1.0f; // 

  // Varre diferentes potências de 10 (desde 0.1 ohm até 1M ohm por exemplo)
  for (int decada = -1; decada <= 6; decada++) { 
    fator = powf(10.0f, decada); // 10^decada

    for (int i = 0; i < 24; i++) {
      float valor_atual = e24[i] * fator;
      float diferenca = fabsf(resistencia - valor_atual);

      if (diferenca < menor_diferenca) {
        menor_diferenca = diferenca;
        melhor_valor = valor_atual;
      }
  }
}

  return melhor_valor;
}

void gerar_cores(float resistencia) {
  int ordem_magnitude = 0;

  // Ajustar resistência para dois dígitos
  while (resistencia >= 100) {
      resistencia /= 10;
      ordem_magnitude++;
  }

  int valor_int = (int)(resistencia + 0.5); // Arredonda para o inteiro mais próximo

  int primeiro_digito = valor_int / 10;
  int segundo_digito = valor_int % 10;

  char* cores[] = {"Preto", "Marrom", "Vermelho", "Laranja", "Amarelo", "Verde", "Azul", "Violeta", "Cinza", "Branco"};

  strcpy(faixa1, cores[primeiro_digito]);
  strcpy(faixa2, cores[segundo_digito]);
  strcpy(faixa3, cores[ordem_magnitude]);
}



int main()
{
  stdio_init_all();
  // Para ser utilizado o modo BOOTSEL com botão B
  gpio_init(Botao_B);
  gpio_set_dir(Botao_B, GPIO_IN);
  gpio_pull_up(Botao_B);
  gpio_set_irq_enabled_with_callback(Botao_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
  // Aqui termina o trecho para modo BOOTSEL com botão B

  gpio_init(Botao_A);
  gpio_set_dir(Botao_A, GPIO_IN);
  gpio_pull_up(Botao_A);
  gpio_set_irq_enabled_with_callback(Botao_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);


  // I2C Initialisation. Using it at 400Khz.
  i2c_init(I2C_PORT, 400 * 1000);

  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
  gpio_pull_up(I2C_SDA);                                        // Pull up the data line
  gpio_pull_up(I2C_SCL);                                        // Pull up the clock line
  ssd1306_t ssd;                                                // Inicializa a estrutura do display
  ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
  ssd1306_config(&ssd);                                         // Configura o display
  ssd1306_send_data(&ssd);                                      // Envia os dados para o display

  // Limpa o display. O display inicia com todos os pixels apagados.
  ssd1306_fill(&ssd, false);
  ssd1306_send_data(&ssd);

  adc_init();
  adc_gpio_init(ADC_PIN); // GPIO 28 como entrada analógica

  char str_media[5]; // Buffer para armazenar a string da media de leituras do adc
  char str_Rx[5]; // Buffer para armazenar a string do resistor desconhecido encontrado
  char str_R_aprox [5];  // Buffer para armazenar a string do resistor desconhecido aproximado

  bool cor = true;
  while (true)
  {
    adc_select_input(2); // Seleciona o ADC para eixo X. O pino 28 como entrada analógica

    float soma = 0.0f;
    for (int i = 0; i < 100; i++)
    {
      soma += adc_read();
      sleep_ms(1);
    }
    float media = soma / 100.0f;

      // Fórmula simplificada: R_x = R_conhecido * ADC_encontrado /(ADC_RESOLUTION - adc_encontrado)
      R_x = (R_conhecido * media) / (ADC_RESOLUTION - media);

    sprintf(str_media, "%1.0f", media); // Converte o inteiro em string
    sprintf(str_Rx, "%1.0f", R_x);   // Converte o float em string

    R_aproximado = aproximar_valor(R_x);
    gerar_cores(R_aproximado);
    sprintf(str_R_aprox, "%1.0f", R_aproximado);   // Converte o float em string


    // cor = !cor;
    //  Atualiza o conteúdo do display com animações
    ssd1306_fill(&ssd, !cor);                          // Limpa o display
    ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);      // Desenha um retângulo
    //ssd1306_line(&ssd, 3, 25, 123, 25, cor);           // Desenha uma linha
    ssd1306_line(&ssd, 3, 37, 123, 37, cor);           // Desenha uma linha
   // ssd1306_draw_string(&ssd, "CEPEDI   TIC37", 8, 6); // Desenha uma string
    ssd1306_draw_string(&ssd, faixa1, 10, 6);  // Desenha uma string
    ssd1306_draw_string(&ssd, faixa2, 10, 16);  // Desenha uma string
    ssd1306_draw_string(&ssd, faixa3, 10, 26);  // Desenha uma string
    ssd1306_line(&ssd, 75, 3, 75, 37, cor);           // Desenha uma linha vertical
    ssd1306_draw_string(&ssd, "R.", 94, 6);    // Desenha uma string
    ssd1306_draw_string(&ssd, str_R_aprox, 80, 20);    // Desenha uma string
   // ssd1306_draw_string(&ssd, "  Ohmimetro", 10, 28);  // Desenha uma string
    ssd1306_draw_string(&ssd, "ADC", 13, 41);          // Desenha uma string
    ssd1306_draw_string(&ssd, "Resisten.", 50, 41);    // Desenha uma string
    ssd1306_line(&ssd, 44, 37, 44, 60, cor);           // Desenha uma linha vertical
    ssd1306_draw_string(&ssd, str_media, 8, 52);           // Desenha uma string
    ssd1306_draw_string(&ssd, str_Rx, 59, 52);          // Desenha uma string
    ssd1306_send_data(&ssd);                           // Atualiza o display

    printf("\n R conhecido: %d", R_conhecido);

    sleep_ms(700);
  }
}