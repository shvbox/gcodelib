#ifndef GMOVE_H
#define GMOVE_H

#include <QString>
#include <QStringList>

struct GMoveModifiers {
    GMoveModifiers() 
        : extruderShift(0.0),
          speedFactor(1.0f), 
          extrudeFactor(1.0f),
          bedTemp(0.0f),
          extTemp(0.0f),
          fanSpeed(0) {}
    
    double extruderShift;
    float speedFactor;
    float extrudeFactor;
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
        Traverse,
        RetractSuck,
        RetractPrime
    };

private:
    GMove();
    GMove(const QStringList &fields, const GMove &previous = GMove(), const GMoveModifiers &mods = GMoveModifiers());
    
    static bool testCode(const QString &code);
    
    double X() const { return mX; }
    double Y() const { return mY; }
    double Z() const { return mZ; }
    double E() const { return mE; }
    double F() const { return mF; }
    int S() const { return mS; }
    
    MoveType type() const { return mType; }
    
    double FE() const { return mF * mMods.speedFactor; } // Feedrate effective value
    double EE() const { return mEE; } // Extrusion effective value
    double ETe() const { return mETE; } // Total extrusion effective value
    double dEe() const { return mDEE; } // Delta E effective value
    double flowE() const { return mFlow; } // Effective flow (dE / distance)
    
    double dE() const { return mDE; } // Delta E value
    double ET() const { return mET; } // Total extrusion value
    double distance() const { return mLen; }
    float bedT() const { return mMods.bedTemp; }
    float extT() const { return mMods.extTemp; }
    float fanSpeed() const { return mMods.fanSpeed / 2.55f; }
    
private:
    double mX;
    double mY;
    double mZ;
    double mE;
    double mF;
    int mS;
    
    GMoveModifiers mMods;
    
    double mDE;
    double mET;
    double mLen;
    
    double mEE;
    double mETE;
    double mDEE;
    
    double mFlow;
    
    MoveType mType;
};

#endif // GMOVE_H
