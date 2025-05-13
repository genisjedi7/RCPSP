# For the record:
# $@ The name of the target file (the one before the colon)
# $< The name of the first (or only) prerequisite file (the first one after the colon)
# $^ The names of all the prerequisite files (space separated)
# $* The stem (the bit which matches the % wildcard in the rule definition.


CC := g++
SHELL := /bin/bash
HOST := $(shell hostname)

SRCROOT := src
RELEASE_BUILDROOT := build/release
DEBUG_BUILDROOT := build/debug
RELEASE_BINROOT := bin/release
DEBUG_BINROOT := bin/debug

DEBUG := 0

DIRECTORIES := 	smtapi/src \
			smtapi/src/util \
			smtapi/src/MDD \
			smtapi/src/encoders \
			smtapi/src/optimizers \
			smtapi/src/controllers \
			smtapi/src/solvers \
		encodings \
			encodings/MRCPSP \
		parser \
		controllers \

SOURCES := $(addprefix smtapi/src/util/, \
	util.cpp \
	errors.cpp \
	bipgraph.cpp \
	disjointset.cpp \
	predgraph.cpp \
)

SOURCES += $(addprefix smtapi/src/optimizers/, \
	optimizer.cpp \
	singlecheck.cpp \
	uboptimizer.cpp \
	buoptimizer.cpp \
	dicooptimizer.cpp \
	nativeoptimizer.cpp \
)


SOURCES += $(addprefix smtapi/src/MDD/, \
	mdd.cpp \
	mddbuilder.cpp \
	amopbmddbuilder.cpp \
	amopbbddbuilder.cpp \
)


SOURCES += $(addprefix smtapi/src/encoders/, \
	encoder.cpp \
	apiencoder.cpp \
	fileencoder.cpp \
	dimacsfileencoder.cpp \
	smtlib2fileencoder.cpp \
)

SOURCES += $(addprefix smtapi/src/controllers/, \
 	solvingarguments.cpp \
 	basiccontroller.cpp \
 	arguments.cpp \
)

SOURCES += $(addprefix smtapi/src/, \
	smtapi.cpp \
	smtformula.cpp \
	encoding.cpp \
)


SOURCES += $(addprefix encodings/MRCPSP/,\
	mrcpsp.cpp \
	mrcpspencoding.cpp \
	smttimeencoding.cpp \
	smttaskencoding.cpp \
	omtsatpbencoding.cpp \
	omtsoftpbencoding.cpp \
	mrcpspsatencoding.cpp \
	doubleorder.cpp \
)
# timeencoding.cpp \
# 	order.cpp \


SOURCES += $(addprefix parser/, \
 	parser.cpp \
)

# ----------------------------------------------------
# GCC Compiler flags
# ----------------------------------------------------
CFLAGS := -w -std=c++11 -Wall -Wextra

ifeq ($(DEBUG),1)
CFLAGS+= -g -O0 -fbuiltin -fstack-protector-all
else
CFLAGS+= -O3
endif

ifeq ($(DEBUG),0)
DEFS+= -DNDEBUG
endif

ifneq ($(TMPFILESPATH),"")
DEFS+= "-DTMPFILESPATH=\"$(TMPFILESPATH)\""
endif


ifeq ($(DEBUG),1)
BUILDROOT := $(DEBUG_BUILDROOT)
BINROOT := $(DEBUG_BINROOT)
else
BUILDROOT := $(RELEASE_BUILDROOT)
BINROOT := $(RELEASE_BINROOT)
endif



# -----------------------------------------------------
# Links
# -----------------------------------------------------
LFLAGS += -L./$(BUILDROOT)



# -----------------------------------------------------
# Include directories
# -----------------------------------------------------
INCLUDES += -I./$(SRCROOT)
INCLUDES += $(addprefix -I./$(SRCROOT)/,$(DIRECTORIES))





OBJS := $(SOURCES:%.cpp=$(BUILDROOT)/%.o)
OBJS := $(OBJS:%.cc=$(BUILDROOT)/%.o)
SOURCES := $(addprefix src/, $(SOURCES))




.PHONY: all mrcpsp2smt

.SECONDARY: $(OBJS)


all: mrcpsp2smt

clean:
	@rm -rf build
	@rm -rf bin

mrcpsp2smt: $(BUILDROOT) $(BINROOT) $(addprefix $(BUILDROOT)/, $(DIRECTORIES)) $(BUILDROOT)/mrcpsp2smt.o $(BINROOT)/mrcpsp2smt

# Compile the binary by calling the compiler with cflags, lflags, and any libs (if defined) and the list of objects.
$(BINROOT)/%: $(OBJS) $(BUILDROOT)/%.o
	@printf "Linking $@ ... "
	@$(CC) $(OBJS) $(@:$(BINROOT)/%=$(BUILDROOT)/%.o) $(CFLAGS) $(INCLUDES) $(LFLAGS) -o $@
	@echo "DONE"


# Get a .o from a .cpp by calling compiler with cflags and includes (if defined)
$(BUILDROOT)/%.o: $(SRCROOT)/%.cpp
	@printf "Compiling $< ... "
	@$(CC) $(CFLAGS) $(INCLUDES) $(DEFS) -c $< -o $@
	@echo "DONE"

$(BUILDROOT)/%.o: $(SRCROOT)/%.cc
	@printf "Compiling $< ... "
	@$(CC) $(CFLAGS) $(INCLUDES) $(DEFS) -c $< -o $@
	@echo "DONE"

$(BUILDROOT):
	@mkdir -p $@


$(BINROOT):
	@mkdir -p $@


$(addprefix $(BUILDROOT)/, $(DIRECTORIES)): % :
	@mkdir -p $@

