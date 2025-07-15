AS = as
LD = ld
OBJCPY = objcopy
IMAGE_NAME_x86 =  i386_image.bin
IMAGE_NAME_ARM =  akita_image.bin

all: x86_loader arm_loader

clean:
	echo 'Removing build files...'
	-rm $(IMAGE_NAME_x86) $(IMAGE_NAME_ARM)
	-rm -f outputs/*

x86_loader:   src/x86_bootloader.s
	echo 'Assembling x86 bootloader code into binary image...'
	$(AS) -o  outputs/x86_bootloader.o   src/x86_bootloader.s;
	$(LD) -Ttext=0x7c00 -e _start -o  outputs/x86_bootloader.elf   outputs/x86_bootloader.o;
	$(OBJCPY) -O binary --only-section=.text   outputs/x86_bootloader.elf  $(IMAGE_NAME_x86)

arm_loader:   src/arm_bootloader.s
	echo 'Assembling ARM bootloader code into binary image...'
	arm-none-eabi-as -o   outputs/arm_bootloader.o   src/arm_bootloader.s;
	arm-none-eabi-ld -Ttext=0x0 -e _start -o   outputs/arm_bootloader.elf   outputs/arm_bootloader.o;
	arm-none-eabi-objcopy -O binary --only-section=.text   outputs/arm_bootloader.elf  $(IMAGE_NAME_ARM)

qemu_i386:
	echo 'Building binary and running x86 i386 in QEMU...'
	make x86_loader
	qemu-system-i386 -nographic -s -device loader,file=assets/lemon_log.bin,addr=0x500 -hda  $(IMAGE_NAME_x86)

qemu_akita:
	echo 'Building binary and running ARM Akita in QEMU...'
	make arm_loader
	qemu-system-arm -M akita -nographic -s -device loader,file=assets/lemon_log.bin,addr=0xA3F00000 -device loader,file=$(IMAGE_NAME_ARM),addr=0x0

qemu_x64:
	echo 'Building binary and running x86 64 in QEMU...'
	make x86_loader
	qemu-system-x86_64 -nographic -s -device loader,file=assets/lemon_log.bin,addr=0x500 -hda  $(IMAGE_NAME_x86)

qemu_raspi2:
	echo 'Building binary and running ARM Akita in QEMU...'
	make arm_loader
	qemu-system-arm -M raspi2b -nographic -s -device loader,file=assets/lemon_log.bin,addr=0xA3F00000 -device loader,file=$(IMAGE_NAME_ARM),addr=0x0
