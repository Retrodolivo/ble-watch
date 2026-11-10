#!/usr/bin/env bash
# Build the firmware inside Nordic's NCS toolchain container.
#
# Used by .github/workflows/build.yml on the self-hosted runner (VPS), and can
# also be run by hand from the repo root: ci/build.sh
#
# The container image has only the toolchain (west, CMake, Zephyr SDK). The NCS
# sources are fetched once with west into $NCS_CACHE on the host and reused by
# every later build.
set -euo pipefail

NCS_VERSION="${NCS_VERSION:-v3.4.2}"
IMAGE="ghcr.io/nrfconnect/sdk-nrf-toolchain:${NCS_VERSION}"
NCS_CACHE="${NCS_CACHE:-$HOME/ncs-cache/${NCS_VERSION}}"
BOARD="nrf52840dk/nrf52840"

mkdir -p "$NCS_CACHE"

# --user: files in the mounted folders belong to the runner user, not root.
# The image's entrypoint is "bash -c", so the last argument is the script.
docker run --rm \
	--user "$(id -u):$(id -g)" \
	-e HOME=/tmp \
	-e NCS_VERSION="$NCS_VERSION" \
	-e BOARD="$BOARD" \
	-v "$NCS_CACHE:/ncs" \
	-v "$(pwd):/src" \
	-w /ncs \
	"$IMAGE" '
set -euo pipefail
# First run only: fetch the NCS sources (shallow, to save disk space).
if [ ! -f ".ci-ready-$NCS_VERSION" ]; then
	[ -d .west ] || west init -m https://github.com/nrfconnect/sdk-nrf --mr "$NCS_VERSION" .
	west update --narrow -o=--depth=1
	touch ".ci-ready-$NCS_VERSION"
fi
west build -p -b "$BOARD" -d /src/build /src/app
'
