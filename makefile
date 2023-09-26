all:
	g++ -o src/assembler src/main.cpp src/assembler.cpp src/section.cpp src/symbol.cpp src/parser.cpp src/relocation.cpp
	g++ -o src/linker src/linker.cpp src/section.cpp src/symbol.cpp
	g++ -o src/emulator src/emulator.cpp

	cd src; assembler.exe -o main.o main.s; \
	assembler.exe -o math.o math.s; \
	assembler.exe -o handler.o handler.s; \
	assembler.exe -o isr_terminal.o isr_terminal.s; \
	assembler.exe -o isr_timer.o isr_timer.s; \
	assembler.exe -o isr_software.o isr_software.s; \
	linker.exe -hex \
  	-place=my_code@0x40000000 -place=math@0xF0000000 \
  	-o program.hex \
  	handler.o math.o main.o isr_terminal.o isr_timer.o isr_software.o; \
	emulator.exe program.hex;
	