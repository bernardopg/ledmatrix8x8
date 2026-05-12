#!/usr/bin/env python3
import re
import sys
import time

import click
import serial
import serial.tools.list_ports

ESPRESSIF_VID = 0x303A
BAUD_RATE = 115200
RESPONSE_TIMEOUT = 2.0
SERIAL_OPEN_DELAY = 1.2
RGB_PATTERN = re.compile(r'^\d{1,3},\d{1,3},\d{1,3}$')
ICON_CHOICES = ['agenda', 'task', 'status', 'focus', 'call', 'break', 'error']
MAX_TEXT_MESSAGE_LENGTH = 155
MAX_ICON_MESSAGE_LENGTH = 148


def _validate_rgb(rgb: str) -> None:
    if not RGB_PATTERN.match(rgb):
        raise click.BadParameter(
            f"formato inválido '{rgb}'. Use r,g,b (ex: 255,140,0)",
            param_hint="'rgb'"
        )

    components = [int(component) for component in rgb.split(',')]
    if any(component < 0 or component > 255 for component in components):
        raise click.BadParameter(
            f"valor inválido '{rgb}'. Cada componente RGB deve estar entre 0 a 255",
            param_hint="'rgb'"
        )


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


def send_and_receive(port: str, commands: list[str], timeout: float = RESPONSE_TIMEOUT) -> None:
    with serial.Serial(port, BAUD_RATE, timeout=0.1) as ser:
        time.sleep(SERIAL_OPEN_DELAY)
        ser.reset_input_buffer()
        ser.write(b'\n')
        for cmd in commands:
            ser.write((cmd + '\n').encode('utf-8'))

        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            line = ser.readline().decode('utf-8', errors='replace').strip()
            if line:
                click.echo(line)


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
    if color is not None:
        _validate_rgb(color)

    if message is None:
        if not sys.stdin.isatty():
            message = sys.stdin.read().strip()
        else:
            raise click.UsageError("Forneça texto como argumento ou via stdin")

    if len(message) > MAX_TEXT_MESSAGE_LENGTH:
        raise click.UsageError(
            f"mensagem muito longa ({len(message)} chars). Máximo: {MAX_TEXT_MESSAGE_LENGTH}"
        )

    port = ctx.obj['port'] or auto_detect_port()
    commands = []
    if color:
        commands.append(f'COLOR:{color}')
    commands.append(f'TEXT:{message}')
    send_and_receive(port, commands)


@cli.command()
@click.argument('name', type=click.Choice(ICON_CHOICES, case_sensitive=False))
@click.argument('message', required=False)
@click.option('--color', default=None, help='Cor RGB: r,g,b (ex: 255,140,0)')
@click.pass_context
def icon(ctx, name, message, color):
    """Envia ICON:nome[:mensagem] para o device."""
    if color is not None:
        _validate_rgb(color)

    if message is not None and len(message) > MAX_ICON_MESSAGE_LENGTH:
        raise click.UsageError(
            f"mensagem muito longa ({len(message)} chars). Máximo: {MAX_ICON_MESSAGE_LENGTH}"
        )

    port = ctx.obj['port'] or auto_detect_port()
    commands = []
    if color:
        commands.append(f'COLOR:{color}')
    icon_name = name.lower()
    command = f'ICON:{icon_name}'
    if message:
        command = f'{command}:{message}'
    commands.append(command)
    send_and_receive(port, commands)


@cli.command()
@click.argument('rgb')
@click.pass_context
def color(ctx, rgb):
    """Envia COLOR:r,g,b para o device."""
    _validate_rgb(rgb)
    port = ctx.obj['port'] or auto_detect_port()
    send_and_receive(port, [f'COLOR:{rgb}'])


@cli.command()
@click.pass_context
def clear(ctx):
    """Envia CLEAR para o device."""
    port = ctx.obj['port'] or auto_detect_port()
    send_and_receive(port, ['CLEAR'])


@cli.command()
@click.argument('value', type=click.IntRange(0, 255))
@click.pass_context
def brightness(ctx, value):
    """Envia BRIGHTNESS:n para o device."""
    port = ctx.obj['port'] or auto_detect_port()
    send_and_receive(port, [f'BRIGHTNESS:{value}'])


@cli.command()
@click.argument('name', type=click.Choice(['cat', 'playback'], case_sensitive=False))
@click.pass_context
def effect(ctx, name):
    """Envia EFFECT:nome para o device. Valores: cat, playback."""
    port = ctx.obj['port'] or auto_detect_port()
    send_and_receive(port, [f'EFFECT:{name}'])


@cli.command()
@click.pass_context
def status(ctx):
    """Envia STATUS para o device e exibe resposta."""
    port = ctx.obj['port'] or auto_detect_port()
    send_and_receive(port, ['STATUS'])


if __name__ == '__main__':
    cli()
