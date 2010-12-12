CFLAGS = -Wall -g
LIBS := -lhighgui -lcvaux -Lcxflann_i386
#LIBS := -lhighgui -lcvaux -lflann               # [JBY] this segfaults?
ALL  := eigenDecomp createEigenvectors createTrainingData fisher

all : $(ALL)

eigenDecomp : eigenDecomp.cpp
createEigenvectors : createEigenvectors.cpp
createTrainingData : createTrainingData.cpp
fisher : fisher.cpp

% : %.cpp
	g++ $< -o $@ $(CFLAGS) $(LIBS)

clean :
	rm -f $(ALL)
