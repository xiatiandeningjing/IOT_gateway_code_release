LIBRARY = 
program := xnn_gateway


SRCDIRS = $(shell pwd)  mqtt
#SOURCES = $(foreach d,$(SRCDIRS),$(wildcard $(addprefix $(d)/*,$(SRCEXTS))))
SRCEXTS=.c
sources := $(foreach d,$(SRCDIRS),$(wildcard $(addprefix $(d)/*,$(SRCEXTS))))
headers := $(wildcard *.h)

objects := $(sources:.c=.o)
deps    := $(sources:.c=.d)
export CROSS_COMPILE1=

export CC=${CROSS_COMPILE1}gcc
export LD=${CROSS_COMPILE1}ld
export AR=${CROSS_COMPILE1}ar

SHARED  := -shared
FPIC    := -fPIC
CFLAGS  := -Wall -Wextra  -I. -Imqtt  -DMQTTCLIENT_PLATFORM_HEADER=MQTTLinux.h 
LDFLAGS :=  -lm -lrt  

RM      := rm -f
TAR     := tar


all: $(program)

$(program):  $(objects)
		echo $(objects)
		$(CC) -o $@ $(CFLAGS) $(objects) $(LDFLAGS) $(LDLIBS) -lpthread  -L. # -liot

dist:
		pkg=`pwd`; tar cvf - $(me) $(sources) $(headers) \
                | $(GZIP) -c > `basename $${pkg}`.tar.gz
clean:
		$(RM) $(objects)
		$(RM) $(deps)
		$(RM) $(program)

$(LIBRARY):$(objects)
	echo "##############lib#############"
	echo $(PATH)
	$(AR) rcs libAT.a  $(objects) 
	
.SUFFIXES: .d
.c.d:
		$(CC) $(CFLAGS) -MM -MF $@ $<

%.o: %.c
	@echo "$(CC) $@"
	@$(CC) $(CFLAGS) -o $@ -c $<
	
-include $(deps)