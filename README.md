# Embarcatech_Ohmimetro

Repositório criado para a tarefa relacionada à construção de um Ohmimetro Digital com a placa BitDogLab.

O projeto implementa um Ohmímetro Digital utilizando a placa BitDogLab com o microcontrolador RP2040. O sistema é capaz de medir a resistência de um resistor desconhecido, utilizando um divisor de tensão em conjunto com um resistor conhecido de referência. O valor obtido é aproximado para um valor padrão da série comercial E24 (5% de tolerância) e, com base nesse valor, o sistema gera automaticamente a sequência de três faixas de cores correspondentes ao resistor.

Além disso, o sistema exibe informações no display OLED SSD1306, como:

-Primeira faixa: primeiro dígito significativo da resistência;

-Segunda faixa: segundo dígito significativo da resistência;

-Terceira faixa: multiplicador da resistência;

-Valor da resistência aproximada;

-Valor real da resistência medida;

-Valor lido pelo conversor analógico-digital (ADC).

Dois botões físicos na placa são utilizados:

-Botão A (GPIO 5): alterna entre dois resistores de referência (10 kΩ e 8,2 kΩ), para ampliar a faixa de medição.

-Botão B (GPIO 6): reinicia o microcontrolador no modo BOOTSEL.

# Instruções de Compilação

Para compilar o código, são necessárias as seguintes extensões:

Raspberry Pi Pico SDK

CMake

Após instalá-las, basta importar através da extensão do raspberry pi e construir (buildar) o projeto utilizando o CMake.

É necessária a utilização de uma protoboard, de resistores e jumpers para implementação do projeto.

# Instruções de Utilização

O projeto possui as seguintes funcionalidades principais:

Medição de Resistência:

-A tensão de saída de 3,3 V alimenta um divisor de tensão formado pelo resistor conhecido e pelo resistor desconhecido.

-O ADC (GPIO 28) realiza a leitura da tensão no divisor para calcular a resistência desconhecida.

Exibição de Valores:

-O valor da resistência, seu valor aproximado e o código de cores são mostrados no display OLED SSD1306, utilizando comunicação via I2C nos pinos GPIO 14 (SDA) e GPIO 15 (SCL).

Controle via Botões:

-Botão A: Alterna entre dois resistores de referência configurados (R_conhecido1 e R_conhecido2).

-Botão B: Reinicia o sistema no modo BOOTSEL.

O circuito deve ser construido com dois resistores em série, uma alimentação conectada à saída constante de 3.3V da placa e a um terminal do resistor de referencia, 
um terra conectado ao GND da placa e a um terminal do resistor desconhecido, e a entrada analógica conectada ao pino 28 da placa e ao nó que une os dois resistores.
