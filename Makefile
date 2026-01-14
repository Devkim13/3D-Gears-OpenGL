# Makefile pour Glfw
#
# Installation des packages :
#   sudo apt install libglfw3-dev libgl1-mesa-dev
#
# Pour tout compiler, tapez : make all
# Pour tout compiler en parallèle, tapez : make -j all
# pour supprimer les .o et exécutables : make clean
# Pour tout recompiler : make clean all

SHELL   = /bin/bash
CPP     = g++
RM      = rm -f
CFLAGS  = -Wall -O2 --std=c++17  # -g pour gdb
LIBS    = -lglfw -lGLU -lGL -lm

# Fichiers à compiler :
# chaque fichier .cpp produira un exécutable du même nom
CFILES  := $(wildcard *.cpp)
EXECS   := $(CFILES:%.cpp=%)

# Règle pour fabriquer les .o à partir des .cpp
%.o : %.cpp
	$(CPP) $(CFLAGS) -c $*.cpp

# Déclaration des cibles factices
.PHONY : all clean

# Règle pour produire tous les exécutables.
all : $(EXECS)

# Règle de production de chaque exécutable
$(EXECS) : % : %.o
	$(CPP) -o $@ $^ $(LIBS)

# Règle de nettoyage - AUTOCLEAN
clean :
	$(RM) *.o *~ $(EXECS) tmp*.*


