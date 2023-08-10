FROM ubuntu:23.04 AS base

RUN set -e

ENV DEBIAN_FRONTEND=noninteractive

# Install packages
RUN apt-get -y update
RUN apt-get install -y --no-install-recommends\
        python3\
	g++\
	gcc\
        make\
        time

FROM base AS base-git
RUN apt-get install -y --no-install-recommends\
        cmake\
        git

FROM base-git AS artifact-pre-1
# copy the artifact
COPY . /opt/rv23-experiments

# initialize vamos-buffers
FROM artifact-pre-1 AS artifact-pre-2
WORKDIR /opt/rv23-experiments/
RUN git clean -xdf
RUN git submodule update --init -- vamos-buffers
WORKDIR /opt/rv23-experiments/vamos-buffers
RUN git clean -xdf
RUN cmake . -DCMAKE_BUILD_TYPE=Release
RUN make -j2

FROM artifact-pre-2 AS artifact-pre-3
WORKDIR /opt/rv23-experiments/
RUN cmake . -DCMAKE_BUILD_TYPE=Release
RUN make -j2

FROM artifact-pre-3 AS artifact-plotting
RUN apt-get install -y --no-install-recommends\
	python3-matplotlib\
	python3-pandas\
	python3-seaborn

FROM artifact-plotting as artifact
#COPY --from=vamos /opt/vamos /opt/vamos
WORKDIR /opt/rv23-experiments

