#include "ResultThread.h"
#include "Features.h"

ResultThread::ResultThread ( QWidget *                         parent,
                             FeatureType                       featureType,
                             const std::vector<FeatureMatch> & matches,
                             const std::vector<Feature> &      f1,
                             size_t                            matchIndex ) :
  QThread ( parent ), matches ( matches ), f1 ( f1 ),
  matchIndex ( matchIndex ), featureType ( featureType )
{
}

void ResultThread::run ()
{
  ImageDatabase db;
  std::stringstream ss;

  ss << featureType << ".db";
  db.load ( ss.str ().c_str (), false );
  const DatabaseItem & item = db[matchIndex];
  const std::vector<Feature> & f2 = item.features;
  h = ransacHomography ( f1, f2, matches, 100, 10, this );
}

CvMat * ResultThread::getHomography ()
{
  return h;
}
