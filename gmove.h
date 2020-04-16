#ifndef GMOVE_H
#define GMOVE_H

#include <QString>
#include <QStringList>

#include "gcodelib.h"

class GCodeLine;

struct GMoveModifiers {
    GMoveModifiers() 
        : extruderShift(0.0),
          speedFactor(1.0f), 
          extrudeFactor(1.0f),
          extrusionIsAbsolute(true),
          bedTemp(0.0f),
          extTemp(0.0f),
          fanSpeed(0) {}
    
    double extruderShift;
    float speedFactor;
    float extrudeFactor;
    bool extrusionIsAbsolute;
    float bedTemp;
    float extTemp;
    int fanSpeed;
};

class GMove
{
    friend class GCode;

public:
    enum MoveType {
        None,
        Extrusion,
        Suck,
        Travel,
        DestringSuck,
        DestringPrime
    };

    enum ArcDirection {
        Undefined = 0,
        CW = -1,
        CCW = 1
    };

    GMove();
    
    double X() const { return mX; }
    double Y() const { return mY; }
    double Z() const { return mZ; }
    double I() const { return mI; } // Arc center offset from current position in X axis
    double J() const { return mJ; } // Arc center offset from current position in Y axis
    double E() const { return mE; } // Extrusion
    double F() const { return mF; } // Feedrate
    int S() const { return mS; }    // S Parameter
    
    MoveType type() const { return mType; }
    
    double length() const { return mLen; }
    
    double CX() const { return mCX; } // Arc center X
    double CY() const { return mCY; } // Arc center Y
    double R() const { return mR; } // Arc radius
    ArcDirection arcDirection() const { return mArcDir; } // Arc direction
    
    double dE() const { return mDE; } // Delta E value
    double ET() const { return mET; } // Total extrusion value
    double distance() const { return mLen; }
    float bedT() const { return mMods.bedTemp; }
    float extT() const { return mMods.extTemp; }
    float fanSpeed() const { return mMods.fanSpeed / 2.55f; }
    
    float Ff() const { return mMods.speedFactor; }
    float Ef() const { return mMods.extrudeFactor; }

    double Fe() const { return mF * mMods.speedFactor; } // Feedrate effective value
    double Ee() const { return mEe; } // Extrusion effective value
    double ETe() const { return mETe; } // Total extrusion effective value
    double dEe() const { return mDEe; } // Delta E effective value
    double flowE() const { return mFlowE; } // Effective flow (dE / distance)

private:
    GMove(const GCodeLine &line, const GMove &previous = GMove(), const GMoveModifiers &mods = GMoveModifiers());
    
    static bool testCode(const QString &code);
    
private:
    double mX;
    double mY;
    double mZ;
    double mI; // Arc center offset from current position in X axis
    double mJ; // Arc center offset from current position in Y axis
    double mCX; // Arc center X
    double mCY; // Arc center Y
    double mR; // Arc radius
    double mE;
    double mF;
    int mS;
    
    GMoveModifiers mMods;
    
    double mDE;
    double mET;
    double mLen;
    
    double mEe;
    double mETe;
    double mDEe;
    
    double mFlowE;
    
    ArcDirection mArcDir;
    MoveType mType;
};

#endif // GMOVE_H
