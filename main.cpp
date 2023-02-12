#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QDebug>
#include <QQmlContext>
#include <QGuiApplication> // added
#include "engineclass.h"

int main(int argc, char *argv[])
{
    qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));
    qputenv("QT_VIRTUALKEYBOARD_STYLE", "pineroot");
    qputenv("QT_QPA_FONTDIR", "/usr/share/fonts/");
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
    engineClass eClass;
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("eClass",&eClass);
    engine.addImportPath(QStringLiteral("qrc:/"));
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    // Vault mode
    QStringList args = app.arguments();
    if (args.count() == 2)
    {
        if ( args.at(1).contains("vault") )
        {
            eClass.setVaultMode(true);
        } else {
            eClass.setVaultMode(false);
        }
    } else {
        eClass.setVaultMode(false);
    }
    return app.exec();
}
