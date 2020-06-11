#!/bin/bash
#
[[ $(ls -ld $0 | egrep -v '^-rwxr-xr-x|^-rwxrwx---|^-rwxrwxr-x' | wc -l) -ne 0 ]] && \
    { echo -e "\nFATAL: incorrect file attributes: $0\n"; exit 1; }

restore_xtrace="set +x"
test -o xtrace && restore_xtrace="set -x"

bld_num=99
[[ ! -z "${CI_PIPELINE_IID}" ]] && bld_num=${CI_PIPELINE_IID}

##

pushd . >/dev/null

# process the list file
out_dir=out
rm -rf    ${out_dir}/api_reference_manual
rc=0
for x in ./build/publish.list; do
  echo -e "\n... process list file: ${x}"
  while read z; do
    [[ ${z} = "#"* ]]  && continue
    [[ ${z} = "" ]]    && continue
    fields=($z)
    [[ ${fields[1]} = "" ]] && fields[1]=${fields[0]}
    dst=${fields[1]}
    [[ ${dst} != ${dst%/*} ]] && mkdir -p ${out_dir}/${dst%/*}
    if [[ ! -f ${fields[0]} ]]; then
      echo "FATAL: cannot find file: ${fields[0]}"
      rc=2
    else
      cp -pv ${fields[0]} ${out_dir}/${dst}
    fi
  done < ${x}
  if [[ ! -z ${z} ]]; then
    echo -e "\nFATAL: file '${x}' has 'No newline at end of file'!"
    rc=3
  fi
done
if [[ ${rc} -ne 0 ]]; then
  echo -e "\nFATAL: processing list files failed!\n"
  exit ${rc}
fi

# apply copyright
if [[ ! -d ./devops_scripts ]]; then
  git clone git@git-ore.aus.cypress.com:devops/devops_scripts.git
fi
echo -e "\n+ make -f ./devops_scripts/apply_copyright.mk DIR=${out_dir}"
             make -f ./devops_scripts/apply_copyright.mk DIR=${out_dir}

# add License / EULA
echo -e "\n+ cp -pv ./devops_scripts/LICENSE.txt out/LICENSE"
             cp -pv ./devops_scripts/LICENSE.txt out/LICENSE

# add version.txt
VERSION_MAJOR=`cat version.txt | awk '{ split($1, version, "."); print version[1]; }'`
VERSION_MINOR=`cat version.txt | awk '{ split($1, version, "."); print version[2]; }'`
VERSION_BUILD=`cat version.txt | awk '{ split($1, version, "."); print version[3]; }'`

echo -e "\n+ echo \"${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_BUILD}.${bld_num}\" > out/version.txt"
             echo  "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_BUILD}.${bld_num}"  > out/version.txt

# move doxygen created docs into the out folder
if [[ -d ./docs ]]; then
  cp -fr docs out
fi

# create deploy content
echo -e "\n... prepare content"
rm -rf    deploy
mkdir -p  deploy
cd out
rm -rf  ../deploy/anycloud-ota.zip
echo -e "\n+ zip -r  ../deploy/anycloud-ota.zip ."
             zip -r  ../deploy/anycloud-ota.zip .
unzip -lqq ../deploy/anycloud-ota.zip | sed -e "s,.* ,," -e "/\/$/d" | sort > ../deploy/anycloud-ota.txt
cd - >/dev/null

##

popd  >/dev/null

##
