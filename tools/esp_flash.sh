#!/usr/bin/env bash

help="
Usage:
  -m only read log from device
  -p set tty device port path
  -b set build directory, relative with project root
"

while [ $# -gt 0 ]; do
  case "$1" in
    -m)
      mintor_only="233"
      ;;
    -p)
      tty_port="$2"
      shift
      ;;
    -b)
      build_dir="$2"
      shift
      ;;
    --port)
      tty_port="$2"
      shift
      ;;
    --help)
      printf "$help"
      exit
      ;;
    -h)
      printf "$help"
      exit
      ;;
    --*)
      echo "Illegal option $1"
      exit
      ;;
  esac
  shift $(( $# > 0 ? 1 : 0 ))
done

if [ ! $tty_port ]; then
  echo 'Error: missing tty device port path'
  exit 1
fi

if [ ! -e $tty_port ]; then
  echo "Error: tty device port path \"$tty_port\" is invalid"
  exit 1
fi

if [ ! $build_dir ]; then
  echo "Build directory not specified, assume it is ./build"
  build_dir='build'
fi

tools_dir="$( cd "$(dirname "$0")" ; pwd -P )"

bootloader=$tools_dir/../${build_dir}/bootloader/bootloader.bin
partition=$tools_dir/../${build_dir}/partition_table/partition-table.bin
node=$tools_dir/../${build_dir}/rt-node.bin
elf=$tools_dir/../${build_dir}/rt-node.elf

echo "using tty port: $tty_port"

if [ ! $mintor_only ]; then
  $tools_dir/esp_tool.py --chip esp32 \
    --port $tty_port \
    --baud 460800 write_flash \
    --flash_mode dio \
    --flash_size detect \
    --flash_freq 40m 0x1000 $bootloader 0x8000 $partition 0x10000 $node
fi

$tools_dir/esp_idf_monitor.py --port $tty_port $elf
