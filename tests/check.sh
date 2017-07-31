#!/bin/bash
WORDS=$1
DIST=$2
APP=$3
REF=$4
DICT=$5
REFDICT=$6
shuf $WORDS | head -100 | cut -f1 | sed "s/.*/approx $DIST &/" > /tmp/queries.txt
cat /tmp/queries.txt | $APP $DICT > us.out 2>/dev/null
cat /tmp/queries.txt | $REF $REFDICT > ref.out 2>/dev/null
diff us.out ref.out
rm us.out ref.out
