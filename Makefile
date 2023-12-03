# Makefile for ingenic-pwm

#CROSS_COMPILE?= mipsel-openipc-linux-musl-

# Compiler settings
CC := $(CROSS_COMPILE)gcc
CFLAGS := -fPIC -std=gnu99 -shared -ldl -lm -pthread -Os -ffunction-sections -fdata-sections -fomit-frame-pointer
LDFLAGS := -Wl,--gc-sections

# Source files
SRC = ingenic-pwm.c

# TARGETput binary name
TARGET = ingenic-pwm

# Fetch the latest commit tag (or hash if no tags are present)
COMMIT_TAG = $(shell git describe --tags --always)

all: $(TARGET)

version.h: version.tpl.h
	@sed 's/COMMIT_TAG/"$(COMMIT_TAG)"/' $< > $@

$(TARGET): version.h $(SRC)
	@echo "Building target $(TARGET) with CC=$(CC)"
	$(CROSS_COMPILE)gcc $(CFLAGS) $(SRC) -o $(TARGET)
	@echo "Stripping target $(TARGET)"
	$(CROSS_COMPILE)strip $(TARGET)

clean:
	rm -f $(TARGET) version.h
