import java.io.IOException;
import java.util.*;
import java.util.HashMap;
import java.util.Map.Entry;

import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapred.*;

public class InvertedIndexes {

    // mapping class
    public static class Map extends MapReduceBase implements Mapper<LongWritable, Text, Text, Text> {
        
        JobConf conf;
        public void configure(JobConf job) {
            this.conf = job;
        }


        public void map(LongWritable docId, Text value, OutputCollector<Text, Text> output, Reporter reporter) throws IOException {
            // retrieve # keywords from JobConf
            int argc = Integer.parseInt(conf.get("argc"));

            // put all key words in the set
            Set<String> keyWords = new HashSet<>();
            for (int i = 0; i < argc; i++ ) {
                keyWords.add(conf.get("keyword" + i));
            }

            // get the current file name
            FileSplit filesSplit = (FileSplit)reporter.getInputSplit();
            String filename = "" + filesSplit.getPath().getName();

            
            //read each line of file splits; tokenize each word with a space;
            // check if it is one of the given keywords; and increment this keyword’s count.
            String line = value.toString();
            StringTokenizer tokenizer = new StringTokenizer(line);
            while (tokenizer.hasMoreTokens()) {
                String word = tokenizer.nextToken();
                if (keyWords.contains(word)) {
                // output.collect(word,, one);
                    output.collect(new Text(word), new Text(filename));
                }
            }

        }
    }

    // Reduce
    public static class Reduce extends MapReduceBase implements Reducer<Text, Text, Text, Text> {
        public void reduce(Text key, Iterator<Text> values, OutputCollector<Text, Text> output, Reporter reporter) throws IOException {

            HashMap<String, Integer> hmap = new HashMap<>();
            // count the numbers of key words in each file
            while (values.hasNext()) {
                String filename = values.next().toString();
                hmap.put(filename, hmap.getOrDefault(filename, 0) + 1);
                System.out.println(filename + " :  " + hmap.get(filename));
                // Or use if hmap.containsKey(filename)

             }

            // create Comparator to sort the result by count number
            Comparator<Entry<String, Integer>> valueComparator = new Comparator<Entry<String, Integer>>() {
                @Override
                public int compare(Entry<String, Integer> e1, Entry<String, Integer> e2) {
                    int v1 = e1.getValue();
                    int v2 = e2.getValue();
                    return v2 - v1; // v2 - v1 is descending while v1 - v2 is ascending order
                }
            };

            //sort
            List<Entry<String, Integer>> docList  = new ArrayList<Entry<String, Integer>>(hmap.entrySet());
            Collections.sort(docList, valueComparator);

            // (keyword1, filename, count), (keyword2, filename2, count), ...
            StringBuilder sb = new StringBuilder();
            for (Entry<String, Integer> e : docList) {
                sb.append(e.getKey() + " " + e.getValue() + "\t");
            }

            Text docListText = new Text(sb.toString().trim());
            // finally, print it out.
            output.collect(key, docListText);

        }
    }

    public static void main(String[] args) throws Exception {
        // timer start
        long startTime = System.currentTimeMillis();

        // InversedIndexing.class is this program's file name
        JobConf conf = new JobConf(InvertedIndexes.class);
        conf.setJobName("InvertedIndexes");

        // set my output files's format
        conf.setOutputKeyClass(Text.class);
        conf.setOutputValueClass(Text.class);

        // set my mapper and reduce classes
        conf.setMapperClass(Map.class);
        // This method will automatic add " 1 "  on each count value
        // conf.setCombinerClass(Reduce.class);
        conf.setReducerClass(Reduce.class);

        // set input/output format
        conf.setInputFormat(TextInputFormat.class);
        // don't need use setCombinerClass, becuase in Reduce class, already combine the count in the hashMap
        // if use Combiner Class, it will combine the Object - Entry<String, Integer>, all the objects occurs 1 time
        // conf.setCombinerClass(Reduce.class);
        conf.setOutputFormat(TextOutputFormat.class);

        // set input/output directories
        FileInputFormat.setInputPaths(conf, new Path(args[0]));
        FileOutputFormat.setOutputPath(conf, new Path(args[1]));

        //
        conf.set("argc", String.valueOf(args.length - 2)); // argc maintains #keywords
        for (int i = 0; i < args.length - 2; i++) {
                conf.set("keyword" + i, args[i + 2]);
        }

        JobClient.runJob(conf);

        System.out.println("Elapsed time = " + (System.currentTimeMillis() - startTime) + " ms.");

    }
}



