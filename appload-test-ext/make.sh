cp ../management.cpp ../management.h ../FBController.cpp ../FBController.h ../log.h ../common.h .
# Do not copy main
qmake6 .
make -j`nproc`
