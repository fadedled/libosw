#Makefile for compiling the Katsu library
#Compiler used:
CC       =	gcc

#Directroies
LIBDIR   :=	lib
BUILD    :=	build
INCLUDES :=	include
SOURCES  :=	src src/platform/linux src/opengl
CFLAGS	 :=	-O2 -Wall -fPIC -std=c89
GLSLDIR  := 	src/opengl/shaders
#BIN2INC	 := 	tools/kt-bin2inc

#All files to compile
DIRS     := $(LIBDIR) $(BUILD)
CFILES	 := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.c))
GLSLFILES := $(wildcard $(GLSLDIR)/*.glsl)
INCLUDE	 := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir))

GLSLINCS := $(GLSLFILES:.glsl=.inc)
OBJS     := $(CFILES:.c=.o)

ifeq ($(OSW_DEBUG), 1)
	CFLAGS += -DOSW_DEBUG -g
endif

.PHONY: clean static dynamic $(DIRS)

#Targets
static: $(BIN2INC) $(DIRS) $(GLSLINCS) $(OBJS)
	@echo "Generating static library libosw.a"
	@ar rcs $(LIBDIR)/libosw.a $(foreach obj, $(notdir $(OBJS)), $(BUILD)/$(obj))

dynamic: static
	@echo "Generating dynamic library libosw.so"
	@$(CC) -shared -o  $(LIBDIR)/libosw.so $(LIBDIR)/libosw.a $(CFLAGS) $(INCLUDE)

$(DIRS):
	@mkdir -p $@

#%.inc: %.glsl
#	@echo $(notdir $<)
#	@$(BIN2INC) -nc $<

%.o: %.c
	@echo $(notdir $<)
	@$(CC) -c -o $(BUILD)/$(notdir $(<:.c=.o)) $< $(CFLAGS) $(INCLUDE)

clean:
	@echo "clean ..."
	@rm -f -d $(BUILD)/*.o $(LIBDIR)/*.a $(LIBDIR)/*.so $(DIRS)
	@$(MAKE) -C tools clean
