from unittest.mock import MagicMock, patch

import click
import pytest
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
        with pytest.raises(click.ClickException):
            auto_detect_port()


def test_auto_detect_multiple_uses_first():
    fake_ports = [
        FakePort('/dev/ttyACM0', 0x303A),
        FakePort('/dev/ttyACM1', 0x303A),
    ]
    with patch('serial.tools.list_ports.comports', return_value=fake_ports), \
         patch('click.echo') as mock_echo:
        from scripts.send import auto_detect_port
        result = auto_detect_port()
    assert result == '/dev/ttyACM0'
    mock_echo.assert_called_once_with(
        'Múltiplas portas ESP32 encontradas. Usando /dev/ttyACM0',
        err=True,
    )


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


def test_color_command():
    runner = CliRunner()
    with patch('scripts.send.auto_detect_port', return_value='/dev/ttyACM0'), \
         patch('scripts.send.send_and_receive') as mock_send:
        from scripts.send import cli
        result = runner.invoke(cli, ['color', '255,140,0'])
        assert result.exit_code == 0
        mock_send.assert_called_once_with('/dev/ttyACM0', ['COLOR:255,140,0'])


def test_color_rejects_values_above_byte_range():
    runner = CliRunner()
    with patch('scripts.send.auto_detect_port') as mock_detect, \
         patch('scripts.send.send_and_receive') as mock_send:
        from scripts.send import cli
        result = runner.invoke(cli, ['color', '256,0,0'])

    assert result.exit_code != 0
    assert '0 a 255' in result.output
    mock_detect.assert_not_called()
    mock_send.assert_not_called()


def test_send_and_receive_waits_for_usb_cdc_reset_before_draining():
    mock_ser = MagicMock()
    mock_ser.readline.return_value = b''

    with patch('serial.Serial') as MockSerial, \
         patch('time.sleep') as mock_sleep, \
         patch('time.monotonic', side_effect=[0.0, 3.0]):
        MockSerial.return_value.__enter__.return_value = mock_ser
        MockSerial.return_value.__exit__.return_value = False

        from scripts.send import SERIAL_OPEN_DELAY, send_and_receive
        send_and_receive('/dev/ttyACM0', ['STATUS'])

    mock_sleep.assert_called_once_with(SERIAL_OPEN_DELAY)
    assert SERIAL_OPEN_DELAY >= 1.0
    mock_ser.reset_input_buffer.assert_called_once_with()
    mock_ser.write.assert_any_call(b'\n')
    mock_ser.write.assert_any_call(b'STATUS\n')


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
