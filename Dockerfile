FROM debian:bookworm AS builder

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake git \
    libssl-dev libcurl4-openssl-dev zlib1g-dev \
    libsqlite3-dev \
    nlohmann-json3-dev \
    libboost-system-dev libboost-dev \   
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /app

RUN git clone https://github.com/SRombauts/SQLiteCpp.git \
    && cd SQLiteCpp \
    && cmake -B build -DCMAKE_BUILD_TYPE=Release -DSQLITECPP_RUN_CPPLINT=OFF \
    && cmake --build build -j$(nproc) \
    && cmake --install build \
    && cd .. && rm -rf SQLiteCpp

RUN git clone https://github.com/reo7sp/tgbot-cpp.git \
    && cd tgbot-cpp \
    && cmake -B build -DCMAKE_INSTALL_PREFIX=/usr/local \
    && cmake --build build -j$(nproc) \
    && cmake --install build \
    && cd .. && rm -rf tgbot-cpp

RUN git clone https://github.com/fmtlib/fmt.git \
    && cd fmt \
    && cmake -B build -DCMAKE_INSTALL_PREFIX=/usr/local \
    && cmake --build build \
    && cmake --install build \
    && cd .. && rm -rf fmt
    
RUN git clone https://github.com/gabime/spdlog.git \ 
    && cd spdlog \
    && cmake -B build -DCMAKE_INSTALL_PREFIX=/usr/local \
    && cmake --build build \
    && cmake --install build \
    && cd .. && rm -rf spdlog

COPY . .

RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build -j$(nproc)

#########################################################################

FROM debian:bookworm-slim

WORKDIR /app

RUN apt-get update && apt-get install -y --no-install-recommends \
    libssl3 libcurl4 zlib1g libsqlite3-0 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*
    
COPY --from=builder /app/build/src/bot ./bot
RUN mkdir data

CMD ["./bot"]