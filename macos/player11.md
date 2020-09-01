# Player 11

## Instruments

| AD   | SR   | Switches           | Filter                     | Pulse                     | Wave                     |
| ---- | ---- | ------------------ | -------------------------- | ------------------------- | ------------------------ |
| `ad` | `sr` | `80`: Hard restart | `xx` &rarr; **Filter** row | `xx` &rarr; **Pulse** row | `xx` &rarr; **Wave** row |
|      |      | `40`: Filter on    |                            |                           |                          |
|      |      | `10`: Osc reset    |                            |                           |                          |
|      |      | `0x`: HR index     |                            |                           |                          |

## Wave

| Waveform | Note                         |
| -------- | ---------------------------- |
| `xx`     | `00`- `7f`: relative         |
| `xx`     | `80`- `ff`: absolute (`-80`) |
| `7f`     | `xx`: jump to index          |

## Pulse

| Pulse.. | ..width                                   | Duration / Index    |
| ------- | ----------------------------------------- | ------------------- |
| `8x`    | `xx`: set pulsewidth to `xxx`             | `yy`                |
| `0x`    | `xx`: add `xxx` to pulsewidth every frame | `yy`                |
| `7f`    | `??`                                      | `xx`: jump to index |

## Filter

| Type / Jump          | Cutoff                | Resonance / Duration / Index    |
| -------------------- | --------------------- | ------------------------------- |
| `xy`: type `x` (x>8) | `yy`: cutoff `yyy`    | `zq`: resonance `z`, voices `q` |
| `0x`                 | `xx`: add cutoff`xxx` | `yy`: duration                  |
| `7f`                 | `--`                  | `xx`: jump to index             |

## Arp

| Note / Jump                     |
| ------------------------------- |
| `00`-`6f`: relative note        |
| `7x`: jump to start index + `x` |

## Commands

| Type                 | Parameter | Parameter  | Description                                   |
| -------------------- | --------- | ---------- | --------------------------------------------- |
| `00` slide           | `xx` spd  | `yy` spd   | Slide @ speed `xxyy`                          |
| `01` vibrato         | `xx` freq | `yy` amp   | Vibrato @ frequency `xx` &amp; amplitude `yy` |
| `02` portamento      | `xx` spd  | `yy` spd   | Portamento @ speed `xxyy` (`8000` to disable) |
| `03` arpeggio        | `xx` spd  | `yy` index | Arpeggio from **Arp** index `yy` @ speed `xx` |
| `08` adsr note       | `ad`      | `sr`       | ADSR `adsr` until next note plays             |
| `09` adsr instrument | `ad`      | `sr`       | ADSR `adsr` until next instrument plays       |
| `0a` filter          | `--`      | `yy`       | Start **Filter** @ index `yy`                 |
| `0b` wave            | `--`      | `yy`       | Start **Wave** @ index `yy`                   |
