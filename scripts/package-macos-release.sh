#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd -P)"
VERSION="1.0.0"
PRODUCT="AudioReactiveImagePOC"
BUILD_DIR="${ROOT_DIR}/build/${PRODUCT}_artefacts/Release"
DIST_DIR="${ROOT_DIR}/dist"
PACKAGE_DIR="${DIST_DIR}/${PRODUCT}-v${VERSION}-macos"
ZIP_PATH="${DIST_DIR}/${PRODUCT}-v${VERSION}-macos.zip"

require_path() {
    local path="$1"
    if [ ! -e "$path" ]; then
        echo "Missing required release artifact: $path" >&2
        echo "Run: cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --config Release" >&2
        exit 1
    fi
}

require_path "${BUILD_DIR}/VST3/${PRODUCT}.vst3"
require_path "${BUILD_DIR}/AU/${PRODUCT}.component"
require_path "${BUILD_DIR}/Standalone/${PRODUCT}.app"
require_path "${ROOT_DIR}/README.md"
require_path "${ROOT_DIR}/RELEASE_NOTES.md"

codesign --force --deep -s - "${BUILD_DIR}/VST3/${PRODUCT}.vst3"
codesign --force --deep -s - "${BUILD_DIR}/AU/${PRODUCT}.component"
codesign --force --deep -s - "${BUILD_DIR}/Standalone/${PRODUCT}.app"

rm -rf "${PACKAGE_DIR}" "${ZIP_PATH}"
mkdir -p "${PACKAGE_DIR}/Plugins/VST3" "${PACKAGE_DIR}/Plugins/AU" "${PACKAGE_DIR}/Standalone"

ditto "${BUILD_DIR}/VST3/${PRODUCT}.vst3" "${PACKAGE_DIR}/Plugins/VST3/${PRODUCT}.vst3"
ditto "${BUILD_DIR}/AU/${PRODUCT}.component" "${PACKAGE_DIR}/Plugins/AU/${PRODUCT}.component"
ditto "${BUILD_DIR}/Standalone/${PRODUCT}.app" "${PACKAGE_DIR}/Standalone/${PRODUCT}.app"
cp "${ROOT_DIR}/README.md" "${PACKAGE_DIR}/README.md"
cp "${ROOT_DIR}/RELEASE_NOTES.md" "${PACKAGE_DIR}/RELEASE_NOTES.md"

cat > "${PACKAGE_DIR}/INSTALL_MACOS.txt" <<EOF
${PRODUCT} v${VERSION}

VST3 install:
  Copy Plugins/VST3/${PRODUCT}.vst3 to:
  ~/Library/Audio/Plug-Ins/VST3/

AU install:
  Copy Plugins/AU/${PRODUCT}.component to:
  ~/Library/Audio/Plug-Ins/Components/

Standalone:
  Open Standalone/${PRODUCT}.app

After installing, restart or rescan plugins in your DAW.
EOF

find "${PACKAGE_DIR}" -name "._*" -type f -delete

cd "${DIST_DIR}"
COPYFILE_DISABLE=1 zip -qry "${ZIP_PATH}" "${PRODUCT}-v${VERSION}-macos"

echo "Release package created:"
echo "${ZIP_PATH}"
