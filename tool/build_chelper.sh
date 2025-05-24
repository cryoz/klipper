#!/bin/bash

cd ../klippy/chelper && docker run -ti -v $PWD:$PWD pellcorp/k1-klipper-fw-build /bin/bash -c "cd $PWD && make clean && make && rm *.o"
