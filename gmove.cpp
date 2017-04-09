#include "gmove.h"

#include "gline.h"

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
      mEe(0.0),
      mETe(0.0),
      mDEe(0.0),
      mFlowE(0.0),
      mType(None)
{
}

GMove::GMove(const GLine &line, const GMove &previous, const GMoveModifiers &mods)
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
      mEe(0.0),
      mETe(0.0),
      mDEe(0.0),
      mFlowE(0.0),
      mType(None)
{
    QString code = line.code();
    if (!testCode(code)) {
        return;
    }
    
    bool ok = false;
    if (code == "G1" || code == "G0") {
        double p = line.parameter('X', &ok);
        mX = ok ? p : previous.X();
        
        p = line.parameter('Y', &ok);
        mY = ok ? p : previous.Y();
        
        p = line.parameter('Z', &ok);
        mZ = ok ? p : previous.Z();
        
        p = line.parameter('E', &ok);
        mE = ok ? p : previous.E();
        
        p = line.parameter('F', &ok);
        mF = ok ? p : previous.F();
        
    } else if (code == "G28") {
        QList<char> pars = line.parameters();
        if (!pars.isEmpty()) {
            mX = pars.contains('X') ? 0.0 :previous.X();
            mY = pars.contains('Y') ? 0.0 :previous.Y();
            mZ = pars.contains('Z') ? 0.0 :previous.Z();
        }
    }
    
    double dX = mX - previous.X();
    double dY = mY - previous.Y();
    double dZ = mZ - previous.Z();
    mLen = qSqrt(dX * dX + dY * dY + dZ * dZ);
    if (qFuzzyCompare(mLen + 1, qreal(1.0))) {
        mLen = 0.0;
    }
    
    mET = mE + mods.extruderShift;
    
    if (qFuzzyCompare(mE, previous.E())) {
        mDE = 0.0;
        mDEe = 0.0;
        mEe = previous.Ee();
        
    } else {
        mDE = mET - previous.ET();
        if (qFuzzyCompare(mDE + 1, 1.0)) {
            mDE = 0.0;
        }
        
        mDEe = mDE * mMods.extrudeFactor;
        if (qFuzzyCompare(mDEe + 1, 1.0)) {
            mDEe = 0.0;
        }
        
        if (previous.ET() == mMods.extruderShift) {
            mEe = mDEe;
            
        } else {
            mEe = previous.Ee() + mDEe;
        }
    }

    mETe = previous.ETe() + mDEe;
    
    mFlowE = mLen == 0.0 ? 0.0 : (mDEe / mLen);
    
    if (mLen > 0.0) {
        mType = mDEe == 0.0 ? Travel : (mDEe > 0 ? Extrusion : Suck);
    } else {
        mType = mDEe == 0.0 ? None : (mDEe > 0 ? DestringPrime : DestringSuck);
    }
}

bool GMove::testCode(const QString &code)
{
    if (code == "G1" || code == "G0" || code == "G28") {
        return true;
    }
    return false;
}

