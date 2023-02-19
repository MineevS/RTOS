#
 # @file	makefile
 # @data	00-JAN-22
 # @author	Mineev S. A.
 # @email	mineeff20@yandex.ru
 # @version	v1.0
 # @license	FS(FreeSoft)
 # @brief	-
 # @detail	-
 # 
#
 # COMPILATOR: 
CC = cc 
GCC = gcc 
CXX = g++ 
CLANG = clang
 # FLAGS: 
CFLAGS = -Wall -Werror -Wextra -o # '#define UNUSED(x) (void)(x)'
CXXFLAGS = -Wall -Werror -Wextra -o # '#define UNUSED(x) (void)(x)'
LIBFLAGS = -lpthread -lm 
GDBFLAGS = -g3 -gdwarf-2 #-gdwarf2
 # SRC + OBJ + TAG: 
SRC = main.c 
OBJ = - 
TAG = PROG
NAME = otp
DEBUGFLAGS = -DEBUG ${GDBFLAGS}
 
###############################################################
all: ${TAG}_RELEASE ${TAG}_DEBUG ${NAME}

release: ${TAG}_RELEASE

debug: ${TAG}_DEBUG

${NAME}: ${SRC}
	${CC} $^ ${LIBFLAGS} ${CFLAGS} $@

${TAG}_RELEASE: ${SRC} 
	${CC} $^ ${LIBFLAGS} ${CFLAGS} $@

${TAG}_DEBUG:  ${SRC}
	${CLANG} ${DEBUGFLAGS} $^ ${LIBFLAGS} ${CFLAGS} $@

 
clean: ${TAG}_RELEASE ${TAG}_DEBUG ${NAME}
	rm $^
###############################################################
.PHONY: all clean install uninstall
###############################################################

