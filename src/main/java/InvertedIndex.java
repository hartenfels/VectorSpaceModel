import java.io.*;

import static java.lang.String.format;


public class InvertedIndex implements Serializable {

    public void addToken(int id, String token) {
        throw new RuntimeException("Not implemented.");
    }


    public void stash(String path) {
        try {
            FileOutputStream file = new FileOutputStream(path);
            ObjectOutputStream objs = new ObjectOutputStream(file);
            objs.writeObject(this);
            objs.close();
            file.close();
        } catch (IOException e) {
            System.err.println(format("Error stashing `%s`: %s", path, e.getMessage()));
        }
    }


    public static InvertedIndex unstash(String path) {
        InvertedIndex index = null;

        try {
            FileInputStream file = new FileInputStream(path);
            ObjectInputStream objs = new ObjectInputStream(file);
            index = (InvertedIndex) objs.readObject();
            objs.close();
            file.close();
        } catch (IOException | ClassNotFoundException e) {
            System.err.println(format("Error unstashing `%s`: %s", path, e.getMessage()));
        }

        return index;
    }

}
