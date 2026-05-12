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
- Animacao de gato com quadros simples de idle/blink/happy como camada de personalidade
- Integracao opcional com Home Assistant via REST para override remoto
- Dashboard Lovelace no Home Assistant para helpers, presets rapidos e diagnostico de uso diario
- Icones 8x8 de estado que podem ser acionados por `ICON:nome[:mensagem]` via Serial ou Home Assistant

## Nao objetivos por enquanto

- consumir APIs externas complexas diretamente no ESP32 alem do Home Assistant REST ja suportado
- armazenar historico, agenda ou estado persistente no microcontrolador
- app mobile ou dashboard web proprio

Integracoes mais pesadas fazem mais sentido em scripts externos ou automacoes locais, que convertem dados reais em mensagens curtas para a serial ou para helpers do Home Assistant.

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

- loop de playback entre gato e letreiro
- injeta mensagens geradas a partir de `config.yaml`
- aplica overrides por Serial e Home Assistant com prioridade definida
- inicializa o runtime da matriz e o efeito ativo

## Requisitos operacionais

- textos curtos e legiveis
- rede opcional e degradacao limpa quando Home Assistant nao estiver configurado
- feedback imediato via Serial para diagnostico
- upload rapido com PlatformIO

## Roadmap util

### Ja validado

- validar no hardware real o novo `STATUS` expandido
- validar e2e Home Assistant -> ESP32 com mensagem e cor
- validar precedencia Serial > Home Assistant > `config.yaml`
- presets de mensagem por contexto no Home Assistant

### Curto prazo

- suporte a mais sprites e estados visuais do gato
- refinar a composicao entre icones 8x8, mensagens e presets reais de agenda/tarefa/status

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
