B = build

.DEFAULT_GOAL := Uni1-VGA16.o

%.o :  Makefile %.psf
	@echo "Making $*.psf an elf object file"
	@mkdir -p build
	objcopy -O elf32-i386 -B i386 -I binary $*.psf $B/$*.o

clean ::
	rm -rf build

-include $B/*.d