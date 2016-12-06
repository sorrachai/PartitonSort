# Variables to control Makefile operation

OVSPATH = OVS/
IOPATH = IO/
MITPATH = PartitionSort/
TRACEPATH = ClassBenchTraceGenerator/
TREEPATH = Trees/
UTILPATH = Utilities/

VPATH = $(OVSPATH) $(MITPATH) $(TRACEPATH) $(IOPATH) $(UTILPATH) $(TREEPATH) $(SPPATH) 

CXX = g++
CXXFLAGS = -g -std=c++14  -fpermissive -O3 $(INCLUDE)

# Targets needed to bring the executable up to date

main: main.o Simulation.o InputReader.o OutputWriter.o trace_tools.o  SortableRulesetPartitioner.o misc.o OptimizedMITree.o PartitionSort.o red_black_tree.o stack.o cmap.o TupleSpaceSearch.o  HyperCuts.o HyperSplit.o  TreeUtils.o IntervalUtilities.o  MapExtensions.o Tcam.o
	$(CXX) $(CXXFLAGS) -o main *.o 

# -------------------------------------------------------------------

main.o: main.cpp ElementaryClasses.h SortableRulesetPartitioner.h InputReader.h Simulation.h BruteForce.h cmap.h TupleSpaceSearch.h trace_tools.h PartitionSort.h IntervalUtilities.h hash.h OptimizedMITree.h
	$(CXX) $(CXXFLAGS) -c main.cpp

Simulation.o: Simulation.cpp Simulation.h ElementaryClasses.h
	$(CXX) $(CXXFLAGS) -c Simulation.cpp
# ** IO **
InputReader.o: InputReader.cpp InputReader.h ElementaryClasses.h
	$(CXX) $(CXXFLAGS) -c $(IOPATH)InputReader.cpp

OutputWriter.o: OutputWriter.cpp OutputWriter.h ElementaryClasses.h
	$(CXX) $(CXXFLAGS) -c $(IOPATH)OutputWriter.cpp
# ** Trace **
trace_tools.o: trace_tools.cc trace_tools.h
	$(CXX) $(CXXFLAGS) -c $(TRACEPATH)trace_tools.cc

# ** PartitionSort **

SortableRulesetPartitioner.o: SortableRulesetPartitioner.cpp SortableRulesetPartitioner.h ElementaryClasses.h IntervalUtilities.h
	$(CXX) $(CXXFLAGS) -c $(MITPATH)SortableRulesetPartitioner.cpp

misc.o: misc.cpp misc.h
	$(CXX) $(CXXFLAGS) -c $(MITPATH)misc.cpp

OptimizedMITree.o: OptimizedMITree.cpp OptimizedMITree.h red_black_tree.h misc.h stack.h ElementaryClasses.h SortableRulesetPartitioner.h IntervalUtilities.h Simulation.h
	$(CXX) $(CXXFLAGS) -c $(MITPATH)OptimizedMITree.cpp

PartitionSort.o: PartitionSort.cpp PartitionSort.h OptimizedMITree.h red_black_tree.h misc.h stack.h ElementaryClasses.h SortableRulesetPartitioner.h IntervalUtilities.h Simulation.h
	$(CXX) $(CXXFLAGS) -c $(MITPATH)PartitionSort.cpp

red_black_tree.o: red_black_tree.cpp red_black_tree.h misc.h stack.h ElementaryClasses.h
	$(CXX) $(CXXFLAGS) -c $(MITPATH)red_black_tree.cpp

stack.o: stack.cpp stack.h misc.h
	$(CXX) $(CXXFLAGS) -c $(MITPATH)stack.cpp

# ** TupleSpace **

cmap.o: cmap.cpp cmap.h hash.h ElementaryClasses.h random.h
	$(CXX) $(CXXFLAGS) -c  $(OVSPATH)cmap.cpp

TupleSpaceSearch.o: TupleSpaceSearch.cpp TupleSpaceSearch.h Simulation.h ElementaryClasses.h cmap.h hash.h
	$(CXX) $(CXXFLAGS) -c $(OVSPATH)TupleSpaceSearch.cpp

# ** Trees **

HyperCuts.o: HyperCuts.cpp HyperCuts.h Simulation.h ElementaryClasses.h
	$(CXX) $(CXXFLAGS) -c $(TREEPATH)HyperCuts.cpp

HyperSplit.o: HyperSplit.cpp HyperSplit.h Simulation.h ElementaryClasses.h
	$(CXX) $(CXXFLAGS) -c $(TREEPATH)HyperSplit.cpp

TreeUtils.o: TreeUtils.cpp TreeUtils.h Simulation.h ElementaryClasses.h
	$(CXX) $(CXXFLAGS) -c $(TREEPATH)TreeUtils.cpp

# ** Utils **
IntervalUtilities.o: IntervalUtilities.cpp IntervalUtilities.h ElementaryClasses.h
	$(CXX) $(CXXFLAGS) -c $(UTILPATH)IntervalUtilities.cpp

MapExtensions.o : MapExtensions.cpp MapExtensions.h
	$(CXX) $(CXXFLAGS) -c $(UTILPATH)MapExtensions.cpp

Tcam.o : Tcam.cpp Tcam.h ElementaryClasses.h
	$(CXX) $(CXXFLAGS) -c $(UTILPATH)Tcam.cpp

.PHONY: clean
.PHONY: uninstall

clean:
	rm *o

uninstall: clean
	rm main
