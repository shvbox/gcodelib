#include "gcode.h"

#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <algorithm>


GCode::GCode(QObject *parent) 
    : QObject(parent),
      mSpeedUnis(Units::mmPerS)
{
}

GCode::~GCode()
{
    clearData();
}

void GCode::clearData()
{
    qDeleteAll(mLines);
    mLines.clear();
    qDeleteAll(mMoves);
    mMoves.clear();
//    mZs.clear();
}

void GCode::buildMapping()
{
    mLMMap.resize(mLines.size());
    mLMMap.fill(-1);
    
    for (int i = 0; i < mMLMap.size(); ++i) {
        mLMMap[mMLMap.at(i)] = i;
    }    
}

void GCode::clearMapping()
{
    mLMMap.clear();
    mMLMap.clear();
}

QString GCode::getStringParameter(const GLine &line, char p) const
{
    for (int i = 0; i < line.fields().length(); ++i) {
        QString f = line.fields().at(i);
        if (f.startsWith(p)) {
            return f.mid(1);
        }
    }
    return QString();
}

double GCode::getDoubleParameter(const GLine &line, char p) const
{
    return getStringParameter(line, p).toDouble();
}

float GCode::getFloatParameter(const GLine &line, char p) const
{
    return getStringParameter(line, p).toFloat();
}

int GCode::getIntParameter(const GLine &line, char p) const
{
    return getStringParameter(line, p).toInt();
}

//void GCode::updateBounds(double X, double Y)
//{
//    if (X > mWidth) mWidth = X;
//    if (Y > mHeight) mHeight = Y;
//}

bool GCode::read(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    emit beginReset();
    clearMapping();
    clearData();
    
    mSelected.clear();
    mVisible.clear();
    
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        mLines.append(new GLine(line));
    }
    
    int size = mLines.size();
    mSelected.fill(false, size);
    mVisible.fill(false, size);
    mMLMap.reserve(size);
    
    GMove *mpp = new GMove();
    GMove *mp = mpp;
    GMoveModifiers mods;
    
    for (int i = 0; i < size; ++i) {
        GLine *l = mLines.at(i);
        
        //@TODO: rewrite
        if (l->code() == "G92") {
            mods.extruderShift = mp->ET() - getDoubleParameter(*l, 'E');
            
        } else if (l->code().startsWith('M')) {
            if (l->code() == "M220") {
                mods.speedFactor = getFloatParameter(*l, 'S') / 100.0;
                
            } else if (l->code() == "M221") {
                mods.extrudeFactor = getFloatParameter(*l, 'S') / 100.0;
                
            } else if (l->code() == "M104") {
                mods.extTemp = getFloatParameter(*l, 'S');
                
            } else if (l->code() == "M109") {
                mods.extTemp = getFloatParameter(*l, 'S');
                if (mods.extTemp == 0) {
                    mods.extTemp = getFloatParameter(*l, 'R');
                }
                
            } else if (l->code() == "M140") {
                mods.bedTemp = getFloatParameter(*l, 'S');
                
            } else if (l->code() == "M190") {
                mods.bedTemp = getFloatParameter(*l, 'S');
                if (mods.bedTemp == 0) {
                    mods.bedTemp = getFloatParameter(*l, 'R');
                }
                
            } else if (l->code() == "M106") {
                mods.fanSpeed = getIntParameter(*l, 'S');
                
            } else if (l->code() == "M107") {
                mods.fanSpeed = 0;
            }
        }
        
        if (GMove::testCode(l->code())) {
            mMLMap.append(i);
            GMove *m = new GMove(l->fields(), *mp, mods);
            mMoves.append(m);
            mp = m;
        }
    }
    delete mpp;
    
    buildMapping();
    
    file.close();
    
    emit endReset();
    return true;
}

int GCode::lineToMove(int line)
{
    if (line < 0 || line >= mLMMap.size()) {
        return -1;
    }
    return mLMMap.at(line);
}

int GCode::lineToMoveForward(int line)
{
    if (line < 0 || line >= mLMMap.size()) {
        return -1;
    }

    int m = mLMMap.at(line++);
    while (m < 0 && line < mLMMap.size()) {
        m = mLMMap.at(line++);
    }
    return m < 0 ? mMoves.size() - 1 : m;
}

int GCode::lineToMoveBackward(int line)
{
    if (line < 0 || line >= mLMMap.size()) {
        return -1;
    }

    int m = mLMMap.at(line--);
    while (m < 0 && line >= 0) {
        m = mLMMap.at(line--);
    }
    return (m < 0 && !mMoves.isEmpty()) ? 0 : m;
}

int GCode::moveToLine(int move)
{
    if (move < 0 || move >= mMLMap.size()) {
        return -1;
    }
    return mMLMap.at(move);
}

double GCode::X(int move) const
{
    Q_ASSERT(move >= 0 && move < mMoves.size());
    return mMoves.at(move)->X();
}

double GCode::Y(int move) const
{
    Q_ASSERT(move >= 0 && move < mMoves.size());
    return mMoves.at(move)->Y();
}

double GCode::Z(int move) const
{
    Q_ASSERT(move >= 0 && move < mMoves.size());
    return mMoves.at(move)->Z();
}

double GCode::E(int move) const
{
    Q_ASSERT(move >= 0 && move < mMoves.size());
    return mMoves.at(move)->E();
}

double GCode::EE(int move) const
{
    Q_ASSERT(move >= 0 && move < mMoves.size());
    return mMoves.at(move)->EE();
}

double GCode::ET(int move) const
{
    Q_ASSERT(move >= 0 && move < mMoves.size());
    return mMoves.at(move)->ETe();
}

double GCode::F(int move) const
{
    Q_ASSERT(move >= 0 && move < mMoves.size());
    float f = mMoves.at(move)->F();
    return mSpeedUnis == Units::mmPerMin ? f : f / 60;
}

double GCode::FE(int move) const
{
    Q_ASSERT(move >= 0 && move < mMoves.size());
    float f = mMoves.at(move)->FE();
    return mSpeedUnis == Units::mmPerMin ? f : f / 60;
}

double GCode::distance(int move) const
{
    Q_ASSERT(move >= 0 && move < mMoves.size());
    return mMoves.at(move)->distance();
}

double GCode::dEE(int move) const
{
    Q_ASSERT(move >= 0 && move < mMoves.size());
    return mMoves.at(move)->dEe();
}

double GCode::flow(int move) const
{
    Q_ASSERT(move >= 0 && move < mMoves.size());
    return mMoves.at(move)->flowE();
}

float GCode::bedT(int move) const
{
    Q_ASSERT(move >= 0 && move < mMoves.size());
    return mMoves.at(move)->bedT();
}

float GCode::extT(int move, int /*n*/) const
{
    Q_ASSERT(move >= 0 && move < mMoves.size());
    return mMoves.at(move)->extT();
}

float GCode::fanSpeed(int move) const
{
    Q_ASSERT(move >= 0 && move < mMoves.size());
    return mMoves.at(move)->fanSpeed();
}

GMove::MoveType GCode::moveType(int move) const
{
    Q_ASSERT(move >= 0 && move < mMoves.size());
    return mMoves.at(move)->type();
}

QPointF GCode::XY(int move) const
{
    Q_ASSERT(move >= 0 && move < mMoves.size());
    return QPointF(mMoves.at(move)->X(), mMoves.at(move)->Y());
}

//double GCode::zLayer(int layer) const
//{
//    Q_ASSERT(layer >= 0 && layer < mZs.size());
//    return mZs.at(layer);
//}

//QRectF GCode::bounds() const
//{
//    return QRectF(0.0f, 0.0f, mWidth, mHeight);
//}

void GCode::selectAll()
{
    select(0, mLines.size() - 1);
}

void GCode::select(int line)
{
    Q_ASSERT(line >= 0 && line < mLines.size());
    if (mSelected.testBit(line)) return;
    if (mVisible.testBit(line)) {
        mSelected.setBit(line);
        emit selectionChanged(line, line);
    }
}

void GCode::select(int firstLine, int lastLine)
{
//    qDebug() << __PRETTY_FUNCTION__ << firstLine << lastLine;
    Q_ASSERT(firstLine >= 0 && firstLine < mLines.size());
    Q_ASSERT(lastLine >= 0 && lastLine < mLines.size());
    int min = qMin(firstLine, lastLine);
    int max = qMax(firstLine, lastLine);

    bool changed = false;
    for (int i = min; i <= max; ++i) {
        if (mVisible.testBit(i)) {
            changed = changed || !mSelected.testBit(i);
            mSelected.setBit(i);
        }
    }
    
    if (changed) {
        emit selectionChanged(min, max);
    }
}

void GCode::deselectAll()
{
    deselect(0, mLines.size() - 1);
}

void GCode::deselect(int line)
{
    Q_ASSERT(line >= 0 && line < mLines.size());
    if (!mSelected.testBit(line)) return;

    mSelected.clearBit(line);
    emit selectionChanged(line, line);
}

void GCode::deselect(int firstLine, int lastLine)
{
//    qDebug() << __PRETTY_FUNCTION__ << firstLine << lastLine;
    Q_ASSERT(firstLine >= 0 && firstLine < mLines.size());
    Q_ASSERT(lastLine >= 0 && lastLine < mLines.size());
    int min = qMin(firstLine, lastLine);
    int max = qMax(firstLine, lastLine);

    bool changed = false;
    for (int i = min; i <= max; ++i) {
        changed = changed || mSelected.testBit(i);
        mSelected.clearBit(i);
    }
    
    if (changed) {
        emit selectionChanged(min, max);
    }
}

bool GCode::toggleSelection(int line) 
{
    Q_ASSERT(line >= 0 && line < mLines.size());
    if (mVisible.testBit(line)) {
        mSelected.toggleBit(line);
        emit selectionChanged(line, line);
    }
    return mSelected.testBit(line); 
}

void GCode::toggleSelection(int firstLine, int lastLine)
{
//    qDebug() << __PRETTY_FUNCTION__ << firstLine << lastLine;
    Q_ASSERT(firstLine >= 0 && firstLine < mLines.size());
    Q_ASSERT(lastLine >= 0 && lastLine < mLines.size());
    int min = qMin(firstLine, lastLine);
    int max = qMax(firstLine, lastLine);

    for (int i = min; i <= max; i++) {
        if (mVisible.testBit(i)) {
            mSelected.toggleBit(i);
        }
    }
    
    emit selectionChanged(min, max);
}

void GCode::showAll()
{
    show(0, mLines.size() - 1);
}

void GCode::show(int line)
{
    Q_ASSERT(line >= 0 && line < mLines.size());
    if (mVisible.testBit(line)) return;

    mVisible.setBit(line);
    emit visibilityChanged(line, line);
}

void GCode::show(int firstLine, int lastLine)
{
    Q_ASSERT(firstLine >= 0 && firstLine < mLines.size());
    Q_ASSERT(lastLine >= 0 && lastLine < mLines.size());
    int min = qMin(firstLine, lastLine);
    int max = qMax(firstLine, lastLine);

    bool changed = false;
    for (int i = min; i <= max; i++) {
        changed = changed || !mVisible.testBit(i);
        mVisible.setBit(i);
    }
    
    if (changed) {
        emit visibilityChanged(min, max);
    }
}

void GCode::hideAll()
{
//    qDebug() << __PRETTY_FUNCTION__;
    hide(0, mLines.size() - 1);
}

void GCode::hide(int line)
{
    Q_ASSERT(line >= 0 && line < mLines.size());
    if (!mVisible.testBit(line)) return;

    mVisible.clearBit(line);
    deselect(line);
    emit visibilityChanged(line, line);
}

void GCode::hide(int firstLine, int lastLine)
{
//    qDebug() << __PRETTY_FUNCTION__ << firstLine << lastLine;
    Q_ASSERT(firstLine >= 0 && firstLine < mLines.size());
    Q_ASSERT(lastLine >= 0 && lastLine < mLines.size());
    int min = qMin(firstLine, lastLine);
    int max = qMax(firstLine, lastLine);

    bool changed = false;
    for (int i = min; i <= max; ++i) {
        changed = changed || mVisible.testBit(i);
        mVisible.clearBit(i);
    }
    
    if (changed) {
        deselect(min, max);
        emit visibilityChanged(min, max);
    }
}

bool GCode::toggleVisible(int line)
{
    Q_ASSERT(line >= 0 && line < mLines.size());
    
    mVisible.toggleBit(line);
    if (!mVisible.testBit(line)) {
        deselect(line);
    }
    emit visibilityChanged(line, line);
    return mVisible.testBit(line);
}

void GCode::toggleVisible(int firstLine, int lastLine)
{
    Q_ASSERT(firstLine >= 0 && firstLine < mLines.size());
    Q_ASSERT(lastLine >= 0 && lastLine < mLines.size());
    int min = qMin(firstLine, lastLine);
    int max = qMax(firstLine, lastLine);

    bool hided = false;
    for (int i = min; i <= max; i++) {
        mVisible.toggleBit(i);
        hided = hided || !mVisible.testBit(i);
    }
    
    if (hided) {
        deselect(min, max);
    }
    
    emit visibilityChanged(min, max);
}

