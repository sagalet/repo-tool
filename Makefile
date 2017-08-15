
CC := gcc
LIBXML_H := /usr/include/libxml2
LIBXML_SO := /usr/lib/x86_64-linux-gnu
CFLAGS := -lxml2
OUT := out
OUT_FILE := $(OUT)/repo-tool
#SRC_FILE := example/reader1.c
SRC_FILE := main.c

all: $(OUT_FILE)

$(OUT_FILE): $(OUT)
	$(CC) -I$(LIBXML_H) -L$(LIBXML_SO) -o $(OUT_FILE) $(SRC_FILE) $(CFLAGS)

$(OUT):
	mkdir -p $@

.PHONY: all
