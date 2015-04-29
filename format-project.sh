#!/bin/sh

runClangFormatOnDir() {
    find "$1" -name "*.cpp" -o -name "*.h" | while read fn; do
        echo "$fn"
        clang-format -style=file -i "$fn"
    done
}
(
cd $(dirname $0)

runClangFormatOnDir OSVRUnreal/Plugins/OSVR/Source/OSVR
runClangFormatOnDir OSVRUnreal/Source/OSVRUnreal

)
