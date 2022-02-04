CC := gcc
CFLAGS := -O0 -ggdb3
LDFLAGS := -lm -lcriterion

src_dir := ../src
c_files := $(shell find $(src_dir)/ -type f -name '*.c' -not -path "$(src_dir)/$(gen_dir)/*") 

SRCS := $(filter-out ../src/main.c,${c_files}) unit_test.c
BIN := unit_test

all: ${BIN}
unit_test: ${SRCS}
	${CC} ${CFLAGS} ${LDFLAGS} -o ${BIN} ${SRCS}

clean:
	rm unit_test