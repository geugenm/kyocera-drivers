# Non-printing container test: build + install the package, then verify the
# full filter chain (PPD raster -> rastertokpsl -> KPSL) with a generated
# sample page. Nothing is sent to a printer.
#
#   docker build -t kyocera-drivers-test .
FROM debian:trixie-slim

# libcupsimage2t64: rastertokpsl_* link libcupsimage.so.2, which the cups
# stack does not pull in on trixie.
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        cmake ninja-build cups cups-filters ghostscript libcupsimage2t64 \
    && rm -rf /var/lib/apt/lists/*

COPY . /src
RUN cmake -S /src -B /build -G Ninja \
    && cmake --install /build --prefix=/usr \
    && bash /src/scripts/verify.sh

CMD ["bash", "/src/scripts/verify.sh"]
