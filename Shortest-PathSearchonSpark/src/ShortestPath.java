import org.apache.spark.SparkConf;
import org.apache.spark.api.java.JavaRDD;
import org.apache.spark.api.java.JavaPairRDD;
import org.apache.spark.api.java.JavaSparkContext;
import org.apache.spark.api.java.function.Function2;
import org.apache.spark.api.java.function.VoidFunction;

import scala.Tuple2;
import java.io.Serializable;

import java.util.ArrayList;
import java.util.List;

// import Data.java


public class ShortestPath {

    public static final String ACTIVE = "ACTIVE";
    public static final String INACTIVE = "INACTIVE";

    public static void main(String[] args) {

        // start Sparks and read a given input file and set source and destination vertex
        String inputFile = args[0];
        String source = args[1];
        String destination = args[2];

        // configure spark
        SparkConf conf = new SparkConf( ).setAppName( "BFS-based Shortest Path Search" );

        // start a spark context
        JavaSparkContext jsc = new JavaSparkContext( conf );

        // read text file to RDD
        JavaRDD<String> lines = jsc.textFile(inputFile);

        // now start a timer
        long startTime = System.currentTimeMillis();

        // Create JavaPairRDD<String, Data>
         JavaPairRDD<String, Data> network  = lines.mapToPair(line -> {
                // create RDD with list of Tuple2<String-vertexId, Data - vertex data/value>
                // get vertexId (considering the format of graph.txt: 
                // vertexId1=neighbor1,linkWeight1;nieghbor2,linkWeight2;neighbor3,linkWeight3;... 
                // vertexId2=...)
                String vertexId = line.substring(0, line.indexOf("="));
                String[] nei_arr = line.substring(line.indexOf("=") + 1).split(";");

                // neighbors: neighbor0, distance0(v,n0); neighbor1, distance(v,n1).....
                List<Tuple2<String, Integer>> neighbors = new ArrayList<>();
                for (int i = 0; i < nei_arr.length; i++) {
                    neighbors.add(new Tuple2<>(nei_arr[i].substring(0, nei_arr[i].indexOf(",")), 
                                            Integer.parseInt(nei_arr[i].substring(nei_arr[i].indexOf(",") + 1))));
                }
                // System.out.println(vertexId + " =: " + neighbors);
                
                // create the Vertex Data
                Data data;
                // set source as ACTIVE and distance = 0, prev = 0
                if (vertexId.equals(source))    data = new Data(neighbors, 0, 0, ACTIVE);
                else data = new Data(neighbors, Integer.MAX_VALUE, Integer.MAX_VALUE, INACTIVE);

                return new Tuple2<>(vertexId, data);
        });

        System.out.println("The total vertex number = " + network.count());
 
        // while (t here are any “ACTIVE” vertices ), 
        // count how many ACTIVE vertices, if count > 0 continue calulate to find the shortest distance
        while (network.filter( v -> {
            if (v._2.status.equals(ACTIVE)) return true;
            else return false;
        }). count() > 0 ) {

            JavaPairRDD<String, Data> propagatedNetwork = network.flatMapToPair( vertex-> {
            // If a vertex is “ACTIVE”, create Tuple2( neighbor, new Data( ... ) ) for 
            // each neighbor where Data should include a new distance to this neighbor. 
            // Add each Tuple2 to a list. Don’t forget this vertex itself back to the 
            // list. Return all the list items.
                List<Tuple2<String, Data>> list = new ArrayList<>();
                // create each neighbor into list with default status - INACTIVE
                // add the vertex itself first
				list.add(new Tuple2<>(vertex._1,
						new Data(vertex._2.neighbors, vertex._2.distance, vertex._2.prev,
								INACTIVE)));

				if (vertex._2.status.equals(ACTIVE)) {
					for (Tuple2<String, Integer> neighbor : vertex._2.neighbors) {
						list.add(new Tuple2<>(neighbor._1,
								new Data(new ArrayList<>(), (neighbor._2 + vertex._2.distance),
										Integer.MAX_VALUE, INACTIVE)));
					}
				}
				return list.iterator();

            } );

            network = propagatedNetwork.reduceByKey( ( k1, k2 ) ->{
            // For each key, (i.e., each vertex), find the shortest distance and
            // update this vertex’ Data attribute.
                List<Tuple2<String, Integer>> neighbors = 
                                    k1.neighbors.size() == 0 ? k2.neighbors : k1.neighbors;
                //
                int dist = Math.min(k1.distance, k2.distance);
                int prev = Math.min(k1.prev, k2.prev);
                return new Data(neighbors, dist, prev, INACTIVE);

            } );
            
            network = network.mapValues( value -> {
            // If a vertex’ new distance is shorter than prev, activate this vertex
            // status and replace prev with the new distance.
                if (value.distance < value.prev) {
                    return new Data(value.neighbors, value.distance, value.distance, ACTIVE);
                }
                return value;

            } ); 

             // try to print all data to check the distance 
            /*
            List<Tuple2<String, Data>> printNetwork = network.collect();
            for (Tuple2<String, Data> v : printNetwork) {
                System.out.println("vertex = " + v._1 + " : " + v._2.status + "  , distance = " + v._2.distance);
                Data d = v._2;
                List<Tuple2<String,Integer>> nei = d.neighbors;
                for (Tuple2<String,Integer> ne : nei) {
                    System.out.print(ne._1 + " : " + ne._2 + "; ");
                }
                System.out.println();
            }
            */


        } // end while loop

        List<Data> ans = network.lookup(destination);
	
    	// print #words
        System.out.println( "from " + source + " to " + destination + " takes distance = " + ans.get(0).distance );

        // print elapsed time
        System.out.println("Elapsed time = " + (System.currentTimeMillis() - startTime) + " ms.");

    }
/*
// This Data class copied from Prof. Fukuda's Data.java
// Using compile.sh to compile the java file with the data class inside of the ShortestPath.java
// Using sp_compile.sh to compile the ShortestPath.java and Data.java
    static class Data implements Serializable {
        List<Tuple2<String,Integer>> neighbors; // <neighbor0, weight0>, <neighbor1, weight1>, ...
        String status;                          // "INACTIVE" or "ACTIVE"
        Integer distance;                       // the distance so far from source to this vertex
        Integer prev;                           // the distance calculated in the previous iteration

        public Data(){
            neighbors = new ArrayList<>();
            status = "INACTIVE";
            distance = 0;
        }

        public Data( List<Tuple2<String,Integer>> neighbors, Integer dist, Integer prev, String status ){
            if ( neighbors != null ) {
                this.neighbors = new ArrayList<>( neighbors );
            } else {
                this.neighbors = new ArrayList<>( );
            }
	        this.distance = dist;
            this.prev = prev;
            this.status = status;
        }
    }   
    */

}


