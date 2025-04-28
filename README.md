# Projeto_Ohmimetro_28_4
Repositório criado para versionamento da atividade sobre ohmímetro digital da residência em software embarcado 


## Descrição do Funcionamento do programa 
No display é mostrado informações referentes às cores das faixas e o multiplicador da resistência a ser medida (considerando a série E24 com 5% de erro em uma faixa de 510 a 100K ohms). É mostrado ainda no canto inferior esquerdo do display o valor da resistência E24 mais próxima da medida. Quando a medida está fora da faixa considerada é exibido a informação “Fora E24” que demonstra para o usuário que a medição está fora da faixa, considerando a margem de erro, e no canto inferior esquerdo não é exibido nenhuma informação referente ao E24 correspondente mais próximo.

## Compilação e Execução

1. Certifique-se de que o SDK do Raspberry Pi Pico está configurado no seu ambiente.
2. Compile o programa utilizando a extensão **Raspberry Pi Pico Project** no VS Code:
   - Abra o projeto no VS Code, na pasta PROJETO_Ohmimetr_28_4 tem os arquivos necessários para importar 
   o projeto com a extensão **Raspberry Pi Pico Project**.
   - Vá até a extensão do **Raspberry pi pico project** e após importar (escolher sdk de sua escolha) os projetos  clique em **Compile Project**.
3. Coloque a placa em modo BOOTSEL e copie o arquivo `Teste_ohmimetro.uf2`  que está na pasta build, para a BitDogLab conectado via USB.


**OBS1: Devem importar os projetos para gerar a pasta build, pois a mesma não foi inserida no repositório**

## Emulação com Wokwi

Para testar os programas sem hardware físico, você pode utilizar o **Wokwi** para emulação no VS Code:

1. Instale a extensão **Wokwi for VS Code**
3. Inicie a emulação:
   - Clique no arquivo `diagram.json` e inicie a emulação.
4. Teste o funcionamento do programa diretamente no ambiente emulado.
   
**OBS 1: Os arquivos diagram.json e wokwi.toml foram inseridos para o projeto.**
**OBS 2: A emulaçao não mostra com precisão os valores do ADC**

## Link com demonstração no youtube

Demonstração do funcionamento do projeto na BitDogLab: (https://youtu.be/x7U7nhrQSeM?si=xMVcMTDumA-rR90p)


## Colaboradores
- [PauloCesar53 - Paulo César de Jesus Di Lauro ] (https://github.com/PauloCesar53)
