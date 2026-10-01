CROSS = riscv64-unknown-elf-
CC = $(CROSS)gcc
LD = $(CROSS)ld
OBJCOPY = $(CROSS)objcopy

CFLAGS = -march=rv64imafdc -mabi=lp64d -Wall -Wextra -O2 -ffreestanding -nostdlib -mcmodel=medany
LDFLAGS = -T linker.ld -nostdlib

KERNEL = kernel.elf

OBJS = \
	kernel/entry.o \
	kernel/start.o \
	kernel/trap.o \
	kernel/trap_asm.o

all: $(KERNEL)

kernel/entry.o: kernel/entry.S
	$(CC) $(CFLAGS) -c $< -o $@

kernel/start.o: kernel/start.c
	$(CC) $(CFLAGS) -c $< -o $@

kernel/trap.o: kernel/trap.c
	$(CC) $(CFLAGS) -c $< -o $@

kernel/trap_asm.o: kernel/trap.S
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL): $(OBJS) linker.ld
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $@

clean:
	rm -f $(OBJS) $(KERNEL)

qemu: $(KERNEL)
	qemu-system-riscv64 \
		-machine virt \
		-bios none \
		-kernel $(KERNEL) \
		-nographic
