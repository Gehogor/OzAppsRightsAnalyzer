#define _OZAPPSRIGHTSANALYZER_CPP_
#include "OzAppsRightsAnalyzer.h"
#define _OZAPPSRIGHTSANALYZER_CPP_

// Qt
#include <QCloseEvent>
#include <QDate>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QTextStream>
#include "ui_OzAppsRightsAnalyzer.h"

// OzApps
#include "PushButtonColor.h"


// Constructor ---------------------------------------------------------------//
OzAppsRightsAnalyzer::OzAppsRightsAnalyzer(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::OzAppsRightsAnalyzer)
{
    ui->setupUi(this);

    initializationOfMenu();
    initializationOfTree();

    ui->m_addColor->setIcon(QIcon("icons/Add_128_128.png"));
    connect(ui->m_addColor,SIGNAL(clicked()),this,SLOT(doWhenAddColor()));

    ui->m_removeColor->setIcon(QIcon("icons/Remove_64_64.png"));
    connect(ui->m_removeColor,SIGNAL(clicked()),this,SLOT(doWhenRemoveColor()));
}


// Destructor ----------------------------------------------------------------//
OzAppsRightsAnalyzer::~OzAppsRightsAnalyzer()
{
    delete ui;
}


// Public method -------------------------------------------------------------//
void OzAppsRightsAnalyzer::loadConfiguration()
{
    QSettings settings(QCoreApplication::applicationName()+".ini",QSettings::IniFormat);

    // Manage the color mapping.
    m_ozAppColor.clear();

    int size = settings.beginReadArray("ozAppColor");
    for(int i=0;i<size;i++)
    {
        settings.setArrayIndex(i);
        QString key = settings.value("key").toString();
        QString color = settings.value("color").toString();

        m_ozAppColor[key] = color;
    }
    settings.endArray();
    initializationOfColor();

    // Manage the file name.
    QString coreRights = settings.value("file/coreRightsFileName").toString();
    QString profils = settings.value("file/profilsFileName").toString();

    ui->m_labelCoreRights->setText(coreRights);
    ui->m_labelProfils->setText(profils);

    if( !coreRights.isEmpty() && !profils.isEmpty() )
    {
        loadFile(coreRights,true);
        loadFile(profils,false);
    }
}


// Private slots -------------------------------------------------------------//
void OzAppsRightsAnalyzer::doWhenNewProject()
{
    QMessageBox msgBox;
    msgBox.setText( trUtf8("-- Demande de nouveau projet --") );
    msgBox.setInformativeText( trUtf8("Etes-vous sûr d'abandonner le projet actuel ?") );
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    msgBox.setIconPixmap( QPixmap("icons/Warning_64_64.png") );

    int result = msgBox.exec();
    if( result != QMessageBox::Yes )
        return;

    m_analyzer.clearAll();

    ui->m_treeRC->clear();
    ui->m_treeRCG->clear();
    ui->m_treeRRG->clear();
    ui->m_treePR->clear();
}

void OzAppsRightsAnalyzer::doWhenOpenFileCoreRights()
{
    QString coreRights = QFileDialog::getOpenFileName(this,
                                                      trUtf8("-- Chargement des droits fins --"),
                                                      "",
                                                      "Oz\'Apps (*.txt *.cs)");
    if( coreRights.isEmpty() )
        return;

    ui->m_labelCoreRights->setText(coreRights);
    loadFile(coreRights,true);
}

void OzAppsRightsAnalyzer::doWhenOpenFileProfils()
{
    QString profils = QFileDialog::getOpenFileName(this,
                                                   trUtf8("-- Chargement des profils --"),
                                                   "",
                                                   "Oz\'Apps (*.txt *.cs)");
    if( profils.isEmpty() )
        return;

    ui->m_labelProfils->setText(profils);
    loadFile(profils,false);
}

void OzAppsRightsAnalyzer::doWhenExportCSV()
{    
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    trUtf8("-- Export au format CSV --"),
                                                    "",
                                                    "Oz\'Apps (*.csv)");
    if( fileName.isEmpty() )
        return;

    fileName.remove(".csv");

    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");

    exportToCSV(fileName+"_DroitsFins_"+time,m_analyzer.rightCore());
    exportToCSV(fileName+"_GroupeDroits_"+time,m_analyzer.rightGroup());
    exportToCSV(fileName+"_Roles_"+time,m_analyzer.roleGroup());
}

void OzAppsRightsAnalyzer::doWhenSearch()
{
    bool ok;
    QString text = QInputDialog::getText(this,
                                         trUtf8("Recherche"),
                                         trUtf8("Eléments:"),
                                         QLineEdit::Normal,
                                         trUtf8("Oz\'App"),
                                         &ok);
    if(ok && !text.isEmpty())
    {
        searchAndExpand(ui->m_treeRC,text);
        searchAndExpand(ui->m_treeRCG,text);
        searchAndExpand(ui->m_treeRRG,text);
        searchAndExpand(ui->m_treePR,text);
    }
}

void OzAppsRightsAnalyzer::doWhenCollapseAll()
{
    ui->m_treeRC->collapseAll();
    ui->m_treeRCG->collapseAll();
    ui->m_treeRRG->collapseAll();
    ui->m_treePR->collapseAll();
}

void OzAppsRightsAnalyzer::doWhenAddColor()
{
    bool ok;
    QString text = QInputDialog::getText(this,
                                         trUtf8("Ajout d'une règle de couleur"),
                                         trUtf8("Nom de l'ID :"),
                                         QLineEdit::Normal,
                                         trUtf8("Oz\'App"),
                                         &ok);
    if(ok && !text.isEmpty())
    {
        m_ozAppColor[text] = "#999999";
        initializationOfColor();
    }
}

void OzAppsRightsAnalyzer::doWhenRemoveColor()
{
    if( m_ozAppColor.size() == 0 )
        return;

    QStringList list;
    bool ok;
    QMap<QString,QString>::iterator it;
    for(it=m_ozAppColor.begin();it!=m_ozAppColor.end();it++)
        list.append( it.key() );

    QString text = QInputDialog::getItem(0,
                                         trUtf8("Suppression d'une règle de couleur"),
                                         trUtf8("Nom de l'ID : "),
                                         list,
                                         0,
                                         false,
                                         &ok);
    if(ok && !text.isEmpty())
    {
        m_ozAppColor.remove(text);
        initializationOfColor();
    }
}

void OzAppsRightsAnalyzer::doWhenColorHasChanged(const QColor& color,
                                                 const QString& text)
{
    m_ozAppColor[text] = color.name();
    updateAll();
}


// Private -------------------------------------------------------------------//
void OzAppsRightsAnalyzer::closeEvent(QCloseEvent* event)
{
    QMessageBox msgBox;
    msgBox.setText( trUtf8("-- Fermeture de l'application --") );
    msgBox.setInformativeText( trUtf8("Etes-vous sûr de vouloire quitter ?") );
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    msgBox.setIconPixmap( QPixmap("icons/Warning_64_64.png") );

    int result = msgBox.exec();
    if( result == QMessageBox::Yes )
    {
        QSettings settings(QCoreApplication::applicationName()+".ini",QSettings::IniFormat);

        settings.setValue("file/coreRightsFileName",ui->m_labelCoreRights->text());
        settings.setValue("file/profilsFileName",ui->m_labelProfils->text());

        QMap<QString,QString>::iterator it;
        int i=0;
        settings.beginWriteArray("ozAppColor");
        for(it=m_ozAppColor.begin();it!=m_ozAppColor.end();i++,it++)
        {
            settings.setArrayIndex(i);
            settings.setValue("key",it.key());
            settings.setValue("color",it.value());
        }
        settings.endArray();

        event->accept();
        return;
    }

    event->ignore();
}

void OzAppsRightsAnalyzer::initializationOfMenu()
{
    QMenuBar* menu = menuBar();

    // File menu ----------------------
    QMenu* fileMenu = menu->addMenu(trUtf8("&Fichier"));

    QAction* action = fileMenu->addAction(QIcon("icons/Project_64_64.png"),
                                          trUtf8("&Nouveau projet"),
                                          this,
                                          SLOT(doWhenNewProject()));
    fileMenu->addSeparator();

    action = fileMenu->addAction(QIcon("icons/Load_16_16.png"),
                                 trUtf8("&Charger les droits fins"),
                                 this,
                                 SLOT(doWhenOpenFileCoreRights()));

    action->setStatusTip(trUtf8("Charge les droits fins provenant du fichier source."));

    action = fileMenu->addAction(QIcon("icons/Load_16_16.png"),
                                 trUtf8("&Charger les profils"),
                                 this,
                                 SLOT(doWhenOpenFileProfils()));

    action->setStatusTip(trUtf8("Charge les profils provenant du fichier source."));

    fileMenu->addSeparator();

    action = fileMenu->addAction(QIcon("icons/Export_16_16.png"),
                                 trUtf8("&Exporter au format CSV"),
                                 this,
                                 SLOT(doWhenExportCSV()),
                                 QKeySequence(tr("Ctrl+e")));

    action->setStatusTip(trUtf8("Exporte les données en trois fichiers CSV."));

    // Tool menu ----------------------
    QMenu* toolMenu = menu->addMenu(trUtf8("&Outils"));

    action = toolMenu->addAction(QIcon("icons/Search_16_16.png"),
                                 trUtf8("&Chercher"),
                                 this,
                                 SLOT(doWhenSearch()),
                                 QKeySequence(tr("Ctrl+f")));

    action = toolMenu->addAction(QIcon("icons/Collapse_16_16.png"),
                                 trUtf8("&Tout replier"),
                                 this,
                                 SLOT(doWhenCollapseAll()),
                                 QKeySequence(tr("Ctrl+q")));
}

void OzAppsRightsAnalyzer::initializationOfTree()
{
    ui->m_treeRC->setColumnCount(4);
    QTreeWidgetItem* header = new QTreeWidgetItem();
    header->setText(0,"ID");
    header->setText(1,"Titre");
    header->setText(2,"Description");
    header->setText(3,"Variable");
    ui->m_treeRC->setHeaderItem(header);

    ui->m_treeRCG->setColumnCount(4);
    header = new QTreeWidgetItem();
    header->setText(0,"ID");
    header->setText(1,"Titre");
    header->setText(2,"Description");
    header->setText(3,"Variable");
    ui->m_treeRCG->setHeaderItem(header);

    ui->m_treeRRG->setColumnCount(4);
    header = new QTreeWidgetItem();
    header->setText(0,"ID");
    header->setText(1,"Titre");
    header->setText(2,"Description");
    header->setText(3,"Variable");
    ui->m_treeRRG->setHeaderItem(header);

    ui->m_treePR->setColumnCount(4);
    header = new QTreeWidgetItem();
    header->setText(0,"ID");
    header->setText(1,"Titre");
    header->setText(2,"Description");
    header->setText(3,"Variable");
    ui->m_treePR->setHeaderItem(header);
}

void OzAppsRightsAnalyzer::initializationOfColor()
{
    QMap<QString,QString>::iterator it;
    int i=0,j=0;

    // Remove all color buttons.
    for(int i=ui->m_colorLayout->count()-1;i>=0;i--)
        ui->m_colorLayout->takeAt(i)->widget()->deleteLater();

    // Create all color buttons.
    for(it=m_ozAppColor.begin();it!=m_ozAppColor.end();it++)
    {
        PushButtonColor* p = new PushButtonColor(QColor(it.value()),it.key(),it.key());
        p->setFixedSize(150,25);
        connect(p,SIGNAL(colorHasChanged(QColor,QString)),
                this,SLOT(doWhenColorHasChanged(QColor,QString)));

        ui->m_colorLayout->addWidget(p,i,j);

        j++;
        if( j%4 == 0 )
        {
            i++;
            j=0;
        }
    }

    QSpacerItem* vertical = new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding);
    ui->m_colorLayout->addItem(vertical,ui->m_colorLayout->count(),0,1,-1);

    QSpacerItem* horizontal = new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding);
    ui->m_colorLayout->addItem(horizontal,0,ui->m_colorLayout->count(),-1,1);
}

void OzAppsRightsAnalyzer::loadFile(const QString& fileName,bool core)
{
    QFile file(fileName);
    if( !file.open(QIODevice::ReadOnly | QIODevice::Text) )
        return;

    QTextStream in(&file);

    if( core )
        m_analyzer.getRights(&in);
    else
        m_analyzer.getProfils(&in);

    file.close();
    updateAll();
}

void OzAppsRightsAnalyzer::updateAll()
{
    if( m_analyzer.rightCore().size()==0 || m_analyzer.profilGroup().size()==0 )
        return;

    QVector<RightsAnalyzer::Group> profils = m_analyzer.profilGroup();
    ui->m_treeRC->setColumnCount(profils.size()+4);
    ui->m_treeRCG->setColumnCount(profils.size()+4);
    ui->m_treeRRG->setColumnCount(profils.size()+4);
    ui->m_treePR->setColumnCount(profils.size()+4);

    for(int i=0;i<profils.size();i++)
    {
        ui->m_treeRC->headerItem()->setText(4+i,profils[i].element.id);
        ui->m_treeRCG->headerItem()->setText(4+i,profils[i].element.id);
        ui->m_treeRRG->headerItem()->setText(4+i,profils[i].element.id);
        ui->m_treePR->headerItem()->setText(4+i,profils[i].element.id);
    }

    m_analyzer.mapAllVariables();

    ui->m_treeRC->clear();
    ui->m_treeRCG->clear();
    ui->m_treeRRG->clear();
    ui->m_treePR->clear();

    updateRecursive(ui->m_treeRC->invisibleRootItem(),m_analyzer.rightCore());
    updateRecursive(ui->m_treeRCG->invisibleRootItem(),m_analyzer.rightGroup());
    updateRecursive(ui->m_treeRRG->invisibleRootItem(),m_analyzer.roleGroup());
    updateRecursive(ui->m_treePR->invisibleRootItem(),m_analyzer.profilGroup());

    // Manage the automatic resizing.
    for(int i=4;i<ui->m_treeRC->columnCount();i++)
        ui->m_treeRC->resizeColumnToContents(i);
    for(int i=4;i<ui->m_treeRCG->columnCount();i++)
        ui->m_treeRCG->resizeColumnToContents(i);
    for(int i=4;i<ui->m_treeRRG->columnCount();i++)
        ui->m_treeRRG->resizeColumnToContents(i);
    for(int i=4;i<ui->m_treePR->columnCount();i++)
        ui->m_treePR->resizeColumnToContents(i);
}

void OzAppsRightsAnalyzer::updateRecursive(QTreeWidgetItem* group,
                                           const QVector<RightsAnalyzer::Group>& groupList)
{
    for(int n=0;n<groupList.size();n++)
    {
        QTreeWidgetItem* item = new QTreeWidgetItem();
        group->addChild( item );

        item->setText(0,groupList[n].element.id);
        item->setText(1,groupList[n].element.title);
        item->setText(2,groupList[n].element.description);
        item->setText(3,groupList[n].element.var);

        QTreeWidgetItem* header = group->treeWidget()->headerItem();
        if( groupList[n].element.error && group != group->treeWidget()->invisibleRootItem() )
        {
            QFont f = item->font(0);
            f.setOverline(true);
            f.setUnderline(true);
            item->setFont(0,f);

            for(int j=1;j<header->columnCount();j++)
            {
                f.setItalic(true);
                item->setFont(j,f);
                item->setTextColor(j,Qt::red);
            }
        }

        QMap<QString,QString>::iterator it;
        for(it=m_ozAppColor.begin();it!=m_ozAppColor.end();it++)
            if( groupList[n].element.id.contains(it.key()) )
            {
                item->setBackgroundColor(0,QColor(it.value()));
                break;
            }

        for(int i=0;i<groupList[n].element.profils.size();i++)
        {
            QString p = groupList[n].element.profils[i];

            for(int j=4;j<header->columnCount();j++)
                if( header->text(j) == p )
                {
                    item->setText(j,"oui");
                    item->setTextAlignment(j,Qt::AlignCenter);
                    item->setBackgroundColor(j,QColor(m_ozAppColor["oui"]));
                }
        }

        updateRecursive(item,groupList[n].list);
    }
}

void OzAppsRightsAnalyzer::exportToCSV(const QString& fileName,
                                       const QVector<RightsAnalyzer::Group>& groupList)
{
    if( groupList.size() == 0 )
        return;

    QFile file(fileName+".csv");
    if( !file.open(QIODevice::ReadWrite) )
        return;

    QTextStream in(&file);

    QTreeWidgetItem* header = ui->m_treeRC->headerItem();

    in<<"Oz\'Apps";
    for(int j=0;j<header->columnCount();j++)
        in<<";"<<header->text(j);
    in<<"\n";

    for(int m=0;m<groupList.size();m++)
    {
        for(int n=0;n<groupList[m].list.size();n++)
        {
            in<<groupList[m].element.id;
            in<<";"<<groupList[m].list[n].element.id;
            in<<";"<<groupList[m].list[n].element.title;
            in<<";"<<groupList[m].list[n].element.description;
            in<<";"<<groupList[m].list[n].element.var;

            for(int j=4;j<header->columnCount();j++)
            {
                for(int i=0;i<groupList[m].list[n].element.profils.size();i++)
                    if( groupList[m].list[n].element.profils[i] == header->text(j) )
                    {
                        in<<";"<<"X";
                        goto RightFound;
                    }

                in<<";"<<"-";
RightFound:
                ;
            }

            in<<"\n";
        }
    }

    file.close();
}

void OzAppsRightsAnalyzer::searchAndExpand(QTreeWidget* tree,const QString& text)
{
    tree->clearSelection();

    QList<QTreeWidgetItem*> list = tree->findItems(text,Qt::MatchRecursive|Qt::MatchContains);
    for(int i=0;i<list.size();i++)
    {
        list[i]->setSelected(true);
        list[i]->setExpanded(true);

        while( list[i]->parent() != 0 )
        {
            list[i]->parent()->setExpanded(true);
            list[i] = list[i]->parent();
        };
    }
}
