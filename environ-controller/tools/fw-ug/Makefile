
# Source code dir details 

PROJ_DIR =./


# source files  

# C files  
SRC += \
 $(PROJ_DIR)/fwug_tool.c\
 $(PROJ_DIR)/../../system/sw/code/common/src/base64.c\

SRCPP += \


# Assembly files 
SRC_A +=\

       

# Include paths 
INC += \
 -I$(PROJ_DIR)/\
 -I$(PROJ_DIR)/../../system/sw/code/common/inc\


	
###############################################################################
#                    Tool chain Details & common flags                        #
###############################################################################
CC = gcc
CPPC = g++
ASM = 
LINK = gcc 


LDFLAGS =  -pthread  
 



# System paths

###############################################################################
#                   Building Release APP                                      #
###############################################################################

fwug: release/fwug

OBJECT_C=$(addprefix release/obj/,$(notdir $(SRC:.c=.o)))

OBJECT_CPP=$(addprefix release/obj/,$(notdir $(SRCPP:.cpp=.o)))

OBJECT_S=$(addprefix release/obj/,$(notdir $(SRC_A:.s=.o)))

OBJECT_D=$(addprefix release/dep/,$(notdir $(SRC:.c=.d)))


# compiler flags 
RELFLAGS += -DLINUX -DCYGWIN_ENVIRONMENT  `mysql_config --cflags --libs`\

  

$(OBJECT_D)     :   release/dep/%.d :
	@echo "Building target dependency for $@"
	@echo [FILE NAME $(@F) ]
	@test -d release || mkdir release
	@test -d release/dep || mkdir release/dep
	@echo [TARGET NAME $@ ]  
	@echo [SOURCE FILE $(filter %$(subst .d,.c,$(@F)),$(SRC)) OVER ]
	@set -e; $(CC) -M  $(INC) $(RELFLAGS) $(filter %$(subst .d,.c,$(@F)),$(SRC)) \
| sed -e 's_$(subst .d,.o,$(@F))_$(addprefix release/obj/,$(subst .d,.o,$(@F))) $@ _' \
| sed  -n  '/C:\\Program Files\\ARM\\RVCT\\Data\\2.2\\503\\include/ !p'\
 > $@;\
[ -s $@ ] || rm -f $@; 
	@echo "" >> $@
	@echo [ COMPLTED Dependency Creation ]

ifeq ($(filter app, $(MAKECMDGOALS)),app)

  #include $(OBJECT_D)

endif 

release/fwug:OBJDIR_RELEASE $(OBJECT_S) $(OBJECT_C) $(OBJECT_CPP)
	$(LINK) $(OBJECT_S) $(OBJECT_C) $(OBJECT_CPP)  -o  $@ $(LDFLAGS) 




$(OBJECT_C): release/obj/%.o: 
	@echo [TARGET NAME $@ ]  
	@echo [COMPILING SOURCE FILE $(filter %$(subst .o,.c,$(@F)),$(SRC)) ]
	$(CC)   -c  $(INC) $(RELFLAGS) -o $@ $(filter %/$(subst .o,.c,$(@F)),$(SRC))
	@echo

$(OBJECT_CPP): release/obj/%.o: 
	@echo [TARGET NAME $@ ]  
	@echo [COMPILING SOURCE FILE $(filter %$(subst .o,.cpp,$(@F)),$(SRCPP)) ]
	$(CPPC)   -c  $(INC) $(RELFLAGS) -o $@ $(filter %/$(subst .o,.cpp,$(@F)),$(SRCPP))
	@echo


$(OBJECT_S): release/obj/%.o : 
	@echo [TARGET NAME $@ ]  
	@echo [COMPILING SOURCE FILE $(filter %$(subst .o,.s,$(@F)),$(SRC_A))]
	$(ASM) $(INC) $(RELFLAGS) -o $@ $(filter %/$(subst .o,.s,$(@F)),$(SRC_A))
	@echo


OBJDIR_RELEASE:
	@echo [Creating Object Directory-"release"]
	@test -d release || mkdir release
	@test -d release/obj || mkdir release/obj
	@echo

###############################################################################
#                   General Rules                                             #
###############################################################################

clean:
	rm -rf ./release
	rm -rf ./debug

clean-rel:
	rm -rf ./release

clean-dbg:
	rm -rf ./debug

help:
	@echo [  make clean :  to remove all the targets]
	@echo [  make clean-rel : to remove release SERVER]
	@echo [  make clean-dbg : to remove debug SERVER]
	@echo [  make server  : to build  release SERVER ]
	@echo [  make dapp : to build debug SERVER  ]
	@echo


###############################################################################
#                   Building  Debug MAC                                       #
###############################################################################

dserver: debug/dserver

DEBUG_OBJECT_C=$(addprefix debug/obj/,$(notdir $(SRC:.c=.o)))

DEBUG_OBJECT_S=$(addprefix debug/obj/,$(notdir $(SRC_A:.s=.o)))

DEBUG_OBJECT_D=$(addprefix debug/dep/,$(notdir $(SRC:.c=.d)))

# Add additional compiler flags here
DEBUGFLAGS += -DDEBUG_PRINT_ON  -DCYGWIN_ENVIRONMENT  `mysql_config --cflags --libs`\


$(DEBUG_OBJECT_D)     :   debug/dep/%.d :
	@echo "Building target dependency for $@"
	@echo [FILE NAME $(@F) ]
	@test -d debug || mkdir debug
	@test -d debug/dep || mkdir debug/dep
	@echo [TARGET NAME $@ ]  
	@echo [SOURCE FILE $(filter %$(subst .d,.c,$(@F)),$(SRC)) OVER ]
	@set -e; $(CC) -M  $(INC) $(RELFLAGS) $(filter %$(subst .d,.c,$(@F)),$(SRC)) \
| sed -e 's_$(subst .d,.o,$(@F))_$(addprefix debug/obj/,$(subst .d,.o,$(@F))) $@ _' \
| sed  -n  '/C:\\Program Files\\ARM\\RVCT\\Data\\2.2\\503\\include/ !p'\
 > $@;\
[ -s $@ ] || rm -f $@; 
	@echo "" >> $@
	@echo [ COMPLTED Dependency Creation ]

ifeq ($(filter dapp, $(MAKECMDGOALS)),dapp)

 #include $(DEBUG_OBJECT_D)

endif 

debug/dserver:OBJDIR_DEBUG $(DEBUG_OBJECT_S) $(DEBUG_OBJECT_C)
	$(LINK) $(LDFLAGS) $(DEBUG_OBJECT_S) $(DEBUG_OBJECT_C) -o $@


$(DEBUG_OBJECT_C): debug/obj/%.o: 
	@echo [TARGET NAME $@ ]  
	@echo [COMPILING SOURCE FILE $(filter %$(subst .o,.c,$(@F)),$(SRC)) ]
	$(CC)   -c -g  $(INC) $(DEBUGFLAGS) -o $@ $(filter %/$(subst .o,.c,$(@F)),$(SRC))
	@echo


$(DEBUG_OBJECT_S): debug/obj/%.o : 
	@echo [TARGET NAME $@ ]  
	@echo [COMPILING SOURCE FILE $(filter %$(subst .o,.s,$(@F)),$(SRC_A)) OVER ]
	$(ASM) $(INC) $(DEBUGFLAGS)   -o $@ $(filter %/$(subst .o,.s,$(@F)),$(SRC_A))
	@echo


OBJDIR_DEBUG:
	@echo [Creating Object Directory-"debug"]
	@test -d debug || mkdir debug
	@test -d debug/obj || mkdir debug/obj
	@echo
