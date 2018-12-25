#!/bin/bash


PROJECT_HOME=$(dirname $(pwd))
export BIN_PATH=$PROJECT_HOME/bin
export SDK_PATH=$PROJECT_HOME

export APP_PATH=$(pwd)
export TOOL_PATH=$PROJECT_HOME/tools/bin
export CONF_PATH=$APP_PATH/config

MODULE="DL3020D-16QCM"
FROM_SW_VER=1.0.0
SW_VER="1.1.0"
HW_VER=$MODULE

cp ./Makefile_DL2106F ./Makefile -f

make clean
./gen_misc_user1.sh

ret=$?
if [ $ret != 0 ]
    then
    echo "compile failed."
    exit 1
fi

make clean
./gen_misc_user2.sh
ret=$?
if [ $ret != 0 ]
    then
    echo "compile failed."
    exit 1
fi



#兼容的升级版本，从last_sw_version 到当前版本都可以使用此升级文件

USER1_OFFSET=160
USER2_OFFSET=409600
USER1_BIN=user1.1024.new.2.bin
USER2_BIN=user2.1024.new.2.bin
OTA_BIN="$MODULE"_OTA_FROM_V"$FROM_SW_VER"_TO_V"$SW_VER".bin
FAC_OTA_BIN="$MODULE"_OTA_FROM_V"$FROM_SW_VER"_TO_V"$SW_VER"_FAC.bin
USER1_NEW="$MODULE"_USER1_1024K_V"$SW_VER".bin
USER2_NEW="$MODULE"_USER2_1024K_V"$SW_VER".bin
RAW_FILE="$MODULE"_RAW_V"$SW_VER".bin

CONFIG_FILE="$MODULE"_MODULE_COMMON.conf
PATCH_CONFIG_FILE="$MODULE"_MODULE_PATCH.conf
PATCH_NONE_FILE=PATCH_NULL.conf

UTIME=`date +%s`
TEMP_DIR=tmp
mkdir -p $TEMP_DIR

cp $PROJECT_HOME/bin/upgrade/$USER1_BIN $TEMP_DIR/
cp $PROJECT_HOME/bin/upgrade/$USER2_BIN $TEMP_DIR/
cd tmp

$TOOL_PATH/ota_linux_addhead $MODULE $HW_VER $SW_VER $UTIME $USER1_OFFSET $USER1_BIN $USER2_OFFSET $USER2_BIN $OTA_BIN $CONF_PATH/$PATCH_CONFIG_FILE
iRet=$?
if [ $iRet != 0 ]
   then 
   echo "MERGE THE OTA BIN FAILED, ret = $iRet"
   rm -rf  $TEMP_DIR
   exit 1
fi

$TOOL_PATH/ota_linux_addhead $MODULE $HW_VER $SW_VER $UTIME $USER1_OFFSET $USER1_BIN $USER2_OFFSET $USER2_BIN $FAC_OTA_BIN $CONF_PATH/$CONFIG_FILE
iRet=$?
if [ $iRet != 0 ]
   then 
   echo "MERGE THE FAC OTA BIN FAILED, ret = $iRet"
   rm -rf  $TEMP_DIR
   exit 1
fi



cp $OTA_BIN $PROJECT_HOME/bin/upgrade/
cp $FAC_OTA_BIN $PROJECT_HOME/bin/upgrade/
mv $PROJECT_HOME/bin/upgrade/$USER1_BIN $PROJECT_HOME/bin/upgrade/$USER1_NEW
mv $PROJECT_HOME/bin/upgrade/$USER2_BIN $PROJECT_HOME/bin/upgrade/$USER2_NEW
cd $APP_PATH
rm -rf $TEMP_DIR


OUT_FILE=upgrade/$RAW_FILE
BLOCK_SIZE=4096
BOOT_BIN_NAME='boot_v1.7.bin'
USER1_BIN_NAME=upgrade/$USER1_NEW
USER1_BIN_BLOCKS=100
USER2_BIN_NAME=upgrade/$USER2_NEW
USER2_BIN_BLOCKS=100
USER2_BIN_SEEK=129
DEFAULT_PARM_BIN_NAME='esp_init_data_default.bin'
#DEFAULT_PARM_BIN ADDR: 0xFC000  BLOCK_SIZE: 252
DEFAULT_PARM_BIN_SEEK=252


cd $PROJECT_HOME/bin/

function create_raw_null_file()
{
    loop=256
    while [ $loop != 0 ]
    do
        let SEEK=256-$loop
        let loop=$loop-1
        dd if=blank.bin  bs=$BLOCK_SIZE count=1 skip=0 seek=$SEEK of=$OUT_FILE  >/dev/null 2>&1
    done    
}

echo $(create_raw_null_file)
dd if=$BOOT_BIN_NAME  bs=$BLOCK_SIZE count=1 skip=0 seek=0 of=$OUT_FILE conv=notrunc
dd if=$USER1_BIN_NAME  bs=$BLOCK_SIZE count=$USER1_BIN_BLOCKS skip=0 seek=1 of=$OUT_FILE conv=notrunc
dd if=$USER2_BIN_NAME  bs=$BLOCK_SIZE count=$USER2_BIN_BLOCKS skip=0 seek=$USER2_BIN_SEEK of=$OUT_FILE conv=notrunc
dd if=$DEFAULT_PARM_BIN_NAME bs=$BLOCK_SIZE count=1 skip=0 seek=$DEFAULT_PARM_BIN_SEEK of=$OUT_FILE conv=notrunc


#CONFIG ADDR 0x7C000=507904
CONFIG_ADDR=507904
$TOOL_PATH/init_config $CONF_PATH/$CONFIG_FILE $OUT_FILE $CONFIG_ADDR













