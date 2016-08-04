exec_projects=main

main_binary  =dist/bin/main
main_objects =main.o tz/tz/gfx/gl_core_3_3.o
main_cc_flags=$$(pkg-config --cflags sdl2) -std=c99 -g -march=native
main_ld_flags=$$(pkg-config --libs   gl sdl2) -lm

all_projects_binaries=$(foreach e,$(exec_projects),$($(e)_binary))

CC=$(PREFIX)gcc
binary_dir =bin/
objects_dir=o/
sources_dir=src/

define rules_template =
$(1)_objects_=$$(addprefix $$(objects_dir),$$($(1)_objects))
$(1)_sources_=$$(addprefix $$(sources_dir),$$($(1)_objects))
$(1)_depends_=$$(addprefix $$(sources_dir),$$($(1)_objects))

$$($(1)_binary): $$($(1)_objects_)
	@mkdir -p $$(@D)
	@echo "LD: $$@"
	@$$(CC) $$($(1)_cc_flags) $$^ -o $$@ $$($(1)_ld_flags)

$$($(1)_objects_): $$(objects_dir)%.o: $$(sources_dir)%.c
	@mkdir -p $$(@D)
	@echo "CC: $$@"
	@$$(CC) $$($(1)_cc_flags) -MM -MT $$@ -MD -o $$@ $$<
	@$$(CC) $$($(1)_cc_flags) -c $$< -o $$@

O+=$$($(1)_objects_)
C+=$$($(1)_sources_:.o=.c)
D+=$$($(1)_objects_:.o=.d)
endef

.PHONY: all clean
all: $(all_projects_binaries)
clean:
	$(RM) $(all_projects_binaries) $(O) $(D)

$(foreach p,$(exec_projects),$(eval $(call rules_template,$(p))))
-include $(D)
