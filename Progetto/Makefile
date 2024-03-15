CC = gcc
CFLAGS = -Wall -Wextra -Werror
BINDIR := bin
OBJDIR := obj
SRCDIR := src
INCDIR := inc

all: $(BINDIR)/main $(BINDIR)/hmiOutput $(BINDIR)/hmiInput $(BINDIR)/brakeByWire $(BINDIR)/forwardFacingRadar $(BINDIR)/steerByWire $(BINDIR)/throttleControl $(BINDIR)/frontWindshieldCamera ${BINDIR}/parkAssist ${BINDIR}/surroundViewCameras

$(BINDIR)/main: $(OBJDIR)/main.o $(OBJDIR)/conn.o $(OBJDIR)/util.o
	$(CC) -o $@ $^

$(BINDIR)/hmiOutput: $(OBJDIR)/hmiOutput.o $(OBJDIR)/conn.o $(OBJDIR)/util.o
	$(CC) -o $@ $^

$(BINDIR)/hmiInput: $(OBJDIR)/hmiInput.o $(OBJDIR)/conn.o $(OBJDIR)/util.o
	$(CC) -o $@ $^

$(BINDIR)/brakeByWire: $(OBJDIR)/brakeByWire.o $(OBJDIR)/conn.o $(OBJDIR)/util.o
	$(CC) -o $@ $^

$(BINDIR)/forwardFacingRadar: $(OBJDIR)/forwardFacingRadar.o $(OBJDIR)/conn.o $(OBJDIR)/util.o
	$(CC) -o $@ $^

$(BINDIR)/steerByWire: $(OBJDIR)/steerByWire.o $(OBJDIR)/conn.o $(OBJDIR)/util.o
	$(CC) -o $@ $^

$(BINDIR)/throttleControl: $(OBJDIR)/throttleControl.o $(OBJDIR)/conn.o $(OBJDIR)/util.o
	$(CC) -o $@ $^

$(BINDIR)/frontWindshieldCamera: $(OBJDIR)/frontWindshieldCamera.o $(OBJDIR)/conn.o $(OBJDIR)/util.o
	$(CC) -o $@ $^

${BINDIR}/surroundViewCameras: $(OBJDIR)/surroundViewCameras.o $(OBJDIR)/conn.o $(OBJDIR)/util.o
	$(CC) -o $@ $^

${BINDIR}/parkAssist: $(OBJDIR)/parkAssist.o $(OBJDIR)/conn.o $(OBJDIR)/util.o
	$(CC) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) -c $< -I $(INCDIR) -o $@

clean:
	rm -f obj/*.o bin/* log/*

.PHONY: all clean