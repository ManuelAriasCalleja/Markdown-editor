#include <QtTest>

#include <QTextDocument>
#include <QTreeWidget>
#include <QTreeWidgetItem>

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

private:
    static QString doc3()
    {
        return QStringLiteral("# A\n\n## A1\n\n## A2\n\n# B\n\n## B1\n");
    }
};

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

QTEST_MAIN(TestOutlinePanel)
#include "tst_outlinepanel.moc"
