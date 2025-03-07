cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

project(
	Kai
	VERSION 0.1
	DESCRIPTION "A command-line utility to download contents from M3U8 playlists"
	HOMEPAGE_URL "https://github.com/AmanoTeam/Kai"
	LANGUAGES C
)

option(KAI_ENABLE_LTO "Turn on compiler Link Time Optimizations" OFF)
option(KAI_ENABLE_UNICODE "Enable unicode support for Windows builds" ON)
option(KAI_ENABLE_HTTP2 "Enable HTTP/2 support in cURL" ON)

set(CMAKE_POLICY_DEFAULT_CMP0048 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(CMAKE_PLATFORM_NO_VERSIONED_SONAME ON)

set(
	WARNING_OPTIONS
	"-Wall"
	"-Wextra"
	# "-Wpedantic"
)

set(BEARSSL_SOURCE_DIRECTORY "${CMAKE_SOURCE_DIR}/submodules/bearssl")
set(FFMPEG_SOURCE_DIRECTORY "${CMAKE_SOURCE_DIR}/submodules/ffmpeg")
set(NGHTTP2_SOURCE_DIRECTORY "${CMAKE_SOURCE_DIR}/submodules/nghttp2")

set(FFMPEG_BUILD_DIRECTORY "${CMAKE_BINARY_DIR}/ffmpeg-build")
set(FFMPEG_INSTALL_PREFIX "${FFMPEG_BUILD_DIRECTORY}/binaries")

set(
	FFMPEG_CONFIGURE_FLAGS
	--disable-all
	--disable-alsa
	--disable-appkit
	--disable-asm
	--disable-audiotoolbox
	--disable-avfoundation
	--disable-bzlib
	--disable-coreimage
	--disable-debug
	--disable-doc
	--disable-iconv
	--disable-libxcb
	--disable-lzma
	--disable-neon
	--disable-network
	--disable-pthreads
	--disable-schannel
	--disable-sdl2
	--disable-securetransport
	--disable-static
	--disable-symver
	--disable-videotoolbox
	--disable-vulkan
	--disable-w32threads
	--disable-xlib
	--disable-zlib
	--enable-avcodec
	--enable-avformat
	--enable-avutil
	--enable-swresample
	--enable-decoder=aac,h264,mp3
	--enable-demuxer=aac,h264,hls,webvtt,mp3
	--enable-muxer=h264,mpegts,mp4,matroska,mp3
	--enable-parser=h264,aac
	--enable-bsf=extract_extradata
	--enable-pic
	--enable-protocol=file,crypto
	--enable-shared
	--enable-small
	--enable-version3
	--extra-cflags=-Dstatic_assert=_Static_assert
	--extra-cflags=-w
	--extra-cflags=${CMAKE_C_FLAGS}
	--extra-ldflags=-fPIC
	--prefix="${FFMPEG_INSTALL_PREFIX}"
)

set(SHARED_LIBRARY_DIRECTORY)

set(
	NGHTTP2_INCLUDE_DIR
	"${NGHTTP2_SOURCE_DIRECTORY}/lib/includes"
	"${CMAKE_CURRENT_BINARY_DIR}/submodules/nghttp2/lib/includes"
)

set(
	NGHTTP2_LIBRARY
	"${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}nghttp2${CMAKE_SHARED_LIBRARY_SUFFIX}"
)

set(
	INCLUDE_DIRECTORIES
	"${CMAKE_SOURCE_DIR}/src"
	"${BEARSSL_SOURCE_DIRECTORY}/src"
	"${BEARSSL_SOURCE_DIRECTORY}/inc"
)

set(
	KAI_SOURCE_FILES
	"${CMAKE_SOURCE_DIR}/src/sutils.c"
	"${CMAKE_SOURCE_DIR}/src/hex.c"
	"${CMAKE_SOURCE_DIR}/src/path.c"
	"${CMAKE_SOURCE_DIR}/src/walkdir.c"
	"${CMAKE_SOURCE_DIR}/src/fs/cp.c"
	"${CMAKE_SOURCE_DIR}/src/fs/cwd.c"
	"${CMAKE_SOURCE_DIR}/src/fs/realpath.c"
	"${CMAKE_SOURCE_DIR}/src/fs/exists.c"
	"${CMAKE_SOURCE_DIR}/src/fs/getexec.c"
	"${CMAKE_SOURCE_DIR}/src/fs/mkdir.c"
	"${CMAKE_SOURCE_DIR}/src/fs/mv.c"
	"${CMAKE_SOURCE_DIR}/src/fs/rm.c"
	"${CMAKE_SOURCE_DIR}/src/os/cpu.c"
	"${CMAKE_SOURCE_DIR}/src/os/emulated.c"
	"${CMAKE_SOURCE_DIR}/src/os/env.c"
	"${CMAKE_SOURCE_DIR}/src/os/privileges.c"
	"${CMAKE_SOURCE_DIR}/src/os/shell.c"
	"${CMAKE_SOURCE_DIR}/src/clioptions.c"
	"${CMAKE_SOURCE_DIR}/src/errors.c"
	"${CMAKE_SOURCE_DIR}/src/argparse.c"
	"${CMAKE_SOURCE_DIR}/src/main.c"
	"${CMAKE_SOURCE_DIR}/src/httpclient.c"
	"${CMAKE_SOURCE_DIR}/src/m3u8sizeof.c"
	"${CMAKE_SOURCE_DIR}/src/m3u8parser.c"
	"${CMAKE_SOURCE_DIR}/src/m3u8stream.c"
	"${CMAKE_SOURCE_DIR}/src/m3u8download.c"
	"${CMAKE_SOURCE_DIR}/src/m3u8.c"
	"${CMAKE_SOURCE_DIR}/src/m3u8utils.c"
	"${CMAKE_SOURCE_DIR}/src/fstream.c"
	"${CMAKE_SOURCE_DIR}/src/strsplit.c"
	"${CMAKE_SOURCE_DIR}/src/terminal.c"
	"${CMAKE_SOURCE_DIR}/src/ffmpeg_muxer.c"
	"${CMAKE_SOURCE_DIR}/src/ffmpegc_muxer.c"
	"${CMAKE_SOURCE_DIR}/src/sslcerts.c"
	"${CMAKE_SOURCE_DIR}/src/showformats.c"
	"${CMAKE_SOURCE_DIR}/src/callbacks.c"
	"${CMAKE_SOURCE_DIR}/src/resources.c"
	"${CMAKE_SOURCE_DIR}/src/signals.c"
	"${CMAKE_SOURCE_DIR}/src/guess_uri.c"
	"${CMAKE_SOURCE_DIR}/src/cir.c"
	"${CMAKE_SOURCE_DIR}/src/sleep.c"
	"${CMAKE_SOURCE_DIR}/src/dump.c"
	"${CMAKE_SOURCE_DIR}/src/threads.c"
)

set(
	BEARSSL_SOURCE_FILES
	"${BEARSSL_SOURCE_DIRECTORY}/src/settings.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/aead/ccm.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/aead/eax.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/aead/gcm.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/codec/ccopy.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/codec/dec16be.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/codec/dec16le.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/codec/dec32be.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/codec/dec32le.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/codec/dec64be.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/codec/dec64le.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/codec/enc16be.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/codec/enc16le.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/codec/enc32be.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/codec/enc32le.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/codec/enc64be.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/codec/enc64le.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/codec/pemdec.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/codec/pemenc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_all_m15.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_all_m31.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_c25519_i15.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_c25519_i31.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_c25519_m15.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_c25519_m31.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_c25519_m62.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_c25519_m64.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_curve25519.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_default.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_keygen.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_p256_m15.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_p256_m31.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_p256_m62.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_p256_m64.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_prime_i15.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_prime_i31.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_pubkey.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_secp256r1.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_secp384r1.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_secp521r1.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ecdsa_atr.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ecdsa_default_sign_asn1.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ecdsa_default_sign_raw.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ecdsa_default_vrfy_asn1.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ecdsa_default_vrfy_raw.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ecdsa_i15_bits.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ecdsa_i15_sign_asn1.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ecdsa_i15_sign_raw.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ecdsa_i15_vrfy_asn1.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ecdsa_i15_vrfy_raw.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ecdsa_i31_bits.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ecdsa_i31_sign_asn1.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ecdsa_i31_sign_raw.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ecdsa_i31_vrfy_asn1.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ecdsa_i31_vrfy_raw.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ecdsa_rta.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/hash/dig_oid.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/hash/dig_size.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/hash/ghash_ctmul.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/hash/ghash_ctmul32.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/hash/ghash_ctmul64.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/hash/ghash_pclmul.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/hash/ghash_pwr8.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/hash/md5.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/hash/md5sha1.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/hash/mgf1.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/hash/multihash.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/hash/sha1.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/hash/sha2big.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/hash/sha2small.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i15_add.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i15_bitlen.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i15_decmod.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i15_decode.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i15_decred.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i15_encode.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i15_fmont.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i15_iszero.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i15_moddiv.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i15_modpow.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i15_modpow2.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i15_montmul.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i15_mulacc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i15_muladd.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i15_ninv15.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i15_reduce.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i15_rshift.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i15_sub.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i15_tmont.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i31_add.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i31_bitlen.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i31_decmod.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i31_decode.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i31_decred.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i31_encode.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i31_fmont.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i31_iszero.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i31_moddiv.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i31_modpow.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i31_modpow2.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i31_montmul.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i31_mulacc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i31_muladd.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i31_ninv31.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i31_reduce.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i31_rshift.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i31_sub.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i31_tmont.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i32_add.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i32_bitlen.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i32_decmod.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i32_decode.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i32_decred.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i32_div32.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i32_encode.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i32_fmont.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i32_iszero.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i32_modpow.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i32_montmul.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i32_mulacc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i32_muladd.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i32_ninv32.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i32_reduce.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i32_sub.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i32_tmont.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i62_modpow2.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/kdf/hkdf.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/kdf/shake.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/mac/hmac.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/mac/hmac_ct.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rand/aesctr_drbg.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rand/hmac_drbg.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rand/sysrng.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_default_keygen.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_default_modulus.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_default_oaep_decrypt.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_default_oaep_encrypt.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_default_pkcs1_sign.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_default_pkcs1_vrfy.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_default_priv.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_default_privexp.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_default_pss_sign.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_default_pss_vrfy.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_default_pub.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_default_pubexp.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i15_keygen.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i15_modulus.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i15_oaep_decrypt.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i15_oaep_encrypt.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i15_pkcs1_sign.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i15_pkcs1_vrfy.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i15_priv.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i15_privexp.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i15_pss_sign.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i15_pss_vrfy.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i15_pub.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i15_pubexp.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i31_keygen.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i31_keygen_inner.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i31_modulus.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i31_oaep_decrypt.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i31_oaep_encrypt.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i31_pkcs1_sign.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i31_pkcs1_vrfy.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i31_priv.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i31_privexp.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i31_pss_sign.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i31_pss_vrfy.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i31_pub.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i31_pubexp.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i32_oaep_decrypt.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i32_oaep_encrypt.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i32_pkcs1_sign.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i32_pkcs1_vrfy.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i32_priv.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i32_pss_sign.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i32_pss_vrfy.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i32_pub.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i62_keygen.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i62_oaep_decrypt.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i62_oaep_encrypt.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i62_pkcs1_sign.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i62_pkcs1_vrfy.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i62_priv.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i62_pss_sign.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i62_pss_vrfy.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i62_pub.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_oaep_pad.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_oaep_unpad.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_pkcs1_sig_pad.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_pkcs1_sig_unpad.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_pss_sig_pad.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_pss_sig_unpad.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_ssl_decrypt.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/prf.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/prf_md5sha1.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/prf_sha256.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/prf_sha384.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_ccert_single_ec.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_ccert_single_rsa.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_client.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_client_default_rsapub.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_client_full.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_engine.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_engine_default_aescbc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_engine_default_aesccm.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_engine_default_aesgcm.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_engine_default_chapol.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_engine_default_descbc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_engine_default_ec.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_engine_default_ecdsa.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_engine_default_rsavrfy.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_hashes.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_hs_client.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_hs_server.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_io.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_keyexport.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_lru.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_rec_cbc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_rec_ccm.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_rec_chapol.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_rec_gcm.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_scert_single_ec.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_scert_single_rsa.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_server.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_server_full_ec.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_server_full_rsa.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_server_mine2c.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_server_mine2g.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_server_minf2c.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_server_minf2g.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_server_minr2g.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_server_minu2g.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_server_minv2g.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_big_cbcdec.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_big_cbcenc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_big_ctr.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_big_ctrcbc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_big_dec.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_big_enc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_common.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_ct.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_ct64.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_ct64_cbcdec.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_ct64_cbcenc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_ct64_ctr.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_ct64_ctrcbc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_ct64_dec.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_ct64_enc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_ct_cbcdec.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_ct_cbcenc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_ct_ctr.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_ct_ctrcbc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_ct_dec.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_ct_enc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_pwr8.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_pwr8_cbcdec.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_pwr8_cbcenc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_pwr8_ctr.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_pwr8_ctrcbc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_small_cbcdec.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_small_cbcenc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_small_ctr.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_small_ctrcbc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_small_dec.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_small_enc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_x86ni.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_x86ni_cbcdec.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_x86ni_cbcenc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_x86ni_ctr.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_x86ni_ctrcbc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/chacha20_ct.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/chacha20_sse2.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/des_ct.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/des_ct_cbcdec.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/des_ct_cbcenc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/des_support.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/des_tab.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/des_tab_cbcdec.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/des_tab_cbcenc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/poly1305_ctmul.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/poly1305_ctmul32.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/poly1305_ctmulq.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/poly1305_i15.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/x509/asn1enc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/x509/encode_ec_pk8der.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/x509/encode_ec_rawder.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/x509/encode_rsa_pk8der.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/x509/encode_rsa_rawder.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/x509/skey_decoder.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/x509/x509_decoder.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/x509/x509_knownkey.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/x509/x509_minimal.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/x509/x509_minimal_full.c"
)

if (UNIX)
	set(SHARED_LIBRARY_DIRECTORY "lib")
elseif(WIN32)
	set(SHARED_LIBRARY_DIRECTORY "bin")
endif()

set(
	MODIFY_OUTPUT_DIRECTORY
	libcurl_shared
)

set(
	CURL_LIBRARY_DEPENDENCIES
	bearssl
)

set(
	INTERNAL_LIBRARY_TARGETS
	kai
	bearssl
	libcurl_shared
)

set(
	EXTERNAL_LIBRARY_TARGETS
	libavformat
	libavcodec
	libavutil
)

# curl
set(PICKY_COMPILER OFF)
set(BUILD_CURL_EXE OFF)
set(HTTP_ONLY ON)
set(ENABLE_THREADED_RESOLVER ON)
set(CURL_USE_BEARSSL ON)
set(CURL_USE_OPENSSL OFF)
set(CURL_USE_LIBPSL OFF)
set(CURL_DISABLE_ALTSVC ON)
set(CURL_DISABLE_CRYPTO_AUTH ON)
set(CURL_DISABLE_COOKIES OFF)
set(CURL_DISABLE_HSTS ON)
set(CURL_DISABLE_HTTP_AUTH ON)
set(CURL_DISABLE_NETRC ON)
set(CURL_DISABLE_WEBSOCKETS ON)
set(USE_LIBIDN2 OFF)
set(ENABLE_UNIX_SOCKETS OFF)
set(CURL_USE_LIBSSH2 OFF)
set(CURL_CA_BUNDLE "none")
set(CURL_CA_PATH "none")
set(CURL_WERROR OFF)
set(CURL_DISABLE_DOH OFF)
set(USE_NGHTTP2 "${KAI_ENABLE_HTTP2}")

if (WIN32 AND KAI_ENABLE_UNICODE)
	set(ENABLE_UNICODE ON)
endif()

# nghttp2
set(ENABLE_LIB_ONLY ON)
set(WITH_LIBXML2 OFF)
set(WITH_JEMALLOC OFF)
set(ENABLE_FAILMALLOC OFF)
set(ENABLE_DOC OFF)

file(READ "${CMAKE_SOURCE_DIR}/submodules/curl/CMake/FindBearSSL.cmake" FILE_CONTENTS) 
string(REPLACE "args(BEARSSL" "args(BearSSL" FILE_CONTENTS "${FILE_CONTENTS}") 
file(WRITE "${CMAKE_SOURCE_DIR}/submodules/curl/CMake/FindBearSSL.cmake" "${FILE_CONTENTS}") 

file(READ "${CMAKE_SOURCE_DIR}/submodules/curl/CMakeLists.txt" FILE_CONTENTS) 
string(REPLACE "DISABLE_FILE ON" "DISABLE_FILE OFF" FILE_CONTENTS "${FILE_CONTENTS}") 
file(WRITE "${CMAKE_SOURCE_DIR}/submodules/curl/CMakeLists.txt" "${FILE_CONTENTS}") 

if (SERENITYOS)
	file(READ "${CMAKE_SOURCE_DIR}/submodules/ffmpeg/libavutil/thread.h" FILE_CONTENTS) 
	string(REPLACE "#if HAVE_PRCTL" "#if !defined(__serenity__) && HAVE_PRCTL" FILE_CONTENTS "${FILE_CONTENTS}") 
	file(WRITE "${CMAKE_SOURCE_DIR}/submodules/ffmpeg/libavutil/thread.h" "${FILE_CONTENTS}") 
endif()

file(READ "${CMAKE_SOURCE_DIR}/submodules/ffmpeg/configure" FILE_CONTENTS) 
string(REPLACE "die \"Host compiler lacks C11 support\"" "true" FILE_CONTENTS "${FILE_CONTENTS}") 
file(WRITE "${CMAKE_SOURCE_DIR}/submodules/ffmpeg/configure" "${FILE_CONTENTS}") 

if (KAI_ENABLE_HTTP2)
	list(APPEND CURL_LIBRARY_DEPENDENCIES nghttp2)
	list(APPEND INTERNAL_LIBRARY_TARGETS nghttp2)
	list(APPEND MODIFY_OUTPUT_DIRECTORY nghttp2)
endif()

include_directories(${INCLUDE_DIRECTORIES})

add_library(
	bearssl
	SHARED
	${BEARSSL_SOURCE_FILES}
)

if (HAIKU)
	target_compile_definitions(
		bearssl
		PRIVATE
		BR_USE_URANDOM BR_USE_UNIX_TIME
	)
endif()

set(BEARSSL_INCLUDE_DIR "${BEARSSL_SOURCE_DIRECTORY}/inc")
set(BEARSSL_LIBRARY $<TARGET_FILE:bearssl>)

add_subdirectory(
	submodules/curl
	EXCLUDE_FROM_ALL
)

if (KAI_ENABLE_HTTP2)
	add_subdirectory(
		submodules/nghttp2
		EXCLUDE_FROM_ALL
	)
	
	if (NOT WIN32)
		target_compile_definitions(
			nghttp2
			PRIVATE
			HAVE_ARPA_INET_H HAVE_NETINET_IN_H
		)
	endif()
endif()

foreach(target ${CURL_LIBRARY_DEPENDENCIES})
	add_custom_command(
		OUTPUT ${target}
		COMMAND ${CMAKE_COMMAND} --build ./ --target ${target}
	)
	
	add_custom_target(
		"ensure_${target}" ALL DEPENDS ${target}
	)
	
	add_dependencies(
		libcurl_shared
		"ensure_${target}"
	)
endforeach()

set_target_properties(
	libcurl_shared
	PROPERTIES
	IMPORTED_LOCATION "${BEARSSL_LIBRARY}"
)

if (KAI_ENABLE_LTO)
	list(APPEND FFMPEG_CONFIGURE_FLAGS "--enable-lto")
endif()

if (CMAKE_CROSSCOMPILING)
	string(TOLOWER "${CMAKE_SYSTEM_NAME}" system_name)
	
	if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
		set(system_name "mingw32")
	elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "SerenityOS")
		set(system_name "none")
	endif()
	
	list(APPEND FFMPEG_CONFIGURE_FLAGS "--cross-prefix=${TOOLCHAIN_PREFIX}-")
	list(APPEND FFMPEG_CONFIGURE_FLAGS "--target-os=${system_name}")
	list(APPEND FFMPEG_CONFIGURE_FLAGS "--arch=${CMAKE_SYSTEM_PROCESSOR}")
	
	if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Android")
		list(APPEND FFMPEG_CONFIGURE_FLAGS "--as=${ANDROID_TOOLCHAIN_ROOT}/bin/${TOOLCHAIN_PREFIX}-clang")
		list(APPEND FFMPEG_CONFIGURE_FLAGS "--ld=${ANDROID_TOOLCHAIN_ROOT}/bin/${TOOLCHAIN_PREFIX}-clang")
		list(APPEND FFMPEG_CONFIGURE_FLAGS "--cc=${ANDROID_TOOLCHAIN_ROOT}/bin/${TOOLCHAIN_PREFIX}-clang")
	else()
		list(APPEND FFMPEG_CONFIGURE_FLAGS "--as=${CMAKE_C_COMPILER}")
		list(APPEND FFMPEG_CONFIGURE_FLAGS "--ld=${CMAKE_C_COMPILER}")
		list(APPEND FFMPEG_CONFIGURE_FLAGS "--cc=${CMAKE_C_COMPILER}")
		
		if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
			list(APPEND FFMPEG_CONFIGURE_FLAGS "--ar=${CMAKE_AR}")
		endif()
	endif()
	
	list(APPEND FFMPEG_CONFIGURE_FLAGS "--nm=${CMAKE_NM}")
	list(APPEND FFMPEG_CONFIGURE_FLAGS "--strip=${CMAKE_STRIP}")
else()
	list(APPEND FFMPEG_CONFIGURE_FLAGS "--cc=${CMAKE_C_COMPILER}")
	list(APPEND FFMPEG_CONFIGURE_FLAGS "--ld=${CMAKE_C_COMPILER}")
endif()

set(FFMPEG_INCLUDE_DIRS "${FFMPEG_INSTALL_PREFIX}/include")

if (NOT EXISTS "${FFMPEG_BUILD_DIRECTORY}")
	file(MAKE_DIRECTORY "${FFMPEG_BUILD_DIRECTORY}")
endif()

if (NOT EXISTS "${FFMPEG_BUILD_DIRECTORY}/Makefile")
	message("-- Configuring FFmpeg")
	
	execute_process(
		COMMAND bash
		"${FFMPEG_SOURCE_DIRECTORY}/configure"
		${FFMPEG_CONFIGURE_FLAGS}
		WORKING_DIRECTORY "${FFMPEG_BUILD_DIRECTORY}"
		COMMAND_ERROR_IS_FATAL ANY
	)
endif()

if (NOT EXISTS "${FFMPEG_INSTALL_PREFIX}")
	add_custom_command(
		OUTPUT "${FFMPEG_INSTALL_PREFIX}"
		COMMAND make
		COMMAND make install
		WORKING_DIRECTORY "${FFMPEG_BUILD_DIRECTORY}"
		COMMENT "-- Building FFmpeg"
	)
endif()

add_custom_target(
	ffmpeg
	ALL
	DEPENDS "${FFMPEG_INSTALL_PREFIX}"
)

# Extract libavformat version
file(READ "${FFMPEG_SOURCE_DIRECTORY}/libavformat/version_major.h" FILE_CONTENTS)
string(REGEX MATCH "#define LIBAVFORMAT_VERSION_MAJOR *([0-9]+)" _ ${FILE_CONTENTS})

set(LIBAVFORMAT_VERSION_MAJOR "${CMAKE_MATCH_1}")

file(READ "${FFMPEG_SOURCE_DIRECTORY}/libavformat/version.h" FILE_CONTENTS)

string(REGEX MATCH "#define LIBAVFORMAT_VERSION_MINOR *([0-9]+)" _ ${FILE_CONTENTS})
set(LIBAVFORMAT_VERSION_MINOR "${CMAKE_MATCH_1}")

string(REGEX MATCH "#define LIBAVFORMAT_VERSION_MICRO *([0-9]+)" _ ${FILE_CONTENTS})
set(LIBAVFORMAT_VERSION_MICRO "${CMAKE_MATCH_1}")

# Extract libavcodec version
file(READ "${FFMPEG_SOURCE_DIRECTORY}/libavcodec/version_major.h" FILE_CONTENTS)
string(REGEX MATCH "#define LIBAVCODEC_VERSION_MAJOR *([0-9]+)" _ ${FILE_CONTENTS})

set(LIBAVCODEC_VERSION_MAJOR "${CMAKE_MATCH_1}")

file(READ "${FFMPEG_SOURCE_DIRECTORY}/libavcodec/version.h" FILE_CONTENTS)

string(REGEX MATCH "#define LIBAVCODEC_VERSION_MINOR *([0-9]+)" _ ${FILE_CONTENTS})
set(LIBAVCODEC_VERSION_MINOR "${CMAKE_MATCH_1}")

string(REGEX MATCH "#define LIBAVCODEC_VERSION_MICRO *([0-9]+)" _ ${FILE_CONTENTS})
set(LIBAVCODEC_VERSION_MICRO "${CMAKE_MATCH_1}")

# Extract libavutil version
file(READ "${FFMPEG_SOURCE_DIRECTORY}/libavutil/version.h" FILE_CONTENTS)

string(REGEX MATCH "#define LIBAVUTIL_VERSION_MAJOR *([0-9]+)" _ ${FILE_CONTENTS})
set(LIBAVUTIL_VERSION_MAJOR "${CMAKE_MATCH_1}")

string(REGEX MATCH "#define LIBAVUTIL_VERSION_MINOR *([0-9]+)" _ ${FILE_CONTENTS})
set(LIBAVUTIL_VERSION_MINOR "${CMAKE_MATCH_1}")

string(REGEX MATCH "#define LIBAVUTIL_VERSION_MICRO *([0-9]+)" _ ${FILE_CONTENTS})
set(LIBAVUTIL_VERSION_MICRO "${CMAKE_MATCH_1}")

# Extract libswresample version
file(READ "${FFMPEG_SOURCE_DIRECTORY}/libswresample/version_major.h" FILE_CONTENTS)
string(REGEX MATCH "#define LIBSWRESAMPLE_VERSION_MAJOR *([0-9]+)" _ ${FILE_CONTENTS})

set(LIBSWRESAMPLE_VERSION_MAJOR "${CMAKE_MATCH_1}")

file(READ "${FFMPEG_SOURCE_DIRECTORY}/libswresample/version.h" FILE_CONTENTS)

string(REGEX MATCH "#define LIBSWRESAMPLE_VERSION_MINOR *([0-9]+)" _ ${FILE_CONTENTS})
set(LIBSWRESAMPLE_VERSION_MINOR "${CMAKE_MATCH_1}")

string(REGEX MATCH "#define LIBSWRESAMPLE_VERSION_MICRO *([0-9]+)" _ ${FILE_CONTENTS})
set(LIBSWRESAMPLE_VERSION_MICRO "${CMAKE_MATCH_1}")

if (UNIX)
	if ("${CMAKE_SYSTEM_NAME}" STREQUAL "OpenBSD")
		set(LIBAVFORMAT_SOVERSION "${LIBAVFORMAT_VERSION_MAJOR}.${LIBAVFORMAT_VERSION_MINOR}")
		set(LIBAVCODEC_SOVERSION "${LIBAVCODEC_VERSION_MAJOR}.${LIBAVCODEC_VERSION_MINOR}")
		set(LIBAVUTIL_SOVERSION "${LIBAVUTIL_VERSION_MAJOR}.${LIBAVUTIL_VERSION_MINOR}")
		set(LIBSWRESAMPLE_SOVERSION "${LIBSWRESAMPLE_VERSION_MAJOR}.${LIBSWRESAMPLE_VERSION_MINOR}")
	else()
		set(LIBAVFORMAT_SOVERSION "${LIBAVFORMAT_VERSION_MAJOR}.${LIBAVFORMAT_VERSION_MINOR}.${LIBAVFORMAT_VERSION_MICRO}")
		set(LIBAVCODEC_SOVERSION "${LIBAVCODEC_VERSION_MAJOR}.${LIBAVCODEC_VERSION_MINOR}.${LIBAVCODEC_VERSION_MICRO}")
		set(LIBAVUTIL_SOVERSION "${LIBAVUTIL_VERSION_MAJOR}.${LIBAVUTIL_VERSION_MINOR}.${LIBAVUTIL_VERSION_MICRO}")
		set(LIBSWRESAMPLE_SOVERSION "${LIBSWRESAMPLE_VERSION_MAJOR}.${LIBSWRESAMPLE_VERSION_MINOR}.${LIBSWRESAMPLE_VERSION_MICRO}")
	endif()
	
	set(SOVERSION_SEPARATOR ".")
	
	if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Android")
		set(AVFORMAT_LIBRARY_NAME "${CMAKE_SHARED_LIBRARY_PREFIX}avformat${CMAKE_SHARED_LIBRARY_SUFFIX}")
		set(AVCODEC_LIBRARY_NAME "${CMAKE_SHARED_LIBRARY_PREFIX}avcodec${CMAKE_SHARED_LIBRARY_SUFFIX}")
		set(AVUTIL_LIBRARY_NAME "${CMAKE_SHARED_LIBRARY_PREFIX}avutil${CMAKE_SHARED_LIBRARY_SUFFIX}")
		set(SWRESAMPLE_LIBRARY_NAME "${CMAKE_SHARED_LIBRARY_PREFIX}swresample${CMAKE_SHARED_LIBRARY_SUFFIX}")
	elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
		set(AVFORMAT_LIBRARY_NAME "${CMAKE_SHARED_LIBRARY_PREFIX}avformat${SOVERSION_SEPARATOR}${LIBAVFORMAT_SOVERSION}${CMAKE_SHARED_LIBRARY_SUFFIX}")
		set(AVCODEC_LIBRARY_NAME "${CMAKE_SHARED_LIBRARY_PREFIX}avcodec${SOVERSION_SEPARATOR}${LIBAVCODEC_SOVERSION}${CMAKE_SHARED_LIBRARY_SUFFIX}")
		set(AVUTIL_LIBRARY_NAME "${CMAKE_SHARED_LIBRARY_PREFIX}avutil${SOVERSION_SEPARATOR}${LIBAVUTIL_SOVERSION}${CMAKE_SHARED_LIBRARY_SUFFIX}")
		set(SWRESAMPLE_LIBRARY_NAME "${CMAKE_SHARED_LIBRARY_PREFIX}swresample${SOVERSION_SEPARATOR}${LIBSWRESAMPLE_SOVERSION}${CMAKE_SHARED_LIBRARY_SUFFIX}")
	else()
		set(AVFORMAT_LIBRARY_NAME "${CMAKE_SHARED_LIBRARY_PREFIX}avformat${CMAKE_SHARED_LIBRARY_SUFFIX}${SOVERSION_SEPARATOR}${LIBAVFORMAT_SOVERSION}")
		set(AVCODEC_LIBRARY_NAME "${CMAKE_SHARED_LIBRARY_PREFIX}avcodec${CMAKE_SHARED_LIBRARY_SUFFIX}${SOVERSION_SEPARATOR}${LIBAVCODEC_SOVERSION}")
		set(AVUTIL_LIBRARY_NAME "${CMAKE_SHARED_LIBRARY_PREFIX}avutil${CMAKE_SHARED_LIBRARY_SUFFIX}${SOVERSION_SEPARATOR}${LIBAVUTIL_SOVERSION}")
		set(SWRESAMPLE_LIBRARY_NAME "${CMAKE_SHARED_LIBRARY_PREFIX}swresample${CMAKE_SHARED_LIBRARY_SUFFIX}${SOVERSION_SEPARATOR}${LIBSWRESAMPLE_SOVERSION}")
	endif()
elseif (WIN32)
	set(LIBAVFORMAT_SOVERSION "${LIBAVFORMAT_VERSION_MAJOR}")
	set(LIBAVCODEC_SOVERSION "${LIBAVCODEC_VERSION_MAJOR}")
	set(LIBAVUTIL_SOVERSION "${LIBAVUTIL_VERSION_MAJOR}")
	set(LIBSWRESAMPLE_SOVERSION "${LIBSWRESAMPLE_VERSION_MAJOR}")
	
	set(SOVERSION_SEPARATOR "-")
	
	set(AVFORMAT_LIBRARY_NAME "avformat${SOVERSION_SEPARATOR}${LIBAVFORMAT_SOVERSION}${CMAKE_SHARED_LIBRARY_SUFFIX}")
	set(AVCODEC_LIBRARY_NAME "avcodec${SOVERSION_SEPARATOR}${LIBAVCODEC_SOVERSION}${CMAKE_SHARED_LIBRARY_SUFFIX}")
	set(AVUTIL_LIBRARY_NAME "avutil${SOVERSION_SEPARATOR}${LIBAVUTIL_SOVERSION}${CMAKE_SHARED_LIBRARY_SUFFIX}")
	set(SWRESAMPLE_LIBRARY_NAME "swresample${SOVERSION_SEPARATOR}${LIBSWRESAMPLE_SOVERSION}${CMAKE_SHARED_LIBRARY_SUFFIX}")
endif()

set(
	libraries
	"${FFMPEG_INSTALL_PREFIX}/${SHARED_LIBRARY_DIRECTORY}/${AVFORMAT_LIBRARY_NAME}"
	"${FFMPEG_INSTALL_PREFIX}/${SHARED_LIBRARY_DIRECTORY}/${AVCODEC_LIBRARY_NAME}"
	"${FFMPEG_INSTALL_PREFIX}/${SHARED_LIBRARY_DIRECTORY}/${AVUTIL_LIBRARY_NAME}"
	# "${FFMPEG_INSTALL_PREFIX}/${SHARED_LIBRARY_DIRECTORY}/${SWRESAMPLE_LIBRARY_NAME}"
)

set(FFMPEG_LIBRARIES)

foreach(source ${libraries})
	get_filename_component(dirname "${source}" DIRECTORY)
	get_filename_component(basename "${source}" NAME)
	
	string(FIND "${basename}" "${SOVERSION_SEPARATOR}" position)
	string(SUBSTRING "${basename}" 0 "${position}" destination)
	
	set(target "${destination}")
	set(target_name "${destination}")
	
	if (${target} STREQUAL "libavformat" OR ${target} STREQUAL "avformat")
		set(major_version "${LIBAVFORMAT_VERSION_MAJOR}")
	elseif (${target} STREQUAL "libavcodec" OR ${target} STREQUAL "avcodec")
		set(major_version "${LIBAVCODEC_VERSION_MAJOR}")
	elseif (${target} STREQUAL "libavutil" OR ${target} STREQUAL "avutil")
		set(major_version "${LIBAVUTIL_VERSION_MAJOR}")
	elseif (${target} STREQUAL "libswresample" OR ${target} STREQUAL "swresample")
		set(major_version "${LIBSWRESAMPLE_VERSION_MAJOR}")
	endif()
	
	if (UNIX)
		if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Android" OR "${CMAKE_SYSTEM_NAME}" STREQUAL "OpenBSD")
			string(APPEND destination "${CMAKE_SHARED_LIBRARY_SUFFIX}")
		elseif ("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
			string(APPEND destination "${SOVERSION_SEPARATOR}${major_version}${CMAKE_SHARED_LIBRARY_SUFFIX}")
		else()
			string(APPEND destination "${CMAKE_SHARED_LIBRARY_SUFFIX}${SOVERSION_SEPARATOR}${major_version}")
		endif()
	elseif (WIN32)
		string(APPEND destination "${SOVERSION_SEPARATOR}${major_version}${CMAKE_SHARED_LIBRARY_SUFFIX}")
	endif()
	
	string(PREPEND destination "${CMAKE_BINARY_DIR}/")
	
	if (NOT EXISTS "${destination}")
		add_custom_command(
			TARGET ffmpeg POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy "${source}" "${destination}"
			WORKING_DIRECTORY "${FFMPEG_BUILD_DIRECTORY}"
			COMMENT "-- Copying shared library from '${source}' to '${destination}'"
		)
	endif()
	
	if (WIN32)
		string(PREPEND target "${CMAKE_SHARED_LIBRARY_PREFIX}")
	endif()
	
	add_library("${target}" SHARED IMPORTED)
	
	set_target_properties(
		"${target}"
		PROPERTIES IMPORTED_LOCATION "${destination}"
	)
	
	if (WIN32)
		set_target_properties(
			"${target}"
			PROPERTIES IMPORTED_IMPLIB "${dirname}/${target_name}.lib"
		)
	endif()
	
	list(APPEND FFMPEG_LIBRARIES "${destination}")
endforeach()

add_executable(
	kai
	${KAI_SOURCE_FILES}
)

target_compile_options(
	kai
	PRIVATE
	${WARNING_OPTIONS}
)

if (WIN32)
	target_compile_definitions(
		kai
		PRIVATE
		CURL_SKIP_INCLUDE_MPRINTF
	)
endif()

find_package(Threads REQUIRED)

add_dependencies(
	kai
	ffmpeg
)

if (APPLE)
	foreach(property BUILD_RPATH INSTALL_RPATH)
		set_target_properties(
			kai
			PROPERTIES
			${property} "@loader_path/../lib"
		)
	endforeach()
elseif (UNIX)
	foreach(property BUILD_RPATH INSTALL_RPATH)
		set_target_properties(
			kai
			PROPERTIES
			${property} "$ORIGIN/../lib:$ORIGIN"
		)
	endforeach()
endif()

target_include_directories(
	kai
	PRIVATE
	"${FFMPEG_INCLUDE_DIRS}"
)

if (APPLE)
	foreach(property BUILD_RPATH INSTALL_RPATH)
		set_target_properties(
			libcurl_shared
			PROPERTIES
			${property} "@loader_path"
		)
	endforeach()
elseif (UNIX)
	foreach(property BUILD_RPATH INSTALL_RPATH)
		set_target_properties(
			libcurl_shared
			PROPERTIES
			${property} "$ORIGIN"
		)
	endforeach()
endif()

foreach(property RUNTIME_OUTPUT_DIRECTORY LIBRARY_OUTPUT_DIRECTORY)
	foreach(target ${MODIFY_OUTPUT_DIRECTORY})
		set_target_properties(
			${target}
			PROPERTIES
			${property} $<TARGET_FILE_DIR:kai>
		)
	endforeach()
endforeach()

# Enable Unicode on Windows builds
if (WIN32 AND KAI_ENABLE_UNICODE)
	target_sources(
		kai
		PRIVATE
		"src/wio.c"
	)
	
	target_compile_definitions(
		kai
		PRIVATE
		UNICODE _UNICODE
	)
	
	target_link_options(
		kai
		BEFORE
		PRIVATE
		-municode
	)
endif()

if (KAI_ENABLE_LTO)
	set(KAI_HAS_LTO OFF)
	
	include(CheckIPOSupported)
	check_ipo_supported(RESULT KAI_HAS_LTO LANGUAGES C)
	
	if (KAI_HAS_LTO)
		foreach(target ${INTERNAL_LIBRARY_TARGETS})
			set_target_properties(
				${target}
				PROPERTIES
				INTERPROCEDURAL_OPTIMIZATION TRUE
			)
		endforeach()
	endif()
endif()

target_link_libraries(
	kai
	Threads::Threads
	libcurl_shared
	libavformat
	libavcodec
	libavutil
	# libswresample
)

foreach(target ${INTERNAL_LIBRARY_TARGETS})
	install(
		TARGETS ${target}
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib NAMELINK_SKIP
	)
endforeach()

install(
	FILES ${FFMPEG_LIBRARIES}
	PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
	DESTINATION "${SHARED_LIBRARY_DIRECTORY}"
)

install(
	FILES "${CMAKE_SOURCE_DIR}/submodules/cabundle/pem/cert.pem"
	DESTINATION etc/tls
	RENAME cert.pem
)

if (APPLE)
	set(
		names
		"../../../libcurl.dylib"
	)
	
	set(index 0)
	
	foreach(library ${libraries})
		get_filename_component(dirname "${library}" DIRECTORY)
		list(GET FFMPEG_LIBRARIES "${index}" library)
		get_filename_component(basename "${library}" NAME)
		
		list(APPEND names "${dirname}/${basename}")
		
		MATH(EXPR index "${index}+1")
	endforeach()
	
	set(executable $<TARGET_FILE:kai>)
	
	foreach(old_rpath ${names})
		get_filename_component(basename "${old_rpath}" NAME)
		set(new_rpath "@loader_path/../lib/${basename}")
		
		add_custom_command(
			TARGET kai POST_BUILD
			COMMAND "${CMAKE_INSTALL_NAME_TOOL}" -change "${old_rpath}" "${new_rpath}" "${executable}"
			COMMENT "-- Changing hardcoded rpath of '${executable}' from '${old_rpath}' to '${new_rpath}'"
		)
	endforeach()
	
	foreach(library ${FFMPEG_LIBRARIES})
		set(names)
		set(index -1)
		
		get_filename_component(basename "${library}" NAME)
		
		if (basename MATCHES "libavformat${SOVERSION_SEPARATOR}${LIBAVFORMAT_VERSION_MAJOR}${CMAKE_SHARED_LIBRARY_SUFFIX}")
			foreach(library ${libraries})
				MATH(EXPR index "${index}+1")
				
				get_filename_component(dirname "${library}" DIRECTORY)
				list(GET FFMPEG_LIBRARIES "${index}" library)
				get_filename_component(basename "${library}" NAME)
				
				if (NOT (basename STREQUAL "libavcodec${SOVERSION_SEPARATOR}${LIBAVCODEC_VERSION_MAJOR}${CMAKE_SHARED_LIBRARY_SUFFIX}" OR basename STREQUAL "libavutil${SOVERSION_SEPARATOR}${LIBAVUTIL_VERSION_MAJOR}${CMAKE_SHARED_LIBRARY_SUFFIX}"))
					continue()
				endif()
				
				list(APPEND names "${dirname}/${basename}")
			endforeach()
		elseif (basename MATCHES "libavcodec${SOVERSION_SEPARATOR}${LIBAVCODEC_VERSION_MAJOR}${CMAKE_SHARED_LIBRARY_SUFFIX}")
			foreach(library ${libraries})
				MATH(EXPR index "${index}+1")
				
				get_filename_component(dirname "${library}" DIRECTORY)
				list(GET FFMPEG_LIBRARIES ${index} library)
				get_filename_component(basename "${library}" NAME)
				
				if (NOT basename STREQUAL "libavutil${SOVERSION_SEPARATOR}${LIBAVUTIL_VERSION_MAJOR}${CMAKE_SHARED_LIBRARY_SUFFIX}")
					continue()
				endif()
				
				list(APPEND names "${dirname}/${basename}")
			endforeach()
		else()
			continue()
		endif()
		
		foreach(old_rpath ${names})
			get_filename_component(basename "${old_rpath}" NAME)
			set(new_rpath "@loader_path/../lib/${basename}")
			
			add_custom_command(
				TARGET ffmpeg POST_BUILD
				COMMAND "${CMAKE_INSTALL_NAME_TOOL}" -change "${old_rpath}" "${new_rpath}" "${library}"
				COMMENT "-- Changing hardcoded rpath of '${library}' from '${old_rpath}' to '${new_rpath}'"
			)
		endforeach()
	endforeach()
endif()
