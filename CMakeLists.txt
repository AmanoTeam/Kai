cmake_minimum_required(VERSION 3.19)

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

set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0048 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

set(CMAKE_PLATFORM_NO_VERSIONED_SONAME ON)

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

if (KAI_ENABLE_HTTP2)
	set(USE_NGHTTP2 ON)
endif()

if (WIN32 AND KAI_ENABLE_UNICODE)
	set(ENABLE_UNICODE ON)
endif()

# nghttp2
set(ENABLE_LIB_ONLY ON)
set(WITH_LIBXML2 OFF)
set(WITH_JEMALLOC OFF)
set(ENABLE_FAILMALLOC OFF)
set(ENABLE_DOC OFF)

if (CMAKE_C_COMPILER_ID STREQUAL "Clang" AND CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
	add_compile_options(-Oz)
endif()

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

if (KAI_ENABLE_HTTP2)
	set(
		NGHTTP2_INCLUDE_DIR
		"${CMAKE_CURRENT_SOURCE_DIR}/submodules/nghttp2/lib/includes"
		"${CMAKE_CURRENT_BINARY_DIR}/submodules/nghttp2/lib/includes"
	)
	
	set(NGHTTP2_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}nghttp2${CMAKE_SHARED_LIBRARY_SUFFIX}")
	
	list(APPEND CURL_LIBRARY_DEPENDENCIES nghttp2)
	list(APPEND INTERNAL_LIBRARY_TARGETS nghttp2)
	list(APPEND MODIFY_OUTPUT_DIRECTORY nghttp2)
endif()

include_directories(
	"${CMAKE_SOURCE_DIR}/src"
	"${CMAKE_SOURCE_DIR}/submodules/bearssl/src"
	"${CMAKE_SOURCE_DIR}/submodules/bearssl/inc"
)

add_library(
	bearssl
	SHARED
	submodules/bearssl/src/settings.c
	submodules/bearssl/src/aead/ccm.c
	submodules/bearssl/src/aead/eax.c
	submodules/bearssl/src/aead/gcm.c
	submodules/bearssl/src/codec/ccopy.c
	submodules/bearssl/src/codec/dec16be.c
	submodules/bearssl/src/codec/dec16le.c
	submodules/bearssl/src/codec/dec32be.c
	submodules/bearssl/src/codec/dec32le.c
	submodules/bearssl/src/codec/dec64be.c
	submodules/bearssl/src/codec/dec64le.c
	submodules/bearssl/src/codec/enc16be.c
	submodules/bearssl/src/codec/enc16le.c
	submodules/bearssl/src/codec/enc32be.c
	submodules/bearssl/src/codec/enc32le.c
	submodules/bearssl/src/codec/enc64be.c
	submodules/bearssl/src/codec/enc64le.c
	submodules/bearssl/src/codec/pemdec.c
	submodules/bearssl/src/codec/pemenc.c
	submodules/bearssl/src/ec/ec_all_m15.c
	submodules/bearssl/src/ec/ec_all_m31.c
	submodules/bearssl/src/ec/ec_c25519_i15.c
	submodules/bearssl/src/ec/ec_c25519_i31.c
	submodules/bearssl/src/ec/ec_c25519_m15.c
	submodules/bearssl/src/ec/ec_c25519_m31.c
	submodules/bearssl/src/ec/ec_c25519_m62.c
	submodules/bearssl/src/ec/ec_c25519_m64.c
	submodules/bearssl/src/ec/ec_curve25519.c
	submodules/bearssl/src/ec/ec_default.c
	submodules/bearssl/src/ec/ec_keygen.c
	submodules/bearssl/src/ec/ec_p256_m15.c
	submodules/bearssl/src/ec/ec_p256_m31.c
	submodules/bearssl/src/ec/ec_p256_m62.c
	submodules/bearssl/src/ec/ec_p256_m64.c
	submodules/bearssl/src/ec/ec_prime_i15.c
	submodules/bearssl/src/ec/ec_prime_i31.c
	submodules/bearssl/src/ec/ec_pubkey.c
	submodules/bearssl/src/ec/ec_secp256r1.c
	submodules/bearssl/src/ec/ec_secp384r1.c
	submodules/bearssl/src/ec/ec_secp521r1.c
	submodules/bearssl/src/ec/ecdsa_atr.c
	submodules/bearssl/src/ec/ecdsa_default_sign_asn1.c
	submodules/bearssl/src/ec/ecdsa_default_sign_raw.c
	submodules/bearssl/src/ec/ecdsa_default_vrfy_asn1.c
	submodules/bearssl/src/ec/ecdsa_default_vrfy_raw.c
	submodules/bearssl/src/ec/ecdsa_i15_bits.c
	submodules/bearssl/src/ec/ecdsa_i15_sign_asn1.c
	submodules/bearssl/src/ec/ecdsa_i15_sign_raw.c
	submodules/bearssl/src/ec/ecdsa_i15_vrfy_asn1.c
	submodules/bearssl/src/ec/ecdsa_i15_vrfy_raw.c
	submodules/bearssl/src/ec/ecdsa_i31_bits.c
	submodules/bearssl/src/ec/ecdsa_i31_sign_asn1.c
	submodules/bearssl/src/ec/ecdsa_i31_sign_raw.c
	submodules/bearssl/src/ec/ecdsa_i31_vrfy_asn1.c
	submodules/bearssl/src/ec/ecdsa_i31_vrfy_raw.c
	submodules/bearssl/src/ec/ecdsa_rta.c
	submodules/bearssl/src/hash/dig_oid.c
	submodules/bearssl/src/hash/dig_size.c
	submodules/bearssl/src/hash/ghash_ctmul.c
	submodules/bearssl/src/hash/ghash_ctmul32.c
	submodules/bearssl/src/hash/ghash_ctmul64.c
	submodules/bearssl/src/hash/ghash_pclmul.c
	submodules/bearssl/src/hash/ghash_pwr8.c
	submodules/bearssl/src/hash/md5.c
	submodules/bearssl/src/hash/md5sha1.c
	submodules/bearssl/src/hash/mgf1.c
	submodules/bearssl/src/hash/multihash.c
	submodules/bearssl/src/hash/sha1.c
	submodules/bearssl/src/hash/sha2big.c
	submodules/bearssl/src/hash/sha2small.c
	submodules/bearssl/src/int/i15_add.c
	submodules/bearssl/src/int/i15_bitlen.c
	submodules/bearssl/src/int/i15_decmod.c
	submodules/bearssl/src/int/i15_decode.c
	submodules/bearssl/src/int/i15_decred.c
	submodules/bearssl/src/int/i15_encode.c
	submodules/bearssl/src/int/i15_fmont.c
	submodules/bearssl/src/int/i15_iszero.c
	submodules/bearssl/src/int/i15_moddiv.c
	submodules/bearssl/src/int/i15_modpow.c
	submodules/bearssl/src/int/i15_modpow2.c
	submodules/bearssl/src/int/i15_montmul.c
	submodules/bearssl/src/int/i15_mulacc.c
	submodules/bearssl/src/int/i15_muladd.c
	submodules/bearssl/src/int/i15_ninv15.c
	submodules/bearssl/src/int/i15_reduce.c
	submodules/bearssl/src/int/i15_rshift.c
	submodules/bearssl/src/int/i15_sub.c
	submodules/bearssl/src/int/i15_tmont.c
	submodules/bearssl/src/int/i31_add.c
	submodules/bearssl/src/int/i31_bitlen.c
	submodules/bearssl/src/int/i31_decmod.c
	submodules/bearssl/src/int/i31_decode.c
	submodules/bearssl/src/int/i31_decred.c
	submodules/bearssl/src/int/i31_encode.c
	submodules/bearssl/src/int/i31_fmont.c
	submodules/bearssl/src/int/i31_iszero.c
	submodules/bearssl/src/int/i31_moddiv.c
	submodules/bearssl/src/int/i31_modpow.c
	submodules/bearssl/src/int/i31_modpow2.c
	submodules/bearssl/src/int/i31_montmul.c
	submodules/bearssl/src/int/i31_mulacc.c
	submodules/bearssl/src/int/i31_muladd.c
	submodules/bearssl/src/int/i31_ninv31.c
	submodules/bearssl/src/int/i31_reduce.c
	submodules/bearssl/src/int/i31_rshift.c
	submodules/bearssl/src/int/i31_sub.c
	submodules/bearssl/src/int/i31_tmont.c
	submodules/bearssl/src/int/i32_add.c
	submodules/bearssl/src/int/i32_bitlen.c
	submodules/bearssl/src/int/i32_decmod.c
	submodules/bearssl/src/int/i32_decode.c
	submodules/bearssl/src/int/i32_decred.c
	submodules/bearssl/src/int/i32_div32.c
	submodules/bearssl/src/int/i32_encode.c
	submodules/bearssl/src/int/i32_fmont.c
	submodules/bearssl/src/int/i32_iszero.c
	submodules/bearssl/src/int/i32_modpow.c
	submodules/bearssl/src/int/i32_montmul.c
	submodules/bearssl/src/int/i32_mulacc.c
	submodules/bearssl/src/int/i32_muladd.c
	submodules/bearssl/src/int/i32_ninv32.c
	submodules/bearssl/src/int/i32_reduce.c
	submodules/bearssl/src/int/i32_sub.c
	submodules/bearssl/src/int/i32_tmont.c
	submodules/bearssl/src/int/i62_modpow2.c
	submodules/bearssl/src/kdf/hkdf.c
	submodules/bearssl/src/kdf/shake.c
	submodules/bearssl/src/mac/hmac.c
	submodules/bearssl/src/mac/hmac_ct.c
	submodules/bearssl/src/rand/aesctr_drbg.c
	submodules/bearssl/src/rand/hmac_drbg.c
	submodules/bearssl/src/rand/sysrng.c
	submodules/bearssl/src/rsa/rsa_default_keygen.c
	submodules/bearssl/src/rsa/rsa_default_modulus.c
	submodules/bearssl/src/rsa/rsa_default_oaep_decrypt.c
	submodules/bearssl/src/rsa/rsa_default_oaep_encrypt.c
	submodules/bearssl/src/rsa/rsa_default_pkcs1_sign.c
	submodules/bearssl/src/rsa/rsa_default_pkcs1_vrfy.c
	submodules/bearssl/src/rsa/rsa_default_priv.c
	submodules/bearssl/src/rsa/rsa_default_privexp.c
	submodules/bearssl/src/rsa/rsa_default_pss_sign.c
	submodules/bearssl/src/rsa/rsa_default_pss_vrfy.c
	submodules/bearssl/src/rsa/rsa_default_pub.c
	submodules/bearssl/src/rsa/rsa_default_pubexp.c
	submodules/bearssl/src/rsa/rsa_i15_keygen.c
	submodules/bearssl/src/rsa/rsa_i15_modulus.c
	submodules/bearssl/src/rsa/rsa_i15_oaep_decrypt.c
	submodules/bearssl/src/rsa/rsa_i15_oaep_encrypt.c
	submodules/bearssl/src/rsa/rsa_i15_pkcs1_sign.c
	submodules/bearssl/src/rsa/rsa_i15_pkcs1_vrfy.c
	submodules/bearssl/src/rsa/rsa_i15_priv.c
	submodules/bearssl/src/rsa/rsa_i15_privexp.c
	submodules/bearssl/src/rsa/rsa_i15_pss_sign.c
	submodules/bearssl/src/rsa/rsa_i15_pss_vrfy.c
	submodules/bearssl/src/rsa/rsa_i15_pub.c
	submodules/bearssl/src/rsa/rsa_i15_pubexp.c
	submodules/bearssl/src/rsa/rsa_i31_keygen.c
	submodules/bearssl/src/rsa/rsa_i31_keygen_inner.c
	submodules/bearssl/src/rsa/rsa_i31_modulus.c
	submodules/bearssl/src/rsa/rsa_i31_oaep_decrypt.c
	submodules/bearssl/src/rsa/rsa_i31_oaep_encrypt.c
	submodules/bearssl/src/rsa/rsa_i31_pkcs1_sign.c
	submodules/bearssl/src/rsa/rsa_i31_pkcs1_vrfy.c
	submodules/bearssl/src/rsa/rsa_i31_priv.c
	submodules/bearssl/src/rsa/rsa_i31_privexp.c
	submodules/bearssl/src/rsa/rsa_i31_pss_sign.c
	submodules/bearssl/src/rsa/rsa_i31_pss_vrfy.c
	submodules/bearssl/src/rsa/rsa_i31_pub.c
	submodules/bearssl/src/rsa/rsa_i31_pubexp.c
	submodules/bearssl/src/rsa/rsa_i32_oaep_decrypt.c
	submodules/bearssl/src/rsa/rsa_i32_oaep_encrypt.c
	submodules/bearssl/src/rsa/rsa_i32_pkcs1_sign.c
	submodules/bearssl/src/rsa/rsa_i32_pkcs1_vrfy.c
	submodules/bearssl/src/rsa/rsa_i32_priv.c
	submodules/bearssl/src/rsa/rsa_i32_pss_sign.c
	submodules/bearssl/src/rsa/rsa_i32_pss_vrfy.c
	submodules/bearssl/src/rsa/rsa_i32_pub.c
	submodules/bearssl/src/rsa/rsa_i62_keygen.c
	submodules/bearssl/src/rsa/rsa_i62_oaep_decrypt.c
	submodules/bearssl/src/rsa/rsa_i62_oaep_encrypt.c
	submodules/bearssl/src/rsa/rsa_i62_pkcs1_sign.c
	submodules/bearssl/src/rsa/rsa_i62_pkcs1_vrfy.c
	submodules/bearssl/src/rsa/rsa_i62_priv.c
	submodules/bearssl/src/rsa/rsa_i62_pss_sign.c
	submodules/bearssl/src/rsa/rsa_i62_pss_vrfy.c
	submodules/bearssl/src/rsa/rsa_i62_pub.c
	submodules/bearssl/src/rsa/rsa_oaep_pad.c
	submodules/bearssl/src/rsa/rsa_oaep_unpad.c
	submodules/bearssl/src/rsa/rsa_pkcs1_sig_pad.c
	submodules/bearssl/src/rsa/rsa_pkcs1_sig_unpad.c
	submodules/bearssl/src/rsa/rsa_pss_sig_pad.c
	submodules/bearssl/src/rsa/rsa_pss_sig_unpad.c
	submodules/bearssl/src/rsa/rsa_ssl_decrypt.c
	submodules/bearssl/src/ssl/prf.c
	submodules/bearssl/src/ssl/prf_md5sha1.c
	submodules/bearssl/src/ssl/prf_sha256.c
	submodules/bearssl/src/ssl/prf_sha384.c
	submodules/bearssl/src/ssl/ssl_ccert_single_ec.c
	submodules/bearssl/src/ssl/ssl_ccert_single_rsa.c
	submodules/bearssl/src/ssl/ssl_client.c
	submodules/bearssl/src/ssl/ssl_client_default_rsapub.c
	submodules/bearssl/src/ssl/ssl_client_full.c
	submodules/bearssl/src/ssl/ssl_engine.c
	submodules/bearssl/src/ssl/ssl_engine_default_aescbc.c
	submodules/bearssl/src/ssl/ssl_engine_default_aesccm.c
	submodules/bearssl/src/ssl/ssl_engine_default_aesgcm.c
	submodules/bearssl/src/ssl/ssl_engine_default_chapol.c
	submodules/bearssl/src/ssl/ssl_engine_default_descbc.c
	submodules/bearssl/src/ssl/ssl_engine_default_ec.c
	submodules/bearssl/src/ssl/ssl_engine_default_ecdsa.c
	submodules/bearssl/src/ssl/ssl_engine_default_rsavrfy.c
	submodules/bearssl/src/ssl/ssl_hashes.c
	submodules/bearssl/src/ssl/ssl_hs_client.c
	submodules/bearssl/src/ssl/ssl_hs_server.c
	submodules/bearssl/src/ssl/ssl_io.c
	submodules/bearssl/src/ssl/ssl_keyexport.c
	submodules/bearssl/src/ssl/ssl_lru.c
	submodules/bearssl/src/ssl/ssl_rec_cbc.c
	submodules/bearssl/src/ssl/ssl_rec_ccm.c
	submodules/bearssl/src/ssl/ssl_rec_chapol.c
	submodules/bearssl/src/ssl/ssl_rec_gcm.c
	submodules/bearssl/src/ssl/ssl_scert_single_ec.c
	submodules/bearssl/src/ssl/ssl_scert_single_rsa.c
	submodules/bearssl/src/ssl/ssl_server.c
	submodules/bearssl/src/ssl/ssl_server_full_ec.c
	submodules/bearssl/src/ssl/ssl_server_full_rsa.c
	submodules/bearssl/src/ssl/ssl_server_mine2c.c
	submodules/bearssl/src/ssl/ssl_server_mine2g.c
	submodules/bearssl/src/ssl/ssl_server_minf2c.c
	submodules/bearssl/src/ssl/ssl_server_minf2g.c
	submodules/bearssl/src/ssl/ssl_server_minr2g.c
	submodules/bearssl/src/ssl/ssl_server_minu2g.c
	submodules/bearssl/src/ssl/ssl_server_minv2g.c
	submodules/bearssl/src/symcipher/aes_big_cbcdec.c
	submodules/bearssl/src/symcipher/aes_big_cbcenc.c
	submodules/bearssl/src/symcipher/aes_big_ctr.c
	submodules/bearssl/src/symcipher/aes_big_ctrcbc.c
	submodules/bearssl/src/symcipher/aes_big_dec.c
	submodules/bearssl/src/symcipher/aes_big_enc.c
	submodules/bearssl/src/symcipher/aes_common.c
	submodules/bearssl/src/symcipher/aes_ct.c
	submodules/bearssl/src/symcipher/aes_ct64.c
	submodules/bearssl/src/symcipher/aes_ct64_cbcdec.c
	submodules/bearssl/src/symcipher/aes_ct64_cbcenc.c
	submodules/bearssl/src/symcipher/aes_ct64_ctr.c
	submodules/bearssl/src/symcipher/aes_ct64_ctrcbc.c
	submodules/bearssl/src/symcipher/aes_ct64_dec.c
	submodules/bearssl/src/symcipher/aes_ct64_enc.c
	submodules/bearssl/src/symcipher/aes_ct_cbcdec.c
	submodules/bearssl/src/symcipher/aes_ct_cbcenc.c
	submodules/bearssl/src/symcipher/aes_ct_ctr.c
	submodules/bearssl/src/symcipher/aes_ct_ctrcbc.c
	submodules/bearssl/src/symcipher/aes_ct_dec.c
	submodules/bearssl/src/symcipher/aes_ct_enc.c
	submodules/bearssl/src/symcipher/aes_pwr8.c
	submodules/bearssl/src/symcipher/aes_pwr8_cbcdec.c
	submodules/bearssl/src/symcipher/aes_pwr8_cbcenc.c
	submodules/bearssl/src/symcipher/aes_pwr8_ctr.c
	submodules/bearssl/src/symcipher/aes_pwr8_ctrcbc.c
	submodules/bearssl/src/symcipher/aes_small_cbcdec.c
	submodules/bearssl/src/symcipher/aes_small_cbcenc.c
	submodules/bearssl/src/symcipher/aes_small_ctr.c
	submodules/bearssl/src/symcipher/aes_small_ctrcbc.c
	submodules/bearssl/src/symcipher/aes_small_dec.c
	submodules/bearssl/src/symcipher/aes_small_enc.c
	submodules/bearssl/src/symcipher/aes_x86ni.c
	submodules/bearssl/src/symcipher/aes_x86ni_cbcdec.c
	submodules/bearssl/src/symcipher/aes_x86ni_cbcenc.c
	submodules/bearssl/src/symcipher/aes_x86ni_ctr.c
	submodules/bearssl/src/symcipher/aes_x86ni_ctrcbc.c
	submodules/bearssl/src/symcipher/chacha20_ct.c
	submodules/bearssl/src/symcipher/chacha20_sse2.c
	submodules/bearssl/src/symcipher/des_ct.c
	submodules/bearssl/src/symcipher/des_ct_cbcdec.c
	submodules/bearssl/src/symcipher/des_ct_cbcenc.c
	submodules/bearssl/src/symcipher/des_support.c
	submodules/bearssl/src/symcipher/des_tab.c
	submodules/bearssl/src/symcipher/des_tab_cbcdec.c
	submodules/bearssl/src/symcipher/des_tab_cbcenc.c
	submodules/bearssl/src/symcipher/poly1305_ctmul.c
	submodules/bearssl/src/symcipher/poly1305_ctmul32.c
	submodules/bearssl/src/symcipher/poly1305_ctmulq.c
	submodules/bearssl/src/symcipher/poly1305_i15.c
	submodules/bearssl/src/x509/asn1enc.c
	submodules/bearssl/src/x509/encode_ec_pk8der.c
	submodules/bearssl/src/x509/encode_ec_rawder.c
	submodules/bearssl/src/x509/encode_rsa_pk8der.c
	submodules/bearssl/src/x509/encode_rsa_rawder.c
	submodules/bearssl/src/x509/skey_decoder.c
	submodules/bearssl/src/x509/x509_decoder.c
	submodules/bearssl/src/x509/x509_knownkey.c
	submodules/bearssl/src/x509/x509_minimal.c
	submodules/bearssl/src/x509/x509_minimal_full.c
)

if (HAIKU)
	target_compile_definitions(
		bearssl
		PRIVATE
		BR_USE_URANDOM BR_USE_UNIX_TIME
	)
endif()

set(BEARSSL_INCLUDE_DIR submodules/bearssl/inc)
set(BEARSSL_LIBRARY $<TARGET_FILE:bearssl>)

add_subdirectory(submodules/curl EXCLUDE_FROM_ALL)

if (KAI_ENABLE_HTTP2)
	add_subdirectory(submodules/nghttp2 EXCLUDE_FROM_ALL)
	
	if (NOT WIN32)
		target_compile_definitions(
			nghttp2
			PRIVATE
			HAVE_ARPA_INET_H HAVE_NETINET_IN_H
		)
	endif()
	
	if (NOT ANDROID)
		target_link_options(
			nghttp2
			BEFORE
			PRIVATE
			-static-libgcc -static-libstdc++
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
	IMPORTED_LOCATION $<TARGET_FILE:bearssl>
)

set(FFMPEG_SOURCE_DIRECTORY "${CMAKE_SOURCE_DIR}/submodules/ffmpeg")
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
	--extra-ldflags=-fPIC
	--prefix="${FFMPEG_INSTALL_PREFIX}"
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

add_custom_target(ffmpeg ALL DEPENDS "${FFMPEG_INSTALL_PREFIX}")

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

if (UNIX)
	set(SHARED_LIBRARY_DIRECTORY "lib")
elseif(WIN32)
	set(SHARED_LIBRARY_DIRECTORY "bin")
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
	src/sutils.c
	src/hex.c
	src/path.c
	src/walkdir.c
	src/filesystem.c
	src/os.c
	src/clioptions.c
	src/errors.c
	src/argparser.c
	src/main.c
	src/m3u8httpclient.c
	src/m3u8sizeof.c
	src/m3u8parser.c
	src/m3u8stream.c
	src/m3u8download.c
	src/m3u8.c
	src/m3u8utils.c
	src/fstream.c
	src/readlines.c
	src/terminal.c
	src/ffmpeg_muxer.c
	src/ffmpegc_muxer.c
	src/sslcerts.c
	src/showformats.c
	src/callbacks.c
	src/resources.c
	src/signals.c
	src/guess_uri.c
	src/cir.c
	src/sleep.c
	src/dump.c
	src/threads.c
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

foreach(target kai)
	target_compile_options(
		${target}
		PRIVATE
		-Wall -Wextra
	)
	
	if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
		target_link_options(
			${target}
			BEFORE
			PRIVATE
			-Wl,-rpath-link,.
		)
	endif()
	
	if (APPLE)
		foreach(property BUILD_RPATH INSTALL_RPATH)
			set_target_properties(
				${target}
				PROPERTIES
				${property} "@loader_path/../lib"
			)
		endforeach()
	elseif (UNIX)
		foreach(property BUILD_RPATH INSTALL_RPATH)
			set_target_properties(
				${target}
				PROPERTIES
				${property} "$ORIGIN/../lib:$ORIGIN"
			)
		endforeach()
	endif()
	
	target_include_directories(
		${target}
		PRIVATE
		"${FFMPEG_INCLUDE_DIRS}"
	)
endforeach()

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
		src/wio.c
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

if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
	target_link_options(
		kai
		BEFORE
		PRIVATE
		-Wl,-rpath-link,. -L.
	)
elseif (WIN32)
	target_link_options(
		kai
		BEFORE
		PRIVATE
		-L.
	)
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
