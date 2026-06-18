# md-editor

Editor/vizualizator WYSIWYG de Markdown în Qt6 + C++17. În mod implicit editezi
direct pe textul deja randat, fără să te lupți cu sintaxa; dar opțional poți vedea
codul Markdown și chiar să ai codul și randarea lui în paralel (vizualizare divizată)
și să editezi în oricare dintre cele două părți. La salvare se serializează mereu în
Markdown curat.

## Ce face pentru tine

- **WYSIWYG real**: vezi rezultatul, nu simbolurile.
- **Round-trip fidel**: ceea ce deschizi este ceea ce salvezi, cu tabele aliniate,
  liste de sarcini, citate, blocuri de cod și formule.
- **Trei moduri de lucru**: doar randat (implicit), doar cod, sau ambele în paralel
  (vizualizare divizată sincronizată).
- **Mod fără distrageri**: coloană de lectură centrată, fără bare (F11), cu cuprinsul
  opțional (îl afișezi sau îl ascunzi).
- **Grijă pentru ochi**: *Lumina caldă nocturnă* atenuează albastrul fundalului în mod
  gradual în funcție de ora din zi, pentru a reduce oboseala oculară pe timpul nopții.
- **Formule TeX**: [în linie](Caracteristicas-ro#formule-tex) și [în bloc](Caracteristicas-ro#formule-tex),
  cu superscripte/subscripte reale și previzualizare în timp real, fără dependențe externe.
- **Export** în PDF, HTML, ODF (.odt) și LaTeX (.tex), păstrând limba documentului
  și formatul formulelor.
- **Vizualizare**: 1) 6 teme deschise și întunecate, 2) zoom pentru întreaga interfață,
  3) interfață tradusă în 9 limbi.

## Cum începi

- [Instalare](Instalacion-ro)
- [Utilizare](Uso-ro)
- [Caracteristici](Caracteristicas-ro)
- [Scurtături de tastatură](Atajos-ro)

---

*md-editor este dezvoltat de Manuel Arias Calleja. Licență GPL-3.0.*
