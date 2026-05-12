# Home Assistant example export

This directory contains a sanitized export of the LED Matrix control surface used in Bernardo's homelab.

Files:

- `ledmatrix8x8-dashboard-view.json`: the Lovelace `LED Matrix` view exported from storage-mode dashboard `lovelace.meu_dashboard`.
- `ledmatrix8x8-scripts.yaml`: the `script.ledmatrix8x8_*` services used by the dashboard buttons and presets.
- `ledmatrix8x8-helpers.yaml`: the two `input_text` helpers polled by the firmware.

Expected entities:

- `input_text.ledmatrix8x8_message`
- `input_text.ledmatrix8x8_color`

Preset scripts:

- `script.ledmatrix8x8_preset_foco` -> `ICON:focus:FOCO`, `255,140,0`
- `script.ledmatrix8x8_preset_call` -> `ICON:call:CALL`, `0,180,255`
- `script.ledmatrix8x8_preset_done` -> `ICON:status:DONE`, `0,255,160`
- `script.ledmatrix8x8_preset_break` -> `ICON:break:BREAK`, `180,0,255`
- `script.ledmatrix8x8_preset_error` -> `ICON:error:ERROR`, `255,0,64`

Import notes:

1. Create or merge the helpers from `ledmatrix8x8-helpers.yaml` into your Home Assistant config.
2. Merge `ledmatrix8x8-scripts.yaml` into `scripts.yaml`, then call `script.reload`.
3. In Lovelace storage mode, add the `view` object from `ledmatrix8x8-dashboard-view.json` to the target dashboard.
4. Reload/save Lovelace through the UI or WebSocket API so Home Assistant updates its in-memory dashboard config.

The export intentionally contains no access tokens, Wi-Fi credentials, or firmware secrets.
