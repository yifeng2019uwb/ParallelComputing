import scala.Tuple2;
import java.io.Serializable;

import org.apache.spark.SparkConf;
import org.apache.spark.api.java.JavaRDD;
import org.apache.spark.api.java.JavaPairRDD;
import org.apache.spark.api.java.JavaSparkContext;
import org.apache.spark.api.java.function.Function2;
import org.apache.spark.api.java.function.VoidFunction;
//import org.apache.spark.util.Vector;
import java.util.regex.Pattern;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Random;

// import Data.java


public class Kmeans {

    public static final int IT_MAX = 100;
    public static final double MIN_DIFF = 0.00000001;

    public static void main(String[] args) {

        // start Sparks and read a given input file and set source and destination vertex
        String inputFile = args[0];
        int k = Integer.parseInt(args[1]);

        // configure spark
        SparkConf conf = new SparkConf( ).setAppName( "K-means" );

        // start a spark context
        JavaSparkContext jsc = new JavaSparkContext( conf );

        // now start a timer
        long startTime = System.currentTimeMillis();

        // read text file to RDD
        JavaRDD<String> lines = jsc.textFile(inputFile);

        //JavaRDD<Point> log_values = numbers.map(x -> Math.log(x)); 
        JavaRDD<Point> data = lines.map(s -> {
            String[] strs = s.split(",");
            return new Point(Double.parseDouble(strs[0]), Double.parseDouble(strs[1]));
            
        }).cache();

        // init centroids
        // final List<Point> centroids = data.takeSample(false, k);
        final List<Point> centroids = new ArrayList<>();
        for (Point p : data.takeSample(false, k)) {
            centroids.add(p);
        }
/*
        System.err.println("Print the init centroids");
        for (int i = 0; i < k; i++) {
            System.err.println("Cluster " + i + " : " + centroids.get(i).x + " " + centroids.get(i).y);
        }
*/
        for (int iteration = 0; iteration < IT_MAX; iteration++) {
            // System.err.println("The iteration = " + iteration);

            // Create JavaPairRDD<Integer,Point> closest by mapping each point to closest centroids
            JavaPairRDD<Integer, Point> closest  = data.mapToPair( p -> {
                int c = closestCentroids(p, centroids);
                return new Tuple2<>(c, p);
            });
            // group by cluster/centroids id 
            JavaPairRDD<Integer, Iterable<Point>> clustersTemp = closest.groupByKey();

            JavaPairRDD<Integer, List<Point>> clusters = clustersTemp.mapValues(cluster -> {
                List<Point> groupPoints = new ArrayList<>();
                for (Point p : cluster) {
                    groupPoints.add(p);
                }
                return groupPoints;
            });

            // and average the points within each cluster to compute centroids
            Point[] newCentroids = new Point[k];
            List<Tuple2<Integer, List<Point>>> group = clusters.collect();
            for (Tuple2<Integer, List<Point>> tup : group ) {
                 System.err.println("Cluster " + tup._1 + "'s size = " + tup._2.size());
                 System.err.println("Cluster centroid = " + centroids.get(tup._1).x + " " + centroids.get(tup._1).y); 
                
                List<Point> d = tup._2;
                Point tmp = new Point(0.0, 0.0);
                for (int i = 0; i < d.size(); i++) {
                    System.err.println(d.get(i).x + ", " + d.get(i).y);
                    tmp.add(d.get(i));
                }
                // System.err.println();
                tmp.scaleDown(d.size());
                newCentroids[tup._1] = tmp;
                // System.err.println("group" + tup._1 + "'s new best center: " + tmp.x + " " + tmp.y);
                // System.err.println();
            }

            // System.err.println("Print the previous centroids and new centroids");

            double diff = 0.0;
            for (int i = 0; i < k; i++) {
                // System.err.println("Cluster " + i);
                // System.err.println("Current centroids : " + centroids.get(i).x + " " + centroids.get(i).y);
                // System.err.println("NewCentroids :      " + newCentroids[i].x + " " + newCentroids[i].y);
                // System.err.println();
                diff += newCentroids[i].getEuclideanDistance(centroids.get(i));

            }

            // System.err.println("different of centroids = " + dist);
            if (diff < MIN_DIFF) break;

            for (int i = 0; i < k; i++) {
                centroids.set(i, newCentroids[i]);
            }
            

        } // end while loop

        // print elapsed time
        System.err.println("Elapsed time = " + (System.currentTimeMillis() - startTime) + " ms.");

    }

    /** Parses numbers split by whitespace to a point */
    static Point parsePoint(String line) {
        String[] pt = line.split(" ");
        return new Point(Double.parseDouble(pt[0]), Double.parseDouble(pt[1]));
    }

    /** Computes the point to which the input point is closest using squared distance */
    static int closestCentroids(Point p, List<Point> centrids) {
        int bestIndex = 0;
        double closest = Double.POSITIVE_INFINITY;
        for (int i = 0; i < centrids.size(); i++) {
           // double dist = p.x * centrids.get(i).x + p.y * centrids.get(i).y;
           double dist = p.getEuclideanDistance(centrids.get(i));
            if (dist < closest) {
                closest = dist;
                bestIndex = i;
            }
        }
        return bestIndex;
  }

    /** Computes the mean across clusters to get new centroids */
    static Point getNewCentroids(List<Point> group) {
        int numPoints = group.size();
        Point pt = new Point(0.0, 0.0);
       
        // sum of all x directions and y direction
        for (int i = 0; i < numPoints; i++) {
            pt.add(group.get(i));
        }
        
        return pt.scaleDown(group.size());

    }

    static class Point implements Serializable {
        double x;
        double y;

        public Point() {}

        public Point(double x, double y) {
            this.x = x;
            this.y = y;
        }

        public double getEuclideanDistance(Point pt) {
            double result = (this.x - pt.x) * (this.x - pt.x) + 
                            (this.y - pt.y) * (this.y - pt.y);

            return Math.sqrt(result);    
        }

        public Point add(Point pt){
            this.x += pt.x;
            this.y += pt.y;
            return this;
        }

        public Point scaleDown(int times) {
            this.x /= times;
            this.y /= times;
            return this;

        }
    }   

}




