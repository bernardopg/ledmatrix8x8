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
