# Design: Script Serial Ergonômico (`send.py`)

**Data:** 2026-05-12
**Escopo:** item 1 de 3 do roadmap de curto prazo

---

## Objetivo

Substituir o acesso manual ao monitor serial por um CLI ergonômico que envia comandos ao firmware via serial, aguarda resposta e imprime o resultado. Permite automações shell sem abrir monitor serial.

---

## Localização

`scripts/send.py` — ao lado de `generate_content.py`.

---

## Interface

```bash
# Enviar texto (argumento ou stdin)
send.py text "Olá Klein"
send.py text --color 255,140,0 "Olá Klein"
echo "Olá Klein" | send.py text

# Cor avulsa (para próximo override)
send.py color 255,140,0

# Controle
send.py clear
send.py status

# Porta manual (se auto-detect falhar)
send.py --port /dev/ttyUSB0 text "msg"
```

---

## Auto-detecção de Porta

- Usa `serial.tools.list_ports` do `pyserial`
- Filtra por VID USB `0x303A` (Espressif / ESP32-S3)
- Escolhe primeira porta encontrada
- Se múltiplas: usa primeira, avisa no stderr
- Se nenhuma: erro com mensagem clara — `"ESP32 não encontrado. Use --port /dev/ttyACM0"`

---

## Subcomandos

| Subcomando | Comando(s) serial enviado(s) | Aguarda resposta |
|---|---|---|
| `text "msg"` | `TEXT:msg\n` | sim, 2s |
| `text --color r,g,b "msg"` | `COLOR:r,g,b\n` → `TEXT:msg\n` | sim, 2s |
| `color r,g,b` | `COLOR:r,g,b\n` | sim, 2s |
| `clear` | `CLEAR\n` | sim, 2s |
| `status` | `STATUS\n` | sim, 2s |

---

## Input Híbrido (`text`)

1. Argumento posicional presente → usa argumento
2. Stdin não é TTY (pipe) → lê stdin
3. Nenhum → erro: `"Forneça texto como argumento ou via stdin"`

---

## Fluxo Serial

1. Abre porta a 115200 baud
2. Monta e envia comando(s) + `\n`
3. Lê linhas por até **2 segundos**, imprime cada uma no stdout
4. Fecha porta
5. Exit 0 em sucesso, exit 1 em erro

Cada invocação abre e fecha a porta — sem estado persistente.

---

## Dependências

Arquivo `requirements.txt` criado na raiz do projeto:

```
pyserial>=3.5
click>=8.0
```

`generate_content.py` não é alterado (usa somente stdlib).

---

## Fora de Escopo

- Modo interativo / REPL
- Histórico de comandos
- Integração direta com APIs externas (Obsidian, GitHub) — competência de scripts externos que invocam este
- Suporte a múltiplos devices simultâneos
