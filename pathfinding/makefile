MAKEFILE_NAME = ${firstword ${MAKEFILE_LIST}}
MKDIR_P = mkdir -p

INCLUDE = -I../../
OBJECTS = graph.o search.o test.o

DEPENDS = ${OBJECTS:.o=.d}

OUTDIR = bin
EXEC = testing.out

CXX = g++
CXXFLAGS = -g -Wall -DDEBUG -MMD -std=c++11 ${INCLUDE}

.PHONY : clean obj

${EXEC} : ${OBJECTS}
	${MKDIR_P} ${OUTDIR}
	${CXX} ${CXXFALGS} $^ -o ${OUTDIR}/${EXEC}

${OBJECTS} : ${MAKEFILE_NAME}

obj : ${OBJECTS} ${MAIN}
		${MKDIR_P} ${OUTDIR}
		${CXX} ${CXXFLAGS} $^ -o ${OUTDIR}/$@

-include ${DEPENDS}

clean :
		rm -f ${DEPENDS} ${OBJECTS} ${OUTDIR}/${EXEC}
