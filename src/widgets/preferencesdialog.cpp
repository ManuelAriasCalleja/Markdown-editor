/// \file
/// \brief Implementación de la ventana de Preferencias.

#include "preferencesdialog.h"

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>

PreferencesDialog::PreferencesDialog(const Settings &settings, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Preferencias"));

    auto *tabs = new QTabWidget(this);

    // --- Apariencia ---
    auto *appearance = new QWidget(this);
    auto *appForm = new QFormLayout(appearance);
    QComboBox *themeCombo = makeExclusiveCombo(settings.themeActions);
    appForm->addRow(tr("Tema:"), themeCombo);
    QCheckBox *warm = makeCheck(settings.warmLight);
    appForm->addRow(warm);
    QCheckBox *follow = makeCheck(settings.followSystem);
    appForm->addRow(follow);
    tabs->addTab(appearance, tr("Apariencia"));

    // Seguir el sistema deshabilita la elección manual de tema (como en el menú).
    if (follow && themeCombo) {
        themeCombo->setEnabled(!follow->isChecked());
        connect(follow, &QCheckBox::toggled, themeCombo, &QWidget::setDisabled);
    }

    // --- Editor ---
    auto *editor = new QWidget(this);
    auto *edForm = new QFormLayout(editor);
    edForm->addRow(tr("Interlineado:"), makeExclusiveCombo(settings.lineSpacingActions));
    edForm->addRow(makeCheck(settings.currentLine));
    edForm->addRow(makeCheck(settings.focusMode));
    tabs->addTab(editor, tr("Editor"));

    // --- Impresión ---
    auto *printing = new QWidget(this);
    auto *prForm = new QFormLayout(printing);
    prForm->addRow(makeCheck(settings.pageNumbers));
    tabs->addTab(printing, tr("Impresión"));

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);

    auto *root = new QVBoxLayout(this);
    root->addWidget(tabs);
    root->addWidget(buttons);
    resize(420, 300);
}

QComboBox *PreferencesDialog::makeExclusiveCombo(const QList<QAction *> &actions)
{
    auto *combo = new QComboBox(this);
    int current = 0;
    for (int i = 0; i < actions.size(); ++i) {
        combo->addItem(actions[i]->text());
        if (actions[i]->isChecked())
            current = i;
    }
    combo->setCurrentIndex(current);
    // Cambiar la opción dispara la acción correspondiente: se aplica y se persiste
    // por la misma vía que el menú.
    connect(combo, &QComboBox::currentIndexChanged, this, [actions](int index) {
        if (index >= 0 && index < actions.size())
            actions[index]->trigger();
    });
    return combo;
}

QCheckBox *PreferencesDialog::makeCheck(QAction *action)
{
    // Reusa el texto (ya traducido) de la acción como rótulo de la casilla.
    auto *check = new QCheckBox(action->text(), this);
    check->setChecked(action->isChecked());
    connect(check, &QCheckBox::toggled, action, &QAction::setChecked);
    return check;
}
