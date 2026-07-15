# syntax=docker/dockerfile:1.7

# ---- build stage --------------------------------------------------
FROM ubuntu:24.04 AS build

RUN apt-get update && apt-get install -y --no-install-recommends \
      build-essential \
      cmake \
      ninja-build \
      git \
      curl \
      zip \
      unzip \
      tar \
      pkg-config \
      ca-certificates \
    && rm -rf /var/lib/apt/lists/*

ENV VCPKG_ROOT=/opt/vcpkg
RUN git clone --depth 1 https://github.com/microsoft/vcpkg.git $VCPKG_ROOT \
 && $VCPKG_ROOT/bootstrap-vcpkg.sh -disableMetrics

# Copy the manifest first and pre-install so the ~30-min gRPC build lands
# in its own layer, not invalidated by source edits. Later `cmake --preset
# vcpkg` re-runs vcpkg install into build/vcpkg_installed/, but everything
# comes from the binary cache in ~/.cache/vcpkg/archives/.
WORKDIR /deps
COPY server/vcpkg.json ./
RUN $VCPKG_ROOT/vcpkg install --x-manifest-root=/deps

WORKDIR /src
COPY proto/  ./proto/
COPY server/ ./server/

WORKDIR /src/server
RUN cmake --preset vcpkg \
 && cmake --build build -j

# ---- runtime stage ------------------------------------------------
FROM ubuntu:24.04 AS runtime

RUN apt-get update && apt-get install -y --no-install-recommends \
      ca-certificates \
    && rm -rf /var/lib/apt/lists/*

COPY --from=build /src/server/build/src/app/robot_server /usr/local/bin/robot_server

RUN useradd --system --uid 10001 robot
USER robot

EXPOSE 50051
CMD ["/usr/local/bin/robot_server"]
