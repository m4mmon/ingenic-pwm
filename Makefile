CROSS_COMPILE?= mipsel-openipc-linux-musl-

CFLAGS = -static

# Source files
SRC = ingenic-pwm.c

# Output binary name
OUT = ingenic-pwm

# Fetch the latest commit tag (or hash if no tags are present)
COMMIT_TAG = $(shell git describe --tags --always)

all: $(OUT)

version.h: version.tpl.h
	@sed 's/COMMIT_TAG/"$(COMMIT_TAG)"/' $< > $@

$(OUT): version.h $(SRC)
	$(CROSS_COMPILE)gcc $(CFLAGS) $(SRC) -o $(OUT)

clean:
	rm -f $(OUT) version.h
