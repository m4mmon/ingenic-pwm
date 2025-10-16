# Makefile for ingenic-pwm

# Uncomment and set CROSS_COMPILE if needed
# CROSS_COMPILE?= mipsel-linux-

# Compiler settings
CC := $(CROSS_COMPILE)gcc
CFLAGS := -fPIC -std=gnu99 -ldl -lm -pthread -Os -ffunction-sections -fdata-sections -fomit-frame-pointer -ggdb
LDFLAGS := -Wl,--gc-sections

# Source files
SRC = ingenic-pwm.c

# Target binary name
TARGET = ingenic-pwm

# Fetch the latest commit tag (or hash if no tags are present)
COMMIT_TAG = $(shell git describe --tags --always)

all: $(TARGET)

# Ensure version.h is rebuilt when the commit tag changes
version.h: version.tpl.h
	@echo "Generating version.h"
	@sed 's/COMMIT_TAG/"$(COMMIT_TAG)"/' $< > $@

$(TARGET): version.h $(SRC)
	@echo "Building target $(TARGET) with CC=$(CC)"
	@$(CC) $(CFLAGS) $(LDFLAGS) $(SRC) -o $(TARGET)  # Use LDFLAGS and CC for linking
	@echo "Stripping target $(TARGET)"
	@$(CROSS_COMPILE)strip $(TARGET)

clean:
	@echo "Cleaning up"
	@rm -f $(TARGET) version.h
