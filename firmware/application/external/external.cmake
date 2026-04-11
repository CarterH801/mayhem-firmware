set(EXTCPPSRC
	#font_viewer 8 byte?!
	external/font_viewer/main.cpp
	external/font_viewer/ui_font_viewer.cpp

	#blespam 336 bytes - array initializers?
	external/blespam/main.cpp
	external/blespam/ui_blespam.cpp

	#coasterp  0 byte
	external/coasterp/main.cpp
	external/coasterp/ui_coasterp.cpp

	#lge  120 byte
	external/lge/main.cpp
	external/lge/lge_app.cpp
	external/lge/rfm69.cpp

	#lcr - 460 byte flash
	external/lcr/main.cpp
	external/lcr/ui_lcr.cpp

	#jammer 144 byte
	external/jammer/main.cpp
	external/jammer/ui_jammer.cpp

	#gpssim  160 byte
	external/gpssim/main.cpp
	external/gpssim/gps_sim_app.cpp

	#spainter   464 byte
	external/spainter/main.cpp
	external/spainter/ui_spectrum_painter.cpp
	external/spainter/ui_spectrum_painter_text.cpp
	external/spainter/ui_spectrum_painter_image.cpp

	#keyfob 216 byte
	external/keyfob/main.cpp
	external/keyfob/ui_keyfob.cpp
	external/keyfob/ui_keyfob.hpp


	#extsensors 192 byte
	external/extsensors/main.cpp
	external/extsensors/ui_extsensors.cpp
	external/extsensors/ui_extsensors.hpp

	#audio_test 192 byte
	external/audio_test/main.cpp
	external/audio_test/ui_audio_test.cpp

	#wardrivemap 64 byte
	external/wardrivemap/main.cpp
	external/wardrivemap/ui_wardrivemap.cpp

	#tpmsrx 920 byte- possible shared part with baseband
	external/tpmsrx/main.cpp
	external/tpmsrx/tpms_app.cpp

	#tpmstx 800 bytes - TPMS transmit with editable fields
	external/tpmstx/main.cpp
	external/tpmstx/tpms_tx_app.cpp

	#protoview 8 byte
	external/protoview/main.cpp
	external/protoview/ui_protoview.cpp

	#adsbtx  3544 byte - adsb shared part
	external/adsbtx/main.cpp
	external/adsbtx/ui_adsb_tx.cpp

	#morse_tx 768 bytes -- disabled because of the new morse tx app with more functions
	#external/morse_tx/main.cpp
	#external/morse_tx/ui_morse.cpp

	#sstvtx 456 bytes
	external/sstvtx/main.cpp
	external/sstvtx/ui_sstvtx.cpp

	#same_tx
	external/same_tx/main.cpp
	external/same_tx/ui_same_tx.cpp

	#mdc_tx
	external/mdc_tx/main.cpp
	external/mdc_tx/ui_mdc_tx.cpp

	#random 464  bytes.
	external/random_password/main.cpp
	external/random_password/ui_random_password.cpp
	external/random_password/sha512.cpp

	#acars
	external/acars_rx/main.cpp
	external/acars_rx/acars_app.cpp

	#wefax_rx 192 bytes
	external/wefax_rx/main.cpp
	external/wefax_rx/ui_wefax_rx.cpp


	#noaaapt_rx  72 bytes
	external/noaaapt_rx/main.cpp
	external/noaaapt_rx/ui_noaaapt_rx.cpp

	#shoppingcart_lock 272 bytes
	external/shoppingcart_lock/main.cpp
	external/shoppingcart_lock/shoppingcart_lock.cpp


	#ookbrute  80 byte
	external/ookbrute/main.cpp
	external/ookbrute/ui_ookbrute.cpp

	#ook_editor  1808 bytes
	external/ook_editor/main.cpp
	external/ook_editor/ui_ook_editor.cpp

	#cvs_spam 24 byte
	external/cvs_spam/main.cpp
	external/cvs_spam/cvs_spam.cpp

	#flippertx  712 bytes
	external/flippertx/main.cpp
	external/flippertx/ui_flippertx.cpp

	#remote 1664 bytes
	external/remote/main.cpp
	external/remote/ui_remote.cpp

	#mcu_temperature    112
	external/mcu_temperature/main.cpp
	external/mcu_temperature/mcu_temperature.cpp

	#fmradio  640
	external/fmradio/main.cpp
	external/fmradio/ui_fmradio.cpp

	#tuner 384
	external/tuner/main.cpp
	external/tuner/ui_tuner.cpp

	#app_manager 40 bytes
	external/app_manager/main.cpp
	external/app_manager/ui_app_manager.cpp

	#hopper 472 bytes
	external/hopper/main.cpp
	external/hopper/ui_hopper.cpp

	# whip calculator  48 bytes
	external/antenna_length/main.cpp
	external/antenna_length/ui_whipcalc.cpp

	# wav viewer 1232 bytes
	external/wav_view/main.cpp
	external/wav_view/ui_view_wav.cpp


	# wipe sdcard 16 byte
	external/sd_wipe/main.cpp
	external/sd_wipe/ui_sd_wipe.cpp

	# playlist editor 232 bytes
	external/playlist_editor/main.cpp
	external/playlist_editor/ui_playlist_editor.cpp

	#debug_pmem  944 byte
	external/debug_pmem/main.cpp
	external/debug_pmem/ui_debug_pmem.cpp

	#scanner 520 byte
	external/scanner/main.cpp
	external/scanner/ui_scanner.cpp

	#level  24 byte
	external/level/main.cpp
	external/level/ui_level.cpp

	#waterfall designer
	external/waterfall_designer/main.cpp
	external/waterfall_designer/ui_waterfall_designer.cpp

	#detector_rx  168 byte
	external/detector_rx/main.cpp
	external/detector_rx/ui_detector_rx.cpp

	#ert 3040 bytes - has common with baseband, could be renamed the namespace, so both could have it, but not kept in fw.
	external/ert/main.cpp
	external/ert/ert_app.cpp

	#epirb_rx 168 byte flash
	external/epirb_rx/main.cpp
	external/epirb_rx/ui_epirb_rx.cpp

	#epirb_tx
	external/epirb_tx/main.cpp
	external/epirb_tx/ui_epirb_tx.cpp

	#soundboard  272byte  - 1236 bytes
	external/soundboard/main.cpp
	external/soundboard/soundboard_app.cpp

	#bht_tx - 3920 byte flash, unknown
	external/bht_tx/main.cpp
	external/bht_tx/ui_bht_tx.cpp
	external/bht_tx/bht.cpp

	#morse_practice - 80 byte flash - bc of array initializers
	external/morse_practice/main.cpp
	external/morse_practice/ui_morse_practice.cpp

	#adult_toys_controller  144 bytes
	external/adult_toys_controller/main.cpp
	external/adult_toys_controller/ui_adult_toys_controller.cpp

	#flex_rx
	external/flex_rx/main.cpp
	external/flex_rx/ui_flex_rx.cpp

	#subcarrx
	external/subcarrx/main.cpp
	external/subcarrx/ui_subcar.cpp

	#siggen
	external/siggen/main.cpp
	external/siggen/ui_siggen.cpp

	#morse_radio
	external/morse_radio/main.cpp
	external/morse_radio/ui_morse_radio.cpp

	#morseradiotx
	external/morseradiotx/main.cpp
	external/morseradiotx/ui_morse_radiotx.cpp

  	external/keeloqtx/main.cpp
  	external/keeloqtx/ui_keeloqtx.cpp

    #pocsag_tx
	external/pocsag_tx/main.cpp
	external/pocsag_tx/ui_pocsag_tx.cpp

	#time_sink
	external/time_sink/main.cpp
	external/time_sink/ui_time_sink.cpp

	#kiss_tnc
	external/kiss_tnc/main.cpp
	external/kiss_tnc/ui_kiss_tnc.cpp

	#fpv_detect
	external/fpv_detect/main.cpp
	external/fpv_detect/ui_fpv_detect.cpp

	#p25_tx
	external/p25_tx/main.cpp
	external/p25_tx/ui_p25_tx.cpp

)

set(EXTAPPLIST
	font_viewer
	blespam
	coasterp
	lge
	lcr
	jammer
	gpssim
	spainter
	keyfob
	extsensors
	audio_test
	wardrivemap
	tpmsrx
	tpmstx
	protoview
	adsbtx
	#morse_tx
	sstvtx
	same_tx
	mdc_tx
	random_password
	acars_rx
	wefax_rx
	noaaapt_rx
	shoppingcart_lock
	ookbrute
	ook_editor
	cvs_spam
	flippertx
	remote
	mcu_temperature
	fmradio
	tuner
	app_manager
	hopper
	antenna_length
	view_wav
	sd_wipe
	playlist_editor
	debug_pmem
	scanner
	level
	waterfall_designer
	detector_rx
	fpv_detect
	ert
	epirb_rx
	epirb_tx
	soundboard
	bht_tx
	morse_practice
	adult_toys_controller
	flex_rx
	subcarrx
	siggen
	morse_radio
	morseradiotx
	keeloqtx
	pocsag_tx
	time_sink
	kiss_tnc
	p25_tx
)

# sdusb has type conflicts with PRALINE (HackRF Pro) - add only for non-PRALINE builds
if(NOT BOARD STREQUAL "PRALINE")
       list(APPEND EXTCPPSRC
               external/sdusb/main.cpp
               external/sdusb/ui_sd_over_usb.cpp
       )
       list(APPEND EXTAPPLIST sdusb)
endif()

# 6 custom radio apps — compiled natively for every target (including HackRF)
# so their .ppma files contain addresses matching the firmware they ship with.
# Cross-copying .ppma files between different firmware builds does not work
# because external apps reference main firmware functions via absolute
# addresses baked in at link time.
list(APPEND EXTCPPSRC
       external/amfm_scanner/main.cpp
       external/amfm_scanner/ui_amfm_scanner.cpp
       external/sigfinder/main.cpp
       external/sigfinder/ui_sigfinder.cpp
       external/range_est/main.cpp
       external/range_est/ui_range_est.cpp
       external/freq_watch/main.cpp
       external/freq_watch/ui_freq_watch.cpp
       external/mod_ident/main.cpp
       external/mod_ident/ui_mod_ident.cpp
       external/tpms_counter/main.cpp
       external/tpms_counter/ui_tpms_counter.cpp
)
list(APPEND EXTAPPLIST amfm_scanner sigfinder range_est freq_watch mod_ident tpms_counter)

# rf_assistant has ~26KB of string-literal help text which would otherwise
# overflow either the HackRF 1MB flash (strings in main .rodata) or the 32KB
# per-app section limit (strings pulled into app section via linker glob).
# Keep it gated for PortaRF/HackRF Pro where main firmware has enough headroom.
# signal_map, noaa_sat, sat_pass, waterfall_rec, hw_test also stay gated.
if(FLASH_MB_LIMIT_SIZE GREATER 1)
       list(APPEND EXTCPPSRC
               external/rf_assistant/main.cpp
               external/rf_assistant/ui_rf_assistant.cpp
               external/signal_map/main.cpp
               external/signal_map/ui_signal_map.cpp
               external/noaa_sat/main.cpp
               external/noaa_sat/ui_noaa_sat.cpp
               external/sat_pass/main.cpp
               external/sat_pass/ui_sat_pass.cpp
               external/waterfall_rec/main.cpp
               external/waterfall_rec/ui_waterfall_rec.cpp
               external/hw_test/main.cpp
               external/hw_test/ui_hw_test.cpp
       )
       list(APPEND EXTAPPLIST rf_assistant signal_map noaa_sat sat_pass waterfall_rec hw_test)
endif()

