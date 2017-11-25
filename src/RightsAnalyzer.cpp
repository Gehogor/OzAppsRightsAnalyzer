#define _RIGHTSANALYZER_CPP_
#include "RightsAnalyzer.h"
#undef _RIGHTSANALYZER_CPP_

// Qt
#include <QTextStream>


// Public methods ------------------------------------------------------------//
void RightsAnalyzer::getRights(QTextStream* stream)
{
    m_rightCore.clear();
    m_rightGroup.clear();
    m_roleGroup.clear();

    QString line;
    do
    {
        Group object;
        line = stream->readLine();

        if( line.contains("DroitDTO") && !line.contains("List") )
            if( classToGroupFromStartLine(line,stream,'{','}',object) )
                m_rightCore.append( object );

        if( line.contains("GroupeDroitsDTO") && !line.contains("List") )
            if( classToGroupFromStartLine(line,stream,'{','}',object) )
                m_rightGroup.append( object );

        if( line.contains("RoleDTO") && !line.contains("List") )
            if( classToGroupFromStartLine(line,stream,'(',')',object) )
                m_roleGroup.append( object );
    }
    while( !line.isNull() );

    // Create right group if it is possible.
    orderGroup(m_rightCore);
    orderGroup(m_rightGroup);
    orderGroup(m_roleGroup);
}

void RightsAnalyzer::getProfils(QTextStream* stream)
{
    m_profilGroup.clear();

    QString line;
    do
    {
        Group object;
        line = stream->readLine();

        if( line.contains("public") &&
            line.contains("static") &&
            line.contains("List<string>") )
            if( listToGroupFromStartLine(line,stream,object) )
                m_profilGroup.append( object );
    }
    while( !line.isNull() );
}

void RightsAnalyzer::mapAllVariables()
{
    updateInterList();

    for(int i=0;i<m_rightCore.size();i++)
        for(int j=0;j<m_rightCore[i].list.size();j++)
        {
            findProfilFromVar(m_rightCore[i].list[j]);
            if( m_rightCore[i].list[j].element.profils.size() == 0 )
                m_rightCore[i].list[j].element.error = true;
            else
                m_rightCore[i].list[j].element.error = false;

        }

    for(int i=0;i<m_rightGroup.size();i++)
        for(int j=0;j<m_rightGroup[i].list.size();j++)
        {
            findProfilFromVar(m_rightGroup[i].list[j]);
            if( m_rightGroup[i].list[j].element.profils.size() == 0 )
                m_rightGroup[i].list[j].element.error = true;
            else
                m_rightGroup[i].list[j].element.error = false;
        }

    for(int i=0;i<m_roleGroup.size();i++)
        for(int j=0;j<m_roleGroup[i].list.size();j++)
        {
            findProfilFromVar(m_roleGroup[i].list[j]);
            if( m_roleGroup[i].list[j].element.profils.size() == 0 )
                m_roleGroup[i].list[j].element.error = true;
            else
                m_roleGroup[i].list[j].element.error = false;
        }

    updateInterList();
}

void RightsAnalyzer::clearAll()
{
    m_rightCore.clear();
    m_rightGroup.clear();
    m_roleGroup.clear();
    m_profilGroup.clear();
}

QVector<RightsAnalyzer::Group> RightsAnalyzer::rightCore() const
{
    return m_rightCore;
}

QVector<RightsAnalyzer::Group> RightsAnalyzer::rightGroup() const
{
    return m_rightGroup;
}

QVector<RightsAnalyzer::Group> RightsAnalyzer::roleGroup() const
{
    return m_roleGroup;
}

QVector<RightsAnalyzer::Group> RightsAnalyzer::profilGroup() const
{
    return m_profilGroup;
}


// Private method ------------------------------------------------------------//
bool RightsAnalyzer::classToGroupFromStartLine(const QString& line,
                                               QTextStream* stream,
                                               const QChar& startSepList,
                                               const QChar& endSepList,
                                               Group& object)
{
    if( !line.contains("new") )
        return false;

    // Fill all class from src in the data string.
    QString data = getBloc(line,stream,object.element.var);

    if( object.element.var.isEmpty() )
        return false;

    // Get all values of the sub list between startSepList and endSepList.
    int start = data.indexOf("{");
    int listStart = data.indexOf(startSepList,start+1);

    if( listStart != -1 )
    {
        int listEnd = data.indexOf(endSepList,listStart+1);

        if( listEnd != -1 )
        {
            QString tmp = data.mid(listStart+1,listEnd-listStart-1);
            QStringList list = tmp.split(",",QString::SkipEmptyParts);
            for(int i=0;i<list.size();i++)
            {
                Group g;
                g.element.var = list[i].simplified();
                if( !g.element.var.isEmpty() )
                    object.list.append(g);
            }

            // Remove all the list to simplify the getting values/keys.
            data.remove(listStart,listEnd-listStart+1);
        }
        else
            return false;
    }

    // Get the values of the target keys.
    QStringList list = data.split(",",QString::SkipEmptyParts);
    for(int i=0;i<list.size();i++)
    {
        if( list[i].contains("droitId") ||
            list[i].contains("groupeDroitsId") ||
            list[i].contains("roleId") )
            object.element.id = getBetweenSep(list[i],'\"').simplified();
        else if( list[i].contains("title") )
            object.element.title = getBetweenSep(list[i],'\"').simplified();
        else if( list[i].contains("description") )
            object.element.description = getBetweenSep(list[i],'\"').simplified();
    }

    return true;
}

bool RightsAnalyzer::listToGroupFromStartLine(const QString& line,
                                              QTextStream* stream,
                                              Group& object)
{
    // Fill all class from src in the data string.
    QString data = getBloc(line,stream,object.element.id);

    if( object.element.id.isEmpty() )
        return false;

    QStringList list = data.split(",",QString::SkipEmptyParts);
    for(int i=0;i<list.size();i++)
    {
        Group g;
        g.element.id = getBetweenSep(list[i],'\"').simplified();

        if( !g.element.id.isEmpty() )
            object.list.append(g);
    }

    return true;
}

QString RightsAnalyzer::getBloc(const QString& line,QTextStream* stream,QString& var)
{
    QString data = line;

    if( !data.contains("};") )
    {
        QString subLine;
        do
        {
            subLine = stream->readLine();
            data += subLine;
        }
        while( !subLine.contains("};") && !subLine.isNull() );
    }

    // Get the variable of the object.
    int newPos = data.indexOf('=');
    if( newPos == -1 )
        return QString();

    for(int i=newPos-2;i>=0;i--)
        if( data[i] == ' ' )
        {
            var = data.mid(i,newPos-i-1).simplified();
            break;
        }

    return data;
}

QString RightsAnalyzer::getBetweenSep(const QString& data,const QChar& sep)
{
    int start = data.indexOf(sep);
    int end = data.indexOf(sep,start+1);
    return data.mid(start+1,end-start-1);
}

void RightsAnalyzer::orderGroup(QVector<Group>& groupList)
{
    QVector<Group> list = groupList;
    groupList.clear();

    for(int i=0;i<list.size();i++)
    {
        QStringList value = list[i].element.id.split("_");

        if( value.size() >= 1 )
        {
            for(int m=0;m<groupList.size();m++)
                for(int n=0;n<groupList[m].list.size();n++)
                {
                    QStringList tmp = groupList[m].list[n].element.id.split("_");

                    if( tmp.size()>=1 && tmp[0].contains(value[0]) )
                    {
                        groupList[m].list.append( list[i] );
                        goto GroupFound;
                    }
                }

            Group group;
            group.element.id = value[0];
            group.list.append( list[i] );
            groupList.append( group );
        }
        else
            groupList.append( list[i] );

GroupFound:
        ;
    }
}

void RightsAnalyzer::findProfilFromVar(Group& current)
{
    for(int i=0;i<m_profilGroup.size();i++)
        findProfilFromVarRecursive(m_profilGroup[i].element.id,
                                   m_profilGroup[i].list,
                                   current);
}

void RightsAnalyzer::findProfilFromVarRecursive(const QString& id,
                                                QVector<Group>& profilList,
                                                Group& current)
{
    for(int i=0;i<profilList.size();i++)
    {
        if( profilList[i].element.var == current.element.var )
            current.element.profils.append(id);

        findProfilFromVarRecursive(id,profilList[i].list,current);
    }
}

void RightsAnalyzer::updateInterList()
{
    // Rights group level.
    for(int i=0;i<m_rightGroup.size();i++)
        for(int j=0;j<m_rightGroup[i].list.size();j++)
            for(int k=0;k<m_rightGroup[i].list[j].list.size();k++)
            {
                QString var = m_rightGroup[i].list[j].list[k].element.var;

                for(int m=0;m<m_rightCore.size();m++)
                    for(int n=0;n<m_rightCore[m].list.size();n++)
                        if( var == m_rightCore[m].list[n].element.var )
                            m_rightGroup[i].list[j].list[k] = m_rightCore[m].list[n];
            }

    // Role group level.
    for(int i=0;i<m_roleGroup.size();i++)
        for(int j=0;j<m_roleGroup[i].list.size();j++)
            for(int k=0;k<m_roleGroup[i].list[j].list.size();k++)
            {
                QString var = m_roleGroup[i].list[j].list[k].element.var;

                for(int m=0;m<m_rightGroup.size();m++)
                    for(int n=0;n<m_rightGroup[m].list.size();n++)
                        if( var == m_rightGroup[m].list[n].element.var )
                            m_roleGroup[i].list[j].list[k] = m_rightGroup[m].list[n];
            }

    // Profil group level.
    for(int i=0;i<m_profilGroup.size();i++)
        for(int j=0;j<m_profilGroup[i].list.size();j++)
        {
            QString roleId = m_profilGroup[i].list[j].element.id;

            for(int m=0;m<m_roleGroup.size();m++)
                for(int n=0;n<m_roleGroup[m].list.size();n++)
                    if( roleId == m_roleGroup[m].list[n].element.id )
                        m_profilGroup[i].list[j] = m_roleGroup[m].list[n];
        }
}
