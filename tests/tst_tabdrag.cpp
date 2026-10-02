#include <QtTest>

#include "mainwindow.h"
#include "tabdrag.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QSettings>
#include <QTemporaryDir>
#include <QTextEdit>

// Pruebas del arrastre de pestañas entre instancias: el MIME propio (ida y vuelta y
// rechazo de lo ajeno o corrupto), cuándo el ratón «sale» de la barra y quién acepta.
class TestTabDrag : public QObject
{
    Q_OBJECT

private slots:
    void mimeRoundTrips();
    void mimeRejectsForeignAndBroken();
    void leavingTheBarNeedsTheMargin();
    void ownDragIsNotAccepted();
    void initTestCase();
    void cleanup() { QSettings().clear(); }
    void windowAcceptsForeignTabOverTheEditor();
    void windowOpensTheDroppedTab();
    void windowClosesWhenItsLastTabIsAdopted();
};

void TestTabDrag::mimeRoundTrips()
{
    const mdtabdrag::Payload in{QStringLiteral("/tmp/ñ ñ.md"), 1234, QStringLiteral("md-editor-u-7")};
    std::unique_ptr<QMimeData> mime(mdtabdrag::toMime(in));
    QVERIFY(!mime->hasUrls());  // a propósito: ver tabdrag.h
    mdtabdrag::Payload out;
    QVERIFY(mdtabdrag::fromMime(mime.get(), out));
    QCOMPARE(out.path, in.path);
    QCOMPARE(out.cursor, in.cursor);
    QCOMPARE(out.source, in.source);
}

void TestTabDrag::mimeRejectsForeignAndBroken()
{
    mdtabdrag::Payload out;
    QVERIFY(!mdtabdrag::fromMime(nullptr, out));
    QMimeData text;
    text.setText(QStringLiteral("hola"));
    QVERIFY(!mdtabdrag::fromMime(&text, out));
    QMimeData broken;
    broken.setData(mdtabdrag::kMimeType, "basura");
    QVERIFY(!mdtabdrag::fromMime(&broken, out));
    // Sin ruta no hay nada que abrir.
    std::unique_ptr<QMimeData> empty(mdtabdrag::toMime({QString(), 0, QStringLiteral("x")}));
    QVERIFY(!mdtabdrag::fromMime(empty.get(), out));
}

void TestTabDrag::leavingTheBarNeedsTheMargin()
{
    const QRect bar(0, 0, 800, 30);
    QVERIFY(!mdtabdrag::leftBar(QPoint(400, 15), bar, 20));   // dentro
    QVERIFY(!mdtabdrag::leftBar(QPoint(400, 45), bar, 20));   // fuera pero dentro del margen
    QVERIFY(mdtabdrag::leftBar(QPoint(400, 60), bar, 20));    // debajo, lejos
    QVERIFY(mdtabdrag::leftBar(QPoint(400, -40), bar, 20));   // encima, lejos
    QVERIFY(mdtabdrag::leftBar(QPoint(900, 15), bar, 20));    // más allá del borde
}

void TestTabDrag::ownDragIsNotAccepted()
{
    const QString self = QStringLiteral("md-editor-u-1");
    QVERIFY(!mdtabdrag::accepts({QStringLiteral("/a.md"), 0, self}, self));
    QVERIFY(mdtabdrag::accepts({QStringLiteral("/a.md"), 0, QStringLiteral("md-editor-u-2")}, self));
    QVERIFY(!mdtabdrag::accepts({QString(), 0, QStringLiteral("md-editor-u-2")}, self));
}

void TestTabDrag::initTestCase()
{
    QCoreApplication::setOrganizationName(QStringLiteral("md-editor-test"));
    QCoreApplication::setApplicationName(QStringLiteral("md-editor-test"));
}

// Integración: el arrastre llega al QTextEdit (que por sí solo rechazaría el MIME
// propio), así que lo tiene que atender el filtro de la ventana.
void TestTabDrag::windowAcceptsForeignTabOverTheEditor()
{
    MainWindow w;
    w.setInstanceName(QStringLiteral("md-editor-test-self"));
    w.show();
    QWidget *target = w.findChild<QTextEdit *>()->viewport();
    const Qt::DropActions acts = Qt::MoveAction;

    std::unique_ptr<QMimeData> foreign(
        mdtabdrag::toMime({QStringLiteral("/tmp/x.md"), 0, QStringLiteral("md-editor-test-otra")}));
    QDragEnterEvent in(QPoint(5, 5), acts, foreign.get(), Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(target, &in);
    QVERIFY(in.isAccepted());

    std::unique_ptr<QMimeData> own(
        mdtabdrag::toMime({QStringLiteral("/tmp/x.md"), 0, QStringLiteral("md-editor-test-self")}));
    QDragEnterEvent mine(QPoint(5, 5), acts, own.get(), Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(target, &mine);
    QVERIFY(!mine.isAccepted());
}

void TestTabDrag::windowOpensTheDroppedTab()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("soltado.md"));
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write("# Titulo\n\ntexto\n");
    f.close();

    MainWindow w;
    w.setInstanceName(QStringLiteral("md-editor-test-self"));
    w.show();
    QVERIFY(!w.hasOpenFile(path));
    // Origen inexistente: el aviso de «adoptada» falla enseguida, sin bloquear.
    std::unique_ptr<QMimeData> mime(
        mdtabdrag::toMime({path, 3, QStringLiteral("md-editor-test-nadie")}));
    QDropEvent drop(QPointF(5, 5), Qt::MoveAction, mime.get(), Qt::LeftButton, Qt::NoModifier);
    // Directo al manejador: un QDropEvent sintético enviado por QApplication::notify
    // sin un QDrag real detrás hace caer a Qt antes de llegar al filtro.
    QVERIFY(w.handleTabDropEvent(&drop));
    QVERIFY(drop.isAccepted());
    QTRY_VERIFY(w.hasOpenFile(path));
}

void TestTabDrag::windowClosesWhenItsLastTabIsAdopted()
{
    QTemporaryDir dir;
    const QString path = dir.filePath(QStringLiteral("unica.md"));
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write("# Titulo\n");
    f.close();
    const QString other = dir.filePath(QStringLiteral("otra.md"));
    QFile g(other);
    QVERIFY(g.open(QIODevice::WriteOnly));
    g.write("# Otra\n");
    g.close();

    // Con otra pestaña viva, la ventana sigue.
    {
        MainWindow w;
        w.show();
        w.openExternalPaths({path, other});
        QVERIFY(w.hasOpenFile(path) && w.hasOpenFile(other));
        w.closeTabForPath(path);
        QTest::qWait(50);
        QVERIFY(w.isVisible());
        QVERIFY(!w.hasOpenFile(path));
        QVERIFY(w.hasOpenFile(other));
    }
    // Con una sola, queda un documento nuevo vacío y la ventana se cierra.
    MainWindow w;
    w.show();
    w.openExternalPaths({path});
    QVERIFY(w.hasOpenFile(path));
    w.closeTabForPath(path);
    QTRY_VERIFY(!w.isVisible());
}

QTEST_MAIN(TestTabDrag)
#include "tst_tabdrag.moc"
