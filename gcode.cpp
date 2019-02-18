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
        mLines.append(new GCodeLine(line));
    }
    
    int size = mLines.size();
    mSelected.fill(false, size);
    mVisible.fill(false, size);
    mMLMap.reserve(size);
    
    GMove *mpp = new GMove();
    GMove *mp = mpp;
    GMoveModifiers mods;
    
    for (int i = 0; i < size; ++i) {
        GCodeLine *l = mLines.at(i);
        
        //@TODO: rewrite
        if (l->code() == "G92") {
            mods.extruderShift = mp->ET() - l->parameter('E');
            
        } else if (l->code().startsWith('M')) {
            if (l->code() == "M220") {
                mods.speedFactor = l->parameterFloat('S') / 100.0;
                
            } else if (l->code() == "M221") {
                mods.extrudeFactor = l->parameterFloat('S') / 100.0;
                
            } else if (l->code() == "M104") {
                mods.extTemp = l->parameterFloat('S');
                
            } else if (l->code() == "M109") {
                mods.extTemp = l->parameterFloat('S');
                if (mods.extTemp == 0) {
                    mods.extTemp = l->parameterFloat('R');
                }
                
            } else if (l->code() == "M140") {
                mods.bedTemp = l->parameterFloat('S');
                
            } else if (l->code() == "M190") {
                mods.bedTemp = l->parameterFloat('S');
                if (mods.bedTemp == 0) {
                    mods.bedTemp = l->parameterFloat('R');
                }
                
            } else if (l->code() == "M106") {
                mods.fanSpeed = l->parameterInt('S');
                
            } else if (l->code() == "M107") {
                mods.fanSpeed = 0;
            }
        }
        
        if (GMove::testCode(l->code())) {
            mMLMap.append(i);
            GMove *m = new GMove(*l, *mp, mods);
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

int GCode::lineToMove(int l) const
{
    if (l < 0 || l >= mLMMap.size()) {
        return -1;
    }
    return mLMMap.at(l);
}

int GCode::lineToMoveForward(int l) const
{
    if (l < 0 || l >= mLMMap.size()) {
        return -1;
    }

    int m = mLMMap.at(l++);
    while (m < 0 && l < mLMMap.size()) {
        m = mLMMap.at(l++);
    }
    return m < 0 ? mMoves.size() - 1 : m;
}

int GCode::lineToMoveBackward(int l) const
{
    if (l < 0 || l >= mLMMap.size()) {
        return -1;
    }

    int m = mLMMap.at(l--);
    while (m < 0 && l >= 0) {
        m = mLMMap.at(l--);
    }
    return (m < 0 && !mMoves.isEmpty()) ? 0 : m;
}

int GCode::moveToLine(int m)
{
    if (m < 0 || m >= mMLMap.size()) {
        return -1;
    }
    return mMLMap.at(m);
}

double GCode::X(int m) const
{
    Q_ASSERT(m >= 0 && m < mMoves.size());
    return mMoves.at(m)->X();
}

double GCode::Y(int m) const
{
    Q_ASSERT(m >= 0 && m < mMoves.size());
    return mMoves.at(m)->Y();
}

double GCode::Z(int m) const
{
    Q_ASSERT(m >= 0 && m < mMoves.size());
    return mMoves.at(m)->Z();
}

double GCode::I(int m) const
{
    Q_ASSERT(m >= 0 && m < mMoves.size());
    return mMoves.at(m)->I();
}

double GCode::J(int m) const
{
    Q_ASSERT(m >= 0 && m < mMoves.size());
    return mMoves.at(m)->J();
}

double GCode::R(int m) const
{
    Q_ASSERT(m >= 0 && m < mMoves.size());
    return mMoves.at(m)->R();
}

double GCode::length(int m) const
{
    Q_ASSERT(m >= 0 && m < mMoves.size());
    return mMoves.at(m)->length();
}

double GCode::E(int m) const
{
    Q_ASSERT(m >= 0 && m < mMoves.size());
    return mMoves.at(m)->E();
}

double GCode::Ee(int m) const
{
    Q_ASSERT(m >= 0 && m < mMoves.size());
    return mMoves.at(m)->Ee();
}

double GCode::ET(int m) const
{
    Q_ASSERT(m >= 0 && m < mMoves.size());
    return mMoves.at(m)->ETe();
}

double GCode::F(int m) const
{
    Q_ASSERT(m >= 0 && m < mMoves.size());
    float f = mMoves.at(m)->F();
    return mSpeedUnis == Units::mmPerMin ? f : f / 60;
}

double GCode::Fe(int m) const
{
    Q_ASSERT(m >= 0 && m < mMoves.size());
    float f = mMoves.at(m)->Fe();
    return mSpeedUnis == Units::mmPerMin ? f : f / 60;
}

double GCode::distance(int m) const
{
    Q_ASSERT(m >= 0 && m < mMoves.size());
    return mMoves.at(m)->distance();
}

double GCode::dEe(int m) const
{
    Q_ASSERT(m >= 0 && m < mMoves.size());
    return mMoves.at(m)->dEe();
}

double GCode::flow(int m) const
{
    Q_ASSERT(m >= 0 && m < mMoves.size());
    return mMoves.at(m)->flowE();
}

float GCode::bedT(int m) const
{
    Q_ASSERT(m >= 0 && m < mMoves.size());
    return mMoves.at(m)->bedT();
}

float GCode::extT(int m, int /*e*/) const
{
    Q_ASSERT(m >= 0 && m < mMoves.size());
    return mMoves.at(m)->extT();
}

float GCode::fanSpeed(int m) const
{
    Q_ASSERT(m >= 0 && m < mMoves.size());
    return mMoves.at(m)->fanSpeed();
}

GMove::ArcDirection GCode::arcDirection(int move) const
{
    Q_ASSERT(move >= 0 && move < mMoves.size());
    return mMoves.at(move)->arcDirection();
}

GMove::MoveType GCode::moveType(int move) const
{
    Q_ASSERT(move >= 0 && move < mMoves.size());
    return mMoves.at(move)->type();
}

QPointF GCode::XY(int m) const
{
    Q_ASSERT(m >= 0 && m < mMoves.size());
    return QPointF(mMoves.at(m)->X(), mMoves.at(m)->Y());
}

QPointF GCode::IJ(int m) const
{
    Q_ASSERT(m >= 0 && m < mMoves.size());
    return QPointF(mMoves.at(m)->I(), mMoves.at(m)->J());
}

QPointF GCode::CXY(int m) const
{
    Q_ASSERT(m >= 0 && m < mMoves.size());
    return QPointF(mMoves.at(m)->CX(), mMoves.at(m)->CY());
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

void GCode::select(int l)
{
    Q_ASSERT(l >= 0 && l < mLines.size());
    if (mSelected.testBit(l)) return;
    if (mVisible.testBit(l)) {
        mSelected.setBit(l);
        emit selectionChanged(l, l);
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

void GCode::deselect(int l)
{
    Q_ASSERT(l >= 0 && l < mLines.size());
    if (!mSelected.testBit(l)) return;

    mSelected.clearBit(l);
    emit selectionChanged(l, l);
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

bool GCode::toggleSelection(int l) 
{
    Q_ASSERT(l >= 0 && l < mLines.size());
    if (mVisible.testBit(l)) {
        mSelected.toggleBit(l);
        emit selectionChanged(l, l);
    }
    return mSelected.testBit(l); 
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

void GCode::show(int l)
{
    Q_ASSERT(l >= 0 && l < mLines.size());
    if (mVisible.testBit(l)) return;

    mVisible.setBit(l);
    emit visibilityChanged(l, l);
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

void GCode::hide(int l)
{
    Q_ASSERT(l >= 0 && l < mLines.size());
    if (!mVisible.testBit(l)) return;

    mVisible.clearBit(l);
    deselect(l);
    emit visibilityChanged(l, l);
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

bool GCode::toggleVisible(int l)
{
    Q_ASSERT(l >= 0 && l < mLines.size());
    
    mVisible.toggleBit(l);
    if (!mVisible.testBit(l)) {
        deselect(l);
    }
    emit visibilityChanged(l, l);
    return mVisible.testBit(l);
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

