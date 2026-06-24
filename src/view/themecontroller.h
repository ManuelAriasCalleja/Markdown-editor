#ifndef THEMECONTROLLER_H
#define THEMECONTROLLER_H

/// \file
/// \brief Gestión del tema claro/oscuro, recoloreado de enlaces y luz cálida nocturna.

#include "themespec.h"

#include <QObject>
#include <QPalette>

#include <Qt>  // Qt::ColorScheme

class QTextEdit;
class CodeBlockHighlighter;
class QColor;
class QTime;
class QTimer;

/// Gestiona el tema claro/oscuro: aplica la paleta de la aplicación, recolorea
/// los enlaces para que sigan siendo legibles, sincroniza el resaltador de
/// código y persiste la elección (vía AppSettings).
///
/// Además ofrece una "luz cálida nocturna": tiñe el fondo del editor de un tono
/// ámbar cuya intensidad depende de la hora (neutro de día, máximo de noche).
/// Un temporizador la refresca periódicamente para que cambie sola al pasar las
/// horas.
class ThemeController : public QObject
{
    Q_OBJECT

public:
    ThemeController(QTextEdit *editor, CodeBlockHighlighter *highlighter,
                    QObject *parent = nullptr);

    /// \brief Tema actualmente aplicado.
    mdtheme::ThemeId currentTheme() const;
    /// \brief Indica si el tema actual es oscuro.
    bool isDark() const;
    /// \brief Indica si la luz cálida nocturna está activada.
    bool isWarmLight() const;
    /// \brief Indica si se está siguiendo el tema claro/oscuro del SO.
    bool followsSystem() const;

    /// Tema que corresponde al esquema de color actual del sistema operativo
    /// (Oscuro si el SO está en oscuro; Claro si está en claro o es desconocido).
    mdtheme::ThemeId systemTheme() const;

    /// \brief Aplica el tema (paleta + enlaces + resaltado) y lo persiste.
    void applyTheme(mdtheme::ThemeId id);

    /// \brief Activa/desactiva la luz cálida nocturna, la persiste y reaplica el tema.
    void setWarmLight(bool on);

    /// Activa/desactiva el seguimiento del tema del SO y lo persiste. Al activarlo
    /// aplica de inmediato el tema que corresponde al esquema actual; al
    /// desactivarlo conserva el tema vigente.
    void setFollowSystem(bool on);

    /// Recolorea los enlaces del documento con el color del tema actual. Útil
    /// tras cargar un archivo, cuyos enlaces traen un color fijo del Markdown.
    void recolorLinks();

signals:
    /// \brief El tema cambió al indicado por `id`.
    void themeChanged(mdtheme::ThemeId id);

private:
    QColor linkColor() const;

    // Mapea un esquema de color del SO a un tema (Oscuro/Claro; Unknown -> Claro).
    static mdtheme::ThemeId themeForScheme(Qt::ColorScheme scheme);

    // Construye la paleta del tema (desde su ThemeSpec) sin tinte cálido.
    QPalette buildPalette(mdtheme::ThemeId id) const;
    // Intensidad del tinte [0,1] según la hora actual (0 si está desactivado).
    double currentWarmth() const;
    // Intensidad del tinte [0,1] para una hora dada: neutro de día, máximo de
    // noche, con rampas suaves al amanecer y al anochecer.
    static double warmthForTime(const QTime &t);
    // Tiñe Base/AlternateBase de la paleta hacia ámbar con intensidad `w`.
    void applyWarmth(QPalette &palette, double w) const;
    // Construye la paleta del tema `id`, le aplica el tinte cálido de la hora
    // actual (recordándolo en m_lastWarmth) y la instala en la aplicación.
    void applyPaletteWithWarmth(mdtheme::ThemeId id);
    // Recalcula la hora y, si el tinte cambió, reaplica solo la paleta (sin
    // recolorear enlaces). Lo dispara el temporizador.
    void refreshWarmth();

    QTextEdit *m_editor;
    CodeBlockHighlighter *m_highlighter;
    mdtheme::ThemeId m_current = mdtheme::ThemeId::Light;
    bool m_warmLight = true;
    bool m_followSystem = false;  // seguir el tema claro/oscuro del SO
    double m_lastWarmth = -1.0;  // último tinte aplicado, para evitar repintados
    QTimer *m_warmTimer = nullptr;
};

#endif // THEMECONTROLLER_H
