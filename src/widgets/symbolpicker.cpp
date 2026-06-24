/// \file
/// \brief Implementación del diálogo «mapa de caracteres» de símbolos especiales.

#include "symbolpicker.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QScrollArea>
#include <QTabWidget>
#include <QToolButton>
#include <QVBoxLayout>

#include "symbolcatalog.h"

SymbolPicker::SymbolPicker(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Símbolos especiales"));

    auto *layout = new QVBoxLayout(this);
    auto *tabs = new QTabWidget(this);
    layout->addWidget(tabs);

    // Símbolos algo más grandes que el texto base, para que se lean bien.
    QFont symbolFont = font();
    symbolFont.setPointSizeF(symbolFont.pointSizeF() * 1.4);

    constexpr int kColumns = 12;
    for (const mdsymbols::Category &category : mdsymbols::categories()) {
        auto *page = new QWidget;
        auto *grid = new QGridLayout(page);
        grid->setSpacing(2);

        int index = 0;
        for (const QString &symbol : category.symbols) {
            auto *button = new QToolButton(page);
            button->setText(symbol);
            button->setFont(symbolFont);
            button->setAutoRaise(true);
            button->setFixedSize(34, 34);
            const uint cp = symbol.toUcs4().value(0);
            button->setToolTip(QStringLiteral("U+%1")
                                   .arg(cp, 4, 16, QLatin1Char('0')).toUpper());
            connect(button, &QToolButton::clicked, this,
                    [this, symbol] { emit symbolChosen(symbol); });
            grid->addWidget(button, index / kColumns, index % kColumns);
            ++index;
        }
        // Empuja los botones a la esquina superior izquierda.
        grid->setRowStretch(grid->rowCount(), 1);
        grid->setColumnStretch(kColumns, 1);

        auto *scroll = new QScrollArea;
        scroll->setWidget(page);
        scroll->setWidgetResizable(true);
        tabs->addTab(scroll, category.name);
    }

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::hide);
    layout->addWidget(buttons);

    resize(540, 380);
}
