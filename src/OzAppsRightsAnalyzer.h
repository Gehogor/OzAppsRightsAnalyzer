#ifndef _OZAPPSRIGHTSANALYZER_H_
#define _OZAPPSRIGHTSANALYZER_H_

// Qt
#include <QMainWindow>
#include <QMap>
class QTreeWidget;
class QTreeWidgetItem;

namespace Ui {
class OzAppsRightsAnalyzer;
}

// OzzApps
#include "RightsAnalyzer.h"


class OzAppsRightsAnalyzer
        : public QMainWindow
{
    Q_OBJECT

public:
    // Constructor
    explicit OzAppsRightsAnalyzer(QWidget* parent=0);

    // Destructor
    ~OzAppsRightsAnalyzer();

    // Method
    void loadConfiguration();

private slots:
    void doWhenNewProject();
    void doWhenOpenFileCoreRights();
    void doWhenOpenFileProfils();
    void doWhenExportCSV();
    void doWhenSearch();
    void doWhenCollapseAll();
    void doWhenAddColor();
    void doWhenRemoveColor();
    void doWhenColorHasChanged(const QColor& color,const QString& text);

private:
    // Event methods
    void closeEvent(QCloseEvent*);

    // Methods
    void initializationOfMenu();
    void initializationOfTree();
    void initializationOfColor();
    void loadFile(const QString& fileName,bool core);
    void updateAll();
    void updateRecursive(QTreeWidgetItem* group,
                         const QVector<RightsAnalyzer::Group>& groupList);
    void exportToCSV(const QString& fileName,
                     const QVector<RightsAnalyzer::Group>& groupList);
    void searchAndExpand(QTreeWidget* tree,const QString& text);

    // Members
    Ui::OzAppsRightsAnalyzer* ui;
    RightsAnalyzer m_analyzer;
    QMap<QString,QString> m_ozAppColor;
};

#endif /* _OZAPPSRIGHTSANALYZER_H_ */
