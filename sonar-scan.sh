#!/bin/bash

# The script is used for determining the options and running a static code
# analysis scan via SonarCloud.
#
# Author: Alex Tereschenko <alext.mkrs@gmail.com>

sonar_cmd_base=" \
    sonar-scanner \
        --debug \
        -Dsonar.projectKey=${SONAR_PROJ_KEY} \
        -Dsonar.projectBaseDir=${MRAA_SRC_DIR} \
        -Dsonar.sources=${MRAA_SRC_DIR} \
        -Dsonar.inclusions='api/**/*,CMakeLists.txt,examples/**/*,imraa/**/*,include/**/*,src/**/*,tests/**/*' \
        -Dsonar.coverage.exclusions='**/*' \
        -Dsonar.cfamily.build-wrapper-output=${MRAA_SRC_DIR}/build/bw-output \
        -Dsonar.host.url=https://sonarqube.com \
        -Dsonar.organization=${SONAR_ORG} \
        -Dsonar.login=${SONAR_TOKEN} \
"
echo $sonar_cmd_base

if [ "${TRAVIS_BRANCH}" == "master" -a "${TRAVIS_PULL_REQUEST}" == "false" ]; then
    # master branch push - do a full-blown scan
    sonar_cmd="${sonar_cmd_base}"
elif [ "${TRAVIS_PULL_REQUEST}" != "false" -a "${TRAVIS_PULL_REQUEST_SLUG}" == "${TRAVIS_REPO_SLUG}" ]; then
    # internal PR - do a preview scan
    sonar_cmd="${sonar_cmd_base} \
               -Dsonar.analysis.mode=preview \
               -Dsonar.github.pullRequest=${TRAVIS_PULL_REQUEST} \
               -Dsonar.github.repository=${TRAVIS_REPO_SLUG} \
               -Dsonar.github.oauth=${GITHUB_TOKEN} \
               "
else
    echo "External pull request or non-master branch push - skipping the scan"
    exit 0
fi

echo "****GOING TO RUN THE SCANNER****"
echo "${sonar_cmd}"

eval "${sonar_cmd}"

#sonar-scanner \
#    --debug \
#    -Dsonar.projectKey=$${SONAR_PROJ_KEY} \
#    -Dsonar.projectBaseDir=$${MRAA_SRC_DIR} \
#    -Dsonar.sources=$${MRAA_SRC_DIR} \
#    -Dsonar.inclusions='api/**/*,CMakeLists.txt,examples/**/*,imraa/**/*,include/**/*,src/**/*,tests/**/*' \
#    -Dsonar.coverage.exclusions='**/*' \
#    -Dsonar.cfamily.build-wrapper-output=$${MRAA_SRC_DIR}/build/bw-output \
#    -Dsonar.host.url=https://sonarqube.com \
#    -Dsonar.organization=$${SONAR_ORG} \
#    -Dsonar.login=$${SONAR_TOKEN} \
#    -Dsonar.analysis.mode=preview \
#    -Dsonar.github.pullRequest=$${TRAVIS_PULL_REQUEST} \
#    -Dsonar.github.repository=$${TRAVIS_REPO_SLUG} \
#    -Dsonar.github.oauth=$${GITHUB_TOKEN}
