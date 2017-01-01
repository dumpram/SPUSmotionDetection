# -------------------------------------------------
# Makefile for SPUS project
# Author: Ivan Pavic <ivan.pavic2@fer.hr>
# Date: 2016/4/1
# Version: 0.1.0
# -------------------------------------------------

INCDIR = inc
OPENCV_INC = /usr/local/include/opencv2
LIB = /usr/local/lib
OPENCV_LIBS = -lopencv_imgproc -lopencv_videoio -lopencv_video -lopencv_imgcodecs -lopencv_highgui -lopencv_core
CC = g++
CFLAGS = -std=c++11 -Wall -I $(INCDIR) -I $(OPENCV_INC)
SRCDIR = src
XSRCDIR = exe_src
OBJDIR = obj
XOBJDIR = exe_obj
BINDIR = bin
LFLAGS = -Wall -I $(INCDIR) -L $(LIB) -lopencv_imgproc -lopencv_videoio -lopencv_video -lopencv_imgcodecs -lopencv_highgui -lopencv_core -pthread
LINKER = g++ -o
EXE = BasicSigmaDelta

SOURCES  := $(wildcard $(SRCDIR)/*.cpp)
XSOURCES := $(wildcard $(XSRCDIR)/*.cpp)
INCLUDES := $(wildcard $(INCDIR)/*.h)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
XOBJECTS :=  $(XSOURCES:$(XSRCDIR)/%.cpp=$(XOBJDIR)/%.o)
rm       = rm -f


all : $(BINDIR)/BasicSigmaDelta $(BINDIR)/SerialHelperTest #$(BINDIR)/MoGTest $(BINDIR)/Streamer


$(BINDIR)/SerialHelperTest : $(OBJECTS) $(XOBJECTS)
	@$(LINKER) $@ $(OBJECTS) exe_obj/SerialHelperTest.o $(LFLAGS)
	@echo "Linking "$@" is complete!"

$(BINDIR)/MoGTest : $(OBJECTS) $(XOBJECTS)
	@$(LINKER) $@ $(OBJECTS) exe_obj/MoGTest.o $(LFLAGS)
	@echo "Linking MoGTest is complete!"


$(BINDIR)/Streamer : $(OBJECTS) $(XOBJECTS)
	@$(LINKER) $@ $(OBJECTS) exe_obj/Streamer.o $(LFLAGS)
	@echo "Linking Streamer is complete!"




$(BINDIR)/BasicSigmaDelta : $(OBJECTS) $(XOBJECTS)
	@$(LINKER) $@ $(OBJECTS) exe_obj/BasicSigmaDelta.o $(LFLAGS)
	@echo "Linking BasicSigmaDelta is complete!"

$(BINDIR)/ConditionalSigmaDelta : $(OBJECTS) $(XOBJECTS)
	@$(LINKER) $@ $(OBJECTS) exe_obj/BasicSigmaDelta.o $(LFLAGS)
	@echo "Linking ConditionalSigmaDelta is complete!"

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"

$(XOBJECTS): $(XOBJDIR)/%.o : $(XSRCDIR)/%.cpp
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"

.PHONEY: clean
clean:
	@$(rm) $(OBJECTS)
	@$(rm) $(XOBJECTS)
	@echo "Cleanup complete!"


.PHONEY: remove
remove: clean
	@$(rm) $(BINDIR)/*
	@echo "Executable removed!"
