#ifndef THEMECONTROLLER_H
#define THEMECONTROLLER_H

#include "themespec.h"

#include <QObject>
#include <QPalette>

class QTextEdit;
class CodeBlockHighlighter;
class QColor;
class QTime;
class QTimer;

// Gestiona el tema claro/oscuro: aplica la paleta de la aplicación, recolorea
// los enlaces para que sigan siendo legibles, sincroniza el resaltador de
// código y persiste la elección (vía AppSettings).
//
// Además ofrece una "luz cálida nocturna": tiñe el fondo del editor de un tono
// ámbar cuya intensidad depende de la hora (neutro de día, máximo de noche).
// Un temporizador la refresca periódicamente para que cambie sola al pasar las
// horas.
class ThemeController : public QObject
{
    Q_OBJECT

public:
    ThemeController(QTextEdit *editor, CodeBlockHighlighter *highlighter,
                    QObject *parent = nullptr);

    mdtheme::ThemeId currentTheme() const;
    bool isDark() const;
    bool isWarmLight() const;

    // Aplica el tema (paleta + enlaces + resaltado) y lo persiste.
    void applyTheme(mdtheme::ThemeId id);

    // Activa/desactiva la luz cálida nocturna, la persiste y reaplica el tema.
    void setWarmLight(bool on);

    // Recolorea los enlaces del documento con el color del tema actual. Útil
    // tras cargar un archivo, cuyos enlaces traen un color fijo del Markdown.
    void recolorLinks();

signals:
    void themeChanged(mdtheme::ThemeId id);

private:
    QColor linkColor() const;

    // Construye la paleta del tema (desde su ThemeSpec) sin tinte cálido.
    QPalette buildPalette(mdtheme::ThemeId id) const;
    // Intensidad del tinte [0,1] según la hora actual (0 si está desactivado).
    double currentWarmth() const;
    // Intensidad del tinte [0,1] para una hora dada: neutro de día, máximo de
    // noche, con rampas suaves al amanecer y al anochecer.
    static double warmthForTime(const QTime &t);
    // Tiñe Base/AlternateBase de la paleta hacia ámbar con intensidad `w`.
    void applyWarmth(QPalette &palette, double w) const;
    // Recalcula la hora y, si el tinte cambió, reaplica solo la paleta (sin
    // recolorear enlaces). Lo dispara el temporizador.
    void refreshWarmth();

    QTextEdit *m_editor;
    CodeBlockHighlighter *m_highlighter;
    mdtheme::ThemeId m_current = mdtheme::ThemeId::Light;
    bool m_warmLight = true;
    double m_lastWarmth = -1.0;  // último tinte aplicado, para evitar repintados
    QTimer *m_warmTimer = nullptr;
};

#endif // THEMECONTROLLER_H
