import java.lang.Comparable;


public class QueryResult implements Comparable<QueryResult> {

    public final Integer id;
    public final Double  rank;

    public QueryResult(Integer i, Double r) {
        id   = i;
        rank = r;
    }

    public int compareTo(QueryResult o) {
        return o.rank.compareTo(rank);
    }

    @Override
    public String toString() {
        return "QueryResult{" +
                "id=" + id +
                ", rank=" + rank +
                '}';
    }
}
