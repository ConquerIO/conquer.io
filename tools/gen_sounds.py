#!/usr/bin/env python3
"""Genera los efectos de sonido del juego (WAV 16-bit mono, estilo 8-bit).

Crea assets/sounds/<evento>/<evento>_1.wav ... _10.wav (10 variantes por evento)
y, si no existe, assets/sounds/seleccion.txt con la variante elegida por evento.

Uso (desde cualquier carpeta):
    python3 tools/gen_sounds.py
"""
import math
import os
import random
import struct
import wave

RATE = 22050
VARIANTS = 10
ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "assets", "sounds")


def _wave(kind, phase):
    """Onda de periodo 1 evaluada en 'phase' (cualquier real)."""
    p = phase % 1.0
    if kind == "square":
        return 1.0 if p < 0.5 else -1.0
    if kind == "pulse":  # cuadrada estrecha, sonido más "nasal"
        return 1.0 if p < 0.25 else -1.0
    if kind == "tri":
        return 4.0 * abs(p - 0.5) - 1.0
    if kind == "saw":
        return 2.0 * p - 1.0
    return math.sin(2.0 * math.pi * p)  # sine


def tone(f0, f1, dur, kind="square", vol=0.5, decay=3.0):
    """Tono con barrido de frecuencia f0 -> f1 y envolvente exponencial."""
    n = int(RATE * dur)
    out, phase = [], 0.0
    for i in range(n):
        t = i / max(1, n - 1)
        f = f0 + (f1 - f0) * t
        phase += f / RATE
        env = math.exp(-decay * t)
        out.append(_wave(kind, phase) * vol * env)
    return out


def noise(dur, vol=0.4, decay=6.0, seed=1):
    rnd = random.Random(seed)
    n = int(RATE * dur)
    return [rnd.uniform(-1, 1) * vol * math.exp(-decay * i / max(1, n - 1)) for i in range(n)]


def silence(dur):
    return [0.0] * int(RATE * dur)


def mix(*parts):
    """Suma varias pistas (listas) alineadas al inicio."""
    n = max(len(p) for p in parts)
    out = [0.0] * n
    for p in parts:
        for i, s in enumerate(p):
            out[i] += s
    return out


def seq(*parts):
    out = []
    for p in parts:
        out.extend(p)
    return out


def save(path, samples):
    # fundido de 2 ms en los extremos para evitar clics
    fade = int(RATE * 0.002)
    for i in range(min(fade, len(samples))):
        k = i / fade
        samples[i] *= k
        samples[-1 - i] *= k
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with wave.open(path, "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(RATE)
        frames = b"".join(
            struct.pack("<h", int(max(-1.0, min(1.0, s)) * 32767)) for s in samples
        )
        w.writeframes(frames)


# --- Recetas: VARIANTS variantes por evento -----------------------------------------

def ui_click():
    return [
        tone(880, 660, 0.06, "square", 0.40, 5),
        tone(600, 600, 0.07, "tri", 0.60, 4),
        tone(1200, 900, 0.05, "pulse", 0.35, 6),
        seq(tone(700, 700, 0.03, "square", 0.35, 3), tone(1000, 1000, 0.05, "square", 0.35, 5)),
        mix(noise(0.03, 0.35, 8, 3), tone(800, 500, 0.06, "tri", 0.50, 5)),
        tone(1000, 400, 0.05, "saw", 0.30, 6),
        tone(450, 450, 0.06, "sine", 0.55, 4),
        seq(tone(1100, 1100, 0.025, "pulse", 0.30, 3), tone(750, 750, 0.05, "pulse", 0.30, 5)),
        mix(tone(900, 700, 0.05, "square", 0.25, 5), tone(1350, 1050, 0.05, "tri", 0.25, 5)),
        mix(noise(0.02, 0.30, 10, 21), tone(1000, 600, 0.05, "square", 0.30, 6)),
    ]


def ui_hover():
    return [
        tone(1200, 1200, 0.03, "tri", 0.18, 4),
        tone(900, 1100, 0.04, "sine", 0.22, 4),
        tone(1500, 1500, 0.02, "square", 0.10, 5),
        tone(700, 900, 0.035, "tri", 0.20, 4),
        noise(0.025, 0.12, 8, 5),
        tone(1000, 1000, 0.03, "sine", 0.20, 4),
        tone(1300, 1600, 0.03, "tri", 0.18, 4),
        tone(800, 800, 0.025, "pulse", 0.12, 5),
        noise(0.02, 0.10, 9, 23),
        tone(1100, 900, 0.03, "square", 0.10, 5),
    ]


def ui_change():
    return [
        seq(tone(600, 600, 0.05, "square", 0.35, 3), tone(800, 800, 0.07, "square", 0.35, 4)),
        seq(tone(500, 500, 0.05, "tri", 0.55, 3), tone(750, 750, 0.08, "tri", 0.55, 4)),
        tone(500, 900, 0.10, "pulse", 0.35, 3),
        seq(tone(900, 900, 0.04, "sine", 0.50, 3), tone(1200, 1200, 0.06, "sine", 0.50, 4)),
        seq(tone(660, 660, 0.04, "square", 0.30, 3), tone(880, 880, 0.04, "square", 0.30, 3),
            tone(1100, 1100, 0.05, "square", 0.30, 4)),
        seq(tone(700, 700, 0.04, "tri", 0.50, 3), tone(500, 500, 0.06, "tri", 0.50, 4)),
        tone(900, 500, 0.10, "square", 0.30, 3),
        seq(tone(550, 550, 0.04, "pulse", 0.30, 3), tone(825, 825, 0.07, "pulse", 0.30, 4)),
        seq(tone(800, 800, 0.04, "sine", 0.50, 3), tone(1000, 1000, 0.04, "sine", 0.50, 3),
            tone(800, 800, 0.05, "sine", 0.50, 4)),
        mix(tone(600, 900, 0.09, "saw", 0.22, 3), tone(300, 450, 0.09, "tri", 0.35, 3)),
    ]


def ui_type():
    return [
        noise(0.02, 0.35, 10, 7),
        tone(1500, 1300, 0.02, "square", 0.25, 6),
        mix(noise(0.02, 0.25, 10, 9), tone(1000, 800, 0.025, "tri", 0.35, 6)),
        tone(1800, 1800, 0.015, "pulse", 0.22, 6),
        tone(900, 700, 0.025, "sine", 0.40, 6),
        noise(0.015, 0.30, 12, 25),
        tone(1200, 900, 0.02, "pulse", 0.25, 6),
        mix(noise(0.015, 0.20, 12, 27), tone(2000, 1500, 0.015, "square", 0.15, 6)),
        tone(700, 500, 0.02, "tri", 0.45, 6),
        tone(1600, 1600, 0.02, "sine", 0.30, 6),
    ]


def game_start():
    def arp(notes, kind, step=0.07, last=0.25):
        parts = [tone(f, f, step, kind, 0.40, 2) for f in notes[:-1]]
        parts.append(tone(notes[-1], notes[-1], last, kind, 0.45, 3))
        return seq(*parts)

    return [
        arp([392, 523, 659, 784], "square"),
        arp([262, 330, 392, 523], "tri"),
        arp([440, 554, 659, 880], "pulse"),
        seq(tone(300, 900, 0.25, "saw", 0.28, 1.5), tone(900, 900, 0.15, "square", 0.30, 4)),
        arp([523, 392, 523, 784], "square"),
        arp([330, 392, 494, 659], "tri"),
        arp([523, 659, 784, 1047], "pulse"),
        arp([294, 370, 440, 587], "square"),
        seq(tone(200, 600, 0.2, "tri", 0.50, 1.5), tone(600, 1200, 0.2, "square", 0.30, 3)),
        arp([392, 330, 392, 523, 784], "square", 0.06, 0.3),
    ]


def attack():
    return [
        mix(noise(0.18, 0.40, 5, 11), tone(600, 150, 0.18, "square", 0.35, 4)),
        tone(900, 200, 0.20, "saw", 0.40, 4),
        seq(tone(500, 500, 0.04, "pulse", 0.40, 3), tone(300, 300, 0.04, "pulse", 0.40, 3),
            tone(700, 200, 0.12, "square", 0.40, 5)),
        mix(noise(0.25, 0.45, 4, 13), tone(200, 80, 0.25, "tri", 0.60, 3)),
        seq(tone(400, 800, 0.08, "square", 0.35, 2), tone(800, 200, 0.12, "square", 0.35, 5)),
        mix(noise(0.12, 0.40, 6, 31), tone(1000, 300, 0.12, "saw", 0.30, 5)),
        seq(tone(800, 800, 0.03, "square", 0.40, 3),
            mix(noise(0.15, 0.35, 5, 33), tone(400, 100, 0.15, "square", 0.30, 4))),
        tone(300, 60, 0.3, "tri", 0.65, 3),
        mix(noise(0.20, 0.40, 4, 35), tone(700, 100, 0.20, "pulse", 0.35, 4)),
        seq(tone(600, 300, 0.05, "saw", 0.40, 3), tone(500, 200, 0.05, "saw", 0.40, 3),
            tone(400, 100, 0.12, "saw", 0.40, 5)),
    ]


def context_menu():
    return [
        tone(500, 900, 0.07, "sine", 0.50, 3),
        tone(700, 1000, 0.06, "tri", 0.55, 4),
        seq(tone(800, 800, 0.03, "square", 0.30, 3), tone(1100, 1100, 0.04, "square", 0.30, 5)),
        tone(400, 1200, 0.09, "pulse", 0.30, 3),
        mix(noise(0.03, 0.20, 8, 17), tone(650, 950, 0.07, "tri", 0.45, 4)),
        tone(600, 800, 0.06, "square", 0.25, 3),
        seq(tone(500, 500, 0.03, "sine", 0.45, 3), tone(750, 750, 0.05, "sine", 0.45, 5)),
        tone(1000, 500, 0.08, "tri", 0.50, 4),
        mix(tone(500, 800, 0.07, "pulse", 0.25, 3), tone(1000, 1600, 0.07, "tri", 0.20, 3)),
        seq(tone(900, 900, 0.03, "tri", 0.50, 3), tone(600, 600, 0.03, "tri", 0.50, 3),
            tone(1200, 1200, 0.04, "tri", 0.50, 5)),
    ]


def game_over():
    def desc(notes, kind, step=0.16, last=0.55):
        parts = [tone(f, f, step, kind, 0.40, 2) for f in notes[:-1]]
        parts.append(tone(notes[-1], notes[-1] * 0.8, last, kind, 0.45, 3))
        return seq(*parts)

    return [
        desc([523, 466, 392, 262], "square"),
        desc([440, 392, 330, 220], "tri", 0.18, 0.6),
        seq(tone(400, 100, 0.8, "saw", 0.35, 2.5)),
        desc([392, 370, 349, 330, 294], "pulse", 0.12, 0.6),
        seq(mix(tone(220, 220, 0.5, "square", 0.25, 2), tone(233, 233, 0.5, "square", 0.25, 2)),
            tone(165, 110, 0.6, "tri", 0.55, 3)),
        desc([392, 330, 262, 196], "square", 0.2, 0.6),
        desc([494, 440, 392, 330, 247], "tri", 0.14, 0.6),
        seq(tone(600, 150, 0.5, "square", 0.30, 2.5), noise(0.3, 0.25, 5, 37)),
        desc([330, 311, 294, 262], "pulse", 0.18, 0.6),
        seq(tone(300, 300, 0.15, "saw", 0.30, 2), tone(250, 250, 0.15, "saw", 0.30, 2),
            tone(200, 100, 0.6, "saw", 0.35, 3)),
    ]


def winner():
    def fan(notes, kind, step=0.12, last=0.6):
        parts = [tone(f, f, step, kind, 0.40, 1.5) for f in notes[:-1]]
        parts.append(mix(tone(notes[-1], notes[-1], last, kind, 0.35, 2.5),
                         tone(notes[-1] * 1.5, notes[-1] * 1.5, last, "tri", 0.25, 2.5)))
        return seq(*parts)

    return [
        fan([523, 523, 523, 659, 784], "square"),
        fan([392, 494, 587, 784], "tri", 0.14, 0.7),
        fan([262, 330, 392, 523, 659, 784], "pulse", 0.09, 0.7),
        seq(fan([523, 659], "square", 0.1, 0.12), silence(0.04), fan([659, 784, 1047], "square", 0.1, 0.7)),
        fan([440, 554, 659, 880], "square", 0.12, 0.7),
        fan([392, 523, 659, 784], "square", 0.1, 0.7),
        fan([330, 440, 554, 659, 880], "tri", 0.1, 0.7),
        fan([523, 659, 784, 1047], "pulse", 0.11, 0.7),
        fan([294, 370, 440, 587, 740], "square", 0.1, 0.7),
        seq(fan([392, 392], "square", 0.08, 0.1), silence(0.04), fan([523, 659, 784], "tri", 0.12, 0.7)),
    ]


EVENTS = {
    "ui_click": ui_click,
    "ui_hover": ui_hover,
    "ui_change": ui_change,
    "ui_type": ui_type,
    "game_start": game_start,
    "attack": attack,
    "context_menu": context_menu,
    "game_over": game_over,
    "winner": winner,
}


def main():
    total = 0
    for name, recipe in EVENTS.items():
        variants = recipe()
        assert len(variants) == VARIANTS, name
        for i, samples in enumerate(variants, start=1):
            save(os.path.join(ROOT, name, f"{name}_{i}.wav"), list(samples))
            total += 1

    sel = os.path.join(ROOT, "seleccion.txt")
    if not os.path.exists(sel):  # no pisar la elección del usuario
        with open(sel, "w", encoding="utf-8") as f:
            f.write("# Variante (1-10) que suena en cada evento. Reinicia el juego tras cambiarla.\n")
            for name in EVENTS:
                f.write(f"{name}=1\n")
    print(f"Generados {total} sonidos en {os.path.normpath(ROOT)}")


if __name__ == "__main__":
    main()
