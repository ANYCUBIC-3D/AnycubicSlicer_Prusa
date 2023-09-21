FROM ubuntu:20.04 as base
RUN sed -i "s@http://.*archive.ubuntu.com@http://mirrors.ustc.edu.cn/@g" /etc/apt/sources.list
RUN sed -i "s@http://.*security.ubuntu.com@http://mirrors.ustc.edu.cn/@g" /etc/apt/sources.list
RUN apt update && apt install -y tzdata && ln -fs /usr/share/zoneinfo/Asia/Shanghai /etc/localtime



FROM base as builder
RUN apt install -y  git cmake build-essential autoconf \
    libgtk-3-dev libdbus-1-dev zlib1g-dev libglu1-mesa-dev

WORKDIR /opt/src/
COPY cmake cmake
COPY deps deps
RUN cmake -S /opt/src/deps -B /opt/build/deps -DDESTDIR:PATH=/opt/install/deps -DDEP_DOWNLOAD_DIR:PATH=/opt/src/deps/download -DCMAKE_BUILD_TYPE:STRING=Release -DSLIC3R_GUI:BOOL=OFF -DDEP_WX_GTK3=ON&& cmake --build /opt/build/deps
COPY src src
COPY CMakeLists.txt CMakeLists.txt
COPY version.inc version.inc

RUN cmake -S /opt/src/ -B /opt/build/slicer -DCMAKE_PREFIX_PATH:PATH=/opt/install/deps/usr/local -DSLIC3R_ENABLE_FORMAT_STEP:BOOL=OFF -DCMAKE_BUILD_TYPE:STRING=Release -DSLIC3R_STATIC:BOOL=ON -DSLIC3R_GUI:BOOL=OFF -DENABLE_CGAL:BOOL=OFF -DCMAKE_INSTALL_PREFIX:PATH=/opt/install/slicer
RUN cmake --build /opt/build/slicer --target install -j10

FROM base 
COPY --from=builder /opt/install/ /opt/




