#ifndef PROGRESSABLE_H
#define PROGRESSABLE_H

class Progressable
{
public:
void emitProgressUpdate ( int );

protected:
virtual void progressUpdate ( int ) = 0;
};

#endif
