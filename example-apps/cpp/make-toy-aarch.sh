#!/bin/sh
source ~/Tools/remarkable-toolchain/environment-setup-cortexa53-crypto-remarkable-linux
aarch64-remarkable-linux-g++ --sysroot ~/Tools/remarkable-toolchain/sysroots/cortexa53-crypto-remarkable-linux toyclient.cpp ../../clients/cpp/qtfb-client.cpp -o toyclient_aarch

