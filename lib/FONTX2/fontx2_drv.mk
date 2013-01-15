# This is the part of FONTX2 Driver 

#/*----- Display library PATH -----*/
FONTX2_INC = $(FONTX2_LIB)/inc
FONTX2_SRC = $(FONTX2_LIB)/src
FONTX2_FNT = $(FONTX2_INC)/fonts

LIBINCDIRS += $(FONTX2_INC)		\
			  $(FONTX2_FNT)

#Source of FONTX2 Support Driver
CFILES += \
 $(FONTX2_SRC)/font_if_datatable.c	\
 $(FONTX2_SRC)/font_if.c
