VENV ?= .venv
PYTHON ?= $(VENV)/bin/python
PORT ?= /dev/ttyACM0
PIO := PATH=$(CURDIR)/$(VENV)/bin:$$PATH pio

.PHONY: setup generate test typecheck format format-check build upload monitor status check

setup:
	python -m venv $(VENV)
	$(PYTHON) -m pip install --upgrade pip
	$(PYTHON) -m pip install -r requirements.txt

generate:
	$(PYTHON) scripts/generate_content.py

test:
	$(PYTHON) -m pytest -q

typecheck:
	$(PYTHON) -m mypy scripts tests

format:
	$(PYTHON) -m isort scripts tests

format-check:
	$(PYTHON) -m isort --check-only scripts tests

build:
	$(PIO) run

upload:
	$(PIO) run -t upload --upload-port $(PORT)

monitor:
	$(PIO) device monitor --port $(PORT)

status:
	$(PYTHON) scripts/send.py --port $(PORT) status

check: test typecheck format-check build
