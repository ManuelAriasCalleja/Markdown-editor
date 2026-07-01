/// \file
/// \brief Implementación del overlay de bloque de código.

#include "codeblockoverlay.h"

#include <QHBoxLayout>
#include <QToolButton>

CodeBlockOverlay::CodeBlockOverlay(QWidget *parent) : QWidget(parent)
{
    // No roba el foco del editor al aparecer/pulsarse.
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_ShowWithoutActivating);

    // Botones normales (no auto-raise): su fondo de tema los hace legibles sobre el
    // panel de código sin necesidad de pintar el fondo del overlay (que se
    // desincronizaría del tema). Así siguen la paleta por propagación.
    m_langButton = new QToolButton(this);
    m_langButton->setFocusPolicy(Qt::NoFocus);
    m_langButton->setToolTip(tr("Cambiar el lenguaje del bloque"));

    m_copyButton = new QToolButton(this);
    m_copyButton->setFocusPolicy(Qt::NoFocus);
    m_copyButton->setText(tr("Copiar"));
    m_copyButton->setToolTip(tr("Copiar el código"));

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);
    layout->addWidget(m_langButton);
    layout->addWidget(m_copyButton);

    connect(m_copyButton, &QToolButton::clicked, this, &CodeBlockOverlay::copyRequested);
    connect(m_langButton, &QToolButton::clicked, this, &CodeBlockOverlay::languageRequested);
}

void CodeBlockOverlay::setLanguage(const QString &language)
{
    m_langButton->setText(language.isEmpty() ? tr("texto") : language);
    adjustSize();
}
