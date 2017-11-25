#ifndef _RIGHTSANALYZER_H_
#define _RIGHTSANALYZER_H_

// Qt
#include <QStringList>
#include <QVector>
class QTextStream;


class RightsAnalyzer
{
public:
    // Struct
    struct Element{
        QString var;
        QString id;
        QString description;
        QString title;
        QStringList profils;
        bool error;
    };

    struct Group{
        Element element;
        QVector<Group> list;
    };

    // Constructor
    RightsAnalyzer(){}

    // Destructor
    ~RightsAnalyzer(){}

    // Methods
    void getRights(QTextStream* stream);
    void getProfils(QTextStream* stream);

    void mapAllVariables();
    void clearAll();

    QVector<Group> rightCore() const;
    QVector<Group> rightGroup() const;
    QVector<Group> roleGroup() const;
    QVector<Group> profilGroup() const;

private:
    // Methods
    bool classToGroupFromStartLine(const QString& line,
                                   QTextStream* stream,
                                   const QChar& startSepList,
                                   const QChar& endSepList,
                                   Group& object);

    bool listToGroupFromStartLine(const QString& line,
                                  QTextStream* stream,
                                  Group& object);

    QString getBloc(const QString& line,QTextStream* stream,QString& var);
    QString getBetweenSep(const QString& data,const QChar& sep);

    void orderGroup(QVector<Group>& groupList);

    void findProfilFromVar(Group& current);
    void findProfilFromVarRecursive(const QString& id,
                                    QVector<Group>& profilList,
                                    Group& current);
    void updateInterList();

    // Members
    QVector<Group> m_rightCore;
    QVector<Group> m_rightGroup;
    QVector<Group> m_roleGroup;
    QVector<Group> m_profilGroup;
};

#endif /* _RIGHTSANALYZER_H_ */
