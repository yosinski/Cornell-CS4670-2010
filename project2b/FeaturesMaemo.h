#ifndef FEATURESMAEMO_H
#define FEATURESMAEMO_H

#include <QWidget>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QString>
#include <map>
#include <opencv/cv.h>
#include "ImageView.h"
#include <FCam/N900.h>
#include <QStackedWidget>
#include "Panorama.h"

class Viewfinder;
class FeaturesWizard;

enum MatchType { SSD = 1, Ratio };
enum FeatureType { Dummy = 1, MOPS, SURF };

// The FeaturesMaemo class controls the Maemo.  Feel free to play around with
// the Maemo if you'd like.
class FeaturesMaemo : public QMainWindow {
Q_OBJECT

std::map<QString, MatchType> matchTypes;
std::map<QString, FeatureType> featureTypes;
Viewfinder * viewFinder;
QStackedWidget * stack;
FeatureType featureType;
MatchType matchType;
Panorama * panorama_widget;
bool panorama;

public:
// Create the Maemo.
FeaturesMaemo ( QWidget * = 0 );
void activateViewfinder ();
void deactivateViewfinder ();
FeatureType getFeatureType ();
MatchType getMatchType ();

signals:
void alert ( const QString & );

// Here are the callback functions.
public slots:
void updateFeatureType ( const QString & );
void updateMatchType ( const QString & );
void updateMode ( const QString & );
void setFeatureTypeAction ();
void setMatchTypeAction ();
void setModeAction ();
void pictureTaken ( const FCam::Frame & );
};

#endif
