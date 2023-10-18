CROSS_COMPILE?= mipsel-openipc-linux-musl-

CFLAGS = -static

# Source files
SRC = ingenic_pwm.c

# Output binary name
OUT = ingenic_pwm

all: $(OUT)

$(OUT): $(SRC)
	$(CROSS_COMPILE)gcc $(CFLAGS) $(SRC) -o $(OUT)

clean:
	rm -f $(OUT)
