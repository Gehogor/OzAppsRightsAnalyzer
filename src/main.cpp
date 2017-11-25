#include "OzAppsRightsAnalyzer.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QApplication::setWindowIcon(QIcon("icons/logo_OZE.png"));
    QApplication::setFont(QFont("Calibri", 10, QFont::Normal));
    QApplication::setStyle("plastique");

    QCoreApplication::setApplicationName("OzAppsRightsAnalyzer");
    QCoreApplication::setOrganizationName("ITOP");
    QCoreApplication::setOrganizationDomain("https://www.oze.education");
    QCoreApplication::setApplicationVersion("1.2");

    OzAppsRightsAnalyzer w;
    w.loadConfiguration();
    w.show();

    return a.exec();
}
