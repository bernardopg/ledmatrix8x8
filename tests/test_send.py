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
