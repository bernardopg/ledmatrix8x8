# Serial Send Script Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Criar `scripts/send.py` — CLI ergonômico com subcomandos para enviar comandos seriais ao firmware, com auto-detect ESP32 por VID USB e leitura de resposta.

**Architecture:** Click group com `--port` global; subcomandos `text`, `color`, `clear`, `status` constroem a string de comando e delegam para `send_and_receive()`. `auto_detect_port()` filtra portas pelo VID Espressif `0x303A`. `text` aceita argumento posicional ou stdin (híbrido).

**Tech Stack:** Python 3, `click>=8.0`, `pyserial>=3.5`, `pytest`

---

## Arquivos

| Ação | Caminho | Responsabilidade |
|---|---|---|
| Criar | `requirements.txt` | Dependências Python do projeto |
| Criar | `pytest.ini` | Configura pythonpath para imports de `scripts.*` |
| Criar | `scripts/__init__.py` | Torna `scripts/` importável nos testes |
| Criar | `scripts/send.py` | CLI completo |
| Criar | `tests/__init__.py` | Pacote de testes |
| Criar | `tests/test_send.py` | Testes unitários |

---

## Task 1: Setup de projeto

**Files:**
- Create: `requirements.txt`
- Create: `pytest.ini`
- Create: `scripts/__init__.py`
- Create: `tests/__init__.py`

- [ ] **Step 1: Criar requirements.txt**

```
pyserial>=3.5
click>=8.0
pytest>=7.0
```

- [ ] **Step 2: Criar pytest.ini**

```ini
[pytest]
pythonpath = .
```

- [ ] **Step 3: Criar scripts/__init__.py e tests/__init__.py**

Ambos vazios.

```bash
touch scripts/__init__.py tests/__init__.py
```

- [ ] **Step 4: Instalar dependências**

```bash
pip install -r requirements.txt
```

Esperado: instalação sem erros de `click` e `pyserial`.

- [ ] **Step 5: Commit**

```bash
git add requirements.txt pytest.ini scripts/__init__.py tests/__init__.py
git commit -m "chore: add python deps and test setup for send script"
```

---

## Task 2: auto_detect_port()

**Files:**
- Create: `tests/test_send.py`
- Modify: `scripts/send.py` (criar)

- [ ] **Step 1: Criar tests/test_send.py com testes para auto_detect_port**

```python
import pytest
from unittest.mock import patch, MagicMock
from click.testing import CliRunner


class FakePort:
    def __init__(self, device, vid):
        self.device = device
        self.vid = vid


def test_auto_detect_finds_esp32():
    fake_ports = [FakePort('/dev/ttyACM0', 0x303A)]
    with patch('serial.tools.list_ports.comports', return_value=fake_ports):
        from scripts.send import auto_detect_port
        assert auto_detect_port() == '/dev/ttyACM0'


def test_auto_detect_no_device_raises():
    with patch('serial.tools.list_ports.comports', return_value=[]):
        from scripts.send import auto_detect_port
        with pytest.raises(SystemExit):
            auto_detect_port()


def test_auto_detect_multiple_uses_first(capsys):
    fake_ports = [
        FakePort('/dev/ttyACM0', 0x303A),
        FakePort('/dev/ttyACM1', 0x303A),
    ]
    with patch('serial.tools.list_ports.comports', return_value=fake_ports):
        from scripts.send import auto_detect_port
        result = auto_detect_port()
    assert result == '/dev/ttyACM0'
```

- [ ] **Step 2: Executar testes — verificar falha**

```bash
pytest tests/test_send.py -v
```

Esperado: `ImportError: cannot import name 'auto_detect_port' from 'scripts.send'` (arquivo não existe ainda).

- [ ] **Step 3: Criar scripts/send.py com auto_detect_port**

```python
#!/usr/bin/env python3
import sys
import time

import click
import serial
import serial.tools.list_ports

ESPRESSIF_VID = 0x303A
BAUD_RATE = 115200
RESPONSE_TIMEOUT = 2.0


def auto_detect_port() -> str:
    ports = [p for p in serial.tools.list_ports.comports() if p.vid == ESPRESSIF_VID]
    if not ports:
        raise click.ClickException(
            "ESP32 não encontrado. Use --port /dev/ttyACM0"
        )
    if len(ports) > 1:
        click.echo(
            f"Múltiplas portas ESP32 encontradas. Usando {ports[0].device}",
            err=True,
        )
    return ports[0].device
```

- [ ] **Step 4: Executar testes — verificar aprovação**

```bash
pytest tests/test_send.py -v
```

Esperado: `3 passed`.

- [ ] **Step 5: Commit**

```bash
git add scripts/send.py tests/test_send.py
git commit -m "feat: add auto_detect_port for ESP32 VID lookup"
```

---

## Task 3: send_and_receive()

**Files:**
- Modify: `tests/test_send.py`
- Modify: `scripts/send.py`

- [ ] **Step 1: Adicionar testes para send_and_receive em tests/test_send.py**

Adicionar ao final do arquivo:

```python
def test_send_and_receive_writes_each_command():
    mock_ser = MagicMock()
    mock_ser.readline.return_value = b''

    with patch('serial.Serial') as MockSerial, \
         patch('time.sleep'), \
         patch('time.monotonic', side_effect=[0.0, 3.0]):
        MockSerial.return_value.__enter__.return_value = mock_ser
        MockSerial.return_value.__exit__.return_value = False

        from scripts.send import send_and_receive
        send_and_receive('/dev/ttyACM0', ['TEXT:Olá', 'COLOR:255,0,0'])

        mock_ser.write.assert_any_call(b'TEXT:Ol\xc3\xa1\n')
        mock_ser.write.assert_any_call(b'COLOR:255,0,0\n')


def test_send_and_receive_prints_response(capsys):
    mock_ser = MagicMock()
    mock_ser.readline.side_effect = [b'Override ativo\n', b'']

    with patch('serial.Serial') as MockSerial, \
         patch('time.sleep'), \
         patch('time.monotonic', side_effect=[0.0, 0.5, 3.0]):
        MockSerial.return_value.__enter__.return_value = mock_ser
        MockSerial.return_value.__exit__.return_value = False

        from scripts.send import send_and_receive
        send_and_receive('/dev/ttyACM0', ['STATUS'])

    captured = capsys.readouterr()
    assert 'Override ativo' in captured.out
```

- [ ] **Step 2: Executar testes — verificar falha**

```bash
pytest tests/test_send.py::test_send_and_receive_writes_each_command tests/test_send.py::test_send_and_receive_prints_response -v
```

Esperado: `ImportError: cannot import name 'send_and_receive'`.

- [ ] **Step 3: Adicionar send_and_receive a scripts/send.py**

Adicionar após `auto_detect_port`:

```python
def send_and_receive(port: str, commands: list[str], timeout: float = RESPONSE_TIMEOUT) -> None:
    with serial.Serial(port, BAUD_RATE, timeout=0.1) as ser:
        time.sleep(0.1)
        ser.flushInput()
        for cmd in commands:
            ser.write((cmd + '\n').encode('utf-8'))

        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            line = ser.readline().decode('utf-8', errors='replace').strip()
            if line:
                click.echo(line)
```

- [ ] **Step 4: Executar todos os testes — verificar aprovação**

```bash
pytest tests/test_send.py -v
```

Esperado: `5 passed`.

- [ ] **Step 5: Commit**

```bash
git add scripts/send.py tests/test_send.py
git commit -m "feat: add send_and_receive with response reading"
```

---

## Task 4: Comando text

**Files:**
- Modify: `tests/test_send.py`
- Modify: `scripts/send.py`

- [ ] **Step 1: Adicionar testes para o comando text**

Adicionar ao final de `tests/test_send.py`:

```python
def test_text_sends_message():
    runner = CliRunner()
    with patch('scripts.send.auto_detect_port', return_value='/dev/ttyACM0'), \
         patch('scripts.send.send_and_receive') as mock_send:
        from scripts.send import cli
        result = runner.invoke(cli, ['text', 'Olá Klein'])
        assert result.exit_code == 0
        mock_send.assert_called_once_with('/dev/ttyACM0', ['TEXT:Olá Klein'])


def test_text_with_color_sends_color_first():
    runner = CliRunner()
    with patch('scripts.send.auto_detect_port', return_value='/dev/ttyACM0'), \
         patch('scripts.send.send_and_receive') as mock_send:
        from scripts.send import cli
        result = runner.invoke(cli, ['text', '--color', '255,140,0', 'Olá Klein'])
        assert result.exit_code == 0
        mock_send.assert_called_once_with(
            '/dev/ttyACM0', ['COLOR:255,140,0', 'TEXT:Olá Klein']
        )


def test_text_reads_stdin():
    runner = CliRunner()
    with patch('scripts.send.auto_detect_port', return_value='/dev/ttyACM0'), \
         patch('scripts.send.send_and_receive') as mock_send:
        from scripts.send import cli
        result = runner.invoke(cli, ['text'], input='Olá via stdin\n')
        assert result.exit_code == 0
        mock_send.assert_called_once_with('/dev/ttyACM0', ['TEXT:Olá via stdin'])


def test_port_flag_skips_autodetect():
    runner = CliRunner()
    with patch('scripts.send.auto_detect_port') as mock_detect, \
         patch('scripts.send.send_and_receive') as mock_send:
        from scripts.send import cli
        result = runner.invoke(cli, ['--port', '/dev/ttyUSB0', 'text', 'msg'])
        assert result.exit_code == 0
        mock_detect.assert_not_called()
        mock_send.assert_called_once_with('/dev/ttyUSB0', ['TEXT:msg'])
```

- [ ] **Step 2: Executar testes — verificar falha**

```bash
pytest tests/test_send.py::test_text_sends_message -v
```

Esperado: `ImportError: cannot import name 'cli'`.

- [ ] **Step 3: Adicionar CLI skeleton e comando text a scripts/send.py**

Adicionar ao final de `scripts/send.py`:

```python
@click.group()
@click.option('--port', default=None, help='Porta serial (auto-detect se omitido)')
@click.pass_context
def cli(ctx, port):
    ctx.ensure_object(dict)
    ctx.obj['port'] = port


@cli.command()
@click.argument('message', required=False)
@click.option('--color', default=None, help='Cor RGB: r,g,b (ex: 255,140,0)')
@click.pass_context
def text(ctx, message, color):
    """Envia TEXT: para o device. Aceita argumento ou stdin."""
    if message is None:
        if not sys.stdin.isatty():
            message = sys.stdin.read().strip()
        else:
            raise click.UsageError("Forneça texto como argumento ou via stdin")

    port = ctx.obj['port'] or auto_detect_port()
    commands = []
    if color:
        commands.append(f'COLOR:{color}')
    commands.append(f'TEXT:{message}')
    send_and_receive(port, commands)


if __name__ == '__main__':
    cli()
```

- [ ] **Step 4: Executar todos os testes — verificar aprovação**

```bash
pytest tests/test_send.py -v
```

Esperado: `9 passed`.

- [ ] **Step 5: Commit**

```bash
git add scripts/send.py tests/test_send.py
git commit -m "feat: add CLI skeleton and text command with stdin support"
```

---

## Task 5: Comandos color, clear, status

**Files:**
- Modify: `tests/test_send.py`
- Modify: `scripts/send.py`

- [ ] **Step 1: Adicionar testes para color, clear e status**

Adicionar ao final de `tests/test_send.py`:

```python
def test_color_command():
    runner = CliRunner()
    with patch('scripts.send.auto_detect_port', return_value='/dev/ttyACM0'), \
         patch('scripts.send.send_and_receive') as mock_send:
        from scripts.send import cli
        result = runner.invoke(cli, ['color', '255,140,0'])
        assert result.exit_code == 0
        mock_send.assert_called_once_with('/dev/ttyACM0', ['COLOR:255,140,0'])


def test_clear_command():
    runner = CliRunner()
    with patch('scripts.send.auto_detect_port', return_value='/dev/ttyACM0'), \
         patch('scripts.send.send_and_receive') as mock_send:
        from scripts.send import cli
        result = runner.invoke(cli, ['clear'])
        assert result.exit_code == 0
        mock_send.assert_called_once_with('/dev/ttyACM0', ['CLEAR'])


def test_status_command():
    runner = CliRunner()
    with patch('scripts.send.auto_detect_port', return_value='/dev/ttyACM0'), \
         patch('scripts.send.send_and_receive') as mock_send:
        from scripts.send import cli
        result = runner.invoke(cli, ['status'])
        assert result.exit_code == 0
        mock_send.assert_called_once_with('/dev/ttyACM0', ['STATUS'])
```

- [ ] **Step 2: Executar testes — verificar falha**

```bash
pytest tests/test_send.py::test_color_command tests/test_send.py::test_clear_command tests/test_send.py::test_status_command -v
```

Esperado: `3 failed` — subcomandos não existem ainda.

- [ ] **Step 3: Adicionar comandos color, clear, status a scripts/send.py**

Inserir antes de `if __name__ == '__main__':`:

```python
@cli.command()
@click.argument('rgb')
@click.pass_context
def color(ctx, rgb):
    """Envia COLOR:r,g,b para o device."""
    port = ctx.obj['port'] or auto_detect_port()
    send_and_receive(port, [f'COLOR:{rgb}'])


@cli.command()
@click.pass_context
def clear(ctx):
    """Envia CLEAR para o device."""
    port = ctx.obj['port'] or auto_detect_port()
    send_and_receive(port, ['CLEAR'])


@cli.command()
@click.pass_context
def status(ctx):
    """Envia STATUS para o device e exibe resposta."""
    port = ctx.obj['port'] or auto_detect_port()
    send_and_receive(port, ['STATUS'])
```

- [ ] **Step 4: Executar todos os testes — verificar aprovação**

```bash
pytest tests/test_send.py -v
```

Esperado: `12 passed`.

- [ ] **Step 5: Verificar help do CLI manualmente**

```bash
python scripts/send.py --help
python scripts/send.py text --help
```

Esperado: lista de subcomandos e opções legíveis.

- [ ] **Step 6: Commit final**

```bash
git add scripts/send.py tests/test_send.py
git commit -m "feat: add color, clear, status subcommands to send.py"
```
