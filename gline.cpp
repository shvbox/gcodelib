#include "gline.h"

const QRegExp commentsSplitter(";");
const QRegExp fieldsSplitter("\\s");

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
    }
}

QString GLine::code() const
{
    if (mFields.isEmpty()) {
        return QString();
    }
    
    return mFields.at(0);
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

