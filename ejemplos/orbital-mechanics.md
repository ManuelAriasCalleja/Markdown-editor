# Orbital mechanics of the Solar System ☄

Gravity $F = G m_1 m_2 / r^2$ governs the orbits, and **Kepler's third law**
relates the period $T$ to the semi-major axis $a$ in a *surprisingly* simple way:

$$T^2 = \frac{4\pi^2}{GM} a^3$$

A single function is enough to estimate the period[^au] of any orbit:

```python
from math import pi, sqrt

G, M = 6.674e-11, 1.989e30         # SI · mass of the Sun
def period(a_m):                   # semi-major axis in metres
    return 2 * pi * sqrt(a_m**3 / (G * M))

print(period(1.496e11) / 86_400)   # Earth → ≈ 365 days
```

![Orbital period of the planets](orbits.png)

---

| Body    | Symbol | Key relation                         |
|---------|:------:|--------------------------------------|
| Sun     |   ☉    | $M \approx 1.99 \times 10^{30}$ kg   |
| Earth   |   ⊕    | $g = \frac{G M_\oplus}{R_\oplus^2}$  |
| Jupiter |   ♃    | $T \propto a^{3/2}$                  |
| Comet   |   ☄    | $v = \sqrt{\frac{G M}{r}}$           |

### Model verification

- [x] Derive the relation $T^2 \propto a^3$
- [ ] Cross-check against real ephemerides
    - [x] Load JPL Horizons data
    - [ ] Relative error below 1 % for the 8 planets

[^au]: 1 AU ≈ 149.6 million km (the mean Earth–Sun distance).
