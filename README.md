# Vier Gewinnt für den Siemens IC35

Natives Vier Gewinnt für den Siemens IC35 als eigenständige `.APP` – ohne BASIC-Laufzeit.
Du spielst gegen eine KI mit drei Schwierigkeitsstufen auf einem grafischen Brett.

*English summary below.*

## Funktionen

- Mensch (schwarze Scheiben) gegen KI (Ringe)
- Schwierigkeit: Leicht, Mittel, Schwer
- Grafisches Brett über den ganzen 240 × 160-Bildschirm mit Auswahlpfeil
- Beim Sieg werden die vier Gewinnsteine invertiert hervorgehoben, dazu die Richtung der Reihe (waagerecht, senkrecht, diagonal)
- Menü mit Kurzanleitung; die App erscheint im IC35 als „Vier Gewinnt“

## Steuerung

| Taste | Funktion |
|---|---|
| Pfeil links / rechts | Spalte wählen |
| Pfeil runter, Enter oder Leertaste | Stein einwerfen |
| `1`–`7` | Spalte direkt wählen und einwerfen |
| `N` | Neue Partie (gleiche Stufe) |
| `S` | Schwierigkeit wählen |
| `Q` | Beenden |

Im Menü wählen Pfeil hoch/runter oder `1`–`3` die Stufe, Enter startet.

## Installation

1. Aktuelles ZIP aus den [Releases](../../releases) laden und `VIER.APP` entpacken.
2. Organizer-Daten und Karteninhalt sichern.
3. `VIER.APP` nach `IC35\APP\VIER.APP` kopieren.

Bei einem Hänger nicht wiederholt neu starten, sondern das Gerät zurücksetzen und die vorherige Version verwenden.

## Bauen

Alle Tests: `python3 -m unittest discover -s tests -v`

- `python3 tools/build_test.py` baut die Tastatur-Test-App `TEST001.APP` (Referenz, unverändert).
- `python3 tools/build_game.py` baut `VIER.APP`; benötigt den Open-Source-Compiler SDCC.

Der GitHub-Workflow „Build native IC35 apps“ erzeugt das Artefakt `IC35-VIER-KI-009-NATIVE-APP`.
Es werden keine proprietären SDK-Binärdateien oder `apilib.obj` verwendet oder hochgeladen.

## Aufbau

- `native/game.c` – Spiellogik, KI und Steuerung
- `native/graphics.h` – Monochrom-Renderer mit Teilaktualisierung
- `tools/` – Packer und Build-Skripte (APP-Aufbau nach der bestätigten TEST001-Basis)
- `tests/` – Logik-, KI-, Steuerungs- und Pixeltests auf dem PC
- `docs/` – Notizen zu den Grafikversionen

## Grenzen

Die PC-Tests führen keine IC35-Firmware aus. Ein grüner Build belegt keine Gerätetauglichkeit; Neuerungen wurden jeweils auf dem echten IC35 geprüft. Die KI ist bewusst begrenzt (kein vollständiger Suchbaum), damit sie auf dem Gerät ohne spürbare Wartezeit zieht.

---

## English summary

Native Connect Four for the Siemens IC35, built as a standalone `.APP` with no BASIC runtime.
Play against an AI (Easy, Medium, Hard) on a full-screen graphical board. Winning discs are
highlighted inverted and the direction of the winning line is shown.

Controls: left/right arrows choose the column; down arrow, Enter or Space drops the disc; `1`–`7` pick a column directly; `N` new game, `S` difficulty, `Q` quit.

Install: download the ZIP from [Releases](../../releases), back up your organizer data and card, and copy `VIER.APP` to `IC35\APP\VIER.APP`.

Build: `python3 -m unittest discover -s tests -v`, then `python3 tools/build_game.py` (requires SDCC). No proprietary SDK binaries are used. Host tests do not run IC35 firmware, so a green build does not imply device compatibility.
