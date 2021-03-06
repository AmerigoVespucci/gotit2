#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/c7f2a17a/GenDef.o \
	${OBJECTDIR}/_ext/ee867b77/ipc.o \
	${OBJECTDIR}/CaffeFn.o \
	${OBJECTDIR}/CheckersIntf.o \
	${OBJECTDIR}/CheckersText.o \
	${OBJECTDIR}/Closeness.o \
	${OBJECTDIR}/DepGrps.o \
	${OBJECTDIR}/ForLearn.o \
	${OBJECTDIR}/Gamer.o \
	${OBJECTDIR}/GenData.pb.o \
	${OBJECTDIR}/GenDataTbls.pb.o \
	${OBJECTDIR}/GenDef.pb.o \
	${OBJECTDIR}/GenSeed.pb.o \
	${OBJECTDIR}/GenTbl.pb.o \
	${OBJECTDIR}/GrpsCntrl.o \
	${OBJECTDIR}/GrpsCreate.o \
	${OBJECTDIR}/GrpsFile.o \
	${OBJECTDIR}/MascFiles.o \
	${OBJECTDIR}/MascReader.o \
	${OBJECTDIR}/NodeConfig.o \
	${OBJECTDIR}/OMA.o \
	${OBJECTDIR}/RF/discrete_dist.o \
	${OBJECTDIR}/RF/instance_set.o \
	${OBJECTDIR}/RF/random_forest.o \
	${OBJECTDIR}/RF/test.o \
	${OBJECTDIR}/RF/tree.o \
	${OBJECTDIR}/RF/tree_node.o \
	${OBJECTDIR}/RF/weights.o \
	${OBJECTDIR}/Regex2D.o \
	${OBJECTDIR}/RegexLearn.o \
	${OBJECTDIR}/StanfordReader.o \
	${OBJECTDIR}/TaskFns.o \
	${OBJECTDIR}/TaskLists.o \
	${OBJECTDIR}/TblGen.o \
	${OBJECTDIR}/WordAligned.o \
	${OBJECTDIR}/WordCorrect.o \
	${OBJECTDIR}/WordGroup.o \
	${OBJECTDIR}/WordSwap.o \
	${OBJECTDIR}/XmlParser.o \
	${OBJECTDIR}/ipc.pb.o \
	${OBJECTDIR}/main.o \
	${OBJECTDIR}/wordnet.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L/usr/lib/x86_64-linux-gnu/hdf5/serial -lboost_system -lboost_filesystem -lhdf5 -lhdf5_cpp -lpthread -lprotobuf -lglog -lgflags

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/gotit2

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/gotit2: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/gotit2 ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/_ext/c7f2a17a/GenDef.o: ../../dev/caffe/src/caffe/GenDef.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/c7f2a17a
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/c7f2a17a/GenDef.o ../../dev/caffe/src/caffe/GenDef.cpp

${OBJECTDIR}/_ext/ee867b77/ipc.o: ../../dev/caffe/src/caffe/util/ipc.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/ee867b77
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/ee867b77/ipc.o ../../dev/caffe/src/caffe/util/ipc.cpp

${OBJECTDIR}/CaffeFn.o: CaffeFn.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/CaffeFn.o CaffeFn.cpp

${OBJECTDIR}/CheckersIntf.o: CheckersIntf.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/CheckersIntf.o CheckersIntf.cpp

${OBJECTDIR}/CheckersText.o: CheckersText.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/CheckersText.o CheckersText.cpp

${OBJECTDIR}/Closeness.o: Closeness.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Closeness.o Closeness.cpp

${OBJECTDIR}/DepGrps.o: DepGrps.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/DepGrps.o DepGrps.cpp

${OBJECTDIR}/ForLearn.o: ForLearn.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/ForLearn.o ForLearn.cpp

${OBJECTDIR}/Gamer.o: Gamer.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Gamer.o Gamer.cpp

${OBJECTDIR}/GenData.pb.o: GenData.pb.cc 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/GenData.pb.o GenData.pb.cc

${OBJECTDIR}/GenDataTbls.pb.o: GenDataTbls.pb.cc 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/GenDataTbls.pb.o GenDataTbls.pb.cc

${OBJECTDIR}/GenDef.pb.o: GenDef.pb.cc 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/GenDef.pb.o GenDef.pb.cc

${OBJECTDIR}/GenSeed.pb.o: GenSeed.pb.cc 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/GenSeed.pb.o GenSeed.pb.cc

${OBJECTDIR}/GenTbl.pb.o: GenTbl.pb.cc 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/GenTbl.pb.o GenTbl.pb.cc

${OBJECTDIR}/GrpsCntrl.o: GrpsCntrl.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/GrpsCntrl.o GrpsCntrl.cpp

${OBJECTDIR}/GrpsCreate.o: GrpsCreate.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/GrpsCreate.o GrpsCreate.cpp

${OBJECTDIR}/GrpsFile.o: GrpsFile.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/GrpsFile.o GrpsFile.cpp

${OBJECTDIR}/MascFiles.o: MascFiles.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/MascFiles.o MascFiles.cpp

${OBJECTDIR}/MascReader.o: MascReader.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/MascReader.o MascReader.cpp

${OBJECTDIR}/NodeConfig.o: NodeConfig.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/NodeConfig.o NodeConfig.cpp

${OBJECTDIR}/OMA.o: OMA.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/OMA.o OMA.cpp

${OBJECTDIR}/RF/discrete_dist.o: RF/discrete_dist.cc 
	${MKDIR} -p ${OBJECTDIR}/RF
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/RF/discrete_dist.o RF/discrete_dist.cc

${OBJECTDIR}/RF/instance_set.o: RF/instance_set.cc 
	${MKDIR} -p ${OBJECTDIR}/RF
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/RF/instance_set.o RF/instance_set.cc

${OBJECTDIR}/RF/random_forest.o: RF/random_forest.cc 
	${MKDIR} -p ${OBJECTDIR}/RF
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/RF/random_forest.o RF/random_forest.cc

${OBJECTDIR}/RF/test.o: RF/test.cc 
	${MKDIR} -p ${OBJECTDIR}/RF
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/RF/test.o RF/test.cc

${OBJECTDIR}/RF/tree.o: RF/tree.cc 
	${MKDIR} -p ${OBJECTDIR}/RF
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/RF/tree.o RF/tree.cc

${OBJECTDIR}/RF/tree_node.o: RF/tree_node.cc 
	${MKDIR} -p ${OBJECTDIR}/RF
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/RF/tree_node.o RF/tree_node.cc

${OBJECTDIR}/RF/weights.o: RF/weights.cc 
	${MKDIR} -p ${OBJECTDIR}/RF
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/RF/weights.o RF/weights.cc

${OBJECTDIR}/Regex2D.o: Regex2D.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Regex2D.o Regex2D.cpp

${OBJECTDIR}/RegexLearn.o: RegexLearn.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/RegexLearn.o RegexLearn.cpp

${OBJECTDIR}/StanfordReader.o: StanfordReader.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/StanfordReader.o StanfordReader.cpp

${OBJECTDIR}/TaskFns.o: TaskFns.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/TaskFns.o TaskFns.cpp

${OBJECTDIR}/TaskLists.o: TaskLists.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/TaskLists.o TaskLists.cpp

${OBJECTDIR}/TblGen.o: TblGen.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/TblGen.o TblGen.cpp

${OBJECTDIR}/WordAligned.o: WordAligned.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/WordAligned.o WordAligned.cpp

${OBJECTDIR}/WordCorrect.o: WordCorrect.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/WordCorrect.o WordCorrect.cpp

${OBJECTDIR}/WordGroup.o: WordGroup.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/WordGroup.o WordGroup.cpp

${OBJECTDIR}/WordSwap.o: WordSwap.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/WordSwap.o WordSwap.cpp

${OBJECTDIR}/XmlParser.o: XmlParser.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/XmlParser.o XmlParser.cpp

${OBJECTDIR}/ipc.pb.o: ipc.pb.cc 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/ipc.pb.o ipc.pb.cc

${OBJECTDIR}/main.o: main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

${OBJECTDIR}/wordnet.o: wordnet.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -DGOTIT_LINUX -IRF -I../../dev/caffe/.build_debug/src -I../../dev/caffe/include -I/usr/include/hdf5/serial -std=c++11 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/wordnet.o wordnet.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/gotit2

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
