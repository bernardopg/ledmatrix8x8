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
- [x] Extrair logica de prioridade de fonte para `LedMatrixMessagePriority.h` com testes.
- [x] Extrair parsers de firmware (brightness, effect mode, erro HA) para `LedMatrixFirmwareCommands.h` com testes.
- [x] Adicionar comando `BRIGHTNESS:n` para ajuste rapido sem reflash.
- [x] Adicionar comando `EFFECT:cat|playback` para alternar efeito em runtime.
- [x] Expor no `STATUS` o ultimo erro HTTP/JSON de forma legivel e resumida.
- [x] Adicionar `CatAnimationEffect` separado para modo sem marquee.
- [x] Fazer upload no ESP32 e validar `STATUS`, `COLOR`, `TEXT`, `BRIGHTNESS`, `EFFECT` e `CLEAR` com o firmware mais recente.
- [x] Validar no Home Assistant real que mensagem e cor atualizam independentemente.

## Rotina de validacao

- [ ] Manter `make check` verde antes de commits relevantes.

## Prioridade 0 - manter verde

- [ ] Adicionar testes para `BRIGHTNESS` e `EFFECT` no `test_send.py` (brightness_command, effect_invalid_noop).
- [ ] Adicionar teste host-side para `parseLedMatrixEffectMode("CAT-ONLY", ...)` com hifem (alias nao coberto).

## Prioridade 1 - robustez do firmware

- [ ] Fazer `LedMatrixEffect::supportsOverrides()` ser usado pelo app: se o efeito ativo nao suporta override, logar aviso ao inves de chamar silenciosamente.
- [ ] Garantir que `EFFECT:cat` limpa o override ativo antes de trocar (para evitar estado inconsistente entre o ponteiro de efeito e `manualOverrideActive`).
- [ ] Adicionar reset de override quando `EFFECT:` muda o efeito (o override manual pode apontar para o efeito anterior).

## Prioridade 2 - experiencia visual

- [ ] Transformar o gato de sprite unico em animacao real: piscar, idle, feliz, erro, loading.
- [ ] Criar presets de mensagem/estado: `FOCO`, `CALL`, `DONE`, `BREAK`, `ERROR`.
- [ ] Criar pequenos icones 8x8 para estados de agenda/tarefa/status.

## Prioridade 3 - integracoes locais

- [ ] Criar script local para puxar uma tarefa do Obsidian/Kanban e enviar via serial/HA.
- [ ] Criar modo "agenda do dia" via automacao externa, mantendo o ESP32 simples.
- [ ] Documentar exemplos de automacoes Home Assistant para atualizar mensagem e cor.
