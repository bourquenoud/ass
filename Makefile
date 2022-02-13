CFLAGS = -O0 -ggdb3
LDFLAGS = -lm
output_file = ass
output_dir = build/bin/

lex_file := ass.l
parse_file := ass.y

gen_src_files := ass.tab.c ass.yy.c
gen_obj_files := ass.tab.o ass.yy.o

src_dir := src
obj_dir := build
gen_dir := generated
sklt_dir := skeletons

gen_src := $(src_dir)/$(gen_dir)/$(gen_src_files)
gen_obj := $(obj_dir)/$(gen_dir)/$(gen_obj_files)

SRCS = $(shell find $(src_dir)/ -type f -name '*.c' -not -path "$(src_dir)/$(gen_dir)/*")
intermediate = $(patsubst $(src_dir)%,$(obj_dir)%,$(SRCS))
OBJS = $(patsubst %.c,%.o,$(intermediate))
OBJSDIRS = $(dir $(OBJS))

#Function to remove duplicates
define uniq =
  $(eval seen :=)
  $(foreach _,$1,$(if $(filter $_,${seen}),,$(eval seen += $_)))
  ${seen}
endef

all: $(output_dir)$(output_file)

########################################################################
#                            DIRECTORIES                               #
########################################################################

$(call uniq,$(obj_dir)/ $(OBJSDIRS)):
		mkdir -p $@

$(src_dir)/$(gen_dir): | $(src_dir)/
		@mkdir $(src_dir)/$(gen_dir)

$(obj_dir)/$(gen_dir): | $(obj_dir)/
		@mkdir $(obj_dir)/$(gen_dir)

$(output_dir):
		@mkdir -p $(output_dir)

########################################################################
#                      SKELETONS FILE GENERATIONS                      #
########################################################################
$(src_dir)/$(gen_dir)/skeleton.h: $(sklt_dir)/skeleton.h.sk
		xxd -i $(sklt_dir)/skeleton.h.sk $(src_dir)/$(gen_dir)/skeleton.h
		sed -i 's/unsigned char .*\[\]/unsigned char SKELETON\[\]/g' src/generated/skeleton.h
		sed -i 's/unsigned int .*_len/unsigned int SKELETON_LEN/g' src/generated/skeleton.h

########################################################################
#                    BISON AND FLEX FILE GENERATIONS                   #
########################################################################

#Generate the files with flex and bison
$(src_dir)/$(gen_dir)/ass.tab.c $(src_dir)/$(gen_dir)/ass.tab.h: $(src_dir)/$(parse_file) | $(src_dir)/$(gen_dir)
		bison -l -t -v --defines=$(src_dir)/$(gen_dir)/ass.tab.h --output=$(src_dir)/$(gen_dir)/ass.tab.c $(src_dir)/$(parse_file)
	
$(src_dir)/$(gen_dir)/ass.yy.c: $(src_dir)/$(lex_file) $(src_dir)/$(gen_dir)/ass.tab.h | $(src_dir)/$(gen_dir)
		flex --outfile=$(src_dir)/$(gen_dir)/ass.yy.c $(src_dir)/$(lex_file)

#TODO: make it not ugly af
#Compile the two generated files
$(obj_dir)/$(gen_dir)/ass.yy.o $(obj_dir)/$(gen_dir)/ass.tab.o : $(src_dir)/$(gen_dir)/ass.yy.c $(src_dir)/$(gen_dir)/ass.tab.c $(src_dir)/$(gen_dir)/ass.tab.h | $(obj_dir)/$(gen_dir)
		@$(foreach file, $(gen_obj_files),\
			echo gcc $(CFLAGS)  -c -o $(obj_dir)/$(gen_dir)/$(file) $(src_dir)/$(gen_dir)/$(patsubst %.o,%.c,$(file));\
			gcc $(CFLAGS)  -c -o $(obj_dir)/$(gen_dir)/$(file) $(src_dir)/$(gen_dir)/$(patsubst %.o,%.c,$(file));\
		)

########################################################################
#                             COMPILING                                #
########################################################################

#Compile all files except the generated ones. Include the header
$(OBJS): $(SRCS) $(src_dir)/$(gen_dir)/ass.tab.h $(obj_dir)/$(gen_dir)/ass.yy.o $(obj_dir)/$(gen_dir)/ass.tab.o $(src_dir)/$(gen_dir)/skeleton.h | $(OBJSDIRS)
		@$(foreach file, $@,\
		echo gcc $(CFLAGS) -c -o $(file) $(patsubst $(obj_dir)%.o,$(src_dir)%.c,$(file));\
		gcc $(CFLAGS) -c -o $(file) $(patsubst $(obj_dir)%.o,$(src_dir)%.c,$(file));\
		)

#Link
$(output_dir)$(output_file): $(OBJS) | $(output_dir)
	gcc -o $(output_dir)/$(output_file) $(LDFLAGS) $(shell find $(obj_dir) -name *.o)

########################################################################
#                               CLEAN                                  #
########################################################################
clean:
		@rm -rf $(obj_dir)/
		@rm -rf $(src_dir)/$(gen_dir)/
		@echo cleaned