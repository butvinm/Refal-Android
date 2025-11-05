APPNAME=refal
RAWDRAWANDROID=rawdrawandroid
PACKAGENAME?=com.github.butvinm.$(APPNAME)
ANDROIDVERSION=35

SRC:=main.c refalrawdraw.c Refal-05-Standalone/lib/refal05rts.c Refal-05-Standalone/lib/Library.c Refal-05-Standalone/lib/Library.c Refal-05-Standalone/bootstrap/LibraryEx.c
CFLAGS += -IRefal-05-Standalone/lib

REFAL05C=Refal-05-Standalone/bin/refal05


include rawdrawandroid/Makefile


# Bootstrap Refal05 compiler from source
$(REFAL05C):
	$(MAKE) -C Refal-05-Standalone bin/refal05


# Generate main C program from Refal program
main.c: $(REFAL05C) main.ref
	$(REFAL05C) main.ref refal05rts


.PHONY: push-run-from-refal
push-run-from-refal: main.c
	$(MAKE) push run
