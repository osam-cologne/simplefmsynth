#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by falkTX, Christopher Arndt, and Patrick Desaulniers
#


# --------------------------------------------------------------
# Installation directories

PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
LIBDIR ?= $(PREFIX)/lib
DSSI_DIR ?= $(LIBDIR)/dssi
LADSPA_DIR ?= $(LIBDIR)/ladspa
ifneq ($(MACOS_OR_WINDOWS),true)
export LV2_DIR ?= $(LIBDIR)/lv2
VST_DIR ?= $(LIBDIR)/vst
endif
ifeq ($(MACOS),true)
LV2_DIR ?= /Library/Audio/Plug-Ins/LV2
VST_DIR ?= /Library/Audio/Plug-Ins/VST
endif
ifeq ($(WINDOWS),true)
LV2_DIR ?= $(COMMONPROGRAMFILES)/LV2
VST_DIR ?= $(COMMONPROGRAMFILES)/VST2
endif

export BINDIR DSSI_DIR LADSPA_DIR LIBDIR LV2_DIR VST_DIR

USER_DSSI_DIR ?= $(HOME)/.dssi
USER_LADSPA_DIR ?= $(HOME)/.ladspa
ifneq ($(MACOS_OR_WINDOWS),true)
USER_LV2_DIR ?= $(HOME)/.lv2
USER_VST_DIR ?= $(HOME)/.vst
endif
ifeq ($(MACOS),true)
USER_LV2_DIR ?= $(HOME)/Library/Audio/Plug-Ins/LV2
USER_VST_DIR ?= $(HOME)/Library/Audio/Plug-Ins/VST
endif
ifeq ($(WINDOWS),true)
USER_LV2_DIR ?= $(APPDATA)/LV2
USER_VST_DIR ?= $(APPDATA)/VST
endif

export USER_DSSI_DIR USER_LADSPA_DIR USER_LV2_DIR USER_VST_DIR

GAMMA_DIR ?= $(shell pwd)/gamma
GAMMA_INCLUDE_DIR ?= $(GAMMA_DIR)
GAMMA_LIB_DIR ?= $(GAMMA_DIR)/build/lib

export GAMMA_DIR GAMMA_INCLUDE_DIR GAMMA_LIB_DIR

# --------------------------------------------------------------

include dpf/Makefile.base.mk

all: libs plugins gen

# --------------------------------------------------------------

submodules:
	git submodule update --init --recursive

libs:
	$(MAKE) NO_AUDIO_IO=1 NO_SOUNDFILE=0 DYNAMIC=1 -C gamma
	@ln -sf libGamma.1.0.so $(GAMMA_LIB_DIR)/libGamma.1.so
	@ln -sf libGamma.1.0.so $(GAMMA_LIB_DIR)/libGamma.so

plugins: libs
	$(MAKE) all -C plugins/SimpleFMSynth

ifneq ($(CROSS_COMPILING),true)
gen: plugins dpf/utils/lv2_ttl_generator
	LD_LIBRARY_PATH=$(GAMMA_LIB_DIR) $(CURDIR)/dpf/utils/generate-ttl.sh
ifeq ($(MACOS),true)
	@$(CURDIR)/dpf/utils/generate-vst-bundles.sh
endif

dpf/utils/lv2_ttl_generator:
	LD_LIBRARY_PATH=$(GAMMA_LIB_DIR) $(MAKE) -C dpf/utils/lv2-ttl-generator
else
gen: plugins dpf/utils/lv2_ttl_generator.exe
	LD_LIBRARY_PATH=$(GAMMA_LIB_DIR) $(CURDIR)/dpf/utils/generate-ttl.sh

dpf/utils/lv2_ttl_generator.exe:
	$(MAKE) -C dpf/utils/lv2-ttl-generator WINDOWS=true
endif

# --------------------------------------------------------------

clean:
	$(MAKE) clean -C dpf/utils/lv2-ttl-generator
	$(MAKE) clean -C plugins/SimpleFMSynth
	rm -rf bin build

install: all
	$(MAKE) install -C plugins/SimpleFMSynth

install-user: all
	$(MAKE) install-user -C plugins/SimpleFMSynth

# --------------------------------------------------------------

.PHONY: all clean install install-user submodule libs plugins gen
