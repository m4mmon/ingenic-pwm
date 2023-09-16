CROSS_COMPILE = /home/turismo/toolchain/mips-gcc472-glibc216-64bit-master/bin/mips-linux-uclibc-gnu-
CFLAGS = -static

# Source file
SRC = ingenic_pwm.c pwm.c

# Output binary name
OUT = ingenic-pwm

all: $(OUT)

$(OUT): $(SRC)
	$(CROSS_COMPILE)gcc $(CFLAGS) $(SRC) -o $(OUT)

clean:
	rm -f $(OUT)
