FROM debian:13

RUN apt update && apt -y install cmake ccache gcc g++ ninja-build pkg-config python3 nasm libssl-dev libglvnd-dev libwayland-dev libinput-dev libpulse-dev libxrandr-dev libxkbcommon-dev

WORKDIR /tmp
COPY CMakeLists.txt .
RUN --mount=type=cache,target=/root/.cache cmake . -GNinja && cmake --build . --parallel --config Release && rm -rf /tmp/*

WORKDIR /src
