import java.lang.Comparable;


public class Result implements Comparable<Result> {

    public final Integer id;
    public final Double  rank;

    public Result(Integer i, Double r) {
        id   = i;
        rank = r;
    }

    public int compareTo(Result o) {
        return o.rank.compareTo(rank);
    }

}
