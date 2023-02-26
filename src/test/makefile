#//////////////////////////////////////////////////////////////////////////////
#/ Name:        ads7924/test/makefile
#/ Purpose:     Makefile for interrupt-test ADS7924-device driver adc.ko
#/ Author:      Ulrich Becker
#/ Modified by:
#/ Created:     2017.05.16
#/ Copyright:   www.INKATRON.de
#//////////////////////////////////////////////////////////////////////////////
EXE_NAME = adctest

BASEDIR = .
SOURCES =  ads7924test.c
SOURCES += parse_opts.c
SOURCES += ads7924parseCmdLine.c


TARGET_DEVICE_USER     ?= root
TARGET_DEVICE_IP       ?= $(shell cat target.ip)
TARGET_DEVICE_DIR      ?= /data

# We need some source files from a git-repository...
GIT_REPOSITORY_URL = https://raw.githubusercontent.com/UlrichBecker/command_line_option_parser/master/src/

VPATH= $(BASEDIR)
INCDIR = $(BASEDIR)

CFLAGS = -g -O0 

POKY_SDK_VERSION = 2.1
POKY_SDK_DIR     ?= /opt/mid-range-fb/$(POKY_SDK_VERSION)/sysroots/
CROSS_ROOT_DIR   ?= $(POKY_SDK_DIR)cortexa9hf-neon-poky-linux-gnueabi
CROSS_CPU        ?= arm9tdmi

CFLAGS += --sysroot=$(CROSS_ROOT_DIR) -mcpu=$(CROSS_CPU)
CFLAGS += -mfloat-abi=hard 
CC  =  $(POKY_SDK_DIR)x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-gcc

PREFIX ?= /usr/local/bin
CFLAGS += $(addprefix -I,$(INCDIR))

OBJDIR=.obj


OBJ = $(addprefix $(OBJDIR)/,$(addsuffix .o,$(notdir $(basename $(SOURCES)))))

.PHONY: all 
all: $(EXE_NAME)

parse_opts.h:
	wget $(GIT_REPOSITORY_URL)parse_opts.h

parse_opts.c: parse_opts.h
	wget $(GIT_REPOSITORY_URL)parse_opts.c

$(OBJDIR):
	mkdir $(OBJDIR)

$(OBJDIR)/%.o: %.c $(SOURCES) $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(EXE_NAME): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -f $(OBJDIR)/*.o $(EXE_NAME) core
	rmdir $(OBJDIR)

.PHONY: wipe
wipe: clean
	rm parse_opts.*

.PHONY: scp
scp: $(EXE_NAME)
	scp $(EXE_NAME) $(TARGET_DEVICE_USER)@$(TARGET_DEVICE_IP):$(TARGET_DEVICE_DIR)


#=================================== EOF ======================================
