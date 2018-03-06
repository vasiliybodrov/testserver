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

#
# The dumb, stupid, simple script for test.
#

export IP='127.0.0.1'
export PORT='4880'

export SEND_FILE='y.bin'

export FILE_0="x0.bin"
export FILE_1="x1.bin"
export FILE_2="x2.bin"
export FILE_3="x3.bin"
export FILE_4="x4.bin"
export FILE_5="x5.bin"
export FILE_6="x6.bin"
export FILE_7="x7.bin"
export FILE_8="x8.bin"
export FILE_9="x9.bin"
export FILE_10="x10.bin"
export FILE_11="x11.bin"
export FILE_12="x12.bin"
export FILE_13="x13.bin"
export FILE_14="x14.bin"
export FILE_15="x15.bin"

export FIFO_0="p0.pipe"
export FIFO_1="p1.pipe"
export FIFO_2="p2.pipe"
export FIFO_3="p3.pipe"
export FIFO_4="p4.pipe"
export FIFO_5="p5.pipe"
export FIFO_6="p6.pipe"
export FIFO_7="p7.pipe"
export FIFO_8="p8.pipe"
export FIFO_9="p9.pipe"
export FIFO_10="p10.pipe"
export FIFO_11="p11.pipe"
export FIFO_12="p12.pipe"
export FIFO_13="p13.pipe"
export FIFO_14="p14.pipe"
export FIFO_15="p15.pipe"

rm -f ${FILE_0}
rm -f ${FILE_1}
rm -f ${FILE_2}
rm -f ${FILE_3}
rm -f ${FILE_4}
rm -f ${FILE_5}
rm -f ${FILE_6}
rm -f ${FILE_7}
rm -f ${FILE_8}
rm -f ${FILE_9}
rm -f ${FILE_10}
rm -f ${FILE_11}
rm -f ${FILE_12}
rm -f ${FILE_13}
rm -f ${FILE_14}
rm -f ${FILE_15}

rm -f ${FIFO_0}
rm -f ${FIFO_1}
rm -f ${FIFO_2}
rm -f ${FIFO_3}
rm -f ${FIFO_4}
rm -f ${FIFO_5}
rm -f ${FIFO_6}
rm -f ${FIFO_7}
rm -f ${FIFO_8}
rm -f ${FIFO_9}
rm -f ${FIFO_10}
rm -f ${FIFO_11}
rm -f ${FIFO_12}
rm -f ${FIFO_13}
rm -f ${FIFO_14}
rm -f ${FIFO_15}

mkfifo ${FIFO_0}
mkfifo ${FIFO_1}
mkfifo ${FIFO_2}
mkfifo ${FIFO_3}
mkfifo ${FIFO_4}
mkfifo ${FIFO_5}
mkfifo ${FIFO_6}
mkfifo ${FIFO_7}
mkfifo ${FIFO_8}
mkfifo ${FIFO_9}
mkfifo ${FIFO_10}
mkfifo ${FIFO_11}
mkfifo ${FIFO_12}
mkfifo ${FIFO_13}
mkfifo ${FIFO_14}
mkfifo ${FIFO_15}

sync

(cat ${SEND_FILE} | nc ${IP} ${PORT} > ${FILE_0} && echo "1" > ${FIFO_0} || echo "0" > ${FIFO_0})&
(cat ${SEND_FILE} | nc ${IP} ${PORT} > ${FILE_1} && echo "1" > ${FIFO_1} || echo "0" > ${FIFO_1})&
(cat ${SEND_FILE} | nc ${IP} ${PORT} > ${FILE_2} && echo "1" > ${FIFO_2} || echo "0" > ${FIFO_2})&
(cat ${SEND_FILE} | nc ${IP} ${PORT} > ${FILE_3} && echo "1" > ${FIFO_3} || echo "0" > ${FIFO_3})&
(cat ${SEND_FILE} | nc ${IP} ${PORT} > ${FILE_4} && echo "1" > ${FIFO_4} || echo "0" > ${FIFO_4})&
(cat ${SEND_FILE} | nc ${IP} ${PORT} > ${FILE_5} && echo "1" > ${FIFO_5} || echo "0" > ${FIFO_5})&
(cat ${SEND_FILE} | nc ${IP} ${PORT} > ${FILE_6} && echo "1" > ${FIFO_6} || echo "0" > ${FIFO_6})&
(cat ${SEND_FILE} | nc ${IP} ${PORT} > ${FILE_7} && echo "1" > ${FIFO_7} || echo "0" > ${FIFO_7})&
(cat ${SEND_FILE} | nc ${IP} ${PORT} > ${FILE_8} && echo "1" > ${FIFO_8} || echo "0" > ${FIFO_8})&
(cat ${SEND_FILE} | nc ${IP} ${PORT} > ${FILE_9} && echo "1" > ${FIFO_9} || echo "0" > ${FIFO_9})&
(cat ${SEND_FILE} | nc ${IP} ${PORT} > ${FILE_10} && echo "1" > ${FIFO_10} || echo "0" > ${FIFO_10})&
(cat ${SEND_FILE} | nc ${IP} ${PORT} > ${FILE_11} && echo "1" > ${FIFO_11} || echo "0" > ${FIFO_11})&
(cat ${SEND_FILE} | nc ${IP} ${PORT} > ${FILE_12} && echo "1" > ${FIFO_12} || echo "0" > ${FIFO_12})&
(cat ${SEND_FILE} | nc ${IP} ${PORT} > ${FILE_13} && echo "1" > ${FIFO_13} || echo "0" > ${FIFO_13})&
(cat ${SEND_FILE} | nc ${IP} ${PORT} > ${FILE_14} && echo "1" > ${FIFO_14} || echo "0" > ${FIFO_14})&
(cat ${SEND_FILE} | nc ${IP} ${PORT} > ${FILE_15} && echo "1" > ${FIFO_15} || echo "0" > ${FIFO_15})&
 
read X0 < ${FIFO_0}; echo "RESULT #0: ${X0}"
read X1 < ${FIFO_1}; echo "RESULT #1: ${X1}"
read X2 < ${FIFO_2}; echo "RESULT #2: ${X2}"
read X3 < ${FIFO_3}; echo "RESULT #3: ${X3}"
read X4 < ${FIFO_4}; echo "RESULT #4: ${X4}"
read X5 < ${FIFO_5}; echo "RESULT #5: ${X5}"
read X6 < ${FIFO_6}; echo "RESULT #6: ${X6}"
read X7 < ${FIFO_7}; echo "RESULT #7: ${X7}"
read X8 < ${FIFO_8}; echo "RESULT #8: ${X8}"
read X9 < ${FIFO_9}; echo "RESULT #9: ${X9}"
read X10 < ${FIFO_10}; echo "RESULT #10: ${X10}"
read X11 < ${FIFO_11}; echo "RESULT #11: ${X11}"
read X12 < ${FIFO_12}; echo "RESULT #12: ${X12}"
read X13 < ${FIFO_13}; echo "RESULT #13: ${X13}"
read X14 < ${FIFO_14}; echo "RESULT #14: ${X14}"
read X15 < ${FIFO_15}; echo "RESULT #15: ${X15}"

sync

echo -n "${FILE_0}:"; ls -la ${FILE_0} | awk '{print $5}' | tr '\n' ':'; md5sum -b ${FILE_0}
echo -n "${FILE_1}:"; ls -la ${FILE_1} | awk '{print $5}' | tr '\n' ':'; md5sum -b ${FILE_1}
echo -n "${FILE_2}:"; ls -la ${FILE_2} | awk '{print $5}' | tr '\n' ':'; md5sum -b ${FILE_2}
echo -n "${FILE_3}:"; ls -la ${FILE_3} | awk '{print $5}' | tr '\n' ':'; md5sum -b ${FILE_3}
echo -n "${FILE_4}:"; ls -la ${FILE_4} | awk '{print $5}' | tr '\n' ':'; md5sum -b ${FILE_4}
echo -n "${FILE_5}:"; ls -la ${FILE_5} | awk '{print $5}' | tr '\n' ':'; md5sum -b ${FILE_5}
echo -n "${FILE_6}:"; ls -la ${FILE_6} | awk '{print $5}' | tr '\n' ':'; md5sum -b ${FILE_6}
echo -n "${FILE_7}:"; ls -la ${FILE_7} | awk '{print $5}' | tr '\n' ':'; md5sum -b ${FILE_7}
echo -n "${FILE_8}:"; ls -la ${FILE_8} | awk '{print $5}' | tr '\n' ':'; md5sum -b ${FILE_8}
echo -n "${FILE_9}:"; ls -la ${FILE_9} | awk '{print $5}' | tr '\n' ':'; md5sum -b ${FILE_9}
echo -n "${FILE_10}:"; ls -la ${FILE_10} | awk '{print $5}' | tr '\n' ':'; md5sum -b ${FILE_10}
echo -n "${FILE_11}:"; ls -la ${FILE_11} | awk '{print $5}' | tr '\n' ':'; md5sum -b ${FILE_11}
echo -n "${FILE_12}:"; ls -la ${FILE_12} | awk '{print $5}' | tr '\n' ':'; md5sum -b ${FILE_12}
echo -n "${FILE_13}:"; ls -la ${FILE_13} | awk '{print $5}' | tr '\n' ':'; md5sum -b ${FILE_13}
echo -n "${FILE_14}:"; ls -la ${FILE_14} | awk '{print $5}' | tr '\n' ':'; md5sum -b ${FILE_14}
echo -n "${FILE_15}:"; ls -la ${FILE_15} | awk '{print $5}' | tr '\n' ':'; md5sum -b ${FILE_15}

echo ""
echo -n "${SEND_FILE}:"; ls -la ${SEND_FILE} | awk '{print $5}' | tr '\n' ':'; md5sum -b ${SEND_FILE}

rm -f ${FIFO_0}
rm -f ${FIFO_1}
rm -f ${FIFO_2}
rm -f ${FIFO_3}
rm -f ${FIFO_4}
rm -f ${FIFO_5}
rm -f ${FIFO_6}
rm -f ${FIFO_7}
rm -f ${FIFO_8}
rm -f ${FIFO_9}
rm -f ${FIFO_10}
rm -f ${FIFO_11}
rm -f ${FIFO_12}
rm -f ${FIFO_13}
rm -f ${FIFO_14}
rm -f ${FIFO_15}

sync

# ##############################################################################
# End of file
# ##############################################################################
