APPNAME=refal
RAWDRAWANDROID=rawdrawandroid
PACKAGENAME?=com.github.butvinm.$(APPNAME)
ANDROIDVERSION=35
SRC:=main.c

REFAL05C=Refal-05-Standalone/bin/refal

include rawdrawandroid/Makefile

$(REFAL05C):
	$(MAKE) -C Refal-05-Standalone bin/refal05
