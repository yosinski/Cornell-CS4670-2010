#include "MatchingThread.h"
#include "Features.h"


MatchingThread::MatchingThread ( QWidget *          parent,
                                 FeatureType        featureType,
                                 MatchType          matchType,
                                 const FeatureSet & features )
  : QThread ( parent ), matchType ( matchType ),
  features ( features ), featureType ( featureType )
{
}

void MatchingThread::run ()
{
  float score;
  ImageDatabase db;
  std::stringstream ss;

  ss << featureType << ".db";
  db.load ( ss.str ().c_str (), false );
  std::cerr << db.size () << " " << features.size () << std::endl;
  performQuery ( features, db, index, matches, score, matchType, this );
  std::cerr << db[index].name << " selected" << std::endl;
  name = db[index].name;
}

const std::vector<FeatureMatch> & MatchingThread::getMatches ()
{
  return matches;
}

size_t MatchingThread::getMatchIndex ()
{
  return index;
}

const std::string & MatchingThread::getName ()
{
  return name;
}
