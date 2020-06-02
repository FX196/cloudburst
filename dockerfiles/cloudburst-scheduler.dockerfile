#  Copyright 2020 U.C. Berkeley RISE Lab
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.

FROM hydroproject/base:latest

ARG repo_org=FX196
ARG source_branch=master
ARG build_branch=docker-build

USER root

# Check out to the appropriate branch on the appropriate fork of the repository
# and build Cloudburst Scheduler.

WORKDIR $HYDRO_HOME/cloudburst
RUN git remote remove origin && git remote add origin https://github.com/$repo_org/cloudburst
RUN git fetch origin && git checkout -b $build_branch origin/$source_branch
RUN git pull --recurse-submodules origin master
RUN bash scripts/build.sh
WORKDIR /


COPY start-cloudburst.sh /start-cloudburst.sh
CMD bash start-cloudburst.sh
