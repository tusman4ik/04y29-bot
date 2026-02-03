FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake git \
    libssl-dev libcurl4-openssl-dev zlib1g-dev \
    libsqlite3-dev \
    nlohmann-json3-dev \
    libboost-system-dev libboost-dev \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

RUN git clone --depth 1 --branch 3.3.1 https://github.com/SRombauts/SQLiteCpp.git \
    && cd SQLiteCpp \
    && cmake -B build -DCMAKE_BUILD_TYPE=Release -DSQLITECPP_RUN_CPPLINT=OFF \
    && cmake --build build -j$(nproc) \
    && cmake --install build \
    && cd .. && rm -rf SQLiteCpp

RUN git clone --depth 1 --branch v1.9.1 https://github.com/reo7sp/tgbot-cpp.git \
    && cd tgbot-cpp \
    && cmake -B build -DCMAKE_INSTALL_PREFIX=/usr/local \
    && cmake --build build -j$(nproc) \
    && cmake --install build \
    && cd .. && rm -rf tgbot-cpp

RUN git clone --depth 1 --branch 10.2.1 https://github.com/fmtlib/fmt.git \
    && cd fmt \
    && cmake -B build -DCMAKE_INSTALL_PREFIX=/usr/local -DFMT_TEST=OFF \
    && cmake --build build -j$(nproc) \
    && cmake --install build \
    && cd .. && rm -rf fmt

RUN git clone --depth 1 --branch v1.17.0 https://github.com/gabime/spdlog.git \
    && cd spdlog \
    && cmake -B build -DCMAKE_INSTALL_PREFIX=/usr/local -DSPDLOG_FMT_EXTERNAL=ON \
    && cmake --build build -j$(nproc) \
    && cmake --install build \
    && cd .. && rm -rf spdlog

    COPY . .

RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=23 && \
    cmake --build build -j$(nproc) && \
    strip build/src/bot

#########################################################################
FROM ubuntu:24.04

WORKDIR /app

RUN apt-get update && apt-get install -y --no-install-recommends \
    libssl3 libcurl4 zlib1g libsqlite3-0 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /app/build/src/bot ./bot
RUN mkdir -p data

CMD ["./bot"]