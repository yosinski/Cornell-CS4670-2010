#include "Progressable.h"
#include "Features.h"

void Progressable::emitProgressUpdate ( int i )
{
  emit progressUpdate ( i );
}
