DIR_INC = ./inc
DIR_SRC = ./src
DIR_OBJ = ./obj
DIR_BIN = ./bin
CSRC = $(wildcard ${DIR_SRC}/*.c)
COBJ = $(patsubst %.c,${DIR_OBJ}/%.o,$(notdir ${CSRC}))
CFLAGS =-I${DIR_INC} 
CLDFLAGS =-lpthread
                   
CROSS_COMPILE = gcc
TARGET = elec_104
BIN_TARGET = ${DIR_BIN}/${TARGET}


CXX = $(CROSS_COMPILE)
${BIN_TARGET}:${COBJ}
	$(CXX) ${COBJ} $(CLDFLAGS) -g -o $@

${DIR_OBJ}/%.o:${DIR_SRC}/%.c
	$(CXX) $(CFLAGS) -g -c $^ -o $@

.PHONY:clean
clean:
	rm -rf  ${DIR_OBJ}/*
	rm -rf  ${DIR_BIN}/*

