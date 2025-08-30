APPNAME=refal
RAWDRAWANDROID=rawdrawandroid
PACKAGENAME?=com.github.butvinm.$(APPNAME)
SRC:=main.c

ANDROIDVERSION=35

include rawdrawandroid/Makefile
