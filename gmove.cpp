#include "gmove.h"

#include <QMap>
#include <QtMath>

GMove::GMove()
    : mX(0.0),
      mY(0.0),
      mZ(0.0),
      mE(0.0),
      mF(0.0),
      mS(0),
      mDE(0.0),
      mET(0.0),
      mLen(0.0),
      mEE(0.0),
      mDEE(0.0),
      mFlow(0.0),
      mType(None)
{
}

GMove::GMove(const QStringList &fields, const GMove &previous, const GMoveModifiers &mods)
    : mX(0.0),
      mY(0.0),
      mZ(0.0),
      mE(0.0),
      mF(0.0),
      mS(0),
      mMods(mods),
      mDE(0.0),
      mET(0.0),
      mLen(0.0),
      mEE(0.0),
      mDEE(0.0),
      mFlow(0.0),
      mType(None)
      
{
    if (fields.isEmpty() || !testCode(fields.at(0))) {
        return;
    }
    
    QMap<char, QString> m;
    for (int i = 1; i < fields.size(); i++) {
        QString f = fields[i];
        char p = f.at(0).toUpper().toLatin1();
        m.insert(p, f.mid(1));
    }
    
    bool ok = false;
    QString code = fields.at(0);
    if (code == "G1" || code == "G0") {
        QString strVal = m.take('X');
        double doubleVal = strVal.toDouble(&ok);
        mX = ok ? doubleVal : previous.X();
        
        strVal = m.take('Y');
        doubleVal = strVal.toDouble(&ok);
        mY = ok ? doubleVal : previous.Y();
        
        strVal = m.take('Z');
        doubleVal = strVal.toDouble(&ok);
        mZ = ok ? doubleVal : previous.Z();
        
        strVal = m.take('E');
        doubleVal = strVal.toDouble(&ok);
        mE = ok ? doubleVal : previous.E();
        
        strVal = m.take('F');
        doubleVal = strVal.toDouble(&ok);
        mF = ok ? doubleVal : previous.F();
        
    } else if (code == "G28") {
        if (!m.isEmpty()) {
            mX = m.contains('X') ? 0.0 :previous.X();
            mY = m.contains('Y') ? 0.0 :previous.Y();
            mZ = m.contains('Z') ? 0.0 :previous.Z();
        }
    }
    
    double dX = mX - previous.X();
    double dY = mY - previous.Y();
    double dZ = mZ - previous.Z();
    mLen = qSqrt(dX * dX + dY * dY + dZ * dZ);
    if (qFuzzyCompare(mLen + 1, qreal(1.0))) {
        mLen = 0.0;
    }
    
//    mEShift = previous.ET() - mods.newExtPosition;
    
    mET = mE + mods.extruderShift;
    
    if (qFuzzyCompare(mE, previous.E())) {
        mDE = 0.0;
        mDEE = 0.0;
        mEE = previous.EE();
        
    } else {
        mDE = ET() - previous.ET();
        if (qFuzzyCompare(mDE + 1, qreal(1.0))) {
            mDE = 0.0;
        }
        
        mDEE = dE() * mMods.extrudeFactor;
        if (qFuzzyCompare(mDEE + 1, qreal(1.0))) {
            mDEE = 0.0;
        }
        
        if (previous.ET() == mMods.extruderShift) {
            mEE = dEe();
            
        } else {
            mEE = previous.EE() + dEe();
        }
    }

    mETE = previous.ETe() + dEe();
    
    mFlow = distance() == 0.0 ? 0.0 : (dEe() / distance());
    
    if (distance() > 0.0) {
        mType = dEe() != 0.0 ? Extrusion : Traverse;
    } else {
        mType = dEe() > 0.0 ? RetractPrime : RetractSuck;
    }
}

bool GMove::testCode(const QString &code)
{
    if (code == "G1" || code == "G0" || code == "G28") {
        return true;
    }
    return false;
}

