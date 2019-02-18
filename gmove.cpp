#include "gmove.h"

#include "gcodeline.h"

#include <QMap>
#include <QtMath>

static const double PI = 3.14159265358979323846264338327950288419717;
static const double PI_2 = 2 * 3.14159265358979323846264338327950288419717;

double distQuad(double x1, double y1, double z1, double x2, double y2, double z2);

GMove::GMove()
    : mX(0.0),
      mY(0.0),
      mZ(0.0),
      mI(0.0),
      mJ(0.0),
      mR(0.0),
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
      mArcDir(Undefined),
      mType(None)
{
}

GMove::GMove(const GCodeLine &line, const GMove &previous, const GMoveModifiers &mods)
    : mX(0.0),
      mY(0.0),
      mZ(0.0),
      mI(0.0),
      mJ(0.0),
      mR(0.0),
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
      mArcDir(Undefined),
      mType(None)
{
    QString code = line.code();
    if (!testCode(code)) {
        return;
    }
    
    bool ok = false;
    if (code == "G1" || code == "G0" || code == "G2" || code == "G3") {
        double p = line.parameter('X', &ok);
        mX = ok ? p : previous.X();
        
        p = line.parameter('Y', &ok);
        mY = ok ? p : previous.Y();
        
        p = line.parameter('Z', &ok);
        mZ = ok ? p : previous.Z();
        
        p = line.parameter('I', &ok);
        mI = ok ? p : 0.0;
        
        p = line.parameter('J', &ok);
        mJ = ok ? p : 0.0;
        
        p = line.parameter('E', &ok);
        mE = ok ? p : previous.E();
        
        p = line.parameter('F', &ok);
        mF = ok ? p : previous.F();
        
        if (code == "G1" || code == "G0") {
            mLen = qSqrt(distQuad(mX, mY, mZ, previous.X(), previous.Y(), previous.Z()));
            
        } else if (code == "G2" || code == "G3") {
            mArcDir = (code == "G2") ? CW : CCW;
            mCX = previous.X() + mI;
            mCY = previous.Y() + mJ;
            double r2 = distQuad(mX, mY, 0.0, mCX, mCY, 0.0); // radius squared
            double s2 = distQuad(mX, mY, 0.0, previous.X(), previous.Y(), 0.0); // segment length squared
            double theta = qAcos( 1 - s2 / (2 * r2));
            
            // Cross product Z. Sign depends on g-code
            double cpZ = -int(mArcDir) * ((previous.X() - mCX) * (mY - mCY) - (previous.Y() - mCY) * (mX - mCX));
            if (cpZ > 0) theta = PI_2 - theta;
            
            mR = qSqrt(r2);
            mLen = theta * mR;
        }
        
    } else if (code == "G28") {
        QList<char> pars = line.parameters();
        if (!pars.isEmpty()) {
            mX = pars.contains('X') ? 0.0 :previous.X();
            mY = pars.contains('Y') ? 0.0 :previous.Y();
            mZ = pars.contains('Z') ? 0.0 :previous.Z();
        }
    }

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
    if (code == "G1" || code == "G0" || code == "G2" || code == "G3" || code == "G28") {
        return true;
    }
    return false;
}

inline double distQuad(double x1, double y1, double z1, double x2, double y2, double z2)
{
    double dX = x1 - x2;
    double dY = y1 - y2;
    double dZ = z1 - z2;
    return dX * dX + dY * dY + dZ * dZ;
}
