.PHONY: build app clean archive remote_package
app_name=AnycubicSlicer
curdate=$(shell date +%Y%m%d)
system=$(shell uname)
temp=$(abspath $(lastword $(MAKEFILE_LIST)))
rootdir=$(dir $(temp))
build_dir=$(rootdir)build



ifeq ($(system), Linux)
include MakefileLinux
else
include MakefileMacos
endif




