#include "gline.h"

const QRegExp commentsSplitter(";");
const QRegExp fieldsSplitter("\\s");

GLine::GLine()
    : mLine(QString()),
      mLineType(Empty),
      mCommand(QString()),
      mComment(QString()),
      mSelected(false)
{
}

GLine::GLine(const QString &line)
    : mLine(line),
      mLineType(Empty),
      mCommand(QString()),
      mComment(QString()),
      mSelected(false)
{
    if (line.length() > 0) {
        int pos = line.indexOf(commentsSplitter);
        if (pos < 0) {
            mCommand = line.trimmed();
            mLineType = Command;
            
        } else if (pos == 0) {
            mComment = line.mid(1).trimmed();
            mLineType = Comment;
            
        } else {
            mCommand = line.left(pos).trimmed();
            mComment = line.mid(pos + 1).trimmed();
            mLineType = Command;
        }
        
        if (mCommand.startsWith("M117", Qt::CaseInsensitive)) {
            mCommand = "M117" + mCommand.mid(4);
            
        } else {
            mCommand = mCommand.toUpper();
        }
        
        if (mLineType == Command) {
            mFields = mCommand.split(fieldsSplitter, QString::SkipEmptyParts);
        }
        
        QString c = code();
        if (!(c.isEmpty() || c == "M117")) {
            for (int i = 1; i < mFields.size(); i++) {
                QString f = mFields[i];
                char p = f.at(0).toUpper().toLatin1();
                mKeys.append(p);
                mParameters.insert(p, f.mid(1));
            }
        }
    }
}

QString GLine::code() const
{
    if (mFields.isEmpty()) {
        return QString();
    }
    
    return mFields.at(0);
}

double GLine::parameter(const char &p, bool *ok) const
{
    QString par = parameterStr(p, ok);
    return par.isEmpty() ? 0.0 :par.toDouble(ok);
}

float GLine::parameterFloat(const char &p, bool *ok) const
{
    QString par = parameterStr(p, ok);
    return par.isEmpty() ? 0.0f :par.toFloat(ok);
}

int GLine::parameterInt(const char &p, bool *ok) const
{
    QString par = parameterStr(p, ok);
    return par.isEmpty() ? 0 :par.toInt(ok);
}

QString GLine::parameterStr(const char &p, bool *ok) const
{
    bool yes = mParameters.contains(p);
    if (ok) {
        *ok = yes;
    }
    
    if (yes) {
        return mParameters.value(p);
    }
    
    return QString();
}

void GLine::select()
{
    mSelected = true;
}

void GLine::deselect()
{
    mSelected = false;
}

bool GLine::toggleSelection()
{
    mSelected = !mSelected;
    return mSelected;
}

