#ifndef GLINE_H
#define GLINE_H

#include <QString>
#include <QStringList>

class GLine 
{
    friend class GCode;
    
public:
    enum LineType {
        Empty = 0,
        Command,
        Comment
    };

private:
    explicit GLine(const QString &line);
    
    QString line() const { return mLine; }
    QString command() const { return mCommand; }
    QString code() const;
    QStringList fields() const { return mFields; }
    QString comment() const { return mComment; }
    LineType type() const { return mLineType; }
    
    bool selected() const { return mSelected; }
    void select();
    void deselect();
    bool toggleSelection();
    
private:
    QString mLine;
    LineType mLineType;
    
    QString mCommand;
    QString mComment;
    QStringList mFields;
    
    bool mSelected;
};

#endif // GLINE_H
