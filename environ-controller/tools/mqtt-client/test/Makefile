
# Source code dir details 

PROJ_DIR =./


# source files  

# C files  
SRC += \
 $(PROJ_DIR)/../src/main.c\
 $(PROJ_DIR)/../src/serial_comport.c\
 $(PROJ_DIR)/../../../system/sw/code/common/src/utility.c\
 $(PROJ_DIR)/../../../system/sw/code/cntrl-app/src/osa.c\
 $(PROJ_DIR)/../../../system/sw/code/cntrl-app/src/mqtt_com.c\
 $(PROJ_DIR)/../../../system/sw/code/cntrl-app/src/nw_wifi_esp.c\
 $(PROJ_DIR)/../../../system/sw/code/cntrl-app/src/list.c\
 $(PROJ_DIR)/../../../system/sw/code/cntrl-app/paho.mqtt.embedded-c-master/MQTTClient-C/src/MQTTClient.c\
 $(PROJ_DIR)/../../../system/sw/code/cntrl-app/paho.mqtt.embedded-c-master/MQTTPacket/src/MQTTConnectClient.c\
 $(PROJ_DIR)/../../../system/sw/code/cntrl-app/paho.mqtt.embedded-c-master/MQTTPacket/src/MQTTConnectServer.c\
 $(PROJ_DIR)/../../../system/sw/code/cntrl-app/paho.mqtt.embedded-c-master/MQTTPacket/src/MQTTDeserializePublish.c\
 $(PROJ_DIR)/../../../system/sw/code/cntrl-app/paho.mqtt.embedded-c-master/MQTTPacket/src/MQTTFormat.c\
 $(PROJ_DIR)/../../../system/sw/code/cntrl-app/paho.mqtt.embedded-c-master/MQTTPacket/src/MQTTPacket.c\
 $(PROJ_DIR)/../../../system/sw/code/cntrl-app/paho.mqtt.embedded-c-master/MQTTPacket/src/MQTTSerializePublish.c\
 $(PROJ_DIR)/../../../system/sw/code/cntrl-app/paho.mqtt.embedded-c-master/MQTTPacket/src/MQTTSubscribeClient.c\
 $(PROJ_DIR)/../../../system/sw/code/cntrl-app/paho.mqtt.embedded-c-master/MQTTPacket/src/MQTTSubscribeServer.c\
 $(PROJ_DIR)/../../../system/sw/code/cntrl-app/paho.mqtt.embedded-c-master/MQTTPacket/src/MQTTUnsubscribeClient.c\
 $(PROJ_DIR)/../../../system/sw/code/cntrl-app/paho.mqtt.embedded-c-master/MQTTPacket/src/MQTTUnsubscribeServer.c\
 $(PROJ_DIR)/../../../system/sw/code/cntrl-app/paho.mqtt.embedded-c-master/MQTTClient-C/src/linux/MQTTLinux.c\






# Assembly files 
SRC_A +=\

       

# Include paths 
INC += \
 -I$(PROJ_DIR)/\
 -I$(PROJ_DIR)/../inc\
 -I$(PROJ_DIR)/../../../system/sw/code/common/inc\
 -I$(PROJ_DIR)/../../../system/sw/code/cntrl-app/inc\
 -I$(PROJ_DIR)/../../../system/sw/code/cntrl-app/paho.mqtt.embedded-c-master/MQTTClient-C/src\
 -I$(PROJ_DIR)/../../../system/sw/code/cntrl-app/paho.mqtt.embedded-c-master/MQTTClient-C/src/linux\
 -I$(PROJ_DIR)/../../../system/sw/code/cntrl-app/paho.mqtt.embedded-c-master/MQTTPacket/src\





	
###############################################################################
#                    Tool chain Details & common flags                        #
###############################################################################
CC = gcc
ASM = 
LINK = gcc 


LDFLAGS =  -pthread  -lrt 



# System paths

###############################################################################
#                   Building Release                                       #
###############################################################################

app: release/app

OBJECT_C=$(addprefix release/obj/,$(notdir $(SRC:.c=.o)))

OBJECT_S=$(addprefix release/obj/,$(notdir $(SRC_A:.s=.o)))

OBJECT_D=$(addprefix release/dep/,$(notdir $(SRC:.c=.d)))


# compiler flags 
#RELFLAGS +=  -DMINGW32 -DWINDOWS_MINGW -DNW_WIFI_ESP
RELFLAGS +=  -DLINUX -DNW_WIFI_ESP

  

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

release/app:OBJDIR_RELEASE $(OBJECT_S) $(OBJECT_C)
	$(LINK) $(OBJECT_S) $(OBJECT_C) $(LDFLAGS)  -o  $@ 



$(OBJECT_C): release/obj/%.o: 
	@echo [TARGET NAME $@ ]  
	@echo [COMPILING SOURCE FILE $(filter %$(subst .o,.c,$(@F)),$(SRC)) ]
	$(CC)   -c  $(INC) $(RELFLAGS) -o $@ $(filter %/$(subst .o,.c,$(@F)),$(SRC))
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
	@echo [  make clean-rel : to remove release MAC]
	@echo [  make clean-dbg : to remove debug MAC]
	@echo [  make app  : to build  release APP ]
	@echo [  make dapp : to build debug DAPP ]
	@echo


###############################################################################
#                   Building  Debug                                        #
###############################################################################

dapp: debug/dapp

DEBUG_OBJECT_C=$(addprefix debug/obj/,$(notdir $(SRC:.c=.o)))

DEBUG_OBJECT_S=$(addprefix debug/obj/,$(notdir $(SRC_A:.s=.o)))

DEBUG_OBJECT_D=$(addprefix debug/dep/,$(notdir $(SRC:.c=.d)))

# Add additional compiler flags here
DEBUGFLAGS +=  \


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

  include $(DEBUG_OBJECT_D)

endif 

debug/dapp:OBJDIR_DEBUG $(DEBUG_OBJECT_S) $(DEBUG_OBJECT_C)
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
