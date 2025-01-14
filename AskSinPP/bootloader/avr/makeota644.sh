#!/bin/bash

BOOTLOADER=Bootloader-OTA-atmega644.hex

if [ "$3" == "" ]; then
  echo "Missing argument";
  echo "usage: makeota.sh DEVMODEL HMID SERIAL [CONFIG]"
  exit 5
fi

if [ ! -f "$BOOTLOADER" ]; then
  echo "Missing Bootloader Hex File: $BOOTLOADER"
  exit 5
fi

DEVMODEL=${1^^}
HMID=${2^^}
SERIAL=$3
CFGSTR=${4^^}

if [ ${#DEVMODEL} != 4 ]; then
  echo "Wrong Device Model: 2 Byte (hex) expected"
  exit 5
fi

if [ ${#HMID} != 6 ]; then
  echo "Wrong Homematic Device ID: 3 Byte (hex) expected"
  exit 5
fi

if [ ${#SERIAL} != 10 ]; then
  echo "Wrong Serial: 10 Byte (ascii) expected"
  exit 5
fi

while [ ${#CFGSTR} -le 31 ]; do
  CFGSTR=$CFGSTR"0"
done

LINEDEVMODEL="02FFF000"
LINEHMID="03FFFC00"
LINESERIAL="0AFFF200"

function checksum {
  SUM=0
  for i in `echo "$1" | grep -o ..`; do 
#    echo $i
	SUM=$(( $SUM + 16#${i} ))
#	printf "%x\n" $SUM
  done
#  printf "%x\n" $(( $SUM % 256 ))
  SUM=$(( 256 - ($SUM % 256) ))
  echo -n $1
  printf "%02X\n" $SUM
}

OUTCFG=`checksum "10FFE000"$CFGSTR`
OUTDEVMODEL=`checksum $LINEDEVMODEL$DEVMODEL`
# checksum $LINEDEVMODEL$DEVMODEL
OUTHMID=`checksum $LINEHMID$HMID`
# checksum $LINEHMID$HMID

# convert serial to HEX
SHEX=
for i in `echo "$SERIAL" | grep -o .`; do 
  NUM=`printf "%02X" "'$i"`
  SHEX=$SHEX$NUM
done
#echo $SHEX
OUTSERIAL=`checksum $LINESERIAL$SHEX`

cat ${BOOTLOADER} | \
  sed -e "s/:"$LINEDEVMODEL".*/:"$OUTCFG"\n:"$OUTDEVMODEL"/g" \
      -e "s/:"$LINEHMID".*/:"$OUTHMID"/g" \
	  -e "s/:"$LINESERIAL".*/:"$OUTSERIAL"/g"
