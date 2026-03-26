# Contexto do Projeto

## Objetivo

Transformar uma matriz NeoPixel 8x8 com ESP32-S3 em um painel utilitario pequeno, rapido de atualizar e divertido de usar no dia a dia.

O projeto deixou de ser um teste de contador e passou a perseguir tres usos principais:

1. Exibir mensagens curtas de valor imediato
2. Receber conteudo externo sem exigir reflash a cada mudanca
3. Ter identidade visual propria, com um personagem animado simples

## Problema que o projeto resolve

Um display 8x8 e extremamente limitado. Se o firmware for pensado apenas como demo, ele morre rapido. O valor real vem de usar esse espaco como um "glanceable display":

- proxima tarefa do dia
- status de foco
- recados curtos
- lembretes puxados de notas ou board

## Direcao atual

- Firmware focado em letreiro
- Conteudo desacoplado em `config.yaml`
- Header gerado para manter build simples no microcontrolador
- Comandos serial para override rapido
- Animacao de gato como camada de personalidade e estado idle

## Nao objetivos por enquanto

- Wi-Fi embarcado no firmware
- autenticar diretamente com APIs externas
- CI/CD
- app mobile ou dashboard web

Essas integracoes fazem mais sentido em scripts externos ou automacoes locais, que convertem dados reais em mensagens curtas.

## Arquitetura

### Conteudo

`config.yaml` define:

- brilho
- ritmo do scroll
- tempo entre mensagens
- velocidade do gato
- lista de mensagens e cores

### Geracao

`scripts/generate_content.py` converte a config em `generated/project_content.h`.

Motivo:

- o firmware continua simples
- Arduino IDE continua viavel
- nao dependemos de parser YAML no ESP32

### Execucao

`ledmatrix8x8_app.h` faz:

- scroll de texto 5x7
- animacao do gato
- parser de comandos serial
- loop de playback com modos `AUTO`, `TEXT` e `CAT`

## Requisitos operacionais

- textos curtos e legiveis
- sem dependencia de rede no firmware
- feedback imediato via Serial
- upload rapido com PlatformIO

## Roadmap util

### Curto prazo

- script local para mandar `TEXT:` para a serial
- suporte a mais sprites
- presets de mensagem por contexto

### Medio prazo

- integracao com Obsidian Local REST API
- integracao com GitHub Projects/Trello
- modo "agenda do dia"

### Longo prazo

- transicoes mais sofisticadas
- composicao com icones pequenos
- multiplexar mais de um tipo de conteudo no mesmo ciclo

## Criterio de sucesso

O projeto esta no caminho certo se:

- fica util mesmo offline
- atualizar uma mensagem e trivial
- o comportamento padrao e agradavel o suficiente para ficar ligado na mesa
