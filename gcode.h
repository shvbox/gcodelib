#ifndef GCODE_H
#define GCODE_H

#include <QObject>
#include <QList>
#include <QVector>
#include <QBitArray>
#include <QPointF>

#include "gcodelib.h"
#include "gline.h"
#include "gmove.h"

class GCode : public QObject
{
    Q_OBJECT
public:
    explicit GCode(QObject *parent = 0);
    ~GCode();
    
    bool read(const QString &fileName);

    int linesCount() const { return mLines.size(); }
    int movesCount() const { return mMoves.size(); }
//    int zCount() const { return mZs.size(); }
    
    // G-Code Lines
    GLine line(int l) const { return *(mLines.at(l)); }
    QString text(int l) const { return mLines.at(l)->text(); }
    QString command(int l) const { return mLines.at(l)->command(); }
    QString comment(int l) const { return mLines.at(l)->comment(); }
    GLine::LineType lineType(int l) const { return mLines.at(l)->type(); }
    QString code(int l) const { return mLines.at(l)->code(); }

    // Moves
    GMove move(int m) const { return *(mMoves.at(m)); }
    int lineToMove(int l) const;
    int lineToMoveForward(int l) const;
    int lineToMoveBackward(int l) const;
    int moveToLine(int m);
    double X(int m) const;
    double Y(int m) const;
    double Z(int m) const;
    double E(int m) const;
    double F(int m) const;
    
    GMove::MoveType moveType(int move) const;
    
    double Fe(int m) const;
    double Ee(int m) const;
    double ET(int m) const;
    double distance(int m) const;
    double dEe(int m) const;
    double flow(int m) const;
    float bedT(int m) const;
    float extT(int m, int e = 0) const;
    float fanSpeed(int m) const;
    
    QPointF XY(int m) const;

    // Selection
    bool selected(int l) const { return mSelected.at(l); }
    QBitArray selection() const { return mSelected; }
    void selectAll();
    void select(int l);
    void select(int firstLine, int lastLine);
    void deselectAll();
    void deselect(int l);
    void deselect(int firstLine, int lastLine);
    bool toggleSelection(int l);
    void toggleSelection(int firstLine, int lastLine);
    
    // Visibility
    bool visible(int l) const { return mVisible.testBit(l); }
    QBitArray visibility() const { return mVisible; }
    void showAll();
    void show(int l);
    void show(int firstLine, int lastLine);
    void hideAll();
    void hide(int l);
    void hide(int firstLine, int lastLine);
    bool toggleVisible(int l);
    void toggleVisible(int firstLine, int lastLine);
    
signals:
    void beginReset();
    void endReset();
    void dataChanged(int top, int bottom);
    void selectionChanged(int top, int bottom);
    void visibilityChanged(int top, int bottom);
    
public slots:
    
private:
    void clearData();
    void buildMapping();
    void clearMapping();
    
    Units::SpeedUnits mSpeedUnis;
    
    QList<GLine*> mLines;
    QList<GMove*> mMoves;
    
    QBitArray mSelected;
    QBitArray mVisible;
    
    QVector<int> mMLMap;
    QVector<int> mLMMap;
};

#endif // GCODE_H
