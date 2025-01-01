source ~/Tools/remarkable-toolchain/environment-setup-cortexa53-crypto-remarkable-linux
cp ../management.cpp ../management.h ../FBController.cpp ../FBController.h ../common.h ../log.h .
# Do not copy main
python3 $XOVI_HOME/util/xovigen.py --cpp -o output qtfb.xovi
mv output/xovi.c output/main.cpp
qmake6 .
make -j`nproc`
