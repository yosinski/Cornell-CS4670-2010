LIBS := -lhighgui -lcvaux -Lcxflann_i386
ALL  := eigenDecomp createEigenvectors createTrainingData

all : $(ALL)

eigenDecomp : eigenDecomp.cpp
createEigenvectors : createEigenvectors.cpp
createTrainingData : createTrainingData.cpp

% : %.cpp
	g++ $< -o $@ $(LIBS)

clean :
	rm -f $(ALL)
