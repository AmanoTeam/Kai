#!/usr/bin/env bash

set -eu

declare -r APP_DIRECTORY="$(realpath "$(( [ -n "${BASH_SOURCE}" ] && dirname "$(realpath "${BASH_SOURCE[0]}")" ) || dirname "$(realpath "${0}")")")/.."

git -C "${APP_DIRECTORY}" submodule update --init --remote
git -C "${APP_DIRECTORY}/submodules/curl" checkout b761eb5addb9e29b2ee0e5841633c09d1fd77704
