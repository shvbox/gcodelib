#ifndef GCODELINE_H
#define GCODELINE_H

#include <QString>
#include <QStringList>
#include <QMap>

class GCodeLine 
{
    friend class GCode;
    
public:
    enum LineType {
        Empty = 0,
        Command,
        Comment
    };

    GCodeLine();
    
    QString text() const { return mLine; }
    QString command() const { return mCommand; }
    QString code() const;
    QStringList fields() const { return mFields; }
    QString comment() const { return mComment; }
    LineType type() const { return mLineType; }
    
    bool selected() const { return mSelected; }

    QList<char> parameters() const { return mKeys; }
    double parameter(const char &p, bool *ok = 0) const;
    float parameterFloat(const char &p, bool *ok = 0) const;
    int parameterInt(const char &p, bool *ok = 0) const;
    QString parameterStr(const char &p, bool *ok = 0) const;
    
private:
    explicit GCodeLine(const QString &text);
    void select();
    void deselect();
    bool toggleSelection();
    
private:
    
    QString mLine;
    LineType mLineType;
    
    QString mCommand;
    QString mComment;
    QStringList mFields;
    QList<char> mKeys;
    QMap<char, QString> mParameters;
    
    bool mSelected;
};

#endif // GCODELINE_H
