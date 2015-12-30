#!/bin/bash
./som_vq \
-il data/speech_db/p100-200.mfccs \
-md data/speech_db/trmaps/ \
-sd data/speech_db/slists/ \
-in data/networks/som50.trained.net
