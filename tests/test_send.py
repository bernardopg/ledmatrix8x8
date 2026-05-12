import pytest
from unittest.mock import patch, MagicMock
from click.testing import CliRunner
import click


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


def test_auto_detect_multiple_uses_first(capsys):
    fake_ports = [
        FakePort('/dev/ttyACM0', 0x303A),
        FakePort('/dev/ttyACM1', 0x303A),
    ]
    with patch('serial.tools.list_ports.comports', return_value=fake_ports):
        from scripts.send import auto_detect_port
        result = auto_detect_port()
    assert result == '/dev/ttyACM0'


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
