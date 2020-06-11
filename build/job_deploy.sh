#!/bin/bash

[[ $(ls -ld $0 | egrep -v '^-rwxr-xr-x|^-rwxrwx---|^-rwxrwxr-x' | wc -l) -ne 0 ]] && \
    { echo -e "\nFATAL: incorrect file attributes: $0\n"; exit 1; }

bld_num=99
[[ ! -z "${CI_PIPELINE_IID}" ]] && bld_num=${CI_PIPELINE_IID}
branch=master
deploy_asset_base_dir=/gpfs/iot/projects/iot_release/ASSETS
[[ -f /home/$USER/.cy_gitlab_configs/deploy_asset_base_dir.txt ]] && \
    deploy_asset_base_dir=$(cat /home/$USER/.cy_gitlab_configs/deploy_asset_base_dir.txt)

mkdir -p deploy
set -x
find deploy ! -type d | sort
{ set +x; } 2>/dev/null

devops_scripts/job_deploy_to_assets.sh

###
VERSION=`cat version.txt`
VERSION_MAJOR=`cat version.txt | awk '{ split($1, version, "."); print version[1]; }'`

export ASSET="repo/anycloud-ota/${branch}"
export ASSET_BUILD="${bld_num}"
export ASSET_ZIP_FILE="anycloud-ota.zip"
export STAGING_BRANCH="develop"
export STAGING_REPO="repo-staging/anycloud-ota.git"
export ASSET_VERSION="${VERSION}.${bld_num}"
export STAGING_REPO_TAG="${ASSET_VERSION} release-v${VERSION} latest-v${VERSION_MAJOR}.X"

rm -rf    staging_repos
mkdir -p  staging_repos
pushd     staging_repos >/dev/null
../devops_scripts/job_deploy_to_staging_repo.sh
popd >/dev/null

###
