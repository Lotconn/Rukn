FROM rockylinux/rockylinux:10-ubi AS builder

RUN dnf install -y https://dl.fedoraproject.org/pub/epel/epel-release-latest-10.noarch.rpm
RUN dnf dsync -y
RUN dnf config-manager --set-enabled crb
RUN dnf -y install bash-color-prompt bash-completion ccache cmake extra-cmake-modules gcc gcc-c++ git libglvnd-devel libinput-devel libXrandr-devel libxkbcommon-devel nasm ninja-build openssl-devel pkgconf pulseaudio-libs-devel python3 rootfiles wayland-devel zstd
RUN dnf reinstall -y rootfiles
RUN rpm --restore rootfiles
RUN dnf clean all
RUN rm -rf /var/cache/dnf/*

RUN git config --system --add safe.directory /src

WORKDIR /tmp
COPY CMakeLists.txt .
RUN --mount=type=cache,target=/root/.ccache \
    --mount=type=cache,target=/root/.cache,sharing=locked \
    cmake . -GNinja -DBUILD_STATIC=ON && \
    cmake --build . --parallel --config Release && \
    rm -rf /tmp/*

RUN echo "cmake -S . -B build -G'Ninja Multi-Config' -DCMAKE_BUILD_TYPE=Release && cmake --build build --config Release --parallel" >> /root/.bash_history
RUN echo "cd pkg/makeself && ./generate_installer" >> /root/.bash_history

WORKDIR /src
