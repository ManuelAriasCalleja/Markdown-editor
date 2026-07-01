#include <QtTest>

#include <QLineEdit>
#include <QTextDocument>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTreeWidgetItemIterator>

#include "outlinepanel.h"

// Caracterización del estado de vista del OutlinePanel (R7): el plegado que hace
// el usuario debe SOBREVIVIR a las reconstrucciones del árbol (antes rebuild hacía
// expandAll incondicional y lo perdía). Friend de OutlinePanel para inspeccionar
// el árbol interno.
class TestOutlinePanel : public QObject
{
    Q_OBJECT
private slots:
    void foldStatePersistsAcrossRebuild();
    void newDocumentStartsExpanded();
    void filterHidesNonMatchingKeepsAncestors();
    void clearingFilterRestoresAll();

private:
    static QString doc3()
    {
        return QStringLiteral("# A\n\n## A1\n\n## A2\n\n# B\n\n## B1\n");
    }
    // Devuelve los textos de los ítems NO ocultos del árbol.
    static QStringList visibleTexts(OutlinePanel &panel);
};

QStringList TestOutlinePanel::visibleTexts(OutlinePanel &panel)
{
    QStringList out;
    for (QTreeWidgetItemIterator it(panel.m_tree); *it; ++it)
        if (!(*it)->isHidden())
            out << (*it)->text(0);
    return out;
}

void TestOutlinePanel::foldStatePersistsAcrossRebuild()
{
    OutlinePanel panel;
    QTextDocument doc;
    doc.setMarkdown(doc3());
    panel.rebuild(&doc);

    // Árbol: A (A1, A2) y B (B1); todo expandido al principio.
    QCOMPARE(panel.m_tree->topLevelItemCount(), 2);
    QVERIFY(panel.m_tree->topLevelItem(0)->isExpanded());

    // El usuario pliega «A».
    panel.m_tree->topLevelItem(0)->setExpanded(false);
    QVERIFY(panel.m_collapsed.contains(QStringLiteral("A")));

    // Reconstruir (como al editar) NO debe reexpandir «A», y «B» sigue expandido.
    panel.rebuild(&doc);
    QVERIFY(!panel.m_tree->topLevelItem(0)->isExpanded());  // «A» sigue plegado
    QVERIFY(panel.m_tree->topLevelItem(1)->isExpanded());   // «B» sigue expandido
}

void TestOutlinePanel::newDocumentStartsExpanded()
{
    OutlinePanel panel;
    QTextDocument doc;
    doc.setMarkdown(doc3());
    panel.rebuild(&doc);
    QVERIFY(panel.m_tree->topLevelItem(0)->isExpanded());
    QVERIFY(panel.m_tree->topLevelItem(1)->isExpanded());
}

void TestOutlinePanel::filterHidesNonMatchingKeepsAncestors()
{
    OutlinePanel panel;
    QTextDocument doc;
    doc.setMarkdown(doc3());  // A(A1,A2) B(B1)
    panel.rebuild(&doc);

    panel.m_filter->setText(QStringLiteral("A1"));
    const QStringList vis = visibleTexts(panel);
    // Coincide «A1» y se conserva su ancestro «A»; nada de la rama B.
    QVERIFY(vis.contains(QStringLiteral("A1")));
    QVERIFY(vis.contains(QStringLiteral("A")));
    QVERIFY(!vis.contains(QStringLiteral("A2")));
    QVERIFY(!vis.contains(QStringLiteral("B")));
    QVERIFY(!vis.contains(QStringLiteral("B1")));
}

void TestOutlinePanel::clearingFilterRestoresAll()
{
    OutlinePanel panel;
    QTextDocument doc;
    doc.setMarkdown(doc3());
    panel.rebuild(&doc);

    panel.m_filter->setText(QStringLiteral("A1"));
    QCOMPARE(visibleTexts(panel).size(), 2);
    panel.m_filter->clear();
    // Todo vuelve a estar visible (5 encabezados).
    QCOMPARE(visibleTexts(panel).size(), 5);
}

QTEST_MAIN(TestOutlinePanel)
#include "tst_outlinepanel.moc"
