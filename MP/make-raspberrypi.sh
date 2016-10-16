#!/bin/bash

	USE_CODEC_VORBIS=0 \
	USE_CODEC_OPUS=0 \
	USE_CURL=1 \
	USE_CURL_DLOPEN=1 \
	USE_OPENAL=1 \
	USE_OPENAL_DLOPEN=1 \
	USE_RENDERER_DLOPEN=0 \
	USE_VOIP=0 \
	USE_LOCAL_HEADERS=1 \
	USE_INTERNAL_JPEG=1 \
	USE_INTERNAL_OPUS=1 \
	USE_INTERNAL_ZLIB=1 \
	USE_OPENGLES=1 \
	USE_BLOOM=0 \
	RASPBERRY_PI=1 \
	USE_MUMBLE=0 \
	BUILD_GAME_SO=1 \
	BUILD_RENDERER_REND2=0 \
	ARCH=arm \
	PLATFORM=linux \
	COMPILE_ARCH=arm \
	COMPILE_PLATFORM=linux \
	make $*

