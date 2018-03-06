#! /bin/sh

# ##############################################################################
# The MIT License (MIT)
#
# Copyright (c) 2018 Vasiliy V. Bodrov aka Bodro, Ryazan, Russia
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
# OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
# THE USE OR OTHER DEALINGS IN THE SOFTWARE.
# ##############################################################################

# ##############################################################################
# You can define these macroses:
#   * DEBUG - use this macros for debug output
#   * TCP_PORT (-DTCP_PORT) - tcp-port
#   * BUFFER_SIZE (-DBUFFER_SIZE) - size of buffer
#   * POLL_TIMEOUT (-DPOLL_TIMEOUT) - timeout for poll
#   * MAX_SOCKET_COUNT (-DMAX_SOCKET_COUNT) - max sockets
#   * TEST_FILENAME (-DTEST_FILENAME) - filename for send
#   * USE_RANDOM_LINGER_RST (-DUSE_RANDOM_LINGER_RST) - use SO_LINGER
#   * FORCE_LINGER_RST(-DFORCE_LINGER_RST) - force for 'use SO_LINGER'
#        0 - SO_LINGER is ON
#        1 - SO_LINGER is OFF
#        2 - SO_LINGER by default (don't set SO_LINGER)
#   * USE_RANDOM_KEEP_ALIVE (-DUSE_RANDOM_KEEP_ALIVE) - use SO_KEEPALIVE
#   * FORCE_KEEP_ALIVE (-DFORCE_KEEP_ALIVE) - force for 'use SO_KEEPALIVE'
#        0 - SO_KEEPALIVE is ON
#        1 - SO_KEEPALIVE is OFF
#        2 - SO_KEEPALIVE by default
#   * USE_RANDOM_TCP_NO_DELAY (-DUSE_RANDOM_TCP_NO_DELAY) - use TCP_NODELAY
#   * FORCE_TCP_NO_DELAY (-DFORCE_TCP_NO_DELAY) - force for 'use TCP_NODELAY'
#        0 - TCP_NODELAY is ON
#        1 - TCP_NODELAY is OFF
#        2 - TCP_NODELAY by default
# ##############################################################################

export RESULT_BIN="testserver"
export TEST_FILE="\"test.bin\""

export COMPILE_LOG_FILE="${RESULT_BIN}.log"

export RANDOM_ON=1
export RANDOM_OFF=0

export FORCE_ON=0
export FORCE_OFF=1
export FORCE_DEFAULT=2

export CPP=g++
export STRIP=strip
export TEE=tee
export BC=bc

export RESULT_VALUE_OK=0
export RESULT_VALUE_COMPILE_ERROR=1
export RESULT_VALUE_SCRIPT_ARGUMENTS_ERROR=2
export RESULT_VALUE_OTHER_ERROR=3

export RESULT_VALUE=${RESULT_VALUE_OTHER_ERROR}

if [ $# -ge 1 ]; then
    if [ "x${1}" = "x--get-result-filename" ]; then
        echo "${RESULT_BIN}"

        RESULT_VALUE=${RESULT_VALUE_OK}
    else
        RESULT_VALUE=${RESULT_VALUE_SCRIPT_ARGUMENTS_ERROR}
    fi

    exit ${RESULT_VALUE}
fi

if [ -f ${COMPILE_LOG_FILE} ]; then
    rm -f ${COMPILE_LOG_FILE}
fi

if [ -f "${RESULT_BIN}" ]; then
    rm -f ${RESULT_BIN}
fi

echo "Compile with g++ version: `${CPP} --version | head -n1`"
echo ""

${CPP} -x c++ -std=gnu++17 -Wall -Wextra -O3 -m64 \
    -mtune=native -march=native -mfpmath=sse \
    -DTCP_PORT=7777 \
    -DBUFFER_SIZE=3000 \
    -DPOLL_TIMEOUT=1000 \
    -DMAX_SOCKET_COUNT=100 \
    -DUSE_RANDOM_LINGER_RST=${RANDOM_ON} \
    -DFORCE_LINGER_RST=${FORCE_OFF} \
    -DUSE_RANDOM_KEEP_ALIVE=${RANDOM_ON} \
    -DUSE_RANDOM_TCP_NO_DELAY=${RANDOM_ON} \
    -DTEST_FILENAME="${TEST_FILE}" \
    -lc -lm testserver.cpp -o ${RESULT_BIN} 2>&1 | ${TEE} ${COMPILE_LOG_FILE}

if [ -f "${RESULT_BIN}" ]; then
    echo "COMPILE OK!"
    echo ""

    BEFORE_SIZE=`ls -la ${RESULT_BIN} | awk '{print $5}'`
    echo "BEFORE STRIP SIZE: ${BEFORE_SIZE} bytes"

    ${STRIP} --strip-all ${RESULT_BIN}

    AFTER_SIZE=`ls -la ${RESULT_BIN} | awk '{print $5}'`

    echo "AFTER STRIP SIZE:  ${AFTER_SIZE} bytes"
    echo "SYMBOLS: `echo "${BEFORE_SIZE} - ${AFTER_SIZE}" | ${BC} -l` bytes"
    echo "K=`echo "scale=2; ${BEFORE_SIZE} / ${AFTER_SIZE}" | ${BC} -l`"
    echo "R=`echo "scale=2; (${AFTER_SIZE} / ${BEFORE_SIZE}) * 100" | \
         ${BC} -l`%"

    echo ""

    echo "OK!"

    unset AFTER_SIZE
    unset BEFORE_SIZE

    RESULT_VALUE=${RESULT_VALUE_OK}
else
    echo "ERROR!"

    RESULT_VALUE=${RESULT_VALUE_COMPILE_ERROR}
fi

unset RESULT_BIN
unset TEST_FILE
unset COMPILE_LOG_FILE
unset RANDOM_ON
unset RANDOM_OFF
unset FORCE_ON
unset FORCE_OFF
unset FORCE_DEFAULT
unset CPP
unset STRIP
unset TEE
unset BC

exit ${RESULT_VALUE}

#unset RESULT_VALUE_OK
#unset RESULT_VALUE_COMPILE_ERROR
#unset RESULT_VALUE_SCRIPT_ARGUMENTS_ERROR
#unset RESULT_VALUE_OTHER_ERROR
#unset RESULT_VALUE

# ##############################################################################
# End of file
# ##############################################################################
