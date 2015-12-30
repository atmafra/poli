#!/bin/bash
nice -15 \
./som_vq \
-il data/speech_db/p100-200.mfccs \
-md data/speech_db/trmaps/ \
--train \
--max-epochs 1000 \
--initial-epoch 500 \
--save-epochs 50 \
-on data/networks/som50.test.net \
-in data/networks/som50.t500.net
