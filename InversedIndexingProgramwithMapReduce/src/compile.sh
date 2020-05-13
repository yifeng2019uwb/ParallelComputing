#!/bin/sh

rm output/part-00000
~/hadoop-0.20.2/bin/hadoop dfs -rmr /user/yifengzh_css534/output
javac -classpath ${HADOOP_HOME}/hadoop-${HADOOP_VERSION}-core.jar InvertedIndexes.java
jar -cvf InvertedIndexes.jar *.class
~/hadoop-0.20.2/bin/hadoop jar InvertedIndexes.jar InvertedIndexes rfc output TCP UDP LAN PPP HDLC
~/hadoop-0.20.2/bin/hadoop fs -get /user/yifengzh_css534/output/part-00000 output
cat output/part-00000


