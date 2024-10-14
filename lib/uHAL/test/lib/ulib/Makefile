NAME := ulib
VERSION_MAJOR := 0
VERSION_MINOR := 2
VERSION_PATCH := 0
VERSION := $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)

TMP_BASE := out
STATIC_TMP := $(TMP_BASE)/static
SHARED_TMP := $(TMP_BASE)/shared

CC ?= cc
AR ?= ar
SIZE ?= size

SPLINT_FLAGS := \
	+limit 5 +posixlib -nullret \
	-DDEBUG=1 -UNDEBUG

# -fshort-enums
_CFLAGS        := -std=c99 -fstrict-aliasing -fno-common \
                  -ffunction-sections -fdata-sections \
                  -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=500 \
                  -I. -iquote include
_DEBUG_CFLAGS  := -Werror -Wall -Wextra -pedantic \
                  -Wstrict-prototypes -Wconversion -Winit-self -Wunreachable-code \
                  -Wdouble-promotion -Wformat-security \
                  -Wnull-dereference -Wstrict-aliasing -Wshadow -Wundef \
                  -fstack-usage -ggdb \
                  -DDEBUG=1 -UNDEBUG
# These are separate from the rest to make it easier to remove because clang
# doesn't support them.
_DEBUG_CFLAGS += -Wformat-overflow -Wformat-truncation -Wformat-signedness -Wstrict-aliasing=3
# These are separate from the rest because they probably won't be used very
# often.
# https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html
#_DEBUG_CFLAGS += -fsanitize=undefined -fsanitize=address
_RELEASE_CFLAGS := -UDEBUG -DNDEBUG=1 -O2
_SHARED_CFLAGS := -fPIC
_STATIC_CFLAGS :=
_LDFLAGS       := -Wl,-soname,lib$(NAME).so.$(VERSION_MAJOR)

C_FILES := $(wildcard src/*.c)
O_FILES := $(patsubst src/%.c, %.o, $(C_FILES))
STATIC_O_FILES := $(addprefix $(STATIC_TMP)/, $(O_FILES))
STATIC_SU_FILES := $(patsubst %.o, %.su, $(STATIC_O_FILES))
SHARED_O_FILES := $(addprefix $(SHARED_TMP)/, $(O_FILES))
SHARED_SU_FILES := $(patsubst %.o, %.su, $(SHARED_O_FILES))
STATIC_LIB := $(STATIC_TMP)/lib$(NAME).a
SHARED_LIB := $(SHARED_TMP)/lib$(NAME).so

CLEAN_STATIC_FILES := $(STATIC_O_FILES) $(STATIC_SU_FILES) $(STATIC_LIB)
CLEAN_SHARED_FILES := $(SHARED_O_FILES) $(SHARED_SU_FILES) $(SHARED_LIB)

#
# Misc Rules
#
all: clean static shared
debug: clean static-debug shared-debug
release: clean static-release shared-release

$(STATIC_TMP):
	mkdir -p $(STATIC_TMP)
$(SHARED_TMP):
	mkdir -p $(SHARED_TMP)

clean-static:
	rm -f $(CLEAN_STATIC_FILES)
clean-shared:
	rm -f $(CLEAN_SHARED_FILES)
clean: clean-static clean-shared

#
# Static library
#
static: _CFLAGS += $(_STATIC_CFLAGS)
static: $(STATIC_TMP) $(STATIC_O_FILES)
	$(AR) rcs $(STATIC_LIB) $(STATIC_O_FILES)
	$(SIZE) --format=gnu --totals $(STATIC_LIB)

static-debug: _CFLAGS += $(_DEBUG_CFLAGS)
static-debug: clean-static static

static-release: _CFLAGS += $(_RELEASE_CFLAGS)
static-release: clean-static static

$(STATIC_O_FILES):
	$(CC) $(_CFLAGS) $(CFLAGS) -o $@ -c $(patsubst $(STATIC_TMP)/%.o, src/%.c, $@);

#
# Shared library
#
shared: _CFLAGS += $(_SHARED_CFLAGS)
shared: $(SHARED_TMP) $(SHARED_O_FILES)
	$(CC) -shared $(SHARED_O_FILES) $(_LDFLAGS) $(LDFLAGS) -o $(SHARED_LIB)
	$(SIZE) --format=gnu $(SHARED_LIB)

shared-debug: _CFLAGS += $(_DEBUG_CFLAGS)
shared-debug: clean-shared shared

shared-release: _CFLAGS += $(_RELEASE_CFLAGS)
shared-release: clean-shared shared

$(SHARED_O_FILES):
	$(CC) $(_CFLAGS) $(CFLAGS) -o $@ -c $(patsubst $(SHARED_TMP)/%.o, src/%.c, $@);

#
# Packaging
#
tarfiles: tarfile-gz tarfile-xz tarfile-lz
tarfile-gz:
	git archive --format=tar --prefix=$(NAME)-$(VERSION)/ HEAD | gzip >$(NAME)-$(VERSION).tar.gz
tarfile-xz:
	git archive --format=tar --prefix=$(NAME)-$(VERSION)/ HEAD | xz >$(NAME)-$(VERSION).tar.xz
tarfile-lz:
	git archive --format=tar --prefix=$(NAME)-$(VERSION)/ HEAD | lzip >$(NAME)-$(VERSION).tar.lz

#
# Debugging
#
lint:
	splint $(SPLINT_FLAGS) +showsummary +showscan +stats +timedist $(C_FILES)
