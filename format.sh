#!/bin/sh

BASE_FOLDER=$(pwd)
BASE_TEST_FOLDER=./tests
BASE_TOOLS_FOLDER=./tools
LIB_FOLDER=./src
TEST_FIRMWARES="test_firmware_usb_loopback test_firmware_loopback test_firmware_hspi test_firmware_serdes test_firmware_usb_speedtest test_firmware_unittests"
TOOLS_FIRMWARES="firmware_debug_board"
SCRIPTS_FOLDERS="${BASE_TOOLS_FOLDER}/scripts ${BASE_TEST_FOLDER}/scripts"

if [ $# -eq 0 ]
then
    echo "Usage : ./build.sh all|lib|tests|tools"
    exit
fi

format_lib(){
    cd ${LIB_FOLDER} || exit
    sh format.sh || (cd "${BASE_FOLDER}" && exit)
    cd ${BASE_FOLDER} || exit
}

format_tests(){
    for firmware in ${TEST_FIRMWARES}
    do
        cd ${BASE_FOLDER}/"${BASE_TEST_FOLDER}"/"$firmware" || exit
        sh format.sh || (cd "${BASE_FOLDER}" && exit)
        cd ${BASE_FOLDER} || exit
    done
}

format_tools(){
    for firmware in ${TOOLS_FIRMWARES}
    do
        cd ${BASE_FOLDER}/"${BASE_TOOLS_FOLDER}"/"$firmware" || exit
        sh format.sh || (cd "${BASE_FOLDER}" && exit)
        cd ${BASE_FOLDER} || exit
    done
}

format_scripts(){
    for folder in ${SCRIPTS_FOLDERS}
    do
        autopep8 -i -r "$folder"/*.py --verbose || exit
    done

}

if [ "$1" = "all" ];
then
    format_lib;
    format_tests;
    format_tools;
    format_scripts
elif [ "$1" = "lib" ];
then
    format_lib;
elif [ "$1" = "tests" ];
then
    format_tests;
elif [ "$1" = "tools" ];
then
    format_tools;
elif [ "$1" = "scripts" ];
then
    format_scripts;
else
    echo "Please enter a valid command"
    exit;
fi
