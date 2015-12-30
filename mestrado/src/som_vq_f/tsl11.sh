#!/bin/bash
echo EU
./som_vq_f \
--input-network som32l11.net \
--test-table ../data/speech_db/mfccs/nomel01.ctl

echo MAURO
./som_vq_f \
--input-network som32l11.net \
--test-table ../data/speech_db/mfccs/nomel04.ctl

echo EDU
./som_vq_f \
--input-network som32l11.net \
--test-table ../data/speech_db/mfccs/nomel05.ctl

echo MAURECIO
./som_vq_f \
--input-network som32l11.net \
--test-table ../data/speech_db/mfccs/nomel09.ctl

echo GALINHA
./som_vq_f \
--input-network som32l11.net \
--test-table ../data/speech_db/mfccs/nomel10.ctl

echo BETO
./som_vq_f \
--input-network som32l11.net \
--test-table ../data/speech_db/mfccs/nomel11.ctl
