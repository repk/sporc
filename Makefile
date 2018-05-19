MKCONF ?= default

$(eval include mkconf/$(MKCONF).mk)

ifeq ($(DEBUG), 1)
	CFLAGS += -g -O0
endif

RULES = $(shell find . -name rules.mk)
_TARGETS =
_TARGETS_RULES=
_CROSSTARGETS =
_CROSSTARGETS_RULES =
_BUNDLES =
_FILES =
_FILES_RULES =

$(call sanitize-dir,BUILDDIR)
$(call sanitize-dir,OUTDIR)

define sanitize-dir
	$(eval $(1) = $$(patsubst /,,$$(patsubst %,%/,$($(1):%/=%))))
endef

define rm-file
	@(rm $(1) 2>/dev/null && \
		echo "rm $(1)") || true;
endef

define rm-dir
	@(rmdir -p $(1) 2>/dev/null && \
		echo "rmdir -p $(1)") || true;
endef

define file-add
	$(foreach f, $(1), \
		$(eval fout = $(2)$(notdir $(f))) \
		$(if $(filter $(fout), $(_FILES)), , \
			$(eval _FILES = $(_FILES) $(fout)) \
			$(eval _$(fout)_SRC=$(f))) \
	)
endef

define clean-env
	$(eval $(1)-OUTDIR=) \
	$(eval $(1)-CSRC=) \
	$(eval $(1)-ASRC=) \
	$(eval $(1)-FILE=) \
	$(eval $(1)-CFLAGS=) \
	$(eval $(1)-INCLUDE=) \
	$(eval $(1)-LDFLAGS=) \
	$(eval $(1)-LDSCRIPT=) \
	$(eval $(1)-DEPS=)
endef

define get-env
	$(if $(filter $(2), $($(1))), , \
		$(eval $(1) = $($(1)) $(2))) \
	$(if $($(2)-OUTDIR), \
		$(eval _$(2)_OUTDIR = $($(2)-OUTDIR))) \
		$(call sanitize-dir,_$(2)_OUTDIR) \
	$(if $($(2)-CSRC), \
		$(eval _$(2)_CSRC += \
			$($(2)-CSRC:%=$(3)%))) \
	$(if $($(2)-ASRC), \
		$(eval _$(2)_ASRC += \
			$($(2)-ASRC:%=$(3)%))) \
	$(if $($(2)-FILE), \
		$(eval _$(2)_FILE += \
			$($(2)-FILE:%=$(3)%))) \
	$(if $($(2)-CFLAGS), \
		$(eval _$(2)_CFLAGS += $($(2)-CFLAGS))) \
	$(if $($(2)-INCLUDE), \
		$(eval _$(2)_CFLAGS += \
			$($(2)-INCLUDE:%=-I $(3)%))) \
	$(if $($(2)-LDFLAGS), \
		$(eval _$(2)_LDFLAGS += $($(2)-LDFLAGS))) \
	$(if $($(2)-DEPS), \
		$(eval _$(2)_DEPS += $($(2)-DEPS))) \
	$(if $($(2)-LDSCRIPT), \
		$(eval _$(2)_LDFLAGS += \
		$($(2)-LDSCRIPT:%=-T $(3)%))) \
	$(call clean-env,$(2))
endef

define get-deps
	$(foreach d, $(_$(1)_DEPS), \
		$(if $(filter $(_$(1)_DEPS_DONE), $(d)), , \
			$(eval _$(1)_DEPS_DONE += $(d)) \
			$(call get-deps,$(d)) \
			$(if $(_$(d)_COBJ), \
				$(eval _$(1)_DEP_OBJ += $(_$(d)_COBJ))) \
			$(if $(_$(d)_AOBJ), \
				$(eval _$(1)_DEP_OBJ += $(_$(d)_AOBJ))) \
			$(if $(_$(d)_DEP_OBJ), \
				$(eval _$(1)_DEP_OBJ += $(_$(d)_DEP_OBJ))) \
			$(if $(_$(d)_CFLAGS), \
				$(eval _$(1)_CFLAGS += $(_$(d)_CFLAGS))) \
			$(if $(_$(d)_LDFLAGS), \
				$(eval _$(1)_LDFLAGS += $(_$(d)_LDFLAGS))) \
			$(call file-add,$(_$(d)_FILE),$(_$(1)_OUTDIR)) \
		) \
	)
endef

# Aggregate all targets environment variables
$(foreach r, $(RULES), \
	$(eval TARGET=) \
	$(eval CROSSTARGET=) \
	$(eval BUNDLE=) \
	$(eval -include $(r)) \
	$(foreach t, $(TARGET), \
		$(call get-env,_TARGETS,$(t),$(subst ./,,$(dir $(r))))) \
	$(foreach t, $(CROSSTARGET), \
		$(call get-env,_CROSSTARGETS,$(t),$(subst ./,,$(dir $(r))))) \
	$(foreach t, $(BUNDLE), \
		$(call get-env,_BUNDLES,$(t),$(subst ./,,$(dir $(r))))) \
)

define file-rule
$(OUTDIR)$(1): $(_$(1)_SRC)
	@mkdir -p $$(dir $$@)
	cp $$< $$@

$(OUTDIR)$(1)-clean:
	$(call rm-file,$$(@:%-clean=%))
	$(call rm-dir,$$(dir $$@))
endef

define cobj-rule
$(eval -include $(2:%.o=%.d))

$(2): $(2:$(BUILDDIR)%.o=%.c)
	@mkdir -p $$(dir $$@)
	$$(CC) -MMD -c $$(CFLAGS) $$(_$(1)_CFLAGS) -o $$@ $$<

$(2)-clean:
	$(call rm-file,$$(@:%-clean=%))
	$(call rm-file,$$(@:%.o-clean=%.d))
	$(call rm-dir,$$(dir $$@))
endef

define aobj-rule
$(2): $(2:$(BUILDDIR)%.o=%.s)
	@mkdir -p $$(dir $$@)
	$$(CC) -c $$(CFLAGS) $$(_$(1)_CFLAGS) -o $$@ $$<

$(2)-clean:
	$(call rm-file,$$(@:%-clean=%))
	$(call rm-dir,$$(dir $$@))
endef

define cross-cobj-rule
$(eval -include $(2:%.o=%.d))

$(2): $(2:$(BUILDDIR)%.o=%.c)
	@mkdir -p $$(dir $$@)
	$$(CROSSCC) -MMD -c $$(CROSSCFLAGS) $$(_$(1)_CFLAGS) -o $$@ $$<

$(2)-clean:
	$(call rm-file,$$(@:%-clean=%))
	$(call rm-file,$$(@:%.o-clean=%.d))
	$(call rm-dir,$$(dir $$@))
endef

define cross-aobj-rule
$(2): $(2:$(BUILDDIR)%.o=%.s)
	@mkdir -p $$(dir $$@)
	$$(CROSSCC) -c $$(CROSSCFLAGS) $$(_$(1)_CFLAGS) -o $$@ $$<

$(2)-clean:
	$(call rm-file,$$(@:%-clean=%))
	$(call rm-dir,$$(dir $$@))
endef

define bundle-rule
$(foreach o, $(_$(1)_COBJ), $(call cobj-rule,$(1),$(o)))
$(foreach o, $(_$(1)_AOBJ), $(call aobj-rule,$(1),$(o)))
endef

define target-rule
ifneq ($(_$(1)_COBJ) $(_$(1)_AOBJ) $(_$(1)_DEP_OBJ),)
$(2): $(_$(1)_COBJ) $(_$(1)_AOBJ) $(_$(1)_DEP_OBJ)
	@mkdir -p $$(dir $$@)
	$$(CC) -o $$@ $$^ $$(LDFLAGS) $$(_$(1)_LDFLAGS)

$(2)-clean: $(_$(1)_COBJ:%=%-clean) $(_$(1)_AOBJ:%=%-clean) \
		$(_$(1)_DEP_OBJ:%=%-clean)
	$(call rm-file,$$(@:%-clean=%))
	$(call rm-dir,$$(dir $$@))

$(foreach o, $(_$(1)_COBJ), $(call cobj-rule,$(1),$(o)))
$(foreach o, $(_$(1)_AOBJ), $(call aobj-rule,$(1),$(o)))
else
$(2):
$(2)-clean:
endif
endef

define cross-target-rule
ifneq ($(_$(1)_COBJ) $(_$(1)_AOBJ) $(_$(1)_DEP_OBJ),)
$(2).elf: $(_$(1)_COBJ) $(_$(1)_AOBJ) $(_$(1)_DEP_OBJ)
	@mkdir -p $$(dir $$@)
	$$(CROSSCC) -o $$@ $$^ $$(CROSSLDFLAGS) $$(_$(1)_LDFLAGS)

$(2): $(2).elf
	$$(CROSSOBJCOPY) -O binary $$< $$@

$(2)-clean: $(_$(1)_COBJ:%=%-clean) $(_$(1)_AOBJ:%=%-clean) \
		$(_$(1)_DEP_OBJ:%=%-clean)
	$(call rm-file,$$(@:%-clean=%.elf))
	$(call rm-file,$$(@:%-clean=%))
	$(call rm-dir,$$(dir $$@))

$(foreach o, $(_$(1)_COBJ), $(call cross-cobj-rule,$(1),$(o)))
$(foreach o, $(_$(1)_AOBJ), $(call cross-aobj-rule,$(1),$(o)))
else
$(2):
$(2)-clean:
endif
endef

$(foreach t, $(_BUNDLES), \
	$(eval _$(t)_COBJ = $(_$(t)_CSRC:%.c=$(BUILDDIR)%.o)) \
	$(eval _$(t)_AOBJ = $(_$(t)_ASRC:%.s=$(BUILDDIR)%.o)) \
)

$(foreach t, $(_TARGETS), \
	$(eval _$(t)_RULE = $(t:%=$(OUTDIR)$(_$(t)_OUTDIR)%)) \
	$(eval _TARGETS_RULES = $(_TARGETS_RULES) $(_$(t)_RULE)) \
	$(call get-deps,$(t)) \
	$(eval _$(t)_COBJ = $(_$(t)_CSRC:%.c=$(BUILDDIR)%.o)) \
	$(eval _$(t)_AOBJ = $(_$(t)_ASRC:%.s=$(BUILDDIR)%.o)) \
	$(call file-add,$(_$(t)_FILE),$(_$(t)_OUTDIR)) \
)

$(foreach t, $(_CROSSTARGETS), \
	$(eval _$(t)_RULE = $(t:%=$(OUTDIR)$(_$(t)_OUTDIR)%)) \
	$(eval _CROSSTARGETS_RULES = $(_CROSSTARGETS_RULES) $(_$(t)_RULE)) \
	$(call get-deps,$(t)) \
	$(eval _$(t)_COBJ = $(_$(t)_CSRC:%.c=$(BUILDDIR)%.o)) \
	$(eval _$(t)_AOBJ = $(_$(t)_ASRC:%.s=$(BUILDDIR)%.o)) \
	$(call file-add,$(_$(t)_FILE),$(_$(t)_OUTDIR)) \
)

$(foreach t, $(_FILES), \
	$(eval _$(t)_RULE = $(t:%=$(OUTDIR)%)) \
	$(eval _FILES_RULES = $(_FILES_RULES) $(_$(t)_RULE)) \
)

all: $(_TARGETS_RULES) $(_CROSSTARGETS_RULES) $(_FILES_RULES)

$(foreach t, $(_FILES), \
	$(eval $(call file-rule,$(t))))

$(foreach t, $(_BUNDLES), \
	$(eval $(call bundle-rule,$(t))))

$(foreach t, $(_TARGETS), \
	$(eval $(call target-rule,$(t),$(_$(t)_RULE))))

$(foreach t, $(_CROSSTARGETS), \
	$(eval $(call cross-target-rule,$(t),$(_$(t)_RULE))))

.PHONY: clean

clean: $(_TARGETS_RULES:%=%-clean) $(_CROSSTARGETS_RULES:%=%-clean) \
		$(_FILES_RULES:%=%-clean)
	$(call rm-dir,$(BUILDDIR))
	$(call rm-dir,$(OUTDIR))

