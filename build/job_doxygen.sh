#!/bin/bash
set -x

ROOT_DIR=`pwd`
build_dir="$(dirname "${BASH_SOURCE[0]}")"

# Path to the doxygen configuration
DOXY_DIR="$ROOT_DIR/doxygen"

# Path to the doxygen logs
DOXY_LOG_DIR="$DOXY_DIR/logs"

# Create logs directory
mkdir -p "$DOXY_LOG_DIR"

# pull doxygen
git clone git@git-ore.aus.cypress.com:devops/tools/doxygen.git -b v187 doxygen_exe

# add paths for both Linux and Windows runners
export PATH="$ROOT_DIR/doxygen_exe/Linux/bin:$ROOT_DIR/doxygen_exe/Win64/bin:$PATH"

# Function to generate HTML documentation with doxygen
# arg#1: name of the doxygen config in $DOXY_DIR
# arg#2: directory name == top-level of code to read
# arg#2: directory name == top-level html name
function generate_doxygen()
{
    local doxy_config=$1
    local doxy_input=$2
    local doxy_output=$3
    local doxy_log="$DOXY_LOG_DIR/${doxy_config%.*}.log"
    echo >&2 "[INFO] doxygen $doxy_config"

    # Switch to build/doxygen directory
    pushd "$DOXY_DIR" >/dev/null
    trap "popd >/dev/null" RETURN

    # Export the environment variables to doxygen config
    export INPUT_DIRECTORY="$doxy_input"
    export OUTPUT_DIRECTORY="$doxy_output"

    # Create output directory
    mkdir -p "$OUTPUT_DIRECTORY"

    # Generate doxygen, check for warnings/errors
    if ! doxygen "$doxy_config" > "$doxy_log"; then
        echo "[ERROR] doxygen generation failed for $doxy_config"
        echo "See $doxy_log for a complete log"
        return -1
    fi
    find . -iname '*.png' -print

    # Remove dir_* index files
    rm -f "$OUTPUT_DIRECTORY"/html/dir_*.html

    # Remove doxygen object databases
    rm -f "$OUTPUT_DIRECTORY"/doxygen_{entrydb,objdb}*.tmp

    # Copy redirect HTML file to output parent directory
    local doxy_html="${doxy_output##*/}.html"
    if [[ -f "$doxy_html" ]]; then
        cp -v "$doxy_html" "$(dirname "$doxy_output")"
    fi
}

# Generate Doxygen
generate_doxygen doxyfile_specific.conf .. ../docs/api_reference_manual
