CFLAGS = -Wall -g
LIBS := -lhighgui -lcvaux -I.. ../FaceDetector.cpp ../Fisher.cpp ../Face.cpp
#LIBS := -lhighgui -lcvaux -lflann               # [JBY] this segfaults?
ALL  := maintest

all : $(ALL)

maintest : maintest.cpp

% : %.cpp
	g++ $< -o $@ $(CFLAGS) $(LIBS)

clean :
	rm -f $(ALL)


