# ledmatrix8x8

Projeto para uma matriz NeoPixel 8x8 com `ESP32-S3`, pensado como um painel utilitario pequeno: mensagens rolando, personagem animado e base pronta para integrações futuras com Kanban e Obsidian.

## O que existe hoje

- Letreiro com mensagens configuradas em `config.yaml`
- Animação de gato em pixel art entre os ciclos
- Comandos via Serial para trocar mensagem, brilho e modo sem recompilar
- Geração automatica de `generated/project_content.h` via `scripts/generate_content.py`
- Build com PlatformIO e compatibilidade mantida com Arduino IDE

## Estrutura

- `src/main.cpp`: entrypoint do PlatformIO
- `ledmatrix8x8.ino`: entrypoint equivalente para Arduino IDE
- `ledmatrix8x8_app.h`: firmware principal
- `config.yaml`: conteudo configuravel do painel
- `generated/project_content.h`: header gerado a partir da config
- `scripts/generate_content.py`: gerador do header
- `PROJECT_CONTEXT.md`: contexto de produto e direcao do projeto
- `docs/INSPIRATIONS.md`: ideias, integrações e referencias

## Hardware

- ESP32-S3-DevKitC-1
- Matriz WS2812B 8x8
- GPIO de dados: `GPIO38`

Se a sua matriz usar mapeamento diferente, ajuste `MATRIX_ZIGZAG` e `ORIGIN_BOTTOM` em `ledmatrix8x8_app.h`.

## Como configurar

O arquivo `config.yaml` foi escrito em YAML compativel com JSON para que o gerador use apenas a stdlib do Python.

1. Edite `config.yaml`
2. Rode `python scripts/generate_content.py`
3. Rode `pio run`
4. Faça upload com `pio run -t upload`

O PlatformIO tambem executa o gerador automaticamente antes da compilacao.

## Comandos via Serial

Abra o monitor serial em `115200` baud e envie uma linha por vez:

- `TEXT:REUNIAO AS 15`
- `CLEAR`
- `MODE:AUTO`
- `MODE:TEXT`
- `MODE:CAT`
- `BRIGHTNESS:32`
- `STATUS`
- `HELP`

`TEXT:` ativa uma mensagem temporaria ate o comando `CLEAR`.

## Casos de uso uteis

- Letreiro de foco pessoal com tarefas de hoje
- Mini painel do seu Kanban atual
- Recados puxados do Obsidian
- Mascote animado para status idle
- Badge de mesa com mensagens contextuais em reunioes

## Ideias de integracao

- Obsidian:
  usar o plugin `obsidian-local-rest-api` para ler uma nota, extrair tarefas e mandar a mensagem via Serial ou regenerar `config.yaml`
- Kanban:
  consultar GitHub Projects ou Trello e converter os cards prioritarios em mensagens curtas
- Automacao local:
  um script no notebook pode trocar o texto exibido sem reflashing, usando o comando serial `TEXT:...`

As referencias de inspiracao estao em [docs/INSPIRATIONS.md](docs/INSPIRATIONS.md).

## Build

Ambiente alvo:

- board: `esp32-s3-devkitc-1`
- framework: `arduino`
- biblioteca: `Adafruit NeoPixel`

## Proximos passos sugeridos

- Criar um sincronizador `Kanban/Obsidian -> Serial`
- Adicionar mais sprites e microanimacoes
- Suportar perfis de exibicao por horario
- Criar um modo compacto para indicadores de status
