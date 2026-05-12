# TODO do projeto

## Concluido nesta rodada

- [x] Criar base de testes para parser C++ de cores (`LedMatrixColorParser.h`).
- [x] Endurecer validacao RGB no script serial (`scripts/send.py`) para rejeitar valores fora de 0..255.
- [x] Aumentar espera inicial do script serial para USB CDC do ESP32 e drenar boot noise antes de enviar comandos.
- [x] Corrigir `CLEAR` para o Home Assistant reassumir com a cor HA atual, nao com a cor padrao do build.
- [x] Migrar `DynamicJsonDocument` para `JsonDocument` no cliente Home Assistant.
- [x] Expandir `STATUS` serial com fonte atual, uptime, Wi-Fi, IP, HTTP, ultimo poll e estados do HA.
- [x] Adicionar `Makefile` para comandos padronizados de build/test/upload/status.
- [x] Adicionar workflow de CI com pytest, mypy, isort e PlatformIO build.
- [x] Atualizar documentacao para refletir Wi-Fi/HA e entidade de cor.

## Prioridade 0 - manter verde

- [ ] Rodar `make check` antes de cada commit relevante.
- [ ] Fazer upload no ESP32 e validar `STATUS`, `COLOR`, `TEXT` e `CLEAR` no hardware real.
- [ ] Validar no Home Assistant real que mensagem e cor atualizam independentemente.

## Prioridade 1 - robustez do firmware

- [ ] Adicionar testes host-side para regras de prioridade: Serial > Home Assistant > config.yaml.
- [ ] Expor no `STATUS` o tempo desde o ultimo poll e talvez o ultimo erro HTTP/JSON como texto.
- [ ] Adicionar comando `BRIGHTNESS:n` para ajuste rapido sem reflash.
- [ ] Adicionar comando `EFFECT:nome` quando existir mais de um efeito ativo.

## Prioridade 2 - experiencia visual

- [ ] Transformar o gato de sprite unico em animacao real: piscar, idle, feliz, erro, loading.
- [ ] Criar presets de mensagem/estado: `FOCO`, `CALL`, `DONE`, `BREAK`, `ERROR`.
- [ ] Criar pequenos icones 8x8 para estados de agenda/tarefa/status.

## Prioridade 3 - integracoes locais

- [ ] Criar script local para puxar uma tarefa do Obsidian/Kanban e enviar via serial/HA.
- [ ] Criar modo "agenda do dia" via automacao externa, mantendo o ESP32 simples.
- [ ] Documentar exemplos de automacoes Home Assistant para atualizar mensagem e cor.
