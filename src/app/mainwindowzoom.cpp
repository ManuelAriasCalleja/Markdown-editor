/// \file
/// \brief Zoom/escalado de toda la interfaz de MainWindow (editor, menús, barras, esquema, iconos).

// Zoom/escalado de toda la interfaz de MainWindow: el editor y, al mismo desfase,
// cada superficie de chrome (menús —con sus workarounds de Qt 6.8 + gtk3—, barras,
// estado, esquema, iconos). Son métodos de MainWindow, separados de mainwindow.cpp
// para aligerarlo. La parte comprobable y pura vive en el módulo `chromezoom`.

#include "mainwindow.h"

#include <QApplication>
#include <QFont>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QWidget>

#include "appsettings.h"
#include "chromezoom.h"
#include "distractionfreecontroller.h"
#include "editorstack.h"
#include "findreplacebar.h"   // upcast m_findBar → QWidget* (tipo completo)
#include "focuseditor.h"
#include "outline.h"
#include "outlinepanel.h"     // upcast m_outline → QWidget* (tipo completo)
#include "splitviewcontroller.h"

void MainWindow::zoomInText()
{
    ++m_zoomDelta;
    applyZoom();
}

void MainWindow::zoomOutText()
{
    --m_zoomDelta;
    applyZoom();
}

void MainWindow::resetZoom()
{
    m_zoomDelta = 0;
    applyZoom();
}

void MainWindow::applyZoom()
{
    // No dejar que el zoom reduzca la fuente por debajo de 1 punto.
    m_zoomDelta = qMax(m_zoomDelta, int(1 - m_baseFontPointSize));

    QFont f = m_stack->editor()->font();
    f.setPointSizeF(chromezoom::scaledPointSize(m_baseFontPointSize, m_zoomDelta));
    m_stack->editor()->setFont(f);

    // Las imágenes (diagramas, `![]()`) no siguen a la fuente: se reescalan aparte
    // al mismo factor. Solo la pestaña activa; las demás, al activarse (setActiveStack).
    m_stack->setContentScale(uiScaleFactor());

    applyChromeZoom();
    AppSettings::setZoomLevel(m_zoomDelta);  // se recuerda para la próxima sesión
}

qreal MainWindow::uiScaleFactor() const
{
    if (m_baseFontPointSize <= 0)
        return 1.0;
    return chromezoom::scaledPointSize(m_baseFontPointSize, m_zoomDelta) / m_baseFontPointSize;
}

void MainWindow::applyMenuFontScale()
{
    if (m_baseMenuPointSize <= 0)
        return;
    QFont mf = QApplication::font("QMenuBar");
    mf.setPointSizeF(chromezoom::scaledPointSize(m_baseMenuPointSize, m_zoomDelta));
    // Cambiar la fuente por clase obliga a Qt a recalcular el sizeHint de los
    // popups la próxima vez que se muestren — sin esto, las anchuras de las
    // QAction quedan congeladas al tamaño con el que se construyeron.
    QApplication::setFont(mf, "QMenuBar");
    QApplication::setFont(mf, "QMenu");
}

void MainWindow::forceMenuWidths()
{
    // Qt 6.8 con la plataforma gtk3 cachea las anchuras de las QAction al primer
    // cálculo, y los cambios de fuente posteriores no las invalidan: ítems largos
    // como «Insertar columna a la izquierda» o «Acerca de» se ven elided cuando la
    // fuente del zoom es mayor que la base. Le fijamos a cada menú el mínimo que
    // necesita (calculado en chromezoom::menuMinimumWidth) para que no pueda elidir.
    for (QMenu *menu : findChildren<QMenu *>()) {
        const int width = chromezoom::menuMinimumWidth(*menu);
        if (width > 0)
            menu->setMinimumWidth(width);
    }
}

void MainWindow::applyChromeZoom()
{
    // Escala cada superficie al mismo desfase que el editor, partiendo de su
    // tamaño base. Se ignora si la fuente base no usa puntos (pointSizeF() < 0),
    // para no romper nada. La barra de estado propaga su fuente al contador.
    const auto scale = [this](QWidget *w, qreal base) {
        if (!w || base <= 0)
            return;
        QFont f = w->font();
        f.setPointSizeF(chromezoom::scaledPointSize(base, m_zoomDelta));
        w->setFont(f);
    };
    // Menús: aplicamos la fuente vía QApplication para que afecte a la
    // medición interna de Qt (ver applyMenuFontScale y el comentario del
    // constructor). Tras eso, también la propagamos a las instancias ya
    // creadas para que el cambio sea visible sin reiniciar.
    applyMenuFontScale();
    scale(menuBar(), m_baseMenuPointSize);
    for (QMenu *menu : findChildren<QMenu *>())
        scale(menu, m_baseMenuPointSize);
    // Y forzamos el ancho mínimo para evitar el elided que Qt 6.8 + gtk3
    // mete cuando la fuente del zoom difiere de la base.
    forceMenuWidths();
    scale(m_formatToolBar, m_baseToolBarPointSize);
    updateToolBarIcons();  // el tamaño de los iconos sigue a la fuente de la barra
    scale(m_findBar, m_baseFindBarPointSize);
    scale(statusBar(), m_baseStatusBarPointSize);
    scale(m_stack->split()->sourceEditor(), m_baseSourceFontPointSize);
    scale(m_outline, m_baseOutlinePointSize);
    // La columna del modo sin distracciones es en px: escálala como la fuente.
    if (m_distraction)
        m_distraction->setUiScale(uiScaleFactor());
}
