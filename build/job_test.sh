#!/bin/bash

[[ $(ls -ld $0 | egrep -v '^-rwxr-xr-x|^-rwxrwx---|^-rwxrwxr-x' | wc -l) -ne 0 ]] && \
    { echo -e "\nFATAL: incorrect file attributes: $0\n"; exit 1; }

restore_xtrace="set +x"
test -o xtrace && restore_xtrace="set -x"

function get_MTB()
{
    ## get specific ModusToolbox tools
    echo ""
    rm -rf  MTB
    mkdir -p MTB/tools_2.1
    pushd    MTB/tools_2.1 >/dev/null

    unset CY_TOOLS_PATHS
    asset_host=iot-webserver.aus.cypress.com
    asset_base=/projects/iot_release/ASSETS
    asset_url_base="http://${asset_host}${asset_base}/repo/mtb_installers/develop"
    asset_bld_num=$(curl --fail -L -s ${asset_url_base}/stable_ANYCONNECT-OTA/_bld_info.txt | sed -e "s,.* ,,") || :
    [[ -z ${asset_bld_num} ]] && \
        asset_bld_num=$(curl --fail -L -s ${asset_url_base}/Latest/_bld_info.txt | sed -e "s,.* ,,")
    if [[ ! -z "${asset_bld_num}" ]]; then
        if [[ $(bash --version | grep cygwin | wc -l) -eq 1 ]]; then
            set -x
            wget -q ${asset_url_base}/${asset_bld_num}/deploy/assets_uris_windows.txt
            wget -q $(grep "tools-make-.*\.zip"             assets_uris_windows.txt  2>/dev/null)
            wget -q $(grep "modus-shell-.*\.zip"            assets_uris_windows.txt  2>/dev/null)
            wget -q $(grep "gcc-.*\.zip"                    assets_uris_windows.txt  2>/dev/null)
            wget -q $(grep "cfg-backend-cli-.*\.zip"        assets_uris_windows.txt  2>/dev/null)
            wget -q $(grep "jre-.*\.zip"                    assets_uris_windows.txt  2>/dev/null)
            wget -q $(grep "capsense-configurator-.*\.zip"  assets_uris_windows.txt  2>/dev/null)
            wget -q $(grep "seglcd-configurator-.*\.zip"    assets_uris_windows.txt  2>/dev/null)
            export CY_TOOLS_PATHS="$(cygpath -ma .)"
            { ${restore_xtrace}; } 2>/dev/null
        elif [[ $(bash --version | grep darwin | wc -l) -eq 1 ]]; then
            set -x
            wget -q ${asset_url_base}/${asset_bld_num}/deploy/assets_uris_macos.txt
            wget -q $(grep "tools-make-.*\.zip"             assets_uris_macos.txt  2>/dev/null)
            wget -q $(grep "modus-shell-.*\.tar.gz"         assets_uris_macos.txt  2>/dev/null)
            wget -q $(grep "gcc-.*\.zip"                    assets_uris_macos.txt  2>/dev/null)
            wget -q $(grep "cfg-backend-cli-.*\.zip"        assets_uris_macos.txt  2>/dev/null)
            wget -q $(grep "jre-.*\.zip"                    assets_uris_macos.txt  2>/dev/null)
            wget -q $(grep "capsense-configurator-.*\.zip"  assets_uris_macos.txt  2>/dev/null)
            wget -q $(grep "seglcd-configurator-.*\.zip"    assets_uris_macos.txt  2>/dev/null)
            export CY_TOOLS_PATHS="$(pwd)"
            { ${restore_xtrace}; } 2>/dev/null
        else
            set -x
            wget -q ${asset_url_base}/${asset_bld_num}/deploy/assets_uris_linux.txt
            wget -q $(grep "tools-make-.*\.zip"             assets_uris_linux.txt  2>/dev/null)
            wget -q $(grep "modus-shell-.*\.tar.gz"         assets_uris_linux.txt  2>/dev/null)
            wget -q $(grep "gcc-.*\.zip"                    assets_uris_linux.txt  2>/dev/null)
            wget -q $(grep "cfg-backend-cli-.*\.zip"        assets_uris_linux.txt  2>/dev/null)
            wget -q $(grep "jre-.*\.zip"                    assets_uris_linux.txt  2>/dev/null)
            wget -q $(grep "capsense-configurator-.*\.zip"  assets_uris_linux.txt  2>/dev/null)
            wget -q $(grep "seglcd-configurator-.*\.zip"    assets_uris_linux.txt  2>/dev/null)
            export CY_TOOLS_PATHS="$(pwd)"
            { ${restore_xtrace}; } 2>/dev/null
        fi
    fi
    echo ""
    for x in $(ls -d *.zip 2>/dev/null); do
        set -x
        unzip -q ${x}
        { ${restore_xtrace}; } 2>/dev/null
    done
    for x in $(ls -d *.tar.gz 2>/dev/null); do
        set -x
        tar -xf ${x}
        { ${restore_xtrace}; } 2>/dev/null
    done
    echo ""

    popd >/dev/null
}

function perform_build()
{
    build_suite=$1
    shift

    ## get ModusToolbox
    if [[ -z ${CY_TOOLS_PATHS:-} ]]; then
        get_MTB
    fi
    set -x
    export CY_GETLIBS_NO_CACHE=1
    { ${restore_xtrace}; } 2>/dev/null

    ## set python virtualenv
    echo ""
    rm -rf env
    echo "+ python3 -m venv env"
            python3 -m venv env
    echo "+ source env/bin/activate"
            source env/bin/activate

    set -x
    which python
    python --version

    which pip
    pip --version
    { ${restore_xtrace}; } 2>/dev/null

    echo "+ pip --no-cache-dir install --upgrade pip"
            pip --no-cache-dir install --upgrade pip
    echo "+ pip --no-cache-dir install click"
            pip --no-cache-dir install click
    echo "+ pip --no-cache-dir install cryptography"
            pip --no-cache-dir install cryptography
    echo "+ pip --no-cache-dir install intelhex"
            pip --no-cache-dir install intelhex
    echo "+ pip --no-cache-dir install cbor"
            pip --no-cache-dir install cbor

    echo "+ pip list"
            pip list

    ## perform the build suite
    echo -e "\n... starting: ${build_suite} $@\n"
    ${build_suite} "$@"
    echo -e "\n... finished: ${build_suite}\n"

    ## cleanup python virtualenv
    { ${restore_xtrace}; } 2>/dev/null
    echo "+ deactivate"
            deactivate

}

function test_gcc_arm_builds()
{
    #mkdir TEST_GCC_ARM
    #pushd TEST_GCC_ARM >/dev/null
    #unzip ../deploy/anycloud-ota.zip

    ## skip the python virtualenv path
    echo "env" >> .cyignore

    ## skip the MTB tools path
    echo "MTB" >> .cyignore

    ## perform the build
    echo "+ make getlibs"
            make getlibs
    echo "+ make -B -j8 build"
            make -B -j8 build
    echo "+ make -B -j8 build TARGET=CY8CKIT-062S2-43012"
            make -B -j8 build TARGET=CY8CKIT-062S2-43012
    echo "+ make -B -j8 build CONFIG=Release"
            make -B -j8 build CONFIG=Release
    echo "+ make -B -j8 build TARGET=CY8CKIT-062S2-43012 CONFIG=Release"
            make -B -j8 build TARGET=CY8CKIT-062S2-43012 CONFIG=Release

    #popd >/dev/null
}

function test_iar_builds()
{
    #mkdir TEST_IAR
    #pushd TEST_IAR >/dev/null
    #unzip ../deploy/anycloud-ota.zip

    ## skip the python virtualenv path
    echo "env" >> .cyignore

    ## skip the MTB tools path
    echo "MTB" >> .cyignore

    ## no spaces in CY_COMPILER_PATH...
    host_iar_bin_path=$(cygpath "${HOST_IAR_PATH_8321:-}")
    if [[ ! -d /cygdrive/c/HOST_IAR_PATH_8321 ]]; then
        echo -e "\n+ ln -fs "${host_iar_bin_path}" /cygdrive/c/HOST_IAR_PATH_8321"
                     ln -fs "${host_iar_bin_path}" /cygdrive/c/HOST_IAR_PATH_8321
    fi

    ## perform the build
    echo "+ make getlibs"
            make getlibs
    echo "+ make -B -j8 build TOOLCHAIN=IAR CY_COMPILER_PATH=/cygdrive/c/HOST_IAR_PATH_8321 VERBOSE=1"
            make -B -j8 build TOOLCHAIN=IAR CY_COMPILER_PATH=/cygdrive/c/HOST_IAR_PATH_8321 VERBOSE=1
    echo "+ make -B -j8 build TARGET=CY8CKIT-062S2-43012 TOOLCHAIN=IAR CY_COMPILER_PATH=/cygdrive/c/HOST_IAR_PATH_8321 VERBOSE=1"
            make -B -j8 build TARGET=CY8CKIT-062S2-43012 TOOLCHAIN=IAR CY_COMPILER_PATH=/cygdrive/c/HOST_IAR_PATH_8321 VERBOSE=1
    echo "+ make -B -j8 build CONFIG=RELEASE TOOLCHAIN=IAR CY_COMPILER_PATH=/cygdrive/c/HOST_IAR_PATH_8321 VERBOSE=1"
            make -B -j8 build CONFIG=RELEASE TOOLCHAIN=IAR CY_COMPILER_PATH=/cygdrive/c/HOST_IAR_PATH_8321 VERBOSE=1
    echo "+ make -B -j8 build TARGET=CY8CKIT-062S2-43012 CONFIG=RELEASE TOOLCHAIN=IAR CY_COMPILER_PATH=/cygdrive/c/HOST_IAR_PATH_8321 VERBOSE=1"
            make -B -j8 build TARGET=CY8CKIT-062S2-43012 CONFIG=RELEASE TOOLCHAIN=IAR CY_COMPILER_PATH=/cygdrive/c/HOST_IAR_PATH_8321 VERBOSE=1

    #popd >/dev/null
}

function coverity_builds()
{
    f_no_push=0
    [[ $# -gt 0 && $1 = "--no-push" ]] && { f_no_push=1; shift; }
    f_single=0
    [[ $# -gt 0 && $1 = "--single" ]] && { f_single=1; shift; }

    # Using MISRA config
    COVERITY_CODING_STANDARD="build/misra_2012.config"
    echo -e "\n+ cat -n $COVERITY_CODING_STANDARD"
                 cat -n $COVERITY_CODING_STANDARD
    echo ""

    coverity_strip_path=$(readlink -e ${0})
    coverity_strip_path=${coverity_strip_path%/build/job_test.sh}

    coverity_build_args=( \
        "--return-emit-failures" \
        "--emit-complementary-info" \
        )

    PARSE_WARNINGS_FILE="echo 'chk \"PW.INCLUDE_RECURSION\": off;' > _coverity_/parse_warnings.conf"
    coverity_analyze_args=( \
        "--all" \
        "--checker-option DEADCODE:no_dead_default:true" \
        "--parse-warnings-config _coverity_/parse_warnings.conf" \
        "--coding-standard-config ${COVERITY_CODING_STANDARD}" \
        )

    coverity_tu_cmd="--tu-pattern "'!'"file('/anycloud-ota/libs/')"

    coverity_format_errors_args=( \
        "--emacs-style"
        )

    # install coverity
    [[ ! -d devops_scripts/.git ]] && git clone git@git-ore.aus.cypress.com:devops/devops_scripts.git
    chmod 0600 devops_scripts/coverity_preco_user_prod_auth_key.txt
    coverity_bin_path="./Coverity/Win64/bin"
    set -x
    time devops_scripts/install_tool.sh  \
            git@git-ore.aus.cypress.com:devops/tools/Coverity.git coverity Win64 bin,config,dtd,.install4j,jars

    cp -pf devops_scripts/coverity_license.dat ${coverity_bin_path}/license.dat
    ${restore_xtrace}

    rm -rf _coverity_
    echo -e "+ ${coverity_bin_path}/cov-configure --config _coverity_/coverity_config.xml --template --comptype gcc --compiler arm-none-eabi-gcc"
               ${coverity_bin_path}/cov-configure --config _coverity_/coverity_config.xml --template --comptype gcc --compiler arm-none-eabi-gcc
    echo -e "+ ${coverity_bin_path}/cov-configure --config _coverity_/coverity_config.xml --list-configured-compilers text"
               ${coverity_bin_path}/cov-configure --config _coverity_/coverity_config.xml --list-configured-compilers text
    echo ""

    ## skip the python virtualenv path
    echo "env" >> .cyignore

    ## skip the MTB tools path
    echo "MTB" >> .cyignore

    ## skip the Coverity tools path
    echo "Coverity" >> .cyignore

    ## perform the builds
    echo "+ make getlibs"
            make getlibs

    echo -e "\n## [cov-build] TARGET=CY8CPROTO-062-4343W\n"
    cov_stream1="CY8CPROTO-062-4343W"

    set -x
    ${coverity_bin_path}/cov-build \
        --config  _coverity_/coverity_config.xml \
        --dir     _coverity_/analyze_data_stream1 \
        ${coverity_build_args[*]} \
        make -B -j8 build TARGET=CY8CPROTO-062-4343W
    { ${restore_xtrace}; } 2>/dev/null
    echo ""

    set -x
    eval ${PARSE_WARNINGS_FILE}
    ${coverity_bin_path}/cov-analyze \
        --dir _coverity_/analyze_data_stream1 \
        --force \
        --strip-path "${coverity_strip_path}/" \
        ${coverity_analyze_args[*]} \
        ${coverity_tu_cmd}
    { ${restore_xtrace}; } 2>/dev/null

    echo -e "\n... report Translation Units:"
    set -x
    ${coverity_bin_path}/cov-manage-emit --dir _coverity_/analyze_data_stream1 \
        ${coverity_tu_cmd} \
        list
    { ${restore_xtrace}; } 2>/dev/null

    echo -e "\n**** dump results ****\n"
    ${coverity_bin_path}/cov-format-errors \
        --dir _coverity_/analyze_data_stream1 \
        ${coverity_format_errors_args[*]} || :
    echo -e "**********************\n"

    if [[ ${f_single} -eq 0 ]]; then
    #{{#
        echo -e "\n## [cov-build] TARGET=CY8CKIT-062S2-43012\n"
        cov_stream2="CY8CKIT-062S2-43012"

        set -x
        ${coverity_bin_path}/cov-build \
            --config  _coverity_/coverity_config.xml \
            --dir     _coverity_/analyze_data_stream2 \
            ${coverity_build_args[*]} \
            make -B -j8 build TARGET=CY8CKIT-062S2-43012
        { ${restore_xtrace}; } 2>/dev/null
        echo ""

        set -x
        eval ${PARSE_WARNINGS_FILE}
        ${coverity_bin_path}/cov-analyze \
            --dir _coverity_/analyze_data_stream2 \
            --force \
            --strip-path "${coverity_strip_path}/" \
            ${coverity_analyze_args[*]} \
            ${coverity_tu_cmd}
        { ${restore_xtrace}; } 2>/dev/null

        echo -e "\n... report Translation Units:"
        set -x
        ${coverity_bin_path}/cov-manage-emit --dir _coverity_/analyze_data_stream2 \
            ${coverity_tu_cmd} \
            list
        { ${restore_xtrace}; } 2>/dev/null

        echo -e "\n**** dump results ****\n"
        ${coverity_bin_path}/cov-format-errors \
            --dir _coverity_/analyze_data_stream2 \
            ${coverity_format_errors_args[*]} || :
        echo -e "**********************\n"
    #}}#
    fi

    echo -e "\n##\n"

    if [[ ${f_no_push} -eq 0 ]]; then
        cov_project="anycloud-ota"
        desc="--description CI_PIPELINE_IID=${CI_PIPELINE_IID:-}"
        bash -eu devops_scripts/coverity/create_project.sh  ${coverity_bin_path} ${cov_project}
        bash -eu devops_scripts/coverity/create_stream.sh   ${coverity_bin_path} ${cov_project} ${cov_stream1}
        bash -eu devops_scripts/coverity/commit_defects.sh  ${coverity_bin_path} ${cov_stream1} _coverity_/analyze_data_stream1 "${desc}"
        bash -eu devops_scripts/coverity/create_project.sh  ${coverity_bin_path} ${cov_project}
        bash -eu devops_scripts/coverity/create_stream.sh   ${coverity_bin_path} ${cov_project} ${cov_stream2}
        bash -eu devops_scripts/coverity/commit_defects.sh  ${coverity_bin_path} ${cov_stream2} _coverity_/analyze_data_stream2 "${desc}"
    fi
}

# Execute the functions based on the input flag
case "${1:-}" in
    "--test_gcc_arm_builds" )
        shift
        perform_build test_gcc_arm_builds
        ;;
    "--test_iar_builds" )
        shift
        perform_build test_iar_builds
        ;;
    "--coverity" )
        shift
        perform_build coverity_builds "$@"
        ;;
    * )
        echo "Unknown argument: $@"
        echo "Usage: $0 {--test_gcc_arm_builds | --test_iar_builds | --coverity}"
        exit 2
        ;;
esac
