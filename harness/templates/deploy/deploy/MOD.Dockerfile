# Imagen multi-stage del núcleo (STD-DEP DEP-01/02)
FROM debian:12.7-slim AS build
RUN apt-get update && apt-get install -y --no-install-recommends gcc cmake make libmosquitto-dev \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /src
COPY . .
RUN cmake --preset linux-container && cmake --build build/linux-container -j

FROM debian:12.7-slim
RUN apt-get update && apt-get install -y --no-install-recommends libmosquitto1 \
    && rm -rf /var/lib/apt/lists/* && useradd -u 10001 -r saf
COPY --from=build /src/build/linux-container/@MOD@_app /usr/local/bin/@MOD@
USER saf
ENTRYPOINT ["/usr/local/bin/@MOD@", "--config", "/etc/saf/@MOD@.json"]
