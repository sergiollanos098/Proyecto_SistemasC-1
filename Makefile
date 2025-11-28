CC = riscv64-unknown-elf-gcc
# Agregamos '-e main' para decirle al Linker que arranque en la función main de C
CFLAGS = -march=rv32im -mabi=ilp32 -nostdlib -e main 
TARGET = satelite
SRCS = scheduler3.c proceso1.s proceso2.s proceso3.s io_driver.s

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

run: $(TARGET)
	qemu-riscv32 $(TARGET)

clean:
	rm -f $(TARGET)
	